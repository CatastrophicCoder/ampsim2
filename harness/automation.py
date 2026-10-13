"""M5 automation check (SPEC §7 M5): every knob over its full range at audio rate, engine at 768 kHz
(`ampwav --block full`, the plugin's 16x at 48 kHz). Each knob alone and all six together, as a 10 Hz
triangle (0 → 1 → 0) and as 0 ↔ 1 jumps every 50 ms (the engine smooths the targets, SPEC §6.2), from the
default knobs and from every knob at 1.0; inputs: the first 5 s of the first DI clip (+12 dBu at 0 dBFS) and
a 300 mV 220 Hz sine (1 s). Checked: no NaN / Inf in the output; Newton statistics reported (failures of the
first solve, rescued / unrescued by the fallback, D-042). Writes harness/reports/M5/<YYYYMMDD>_automation.md.

    python -m harness.automation [--jobs N]
"""
import argparse
import datetime
import json
import os
import subprocess
import tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

import numpy as np

from . import config, signals
from .compare import _git
from .wavio import read_wav, write_wav

KNOBS = ('preampVolume', 'treble', 'middle', 'bass', 'master', 'presence')
BASES = {'default': dict(config.DEFAULT_KNOBS), 'max': {k: 1.0 for k in KNOBS}}
LFO_HZ = 10.0
JUMP_S = 0.05


def ramps(knob, kind, seconds):
    """--knob-ramp arguments: a triangle (LFO) or jumps between 0 and 1 for the whole signal."""
    out, t, up = [], 0.0, True
    half = 0.5 / LFO_HZ if kind == 'lfo' else JUMP_S
    while t < seconds:
        a, b = (0.0, 1.0) if up else (1.0, 0.0)
        dur = half if kind == 'lfo' else 0.0
        out += ['--knob-ramp', f'{knob}={t!r},{dur!r},{a!r},{b!r}']
        t += half
        up = not up
    return out


def inputs():
    t = np.arange(int(1.0 * config.FS)) / config.FS
    sigs = [('SIN-220Hz-0.3V', 0.3 * np.sin(2 * np.pi * 220 * t))]
    di = signals.di_signals()
    if di:
        sigs.append((di[0].id, di[0].x))
    return sigs


def one(base, knobset, kind, name, x):
    with tempfile.TemporaryDirectory(prefix='amp2203_auto_') as d:
        d = Path(d)
        write_wav(d / 'in.wav', x)
        args = [str(config.AMPWAV), '--block', 'full', '--in', str(d / 'in.wav'), '--out', str(d / 'out.wav'),
                '--rate', str(config.FS), '--stats', str(d / 's.json')]
        args += [a for k, v in BASES[base].items() if k not in knobset for a in ('--param', f'{k}={v!r}')]
        for k in knobset:
            args += ramps(k, kind, len(x) / config.FS)
        r = subprocess.run(args, capture_output=True, text=True)
        if r.returncode != 0:
            raise RuntimeError(r.stderr)
        y = read_wav(d / 'out.wav')[0]
        st = json.loads((d / 's.json').read_text())
    return {'base': base, 'knobs': '+'.join(knobset) if len(knobset) < 6 else 'all', 'kind': kind, 'signal': name,
            'finite': bool(np.all(np.isfinite(y))), 'peak': float(np.max(np.abs(y))), **st}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--jobs', type=int)
    a = ap.parse_args()
    work = [(base, ks, kind, n, x) for base in BASES for ks in [(k,) for k in KNOBS] + [KNOBS]
            for kind in ('lfo', 'jump') for n, x in inputs()]
    with ThreadPoolExecutor(max_workers=a.jobs or os.cpu_count()) as ex:
        rows = list(ex.map(lambda w: one(*w), work))
    nonfinite = sum(not r['finite'] for r in rows)
    fails = sum(r['newton_failures'] for r in rows)
    unresc = sum(r['newton_unrescued'] for r in rows)
    lines = ['# M5 automation: every knob over its full range at audio rate', '',
             f'- Date: {datetime.date.today().isoformat()}, commit `{_git()}`, engine `ampwav --block full` at {config.FS} Hz',
             f'- Modulation: {LFO_HZ:g} Hz triangle 0 → 1 → 0 (`lfo`) or 0 ↔ 1 jumps every {JUMP_S * 1e3:g} ms (`jump`) on the '
             'knob targets (smoothed by the engine, 20 ms); bases: default knobs, every knob at 1.0.',
             '- SPEC §7 M5: no NaN / Inf, no Newton failures. Newton failures count first solves that did not converge '
             'in 12 iterations; the fallback (D-042) rescued or did not rescue them.', '',
             f'Runs: {len(rows)} · non-finite outputs: {nonfinite} · Newton failures (first solve): {fails} · unrescued: {unresc}', '',
             '| Base | Knobs | Modulation | Signal | finite | peak [V] | solves | failures | rescued | unrescued | median / max it | ns/sample |',
             '|---|---|---|---|---|---|---|---|---|---|---|---|']
    for r in rows:
        lines.append(f'| {r["base"]} | {r["knobs"]} | {r["kind"]} | {r["signal"]} | {"yes" if r["finite"] else "NO"} | '
                     f'{r["peak"]:.1f} | {r["newton_solves"]} | {r["newton_failures"]} | {r["newton_rescued"]} | '
                     f'{r["newton_unrescued"]} | {r["median_iterations"]} / {r["max_iterations_used"]} | {r["ns_per_sample"]:.0f} |')
    out = config.REPORT_DIR / 'M5'
    out.mkdir(parents=True, exist_ok=True)
    path = out / f'{datetime.date.today().strftime("%Y%m%d")}_automation.md'
    path.write_text('\n'.join(lines) + '\n')
    print(f'automation: wrote {path.relative_to(config.ROOT)} (non-finite {nonfinite}, failures {fails}, unrescued {unresc})')


if __name__ == '__main__':
    main()
