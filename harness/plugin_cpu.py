"""M6: plugin CPU (`amp2203_render plugin`, the Amp2203Processor offline, oversampling included) at its defaults,
static and under knob automation between host blocks (lfo: 10 Hz triangle 0 → 10 → 0; jump: 0 ↔ 10 every 50 ms;
each knob alone and all six), from the default knobs and from every knob at 10. One render at a time (timing).

    python -m harness.plugin_cpu [--seconds 20] [--factor 2]

Writes harness/reports/M6/<YYYYMMDD>_plugin_cpu.md.
"""
import argparse
import datetime
import re
import subprocess
import tempfile
from pathlib import Path

from . import config
from . import x3_neural as x3
from .compare import _git
from .plugin_null import RENDER
from .wavio import write_wav

RATE = 48000
KNOBS = ('preampVolume', 'treble', 'middle', 'bass', 'master', 'presence')
BASES = {'default': [], 'every knob 10': [a for k in KNOBS for a in ('--param', f'{k}=10')]}


def _render(x_path, out_path, factor, extra):
    r = subprocess.run([str(RENDER), 'plugin', '--in', x_path, '--out', out_path, '--factor', str(factor)] + extra,
                       capture_output=True, text=True)
    if r.returncode != 0:
        raise RuntimeError(r.stderr)
    m = re.search(r'([\d.]+) % of one core', r.stderr)
    f = re.search(r'Newton failures (\d+) \(rescued (\d+)\)', r.stderr)
    return float(m.group(1)), int(f.group(1)), int(f.group(2))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--seconds', type=float, default=20.0)
    ap.add_argument('--factor', type=int, default=2)
    a = ap.parse_args()
    x = x3._tracks()['GTR L'][RATE:RATE + int(a.seconds * RATE)]
    rows = []
    with tempfile.TemporaryDirectory(prefix='amp2203_cpu_') as d:
        d = Path(d)
        write_wav(d / 'x.wav', x, RATE)
        cases = [(base, 'static', '—', 512) for base in BASES]
        cases += [(base, kind, knob, 512) for base in BASES for knob in KNOBS + ('all',) for kind in ('lfo', 'jump')]
        cases += [(base, 'jump', 'all', 64) for base in BASES]
        for base, kind, knob, block in cases:
            extra = BASES[base] + ['--block', str(block)]
            if kind != 'static':
                extra += ['--automate', f'{knob}={kind}']
            cpu, fail, resc = _render(d / 'x.wav', d / 'y.wav', a.factor, extra)
            rows.append((base, knob, kind, block, cpu, fail, resc))
            print(f'plugin_cpu: {base:14s} {knob:12s} {kind:6s} block {block:3d}: {cpu:5.1f} % (failures {fail}, '
                  f'rescued {resc})', flush=True)
    lines = ['# M6 plugin CPU under knob automation', '',
             f'- Date {datetime.date.today().isoformat()}, commit `{_git()}`; `amp2203_render plugin` at the plugin defaults '
             f'({a.factor}x, Power Amp Rate Half, exact J4, Newton tol 1e-4 V, half-steps on; D-054 – D-056), FIR '
             f'oversampling, {a.seconds:g} s of DI GTR L at {RATE} Hz; processBlock time only.',
             '- Automation between host blocks: lfo = 10 Hz triangle 0 → 10 → 0, jump = 0 ↔ 10 every 50 ms; "all" moves '
             'the six knobs together. One render at a time.', '',
             '| Base | Knob(s) | Modulation | Host block | CPU (% of one core) | Newton failures (rescued) |',
             '|---|---|---|---|---|---|']
    lines += [f'| {b} | {k} | {m} | {bl} | {c:.1f} | {f} ({r}) |' for b, k, m, bl, c, f, r in rows]
    out = config.REPORT_DIR / 'M6'
    out.mkdir(parents=True, exist_ok=True)
    path = out / f'{datetime.date.today().strftime("%Y%m%d")}_plugin_cpu.md'
    path.write_text('\n'.join(lines) + '\n')
    print(f'plugin_cpu: wrote {path.relative_to(config.ROOT)}')


if __name__ == '__main__':
    main()
