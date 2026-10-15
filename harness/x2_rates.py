"""M6 experiments X2 / A1 (docs/ARCHITECTURE_STUDY.md §6): the full engine (exact J4) at lower internal rates,
scored as the real-time plugin model check (VERIFICATION §7, D-051).

    python -m harness.x2_rates                                     # X2: 2x / 4x / 8x of 48 kHz
    python -m harness.x2_rates --tag a1 --power-rate-divisor 2 --newton-tol 1e-5   # A1 variants

Checks per rate: ESR vs SPICE (cached 384 kHz references, 48 kHz band, held-out 5 s DI at CRUNCH, DRIVE, HOT);
ESR vs the engine at 16x on the 16 X4 validation settings; Newton failures; aliasing vs 16x; cost (ampwav --stats,
one run at a time). Engine runs are cached in harness/cache/x2/.
"""
import argparse
import datetime
import hashlib
import json
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

import numpy as np
from scipy.signal import resample_poly

from . import config, metrics, tolerances
from . import x3_neural as x3
from . import x4_conditioned as x4
from .presets import PRESETS
from .wavio import read_wav, write_wav

RATE = 48000
REF_FACTOR = 16
CACHE = config.HARNESS / 'cache' / 'x2'
WORKERS = 6
VAL_SECONDS = 10.0
COST_SECONDS = 10.0
LIMIT = tolerances.TOLERANCES['plugin_model_esr_db'][0]


_ENGINE_ID = None


def _engine_id():
    """Hash of the ampwav binary: cached runs belong to the engine build that made them."""
    global _ENGINE_ID
    if _ENGINE_ID is None:
        _ENGINE_ID = hashlib.sha256(x3.AMPWAV.read_bytes()).hexdigest()[:16]
    return _ENGINE_ID


def _run(x48, knobs, factor, opts, stats_only=False):
    """Full amp (exact J4) at 48 kHz × factor on a 48 kHz full-scale input. Returns (out at 48 kHz, stats).
    Cached by input, knobs, factor and options (not for cost runs)."""
    key = hashlib.sha256(x48.astype(np.float64).tobytes() + json.dumps([knobs, factor, opts], sort_keys=True).encode()
                         + _engine_id().encode())
    path = CACHE / f'{key.hexdigest()[:24]}.npz'
    if path.exists() and not stats_only:
        with np.load(path) as z:
            return z['y'], json.loads(str(z['stats']))
    hi = resample_poly(x48, factor, 1)
    with tempfile.TemporaryDirectory(prefix='amp2203_x2_') as d:
        d = Path(d)
        write_wav(d / 'in.wav', hi, RATE * factor)
        args = [str(x3.AMPWAV), '--block', 'full', '--in', str(d / 'in.wav'), '--out', str(d / 'out.wav'),
                '--units', 'fs', '--rate', str(RATE * factor), '--j4', 'exact', '--stats', str(d / 'stats.json')]
        args += [a for k, v in knobs.items() for a in ('--param', f'{k}={v!r}')]
        if opts.get('divisor', 1) != 1:
            args += ['--power-rate-divisor', str(opts['divisor'])]
        if opts.get('tol'):
            args += ['--newton-tol', repr(opts['tol'])]
        if 'trigger' in opts:
            args += ['--half-step-trigger', repr(opts['trigger'])]
        r = subprocess.run(args, capture_output=True, text=True)
        if r.returncode != 0:
            raise RuntimeError(r.stderr)
        stats = json.loads((d / 'stats.json').read_text())
        if stats_only:
            return None, stats
        y = resample_poly(read_wav(d / 'out.wav')[0], 1, factor)[:len(x48)]
    CACHE.mkdir(parents=True, exist_ok=True)
    np.savez(path, y=y, stats=json.dumps(stats))
    return y, stats


def _jobs(factors, opts):
    """Accuracy jobs: (kind, id, x48, knobs, factor, opts)."""
    tracks = x3._tracks()
    n5 = int(x3.HOLD_HEAD_S * RATE)
    jobs = []
    for f in factors:
        for pid in x4.EVAL_PRESETS:
            for t in sorted(tracks):
                jobs.append(('spice', f'{pid}|{t}', tracks[t][:n5], PRESETS[pid].all_knobs(), f, opts))
    vx, _, vk = x4._load('val')
    nv = int(VAL_SECONDS * RATE)
    for i, (x, k) in enumerate(zip(vx, vk)):
        knobs = x4._all_knobs(k)
        for f in factors:
            jobs.append(('val', str(i), x[:nv].astype(np.float64), knobs, f, opts))
        jobs.append(('val', str(i), x[:nv].astype(np.float64), knobs, REF_FACTOR, {}))
    return jobs


def run(factors, opts, tag):
    refs = dict(np.load(x4.CACHE / 'spice_refs.npz'))
    jobs = _jobs(factors, opts)
    with ThreadPoolExecutor(max_workers=WORKERS) as ex:
        res = list(ex.map(lambda j: _run(j[2], j[3], j[4], j[5]), jobs))
        print(f'x2: {len(jobs)} engine runs done', flush=True)
    s = slice(int(0.1 * RATE), int(x3.HOLD_HEAD_S * RATE))
    sv = slice(RATE, int(VAL_SECONDS * RATE))
    spice, val, newton = {}, {}, {}
    ref16 = {j[1]: r[0] for j, r in zip(jobs, res) if j[0] == 'val' and j[4] == REF_FACTOR}
    for (kind, ident, _, _, f, o), (y, st) in zip(jobs, res):
        if f == REF_FACTOR and kind == 'val':
            continue
        nw = newton.setdefault(f, [0, 0, 0, 0])
        nw[0] += st['newton_failures']
        nw[1] += st.get('newton_unrescued', 0)
        nw[2] = max(nw[2], st['max_iterations_used'])
        nw[3] += 1
        if kind == 'spice':
            ref = refs[ident]
            spice.setdefault(f, {})[ident] = metrics.esr_db(ref[s], y[s])
        else:
            val.setdefault(f, {})[ident] = metrics.esr_db(ref16[ident][sv], y[sv])
    alias = {}
    for f in factors:
        for pid in ('CRUNCH', 'HOT'):
            knobs = PRESETS[pid].all_knobs()
            alias[(f, pid)] = x3._alias_test(lambda x: _run(x, knobs, f, opts)[0],
                                             lambda x: _run(x, knobs, REF_FACTOR, {})[0])
    n = RATE
    t = np.arange(2 * n) / RATE
    alias_newton = {}  # the same sines as x3._alias_test: (failures, not rescued) per rate
    for f in factors + [REF_FACTOR]:
        for pid in ('CRUNCH', 'HOT'):
            knobs = PRESETS[pid].all_knobs()
            for f0 in (5000.0, 9000.0):
                for amp in (0.1, 0.4):
                    st = _run(amp * np.sin(2 * np.pi * f0 * t), knobs, f, opts if f != REF_FACTOR else {})[1]
                    alias_newton[(f, pid, f0, amp)] = (st['newton_failures'], st.get('newton_unrescued', 0))
    print('x2: aliasing done', flush=True)
    tracks = x3._tracks()
    xc = tracks['GTR L'][RATE:RATE + int(COST_SECONDS * RATE)]
    cost = {}
    for f in factors:  # one at a time: timing
        for name, knobs in (('default', config.DEFAULT_KNOBS), ('HOT', PRESETS['HOT'].all_knobs())):
            _, st = _run(xc, knobs, f, opts, stats_only=True)
            cost[(f, name)] = (st['ns_per_sample'], st['ns_per_sample'] * f * RATE / 1e7)
            print(f'x2: cost {f}x {name}: {st["ns_per_sample"]:.0f} ns per engine sample = '
                  f'{cost[(f, name)][1]:.1f} % of one core', flush=True)
    write_report(factors, opts, tag, spice, val, newton, alias, alias_newton, cost)


def write_report(factors, opts, tag, spice, val, newton, alias, alias_newton, cost):
    out = config.REPORT_DIR / 'M6'
    out.mkdir(parents=True, exist_ok=True)
    path = out / f'{datetime.date.today().strftime("%Y%m%d")}_x2_rates{"_" + tag if tag else ""}.md'
    o = ', '.join(f'{k} {v}' for k, v in opts.items()) or 'defaults (power amp at full rate, Newton tol 1e-9 V)'
    lines = [f'# M6 {"A1" if tag else "X2"}: full engine (exact J4) at lower internal rates, plugin model check', '',
             f'- Date {datetime.date.today().isoformat()}; options: {o}; rates 48 kHz × {factors}.',
             f'- Plugin model check (D-051): ESR ≤ {LIMIT:g} dB vs SPICE (cached {x4.SPICE_FS} Hz references, 48 kHz '
             'band, first 5 s of every DI track, first 0.1 s excluded). Unseen settings: the 16 X4 validation '
             f'settings, {VAL_SECONDS:g} s, vs the engine at {REF_FACTOR}x (first 1 s excluded). Cost: ampwav '
             f'--stats on {COST_SECONDS:g} s of DI GTR L, one run at a time; oversampling filters not included.', '',
             '## Summary', '',
             '| Rate | vs SPICE worst (CRUNCH / DRIVE / HOT) | Check | Unseen settings vs 16x (worst / median) | '
             'Newton failures (not rescued) / runs, max iterations | Cost default / HOT (% of one core) |',
             '|---|---|---|---|---|---|']
    for f in factors:
        w = {pid: max(e for k, e in spice[f].items() if k.startswith(pid)) for pid in x4.EVAL_PRESETS}
        worst = max(w.values())
        v = list(val[f].values())
        nw = newton[f]
        lines.append(f'| {f}x ({RATE * f // 1000} kHz) | {w["CRUNCH"]:.1f} / {w["DRIVE"]:.1f} / {w["HOT"]:.1f} dB | '
                     f'{"PASS" if worst <= LIMIT else "FAIL"} | {max(v):.1f} / {np.median(v):.1f} dB | '
                     f'{nw[0]} ({nw[1]}) / {nw[3]}, {nw[2]} | {cost[(f, "default")][1]:.1f} / {cost[(f, "HOT")][1]:.1f} |')
    lines += ['', '## vs SPICE per clip', '', '| Clip | ' + ' | '.join(f'{f}x' for f in factors) + ' |',
              '|---|' + '---|' * len(factors)]
    for k in spice[factors[0]]:
        lines.append(f'| {k.replace("|", " ")} | ' + ' | '.join(f'{spice[f][k]:.1f} dB' for f in factors) + ' |')
    lines += ['', '## Aliasing (power outside the harmonics of f0, relative to the total; 16x for reference)', '',
              'Each cell: non-harmonic power, then Newton failures (not rescued) in that 2 s run. Non-converged '
              'samples add non-harmonic power, so a cell with many failures measures solver error as well as '
              'aliasing.', '',
              '| Preset | f0 | amplitude (FS) | 16x | ' + ' | '.join(f'{f}x' for f in factors) + ' |',
              '|---|---|---|---|' + '---|' * len(factors)]
    cell = lambda e, nw: f'{e:.1f} dB, {nw[0]} ({nw[1]})'
    for pid in ('CRUNCH', 'HOT'):
        for i, (f0, amp, t, _) in enumerate(alias[(factors[0], pid)]):
            lines.append(f'| {pid} | {f0:.0f} Hz | {amp:g} | {cell(t, alias_newton[(REF_FACTOR, pid, f0, amp)])} | ' +
                         ' | '.join(cell(alias[(f, pid)][i][3], alias_newton[(f, pid, f0, amp)]) for f in factors) + ' |')
    lines += ['', '## Cost', '', '| Rate | ns per engine sample (default / HOT) | % of one core at 48 kHz |', '|---|---|---|']
    for f in factors:
        lines.append(f'| {f}x | {cost[(f, "default")][0]:.0f} / {cost[(f, "HOT")][0]:.0f} | '
                     f'{cost[(f, "default")][1]:.1f} / {cost[(f, "HOT")][1]:.1f} |')
    path.write_text('\n'.join(lines) + '\n')
    print(f'x2: wrote {path.relative_to(config.ROOT)}')


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--factors', type=int, nargs='+', default=[2, 4, 8])
    ap.add_argument('--power-rate-divisor', type=int, default=1)
    ap.add_argument('--newton-tol', type=float, default=None)
    ap.add_argument('--half-step-trigger', type=float, default=None, help='V; 0 = off (D-043 default 100)')
    ap.add_argument('--tag', default='')
    a = ap.parse_args()
    opts = {}
    if a.power_rate_divisor != 1:
        opts['divisor'] = a.power_rate_divisor
    if a.newton_tol:
        opts['tol'] = a.newton_tol
    if a.half_step_trigger is not None:
        opts['trigger'] = a.half_step_trigger
    run(a.factors, opts, a.tag)


if __name__ == '__main__':
    sys.exit(main())
