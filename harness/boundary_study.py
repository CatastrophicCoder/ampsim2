"""M6 experiment X6: where can the amp be split into per-block models? Engine only.

    python -m harness.boundary_study            # renders (cached in harness/cache/boundary/) and writes the report

For each candidate boundary node b (signal handed from one block to the next) the knobs downstream of b must not
change v_b, or the model of the block ending at b needs those knobs as inputs. Measured with the full amp
(ampwav, 768 kHz, --j4 exact): v_b at a changed downstream setting vs v_b at the default downstream knobs, same
upstream knobs, same input. ESR in the 48 kHz band, each signal minus its value at t = 0 (VERIFICATION §6).
"""
import datetime
import os
import subprocess
import sys
import tempfile
import time
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

import numpy as np
from scipy.signal import butter, resample_poly, sosfiltfilt

from . import config, metrics
from . import x3_neural as x3
from .wavio import read_wav, write_wav

RATE, FACTOR = x3.RATE, x3.TEACHER_FACTOR
CACHE = config.HARNESS / 'cache' / 'boundary'
NODES = ('p1a', 'p1b', 'p2a', 'k2b', 'y', 'bpa', 'bpx')
# knobs downstream of each boundary (signal order: V1a, Volume, V1b, V2a, V2b, tone stack, Master -> y, PI + Presence)
DOWNSTREAM = {
    'p1a': ('preampVolume', 'treble', 'middle', 'bass', 'master', 'presence'),
    'p1b': ('treble', 'middle', 'bass', 'master', 'presence'),
    'p2a': ('treble', 'middle', 'bass', 'master', 'presence'),
    'k2b': ('treble', 'middle', 'bass', 'master', 'presence'),
    'y': ('presence',),
    'bpa': ('preampVolume', 'treble', 'middle', 'bass', 'master', 'presence'),  # supply: every knob (information)
    'bpx': ('preampVolume', 'treble', 'middle', 'bass', 'master', 'presence'),
}
VOLUMES = (0.2, 0.6, 1.0)
DOWN_KNOBS = ('treble', 'middle', 'bass', 'master', 'presence')
TRACKS = ('GTR L', 'GTR RR')
GAINS_DB = (0.0, 12.0)
SECONDS = 5.0
WORKERS = 6
LIMITS = (-20.0, -40.0)
HP_HZ = 20.0  # second figure: both signals high-passed (sub-audio sag drift removed)


def _settings():
    """{label: knobs}: per Volume the defaults, each downstream knob at 0 and 1 alone, all at 0, all at 1."""
    out = {}
    for v in VOLUMES:
        base = {**config.DEFAULT_KNOBS, 'preampVolume': v}
        out[f'V{v:g} base'] = base
        for k in DOWN_KNOBS:
            for val in (0.0, 1.0):
                out[f'V{v:g} {k}={val:g}'] = {**base, k: val}
        out[f'V{v:g} all=0'] = {**base, **{k: 0.0 for k in DOWN_KNOBS}}
        out[f'V{v:g} all=1'] = {**base, **{k: 1.0 for k in DOWN_KNOBS}}
    return out


def _signals():
    tracks = x3._tracks()
    n = int(SECONDS * RATE)
    return {f'{t} {g:+.0f}dB': tracks[t][:n] * 10 ** (g / 20) for t in TRACKS for g in GAINS_DB}


def _render(x48, knobs):
    """Full amp at 768 kHz (exact J4) on a 48 kHz full-scale input; NODES in volts, decimated to 48 kHz."""
    hi = resample_poly(x48, FACTOR, 1)
    with tempfile.TemporaryDirectory(prefix='amp2203_x6_') as d:
        d = Path(d)
        write_wav(d / 'in.wav', hi, RATE * FACTOR)
        args = [str(x3.AMPWAV), '--block', 'full', '--in', str(d / 'in.wav'), '--out', str(d / 'out.wav'),
                '--units', 'fs', '--rate', str(RATE * FACTOR), '--j4', 'exact']
        args += [a for k, v in knobs.items() for a in ('--param', f'{k}={v!r}')]
        args += [a for n in NODES for a in ('--dump-node', f'{n}={d / n}.wav')]
        r = subprocess.run(args, capture_output=True, text=True)
        if r.returncode != 0:
            raise RuntimeError(r.stderr)
        out = {}
        for n in NODES:
            v, _ = read_wav(d / f'{n}.wav')
            out[n] = v[0] + resample_poly(v - v[0], 1, FACTOR)  # band-limit the signal, keep the t = 0 value
    return out


def render():
    CACHE.mkdir(parents=True, exist_ok=True)
    sigs, sets = _signals(), _settings()
    jobs = [(s, k) for s in sigs for k in sets]

    def one(job):
        s, k = job
        path = CACHE / f'{s}|{k}.npz'.replace(' ', '_')
        if path.exists():
            return path, 0.0
        t0 = time.time()
        np.savez(path, **_render(sigs[s], sets[k]))
        return path, time.time() - t0

    with ThreadPoolExecutor(max_workers=WORKERS) as ex:
        for i, (path, secs) in enumerate(ex.map(one, jobs)):
            print(f'x6: {i + 1}/{len(jobs)} {path.name} ({secs:.0f} s)', flush=True)


def _load(s, k):
    with np.load(CACHE / f'{s}|{k}.npz'.replace(' ', '_')) as z:
        return {n: z[n] for n in z.files}


def analyse():
    sigs, sets = _signals(), _settings()
    first = int(0.1 * RATE)
    hp = butter(4, HP_HZ, 'highpass', fs=RATE, output='sos')
    rows = []  # (node, setting label, changed knobs, worst ESR, signal at worst, worst ESR after the high-pass)
    for node in NODES:
        down = set(DOWNSTREAM[node])
        for label, knobs in sets.items():
            v = label.split()[0]
            base_label = f'{v} base' if node != 'p1a' and node not in ('bpa', 'bpx') else 'V0.6 base'
            base = sets[base_label]
            changed = sorted(k for k in knobs if knobs[k] != base[k])
            if not changed or not set(changed) <= down:
                continue  # an upstream knob differs: not a test of this boundary
            worst, worst_hp = (-np.inf, ''), -np.inf
            for s in sigs:
                a, b = _load(s, base_label)[node], _load(s, label)[node]
                a, b = a - a[0], b - b[0]
                worst = max(worst, (metrics.esr_db(a[first:], b[first:]), s))
                worst_hp = max(worst_hp, metrics.esr_db(sosfiltfilt(hp, a)[first:], sosfiltfilt(hp, b)[first:]))
            rows.append((node, label, changed, worst[0], worst[1], worst_hp))
    return rows


def write_report(rows):
    out = config.REPORT_DIR / 'M6'
    out.mkdir(parents=True, exist_ok=True)
    path = out / f'{datetime.date.today().strftime("%Y%m%d")}_x6_boundaries.md'
    lines = ['# M6 experiment X6: block boundaries for per-block models (engine only)', '',
             f'- Date {datetime.date.today().isoformat()}; full amp `ampwav --j4 exact` at {RATE * FACTOR} Hz; '
             f'signals {", ".join(f"DI {t}" for t in TRACKS)} (first {SECONDS:g} s) at {GAINS_DB} dB; Preamp '
             f'Volume {VOLUMES}; tone 0.5, Master 0.3, Presence 0.5 unless changed.',
             '- Value: ESR (48 kHz band, first 0.1 s excluded) of the boundary node at the changed setting vs the '
             'same node at the default downstream knobs, worst of the four signals. Lower = the downstream knob '
             'changes the boundary less. `clean` ≤ −40 dB, `ok` ≤ −20 dB (D-051), `coupled` above −20 dB.',
             '- Supply nodes bpa / bpx are compared against Volume 0.6 with every knob counted as downstream '
             '(information: how far the power amp\'s load reaches back through the supply).', '',
             '## Summary: worst case per boundary and knob', '',
             f'| Boundary | Knob(s) changed | Worst ESR | Setting | Signal | Class | Worst ESR > {HP_HZ:g} Hz | Class > {HP_HZ:g} Hz |',
             '|---|---|---|---|---|---|---|---|']
    cls = lambda e: 'clean' if e <= LIMITS[1] else 'ok' if e <= LIMITS[0] else 'coupled'
    groups = {}
    for node, label, changed, e, s, ehp in rows:
        key = (node, '+'.join(changed) if len(changed) < 5 else 'all downstream')
        if key not in groups or e > groups[key][0]:
            groups[key] = (e, label, s, max(ehp, groups.get(key, (0, 0, 0, -np.inf))[3]))
        else:
            groups[key] = (*groups[key][:3], max(ehp, groups[key][3]))
    for (node, knobs), (e, label, s, ehp) in groups.items():
        lines.append(f'| {node} | {knobs} | {e:.1f} dB | {label} | {s} | {cls(e)} | {ehp:.1f} dB | {cls(ehp)} |')
    lines += ['', '## All comparisons', '', f'| Boundary | Setting | Worst ESR | Signal | Worst ESR > {HP_HZ:g} Hz |',
              '|---|---|---|---|---|']
    lines += [f'| {n} | {label} | {e:.1f} dB | {s} | {ehp:.1f} dB |' for n, label, _, e, s, ehp in rows]
    path.write_text('\n'.join(lines) + '\n')
    print(f'x6: wrote {path.relative_to(config.ROOT)}')


def main():
    render()
    write_report(analyse())


if __name__ == '__main__':
    sys.exit(main())
