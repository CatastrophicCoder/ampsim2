"""M0 step 5: junction Thevenin resistances (SPEC §7 M0.5, CIRCUIT.md §4). For each junction, a 1 A AC
current is injected at the open-circuited output node of the upstream block testbench, with the block
input AC-grounded, at the DC operating point. Reported: the DC small-signal value (1 mHz, capacitors
effectively open; cross-checked by a finite difference of two DC operating points), the complex value
at 1 kHz, the spread over all presets, and |Z(f)|. Writes harness/reports/M0/<YYYYMMDD>_junctions.md.

    python -m harness.rth_measure
"""
import argparse
import datetime
from dataclasses import dataclass

import matplotlib

matplotlib.use('Agg')
import matplotlib.pyplot as plt  # noqa: E402
import numpy as np  # noqa: E402

from . import config, spice, testbench_gen  # noqa: E402
from .junctions import RTH  # noqa: E402
from .presets import PRESETS  # noqa: E402

F_DC = 1e-3       # Hz; "DC" small-signal point of the AC sweep
F_AC = 1000.0     # Hz; SPEC M0.5 measurement frequency
I_FD = 1e-6       # A; finite-difference step for the DC cross-check


@dataclass(frozen=True)
class Junction:
    key: str           # harness/junctions.py RTH key
    name: str          # CIRCUIT.md §4 name
    testbench: str     # upstream block testbench
    node: str          # upstream output node (open)
    observe: tuple = ()  # other nodes whose transimpedance is reported (LTP coupling)


JUNCTIONS = (
    Junction('j2', 'J2', 'b12', 'p1b'),
    Junction('j3', 'J3', 'b3', 'k2b'),
    Junction('j5a', 'J5a (R_thA)', 'b5open', 'p3a', ('p3b',)),
    Junction('j5b', 'J5b (R_thB)', 'b5open', 'p3b', ('p3a',)),
)


def _at(f, z, f0):
    i = int(np.argmin(np.abs(np.log(f / f0))))
    if abs(f[i] / f0 - 1) > 1e-9:
        raise ValueError(f'{f0} Hz is not on the sweep grid')
    return complex(z[i])


def testbench_text(j, rth):
    """Upstream testbench of junction j with the given upstream junction resistances."""
    return testbench_gen.generate(rth)[f'{j.testbench}.cir']


def measure_one(j, rth, knobs=None):
    f, z = spice.impedance(testbench_text(j, rth), knobs or {}, j.node, j.observe)
    return {'f': f, 'z': z, 'dc': _at(f, z[j.node], F_DC), 'ac': _at(f, z[j.node], F_AC),
            'mutual_dc': {o: _at(f, z[o], F_DC) for o in j.observe},
            'mutual_ac': {o: _at(f, z[o], F_AC) for o in j.observe}}


def dc_finite_difference(j, rth):
    """R_th at DC from two operating points with ±I_FD injected (independent of the AC analysis)."""
    text = spice._strip_end(testbench_text(j, rth))
    v = [spice.op(text + f'\nIfd_zth 0 {j.node} DC {s * I_FD!r}\n', {}, [j.node])[j.node] for s in (1, -1)]
    return (v[0] - v[1]) / (2 * I_FD)


def upstream_rth(results, basis, force=False):
    """Junction resistances for the upstream testbenches: junctions.py values where set (unless `force`),
    otherwise the value measured here (basis 'dc' or 'ac' = |Z(1 kHz)|)."""
    rth = dict(RTH)
    for k, r in results.items():
        if force or rth[k] is None:
            rth[k] = float(r['dc'].real if basis == 'dc' else abs(r['ac']))
    return rth


def measure(basis='dc', force=False):
    """Measures every junction in chain order; upstream junctions use `upstream_rth(.., basis, force)`."""
    results = {}
    for j in JUNCTIONS:
        results[j.key] = measure_one(j, upstream_rth(results, basis, force))
    return results


def preset_spread(results):
    """(min, max) of R_th at DC and |Z| at 1 kHz over the unique knob sets of all presets."""
    rth = upstream_rth(results, 'dc')
    knob_sets = {tuple(sorted(p.all_knobs().items())) for p in PRESETS.values()}
    spread = {}
    for j in JUNCTIONS:
        vals = [measure_one(j, rth, dict(k)) for k in knob_sets]
        dc = [v['dc'].real for v in vals]
        ac = [abs(v['ac']) for v in vals]
        spread[j.key] = (min(dc), max(dc), min(ac), max(ac), len(vals))
    return spread


def upstream_sensitivity(results):
    """Max relative change of each R_th when the upstream R_th switches between its DC and 1 kHz value."""
    a, b = measure('dc', force=True), measure('ac', force=True)
    return {k: max(abs(a[k]['dc'] - b[k]['dc']) / abs(a[k]['dc']), abs(a[k]['ac'] - b[k]['ac']) / abs(a[k]['ac']))
            for k in results}


def _k(v):
    return f'{v / 1e3:.4g} kΩ' if abs(v) >= 1e3 else f'{v:.4g} Ω'


def _z(z):
    return f'{_k(abs(z))} ∠{np.degrees(np.angle(z)):+.1f}°'


def plot(results, path):
    fig, (a1, a2) = plt.subplots(2, 1, figsize=(9, 7), sharex=True)
    for j in JUNCTIONS:
        r = results[j.key]
        f, z = r['f'], r['z'][j.node]
        a1.loglog(f, np.abs(z), label=f'{j.name} ({j.node})')
        a2.semilogx(f, np.degrees(np.angle(z)), label=j.name)
    a1.set_ylabel('|Z| [Ω]')
    a2.set_ylabel('phase [°]')
    a2.set_xlabel('frequency [Hz]')
    for a in (a1, a2):
        a.grid(True, which='both', alpha=0.3)
        a.axvline(F_AC, color='k', lw=0.5)
    a1.legend(fontsize=8)
    a1.set_title('Upstream output impedance at the junction nodes (default parameters)')
    fig.tight_layout()
    fig.savefig(path, dpi=110)
    plt.close(fig)


def write_report(results, fd, spread, sens):
    out = config.REPORT_DIR / 'M0'
    out.mkdir(parents=True, exist_ok=True)
    stem = f'{datetime.date.today().strftime("%Y%m%d")}_junctions'
    plot(results, out / f'{stem}.png')
    lines = [
        '# M0 junction Thevenin resistances', '',
        f'- Date: {datetime.date.today().isoformat()}, ngspice {spice.ngspice_version()}, default parameters '
        'unless stated',
        '- Method (SPEC §7 M0.5): 1 A AC current into the open-circuited output node of the upstream block '
        'testbench, block input AC-grounded, linearised at the DC operating point. DC = 1 mHz point of the '
        f'sweep (capacitors effectively open), cross-checked by a ±{I_FD * 1e6:g} µA finite difference of '
        'two DC operating points.',
        '- Upstream junctions inside a testbench (Rth_j2 in b3) use the junctions.py value, '
        'or the DC value measured here while it is unset.',
        '- junctions.py (used by the testbenches): ' + ', '.join(f'{k} = {_k(v)}' for k, v in RTH.items() if v)
        + ' (|Z(1 kHz)|, D-018)', '',
        '| Junction | Node | Testbench | R_th DC | DC (op finite diff.) | Z @ 1 kHz | Re Z @ 1 kHz |',
        '|---|---|---|---|---|---|---|']
    for j in JUNCTIONS:
        r = results[j.key]
        lines.append(f'| {j.name} | {j.node} | {j.testbench} | {_k(r["dc"].real)} | {_k(fd[j.key])} | '
                     f'{_z(r["ac"])} | {_k(r["ac"].real)} |')
    lines += ['', '## Spread over presets', '',
              '| Junction | Knob sets | R_th DC min–max | \\|Z\\| @ 1 kHz min–max |', '|---|---|---|---|']
    for j in JUNCTIONS:
        lo_dc, hi_dc, lo_ac, hi_ac, n = spread[j.key]
        lines.append(f'| {j.name} | {n} | {_k(lo_dc)} – {_k(hi_dc)} | {_k(lo_ac)} – {_k(hi_ac)} |')
    lines += ['', '## LTP coupling (J5)', '',
              'Transimpedance to the other PI plate (v(other) per A injected); the J-Thevenin model has no '
              'such term.', '', '| Injected at | Observed | DC | @ 1 kHz | Ratio to self @ 1 kHz |',
              '|---|---|---|---|---|']
    for j in JUNCTIONS:
        r = results[j.key]
        for o in j.observe:
            lines.append(f'| {j.node} | {o} | {_k(r["mutual_dc"][o].real)} | {_z(r["mutual_ac"][o])} | '
                         f'{abs(r["mutual_ac"][o]) / abs(r["ac"]):.2f} |')
    lines += ['', '## Sensitivity to the upstream R_th', '',
              'Max relative change when every upstream R_th is switched from its DC value to |Z(1 kHz)|: ' +
              ', '.join(f'{k} {v:.1e}' for k, v in sens.items()), '',
              '## |Z(f)|', '', f'![impedance]({stem}.png)', '', '## |Z| at selected frequencies', '',
              '| Junction | ' + ' | '.join(f'{f:g} Hz' for f in (1e-3, 10, 100, 1e3, 1e4)) + ' |',
              '|---|' + '---|' * 5]
    for j in JUNCTIONS:
        r = results[j.key]
        lines.append(f'| {j.name} | ' + ' | '.join(_z(_at(r['f'], r['z'][j.node], f))
                                                     for f in (1e-3, 10, 100, 1e3, 1e4)) + ' |')
    path = out / f'{stem}.md'
    path.write_text('\n'.join(lines) + '\n')
    return path


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--no-report', action='store_true')
    a = ap.parse_args()
    results = measure()
    fd = {j.key: dc_finite_difference(j, upstream_rth(results, 'dc')) for j in JUNCTIONS}
    for j in JUNCTIONS:
        r = results[j.key]
        print(f'{j.name:12s} {j.node:4s}  DC {_k(r["dc"].real):>10s} (op FD {_k(fd[j.key])})   '
              f'1 kHz {_z(r["ac"])}')
    if not a.no_report:
        path = write_report(results, fd, preset_spread(results), upstream_sensitivity(results))
        print(f'rth_measure: wrote {path.relative_to(config.ROOT)}')


if __name__ == '__main__':
    main()
