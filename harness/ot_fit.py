"""Output transformer parameters from the Hammond 1750U datasheet (drop-in replacement for the 2203's
C2668; DECISIONS D-023, CIRCUIT.md §3). The datasheet gives DCRs, the 1 V / 1 kHz inductances, the
leakage inductance with the secondary shorted, and response / phase curves measured at 27 dBu in its
test circuit (RS/2 = 850 Ω at each primary end, 16 Ω load). This module
- fits the primary inductance `lpt` to the curve readings below, with the coupling `kot` (all three
  pairs) set so that the shorted-secondary inductance equals the measured 7.97 mH, and an optional
  plate-to-plate capacitance (the fit sets it to zero), and
- checks the values in the reference netlist against the datasheet (`--check`).

    python -m harness.ot_fit           # fit and print
    python -m harness.ot_fit --check   # netlist transformer vs datasheet (exit 1 on mismatch)
"""
import argparse
import re
import sys

import numpy as np
from scipy.optimize import brentq, least_squares

from . import config, netlist, spice

N2 = 1700.0 / 16.0                  # owner decision: nominal ratio (PROGRESS Q8)
RW1, RW2, RWS = 15.36, 16.56, 0.320  # DCR Blue-White (pA side, O7), White-Red, Violet-Green
L_SC = 7.97e-3                       # H, Blue-Red with the secondary shorted, 1 kHz, 1 V
F_REF = 1000.0
# 16 Ω curves of the datasheet's response and phase graphs, read by eye (±0.05 dB, ±1°)
MAG_DB = {50: -0.17, 70: -0.12, 100: -0.08, 200: -0.03, 5000: -0.07, 7000: -0.12, 10000: -0.2,
          20000: -0.5, 30000: -0.85}
PHASE_DEG = {50: 5.5, 70: 4.5, 100: 3.5, 200: 2.0, 500: 0.5, 2000: -1.5, 5000: -4.5, 10000: -9.0,
             20000: -16.0, 30000: -22.0}
OT_ELEMENTS = ('rwp1', 'lp1', 'lp2', 'rwp2', 'ls', 'rws', 'k1', 'k2', 'k3')


def ot_text(lpt, kot, cpp=0.0):
    """The transformer as in the netlist (same element names), plus a 1 GΩ DC path for the CT."""
    lpt, kot, cpp = float(lpt), float(kot), float(cpp)
    c = f'Cpp pA pB {cpp!r}\n' if cpp > 0 else ''
    return (f'.param lpt={lpt!r} npr={{sqrt({N2!r})}} kot={kot!r}\n'
            f'Rwp1 pA pA1 {RW1}\nLp1 pA1 ct {{lpt/4}}\nLp2 ct pB1 {{lpt/4}}\nRwp2 pB1 pB {RW2}\n'
            f'Ls out sl {{lpt/(npr*npr)}}\nRws sl 0 {RWS}\nK1 Lp1 Lp2 {{kot}}\nK2 Lp1 Ls {{kot}}\n'
            f'K3 Lp2 Ls {{kot}}\nRct ct 0 1G\n' + c)


def netlist_ot_text():
    """The transformer lines of the reference netlist (with its .param lines) plus the CT DC path."""
    text = config.NETLIST.read_text()
    by = netlist.parse(text).by_name()
    params = '\n'.join(ln for ln in text.splitlines() if re.match(r'\.param\s+lpt=', ln, re.I))
    return params + '\n' + '\n'.join(by[e].line for e in OT_ELEMENTS) + '\nRct ct 0 1G\n'


def primary_inductance(ot, shorted):
    """Blue-Red inductance at 1 kHz (Im Z / ω), secondary open or shorted."""
    tb = '* primary impedance\nRret pB 0 1u\n' + ot + ('Rshort out 0 1u\n' if shorted else '')
    f, z = spice.impedance(tb, {}, 'pA', fstart=F_REF, fstop=F_REF * 1.0001, ppd=1)
    return z['pA'][0].imag / (2 * np.pi * F_REF)


def response(ot):
    """Datasheet test circuit, normalised to 1 kHz. Returns (f, complex H)."""
    tb = '* datasheet test circuit\nVs src 0 DC 0 AC 1\nRs1 src pA 850\nRs2 pB 0 850\nRload out 0 16\n' + ot
    f, h = spice.ac(tb, {}, 'out', 'src', fstart=10, fstop=1e5, ppd=192)
    return f, h / h[np.argmin(np.abs(f - F_REF))]


def residuals(ot):
    """(magnitude residuals in dB, phase residuals in degrees) against the curve readings."""
    f, h = response(ot)
    mag = np.array([20 * np.log10(np.interp(x, f, np.abs(h))) - m for x, m in MAG_DB.items()])
    ph = np.array([np.degrees(np.interp(x, f, np.unwrap(np.angle(h)))) - p for x, p in PHASE_DEG.items()])
    return mag, ph


def k_for(lpt):
    return brentq(lambda k: primary_inductance(ot_text(lpt, k), True) - L_SC, 0.99, 0.9999999, xtol=1e-13)


def fit():
    def res(p):
        lpt, cpp = np.exp(p)
        mag, ph = residuals(ot_text(lpt, k_for(lpt), cpp))
        return np.r_[mag / 0.05, ph / 1.0]          # weights: reading accuracy
    sol = least_squares(res, [np.log(25.0), np.log(200e-12)], x_scale=[0.3, 1.0])
    lpt, cpp = np.exp(sol.x)
    return lpt, k_for(lpt), cpp


def check():
    """Netlist transformer vs datasheet: DCRs, ratio, shorted-secondary inductance, curve residuals."""
    ot = netlist_ot_text()
    problems = []
    for name, want in (('Rwp1', RW1), ('Rwp2', RW2), ('Rws', RWS)):
        m = re.search(rf'^{name}\s+\S+\s+\S+\s+(\S+)\s*$', ot, re.M | re.I)
        if not m or float(m.group(1)) != want:
            problems.append(f'{name} is not {want} ohm')
    l_sc = primary_inductance(ot, True)
    if abs(l_sc / L_SC - 1) > 1e-3:
        problems.append(f'shorted-secondary inductance {l_sc * 1e3:.4g} mH, datasheet {L_SC * 1e3:g} mH')
    mag, ph = residuals(ot)
    if np.max(np.abs(mag)) > 0.2 or np.max(np.abs(ph)) > 1.5:
        problems.append(f'curve residuals {np.max(np.abs(mag)):.3f} dB / {np.max(np.abs(ph)):.2f} deg')
    return problems, l_sc, primary_inductance(ot, False), mag, ph


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--check', action='store_true')
    a = ap.parse_args()
    if a.check:
        problems, l_sc, l_oc, mag, ph = check()
        print(f'netlist OT: L(Blue-Red) {l_oc:.4g} H open, {l_sc * 1e3:.4g} mH shorted; '
              f'curve residuals max {np.max(np.abs(mag)):.3f} dB, {np.max(np.abs(ph)):.2f} deg')
        for p in problems:
            print('MISMATCH: ' + p)
        sys.exit(1 if problems else 0)
    lpt, kot, cpp = fit()
    mag, ph = residuals(ot_text(lpt, kot, cpp))
    print(f'fit: lpt = {lpt:.4g} H, kot = {kot:.7f}, Cpp = {cpp * 1e12:.3g} pF')
    print('  magnitude residuals [dB]: ' + ' '.join(f'{f:g}:{r:+.3f}' for f, r in zip(MAG_DB, mag)))
    print('  phase residuals [deg]:    ' + ' '.join(f'{f:g}:{r:+.2f}' for f, r in zip(PHASE_DEG, ph)))
    print(f'  for reference, the 1 V datasheet primary inductance 8.85 H gives kot = {k_for(8.85):.7f}')


if __name__ == '__main__':
    main()
