"""M1 acceptance checks for B4 ToneStackMaster (SPEC §7 M1, VERIFICATION §7):
- magnitude: engine response from SWP-lo (1 V) vs SPICE AC for 5×5×5 Treble/Middle/Bass at
  Master ∈ {0.2, 0.5, 1.0}: max |dB error| ≤ 0.1 over 20 Hz–10 kHz and ≤ 0.3 over 10–20 kHz;
- time domain (`--tran`): engine vs SPICE transient for the log sweep, ESR ≤ −60 dB, at DEFAULT and the
  8 Treble/Middle/Bass corners at Master 0.5;
- R_th4: the engine's `rth4` probe vs the testbench formula evaluated by ngspice (D-015).
Writes harness/reports/M1/<YYYYMMDD>_b4_tonestack.md plus PNGs.

    python -m harness.tonestack_check [--tran] [--jobs N]
"""
import argparse
import datetime
import itertools
import os
import tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

import matplotlib

matplotlib.use('Agg')
import matplotlib.pyplot as plt  # noqa: E402
import numpy as np  # noqa: E402

from . import config, metrics, signals, spice  # noqa: E402
from .blocks import BLOCKS  # noqa: E402
from .compare import _git, _sha  # noqa: E402
from .config import DEFAULT_KNOBS  # noqa: E402
from .run_engine import _ampwav  # noqa: E402
from .tolerances import TOLERANCES  # noqa: E402
from .wavio import read_wav, write_wav  # noqa: E402

ROTATIONS = (0.0, 0.25, 0.5, 0.75, 1.0)
MASTERS = (0.2, 0.5, 1.0)
BANDS = {'linear_mag_db_20_10k': (20.0, 10e3), 'linear_mag_db_10k_20k': (10e3, 20e3)}


def settings():
    return [dict(DEFAULT_KNOBS, treble=t, middle=m, bass=b, master=v)
            for v in MASTERS for t, m, b in itertools.product(ROTATIONS, repeat=3)]


def tran_settings():
    corners = [dict(DEFAULT_KNOBS, treble=t, middle=m, bass=b, master=0.5)
               for t, m, b in itertools.product((0.0, 1.0), repeat=3)]
    return [dict(DEFAULT_KNOBS)] + corners


def label(k):
    return f'T{k["treble"]:g} M{k["middle"]:g} B{k["bass"]:g} V{k["master"]:g}'


class Engine:
    """Runs ampwav --block b4 on one input signal for many knob settings."""

    def __init__(self, x, workdir):
        self.dir = Path(workdir)
        self.inp = self.dir / 'in.wav'
        write_wav(self.inp, x)

    def run(self, knobs, dumps=()):
        tag = '_'.join(f'{v:g}' for v in knobs.values())
        out = self.dir / f'out_{tag}.wav'
        args = ['--block', 'b4', '--in', str(self.inp), '--out', str(out), '--rate', str(config.FS)]
        args += [x for k, v in knobs.items() for x in ('--param', f'{k}={v!r}')]
        args += [x for d in dumps for x in ('--dump-node', f'{d}={self.dir / (tag + "_" + d + ".wav")}')]
        _ampwav(args)
        y = read_wav(out)[0]
        nodes = {d: read_wav(self.dir / f'{tag}_{d}.wav')[0] for d in dumps}
        out.unlink()
        return y, nodes


def magnitude_check(text, sweep, x, engine, jobs):
    def one(k):
        f, h = spice.ac(text, k, 'y', 'j3s')
        y, _ = engine.run(k)
        e_db = metrics.sweep_response_db(sweep.x, metrics.ac_part(y), f)
        err = e_db - 20 * np.log10(np.abs(h))
        return k, f, err, {c: float(np.max(np.abs(err[(f >= lo) & (f <= hi)]))) for c, (lo, hi) in BANDS.items()}
    with ThreadPoolExecutor(max_workers=jobs) as ex:
        return list(ex.map(one, settings()))


def tran_check(text, sweep, x, engine, jobs):
    def one(k):
        ref = spice.tran(text, k, x, ['y'])['y']
        y, _ = engine.run(k)
        return k, metrics.esr_db(metrics.ac_part(ref), metrics.ac_part(y)), metrics.best_lag(
            metrics.ac_part(ref), metrics.ac_part(y))[0]
    with ThreadPoolExecutor(max_workers=jobs) as ex:
        return list(ex.map(one, tran_settings()))


def rth4_check(engine):
    b5 = BLOCKS['b5open'].path.read_text()
    rows = []
    for k in [dict(DEFAULT_KNOBS)] + tran_settings()[1:] + [dict(DEFAULT_KNOBS, master=1.0),
                                                           dict(DEFAULT_KNOBS, master=0.0)]:
        spice_r = spice.op(b5, k, ['@rth_j4[resistance]'])['@rth_j4[resistance]']
        _, nodes = engine.run(k, dumps=('rth4',))
        rows.append((k, spice_r, float(nodes['rth4'][0])))
    return rows


def _status(check, value):
    limit, _ = TOLERANCES[check]
    return 'PASS' if value <= limit else 'FAIL'


def plot(mag, outdir, stem):
    fig, ax = plt.subplots(figsize=(9, 4.5))
    f = mag[0][1]
    for v in MASTERS:
        errs = np.array([e for k, _, e, _ in mag if k['master'] == v])
        ax.semilogx(f, np.max(np.abs(errs), axis=0), label=f'Master {v:g}: max |error| over 125 settings')
    ax.axhline(0.1, color='k', lw=0.6, ls='--')
    ax.axhline(0.3, color='k', lw=0.6, ls=':')
    ax.set_xlabel('frequency [Hz]')
    ax.set_ylabel('|engine − SPICE AC| [dB]')
    ax.set_yscale('log')
    ax.grid(True, which='both', alpha=0.3)
    ax.legend(fontsize=8)
    ax.set_title('B4 magnitude error, 375 settings (dashed: 0.1 dB, dotted: 0.3 dB)')
    fig.tight_layout()
    path = outdir / f'{stem}_magnitude_error.png'
    fig.savefig(path, dpi=110)
    plt.close(fig)
    return path.name


def write_report(mag, tran, rth4):
    outdir = config.REPORT_DIR / 'M1'
    outdir.mkdir(parents=True, exist_ok=True)
    stem = f'{datetime.date.today().strftime("%Y%m%d")}_b4_tonestack'
    png = plot(mag, outdir, stem)
    tb = BLOCKS['b4'].path
    worst = {c: max(mag, key=lambda r: r[3][c]) for c in BANDS}
    lines = [
        '# M1 report: B4 ToneStackMaster checks', '',
        f'- Date: {datetime.date.today().isoformat()}, engine commit `{_git()}`, ngspice {spice.ngspice_version()}, '
        f'fs = {config.FS} Hz, SPICE reltol {config.SPICE_RELTOL}',
        f'- Testbench `{tb.relative_to(config.ROOT)}` sha256 `{_sha(tb)}`; netlist sha256 `{_sha(config.NETLIST)}`',
        '- Engine input: the J3 source voltage (DC operating point + signal, D-022); output v_oc(y).', '',
        '## Summary', '', '| Check | Measured (worst) | Tolerance | Status | Worst setting |', '|---|---|---|---|---|']
    for c in BANDS:
        k, _, _, b = worst[c]
        lim = TOLERANCES[c][0]
        lines.append(f'| {c} (375 settings) | {b[c]:.4f} dB | ≤ {lim:g} dB | {_status(c, b[c])} | {label(k)} |')
    if tran:
        k, e, _ = max(tran, key=lambda r: r[1])
        lines.append(f'| linear_esr_db (log sweep, {len(tran)} settings) | {e:.1f} dB | ≤ −60 dB | '
                     f'{_status("linear_esr_db", e)} | {label(k)} |')
    rel = max(abs(e / s - 1) for _, s, e in rth4)
    lines.append(f'| R_th4 engine vs testbench formula ({len(rth4)} settings) | {rel:.1e} relative | — | INFO | |')
    lines += ['', 'Master scales v_oc(y) by a frequency-independent ratio (unloaded wiper, VR2 total fixed), so '
              'the dB error is the same at every Master setting.', '', '## Magnitude error per Master', '',
              '| Master | max \\|error\\| 20 Hz–10 kHz | 10–20 kHz | settings |', '|---|---|---|---|']
    for v in MASTERS:
        rows = [r for r in mag if r[0]['master'] == v]
        lines.append(f'| {v:g} | {max(r[3]["linear_mag_db_20_10k"] for r in rows):.4f} dB | '
                     f'{max(r[3]["linear_mag_db_10k_20k"] for r in rows):.4f} dB | {len(rows)} |')
    lines += ['', f'![magnitude error]({png})', '']
    if tran:
        lines += ['## Time-domain ESR (log sweep 20 Hz–20 kHz, 1 V, 2 s + 50 ms tail)', '',
                  '| Setting | ESR | Best lag |', '|---|---|---|']
        lines += [f'| {label(k)} | {e:.1f} dB | {lag} |' for k, e, lag in tran]
        lines.append('')
    lines += ['## R_th4', '', '| Setting | Testbench (ngspice) | Engine | Rel. difference |', '|---|---|---|---|']
    lines += [f'| {label(k)} | {s:.6g} Ω | {e:.6g} Ω | {abs(e / s - 1):.1e} |' for k, s, e in rth4]
    path = outdir / f'{stem}.md'
    path.write_text('\n'.join(lines) + '\n')
    return path


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--tran', action='store_true', help='also run the SPICE transient sweeps (minutes)')
    ap.add_argument('--jobs', type=int, default=os.cpu_count())
    a = ap.parse_args()
    block = BLOCKS['b4']
    text = block.path.read_text()
    sweep = signals.sweep(block.levels['nominal'], 'SWP-lo')
    from .render_spice import block_input
    x = block_input(block, sweep)
    with tempfile.TemporaryDirectory(prefix='amp2203_b4_') as d:
        engine = Engine(x, d)
        mag = magnitude_check(text, sweep, x, engine, a.jobs)
        print(f'tonestack_check: magnitude for {len(mag)} settings: worst '
              + ', '.join(f'{c} {max(r[3][c] for r in mag):.4f} dB' for c in BANDS))
        tran = tran_check(text, sweep, x, engine, a.jobs) if a.tran else []
        if tran:
            print(f'tonestack_check: time-domain ESR worst {max(e for _, e, _ in tran):.1f} dB')
        rth4 = rth4_check(engine)
    path = write_report(mag, tran, rth4)
    print(f'tonestack_check: wrote {path.relative_to(config.ROOT)}')


if __name__ == '__main__':
    main()
