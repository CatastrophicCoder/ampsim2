"""Diagnostics for block-exact deviations (M2): separates engine error from reference error and from the
engine's discretisation. Not scored; the numbers go into the milestone report and DECISIONS.

1. Grid-diode CJO/TT (tubes.lib, ignored by the engine per SPEC §5): SPICE with CJO = TT = 0 (a copy of
   tubes.lib in harness/cache/, the reference files are not changed) vs SPICE, and the engine vs both.
2. Engine discretisation: the engine at 1x, 2x and 4x the verification rate on the piecewise-linear
   input that SPICE sees (linear interpolation of the 384 kHz samples), compared at the 384 kHz
   instants; and ESR after low-pass filters on both signals.
3. SPICE reference convergence: harmonics 2–4 of a sine vs ngspice RELTOL.
Writes harness/reports/<milestone>/<YYYYMMDD>_<block>_diagnostics.md.

    python -m harness.diagnostics --block b12 [--milestone M2]
"""
import argparse
import datetime
import re
import tempfile
from pathlib import Path

import numpy as np
import scipy.signal as ss

from . import config, metrics, signals, spice
from .blocks import BLOCKS
from .compare import _git
from .render_spice import block_input
from .run_engine import _ampwav
from .wavio import read_wav, write_wav

CJO_SIGNALS = [('SIN', 100.0, 0.01), ('SIN', 1000.0, 0.01), ('SIN', 5000.0, 0.01), ('SIN', 5000.0, 2.0)]
RATE_SIGNALS = [('STEP', None, 0.1), ('SIN', 5000.0, 0.1), ('SIN', 5000.0, 2.0)]
LOWPASS = (40e3, 20e3, 5e3)
RELTOLS = (1e-5, 1e-6, 1e-7, 1e-8)
VOLUMES = (0.2, 0.6, 1.0)


def _signal(kind, f, amp):
    return signals.step(amp) if kind == 'STEP' else signals.sine(f, amp)


def _no_diode_caps_lib():
    text = config.TUBES.read_text()
    new, n = re.subn(r'D\(IS=1n RS=1 CJO=1p TT=1n\)', 'D(IS=1n RS=1 CJO=0 TT=0)', text)
    assert n == 2, 'tubes.lib diode models changed; update harness/diagnostics.py'
    path = config.CACHE_DIR / 'tubes_no_diode_caps.lib'
    path.parent.mkdir(parents=True, exist_ok=True)
    if not path.exists() or path.read_text() != new:
        path.write_text(new)
    return path


def _engine(block, x, knobs, fs, workdir):
    d = Path(workdir)
    write_wav(d / 'in.wav', x, fs)
    args = ['--block', block.engine_block, '--in', str(d / 'in.wav'), '--out', str(d / 'out.wav'), '--rate', str(fs)]
    args += [a for k, v in knobs.items() for a in ('--param', f'{k}={v!r}')]
    _ampwav(args)
    return read_wav(d / 'out.wav')[0]


def _spice(block, sig, knobs, reltol=config.SPICE_RELTOL, tubes=None):
    saved = config.TUBES
    try:
        if tubes is not None:
            config.TUBES = tubes
        return spice.tran(block.path.read_text(), knobs, block_input(block, sig), [block.output_node],
                          reltol=reltol)[block.output_node]
    finally:
        config.TUBES = saved


def _esr(ref, y):
    return metrics.esr_db(metrics.ac_part(ref), metrics.ac_part(y))


def diode_caps(block, knobs, workdir):
    lib = _no_diode_caps_lib()
    rows = []
    for kind, f, amp in CJO_SIGNALS:
        sig = _signal(kind, f, amp)
        ref, alt = _spice(block, sig, knobs), _spice(block, sig, knobs, tubes=lib)
        y = _engine(block, block_input(block, sig), knobs, config.FS, workdir)
        rows.append((sig.id, _esr(ref, alt), _esr(ref, y), _esr(alt, y)))
    return rows


def rates(block, knobs, workdir):
    rows = []
    for kind, f, amp in RATE_SIGNALS:
        sig = _signal(kind, f, amp)
        x = block_input(block, sig)
        ref = _spice(block, sig, knobs)
        t = np.arange(len(x)) / config.FS
        esr = []
        for m in (1, 2, 4):
            tt = np.arange(len(x) * m - (m - 1)) / (config.FS * m)
            y = _engine(block, np.interp(tt, t, x), knobs, config.FS * m, workdir)[::m]
            esr.append(_esr(ref, y))
        y = _engine(block, x, knobs, config.FS, workdir)
        lp = []
        for fc in LOWPASS:
            sos = ss.butter(8, fc, fs=config.FS, output='sos')
            lp.append(metrics.esr_db(ss.sosfiltfilt(sos, metrics.ac_part(ref)), ss.sosfiltfilt(sos, metrics.ac_part(y))))
        rows.append((sig.id, esr, lp))
    return rows


def reltols(block, knobs, workdir, f=5000.0, amp=0.1):
    sig = signals.sine(f, amp)
    y = _engine(block, block_input(block, sig), knobs, config.FS, workdir)
    he = metrics.harmonics(y, f)
    rows = [('engine', 20 * np.log10(he[1:4] / he[0]), None)]
    for rt in RELTOLS:
        ref = _spice(block, sig, knobs, reltol=rt)
        h = metrics.harmonics(ref, f)
        rows.append((f'SPICE reltol {rt:g}', 20 * np.log10(h[1:4] / h[0]), _esr(ref, y)))
    return sig.id, rows


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--block', default='b12', choices=['b12'])
    ap.add_argument('--milestone', default='M2')
    a = ap.parse_args()
    block = BLOCKS[a.block]
    lines = [f'# {a.milestone} diagnostics: {block.id}', '',
             f'- Date: {datetime.date.today().isoformat()}, engine commit `{_git()}`, ngspice {spice.ngspice_version()}, '
             f'fs = {config.FS} Hz, SPICE reltol {config.SPICE_RELTOL} unless stated', f'- Output node {block.output_node}; '
             'ESR on each signal minus its own DC operating point (D-018). Not scored.', '']
    with tempfile.TemporaryDirectory(prefix='amp2203_diag_') as d:
        lines += ['## 1. Grid-diode CJO/TT (ignored by the engine, SPEC §5)', '',
                  'SPICE′ = SPICE with CJO = TT = 0 in a copy of tubes.lib (harness/cache/; reference unchanged).', '',
                  '| Preamp Volume | Signal | SPICE′ vs SPICE | engine vs SPICE | engine vs SPICE′ |', '|---|---|---|---|---|']
        for vol in VOLUMES:
            knobs = dict(config.DEFAULT_KNOBS, preampVolume=vol)
            for sid, a1, a2, a3 in diode_caps(block, knobs, d):
                lines.append(f'| {vol:g} | {sid} | {a1:.1f} dB | {a2:.1f} dB | {a3:.1f} dB |')
                print(f'diagnostics: CJO vol {vol:g} {sid}: {a1:.1f} / {a2:.1f} / {a3:.1f} dB')
        lines += ['', '## 2. Engine discretisation', '',
                  'ESR vs SPICE with the engine at 1x / 2x / 4x of 384 kHz (input: the piecewise-linear signal through '
                  'the 384 kHz samples, as SPICE sees it; output at the 384 kHz instants), and at 1x after an '
                  f'8th-order low-pass ({", ".join(f"{fc / 1e3:g} kHz" for fc in LOWPASS)}, zero phase) on both signals.', '',
                  '| Preamp Volume | Signal | 384 kHz | 768 kHz | 1536 kHz | ' + ' | '.join(f'LP {fc / 1e3:g} kHz' for fc in LOWPASS)
                  + ' |', '|---|---|---|---|---|' + '---|' * len(LOWPASS)]
        for vol in VOLUMES:
            knobs = dict(config.DEFAULT_KNOBS, preampVolume=vol)
            for sid, esr, lp in rates(block, knobs, d):
                lines.append(f'| {vol:g} | {sid} | ' + ' | '.join(f'{e:.1f} dB' for e in esr + lp) + ' |')
                print(f'diagnostics: rates vol {vol:g} {sid}: ' + ' '.join(f'{e:.1f}' for e in esr + lp))
        lines += ['', '## 3. SPICE reference convergence (harmonics vs RELTOL)', '']
        for vol in (0.2,):
            knobs = dict(config.DEFAULT_KNOBS, preampVolume=vol)
            sid, rows = reltols(block, knobs, d)
            lines += [f'Preamp Volume {vol:g}, {sid}:', '', '| Source | H2 | H3 | H4 | ESR engine vs this |', '|---|---|---|---|---|']
            for name, h, e in rows:
                lines.append(f'| {name} | ' + ' | '.join(f'{v:.2f} dBc' for v in h) + f' | {"—" if e is None else f"{e:.1f} dB"} |')
                print(f'diagnostics: {name}: ' + ' '.join(f'{v:.3f}' for v in h))
    out = config.REPORT_DIR / a.milestone
    out.mkdir(parents=True, exist_ok=True)
    path = out / f'{datetime.date.today().strftime("%Y%m%d")}_{block.id}_diagnostics.md'
    path.write_text('\n'.join(lines) + '\n')
    print(f'diagnostics: wrote {path.relative_to(config.ROOT)}')


if __name__ == '__main__':
    main()
