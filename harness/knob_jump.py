"""Knob-jump test (SPEC §6.2, VERIFICATION §7, DECISIONS D-025) for B4 ToneStackMaster.

Each case ramps knob targets 0 → 1 in 10 ms during a 1 kHz, 1 V tone (on the J3 DC level). The engine runs
through `ampwav --knob-ramp` (it smooths the targets and updates its scattering matrix every
kParamUpdateInterval samples). The reference is SPICE with the pot resistances as expressions of recorded
rotation nodes that follow the same smoothed trajectory continuously. Pass: no NaN/Inf and ESR ≤ −40 dB
over the ramp and the 20 ms after it. Peak error and error energy above 5 kHz are reported as information.
Writes harness/reports/M1/<YYYYMMDD>_b4_knob_jump.md.

    python -m harness.knob_jump
"""
import argparse
import datetime
import math
import re
import tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

import numpy as np

from . import config, metrics, spice
from .blocks import BLOCKS
from .compare import _git
from .config import DEFAULT_KNOBS
from .run_engine import _ampwav
from .tolerances import TOLERANCES
from .wavio import read_wav, write_wav

T_START, T_RAMP, T_AFTER, T_TOTAL = 0.020, 0.010, 0.020, 0.080
SMOOTHING_MS = 20.0              # EngineConfig::paramSmoothingMs default
SMOOTH_EPS = 1e-7                # KnobSmoother::kEpsilon
TONE_HZ, TONE_V = 1000.0, 1.0
KNOBS = ('treble', 'middle', 'bass', 'master')
CASES = [(k,) for k in KNOBS] + [KNOBS]
# pot expressions of the b4 testbench with the rotation replaced by a node voltage (netlist tapers)
LTAP = 'min(max(V(rot_{k}),1e-4),0.9999)'
ATAP = 'max((pwr(81,V(rot_{k}))-1)/80,1e-4)'
POTS = {
    'RVR3a': ('tt tw', '220k*(1-' + LTAP + ')', 'treble'),
    'RVR3b': ('tw nE', '220k*' + LTAP, 'treble'),
    'RVR5': ('nE nbb', '1Meg*' + ATAP + '+1', 'bass'),
    'RVR4a': ('nbb mw', '22k*(1-' + LTAP + ')', 'middle'),
    'RVR4b': ('mw 0', '22k*' + LTAP, 'middle'),
    'RVR2a': ('tw y', '1Meg*(1-' + ATAP + ')+1', 'master'),
    'RVR2b': ('y 0', '1Meg*' + ATAP + '+1', 'master'),
}


def targets(n, knobs_ramped, knob):
    """Target rotation per sample, exactly as ampwav's --knob-ramp (from 0 to 1)."""
    if knob not in knobs_ramped:
        return np.full(n, DEFAULT_KNOBS[knob])
    t = np.arange(n) / config.FS
    return np.clip((t - T_START) / T_RAMP, 0.0, 1.0)


def smoothed(target):
    """The engine's KnobSmoother applied per sample (value after the step at sample n)."""
    coeff = math.exp(-1.0 / (SMOOTHING_MS * 1e-3 * config.FS))
    out = np.empty_like(target)
    v = target[0]
    for n, tg in enumerate(target):
        if v != tg:
            v = tg + coeff * (v - tg)
            if abs(v - tg) < SMOOTH_EPS:
                v = tg
        out[n] = v
    return out


def spice_testbench():
    text = BLOCKS['b4'].path.read_text()
    for name, (nodes, expr, knob) in POTS.items():
        text, n = re.subn(rf'^{name} .*$', f"{name} {nodes} R='{expr.format(k=knob)}'", text, flags=re.M)
        assert n == 1, name
    return text + ''.join(f'Vrec_rot_{k} rot_{k} 0 DC 0\n' for k in KNOBS)


def run_case(case, x, workdir):
    n = len(x)
    rot = {k: smoothed(targets(n, case, k)) for k in KNOBS}
    ref = spice.tran(spice_testbench(), {}, x, ['y'], recorded={f'rot_{k}': rot[k] for k in KNOBS})['y']

    d = Path(workdir)
    tag = '_'.join(case)
    inp, out = d / f'in_{tag}.wav', d / f'out_{tag}.wav'
    write_wav(inp, x)
    args = ['--block', 'b4', '--in', str(inp), '--out', str(out), '--rate', str(config.FS)]
    args += [a for k in KNOBS if k not in case for a in ('--param', f'{k}={DEFAULT_KNOBS[k]!r}')]
    args += [a for k in case for a in ('--knob-ramp', f'{k}={T_START!r},{T_RAMP!r},0,1')]
    _ampwav(args)
    y = read_wav(out)[0]

    w = slice(int(T_START * config.FS), int((T_START + T_RAMP + T_AFTER) * config.FS))
    r, e = metrics.ac_part(ref)[w], metrics.ac_part(y)[w]
    err = e - r
    spec = np.fft.rfft(err * np.hanning(len(err)))
    f = np.fft.rfftfreq(len(err), 1 / config.FS)
    hf = float(np.sum(np.abs(spec[f > 5000]) ** 2))
    tone = float(np.sum(np.abs(np.fft.rfft(r * np.hanning(len(r)))) ** 2))
    return {'case': '+'.join(case), 'finite': bool(np.all(np.isfinite(y))),
            'esr': metrics.esr_db(r, e), 'peak_rel': float(np.max(np.abs(err)) / np.max(np.abs(r))),
            'hf_db': 10 * math.log10(max(hf, 1e-300) / tone)}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.parse_args()
    dc = spice.vin_dc(BLOCKS['b4'].path.read_text())
    t = np.arange(int(T_TOTAL * config.FS)) / config.FS
    x = dc + TONE_V * np.sin(2 * np.pi * TONE_HZ * t)
    with tempfile.TemporaryDirectory(prefix='amp2203_knob_') as d, ThreadPoolExecutor() as ex:
        rows = list(ex.map(lambda c: run_case(c, x, d), CASES))
    limit = -40.0
    lines = ['# M1 report: B4 knob-jump test', '',
             f'- Date: {datetime.date.today().isoformat()}, engine commit `{_git()}`, ngspice '
             f'{spice.ngspice_version()}, fs = {config.FS} Hz, SPICE reltol {config.SPICE_RELTOL}',
             f'- Knob targets 0 → 1 in {T_RAMP * 1e3:g} ms from t = {T_START * 1e3:g} ms during a {TONE_HZ:g} Hz, '
             f'{TONE_V:g} V tone on the J3 DC level ({dc:.4f} V); other knobs at their defaults. Engine: smoothing '
             f'{SMOOTHING_MS:g} ms, matrix update every 32 samples. SPICE: pot resistances follow the same smoothed '
             'trajectory continuously.',
             f'- Window: the ramp and the {T_AFTER * 1e3:g} ms after it. Pass (D-025): no NaN/Inf, ESR ≤ {limit:g} dB.', '',
             '| Case | Finite | ESR | Status | Peak error / ref peak | Error energy > 5 kHz vs tone |',
             '|---|---|---|---|---|---|']
    for r in rows:
        ok = r['finite'] and r['esr'] <= limit
        lines.append(f'| {r["case"]} | {"yes" if r["finite"] else "NO"} | {r["esr"]:.1f} dB | '
                     f'{"PASS" if ok else "FAIL"} | {r["peak_rel"]:.2e} | {r["hf_db"]:.1f} dB |')
        print(f'knob_jump {r["case"]:28s} ESR {r["esr"]:7.1f} dB  peak {r["peak_rel"]:.2e}  HF {r["hf_db"]:.1f} dB  '
              f'{"PASS" if ok else "FAIL"}')
    assert TOLERANCES['nonlinear_esr_db'][0] == limit  # same level as the block-exact ESR (D-025)
    out = config.REPORT_DIR / 'M1'
    out.mkdir(parents=True, exist_ok=True)
    path = out / f'{datetime.date.today().strftime("%Y%m%d")}_b4_knob_jump.md'
    path.write_text('\n'.join(lines) + '\n')
    print(f'knob_jump: wrote {path.relative_to(config.ROOT)}')


if __name__ == '__main__':
    main()
