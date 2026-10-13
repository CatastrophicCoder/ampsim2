"""M5 wrapper null test (SPEC §7 M5): an offline render through the plugin's processor at 16x
(`amp2203_render plugin`) against `ampwav --block full` at 768 kHz on the processor's own oversampled input,
followed by the same resampling: down(ampwav(up(x))), with the oversampler's up and down stages run alone
(`amp2203_render up / down`). Default levels (Interface Level +12 dBu, trim and output 0 dB), default knobs.
ESR ≤ −60 dB proves the wrapper adds nothing unintended. Both oversampling filters (linear-phase FIR,
polyphase IIR). Signals at 48 kHz: sines (100 Hz, 1 kHz, 5 kHz at −40 / −20 / −6 dBFS, 0.2 s) and the first
5 s of each DI clip (resampled 44.1 → 48 kHz). Writes harness/reports/M5/<YYYYMMDD>_plugin_null.md.

    python -m harness.plugin_null [--jobs N]
"""
import argparse
import datetime
import os
import subprocess
import tempfile
from concurrent.futures import ThreadPoolExecutor
from math import gcd
from pathlib import Path

import numpy as np
from scipy.signal import resample_poly

from . import config, metrics
from .compare import _git
from .wavio import read_wav, write_wav

HOST_RATE = 48000
FACTOR = 16
RENDER = config.ROOT / 'build' / 'plugin' / 'amp2203_render_artefacts' / 'Release' / 'amp2203_render'
LIMIT_DB = -60.0


def signals():
    out = []
    t = np.arange(int(0.2 * HOST_RATE)) / HOST_RATE
    for f in (100.0, 1000.0, 5000.0):
        for dbfs in (-40.0, -20.0, -6.0):
            out.append((f'SIN-{f:g}Hz-{dbfs:g}dBFS', 10 ** (dbfs / 20) * np.sin(2 * np.pi * f * t)))
    for path in sorted(config.DI_DIR.glob('*.wav')):
        x, rate = read_wav(path, mono=True)
        x = x[: int(5.0 * rate)]
        g = gcd(int(rate), HOST_RATE)
        out.append((f'DI-{path.stem}', resample_poly(x, HOST_RATE // g, int(rate) // g)))
    return out


def _run(args):
    r = subprocess.run([str(a) for a in args], capture_output=True, text=True)
    if r.returncode != 0:
        raise RuntimeError(f'{args[0]} failed: {r.stderr}')
    return r.stderr


def one(name, x, filt):
    with tempfile.TemporaryDirectory(prefix='amp2203_null_') as d:
        d = Path(d)
        write_wav(d / 'x.wav', x, HOST_RATE)
        log = _run([RENDER, 'plugin', '--in', d / 'x.wav', '--out', d / 'plug.wav', '--factor', FACTOR, '--filter', filt])
        _run([RENDER, 'up', '--in', d / 'x.wav', '--out', d / 'up.wav', '--factor', FACTOR, '--filter', filt])
        _run([config.AMPWAV, '--block', 'full', '--units', 'fs', '--in', d / 'up.wav', '--out', d / 'eng.wav',
              '--rate', HOST_RATE * FACTOR])
        _run([RENDER, 'down', '--in', d / 'eng.wav', '--out', d / 'ref.wav', '--factor', FACTOR, '--filter', filt])
        a, b = read_wav(d / 'plug.wav')[0], read_wav(d / 'ref.wav')[0]
    n = min(len(a), len(b))
    return {'signal': name, 'filter': filt, 'esr': metrics.esr_db(b[:n], a[:n]), 'peak': float(np.max(np.abs(b[:n]))),
            'log': log.strip()}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--jobs', type=int)
    a = ap.parse_args()
    sigs = signals()
    with ThreadPoolExecutor(max_workers=a.jobs or os.cpu_count()) as ex:
        rows = list(ex.map(lambda item: one(*item), [(n, x, f) for f in ('fir', 'iir') for n, x in sigs]))
    nfail = sum(r['esr'] > LIMIT_DB for r in rows)
    lines = ['# M5 wrapper null test: plugin vs ampwav at 768 kHz with the same resampling', '',
             f'- Date: {datetime.date.today().isoformat()}, commit `{_git()}`; host rate {HOST_RATE} Hz, {FACTOR}x',
             '- Plugin: `amp2203_render plugin` (the Amp2203Processor offline). Reference: the processor\'s oversampler '
             'up stage, `ampwav --block full --units fs` at 768 kHz, the down stage. Default knobs and levels.',
             f'- Tolerance (SPEC §7 M5): ESR ≤ {LIMIT_DB:g} dB.', '',
             f'PASS {len(rows) - nfail} · FAIL {nfail}', '',
             '| Filter | Signal | Peak out [FS] | ESR | Status | Render log |', '|---|---|---|---|---|---|']
    for r in rows:
        lines.append(f'| {r["filter"]} | {r["signal"]} | {r["peak"]:.3f} | {r["esr"]:.1f} dB | '
                     f'{"PASS" if r["esr"] <= LIMIT_DB else "FAIL"} | {r["log"].replace("amp2203_render: ", "")} |')
    out = config.REPORT_DIR / 'M5'
    out.mkdir(parents=True, exist_ok=True)
    path = out / f'{datetime.date.today().strftime("%Y%m%d")}_plugin_null.md'
    path.write_text('\n'.join(lines) + '\n')
    print(f'plugin_null: wrote {path.relative_to(config.ROOT)} ({nfail} FAIL)')


if __name__ == '__main__':
    main()
