"""M3 preamp cascade: partition and total error at the master wiper y (SPEC §7 M3, VERIFICATION §1, §7).

For each Preamp Volume preset (VOL0.2, DEFAULT = 0.6, VOL1.0; other knobs default) and the standard
signal set at the B12 levels:
- F: the full netlist (the PI load at y present),
- P: the SPICE chain b12 -> b34 (B3 and B4 in one tree, D-033), its J2 source driven by b12's
  open-circuit p1b,
- E: the engine cascade (`ampwav --block preamp`: B12 -> B34).
P and E use F's recorded supply voltages bpA and bpX (VERIFICATION §1), so supply modelling (B7, M4)
is not part of the partition error. At y, P and E are open-circuit voltages while F's y carries the PI
load; the column `P56` therefore also drives the b56 testbench (J4 source R_th4 into the PI, power stage
and supply) with P's v_oc(y) and compares its loaded y and the speaker voltage with F.
Compared at p1b and k2b (loaded nodes in F, P and E) and at y (v_oc in P and E): P vs F is the partition
error (J2 and the PI load at J4), E vs F the total error, E vs P the block-exact error of the chain. Scored at y on sines: P vs F
and E vs F were the SPEC §7 M3 wording; with the owner's decision Q15 (DECISIONS D-036) the scored
checks are: partition error = P56 vs F at the loaded y, ESR ≤ −26 dB on sines (DI ≤ −20 dB; skipped
when there are no DI clips); engine = E vs P at y with the block-exact limits by level (≤ −40 dB,
≤ −30 dB for the 2 V overdrive sines, harmonics 2–5 within 0.5 dB) and the Newton checks. P vs F and
E vs F at the open-circuit y are reported. Also the small-signal partition error |H_P / H_F| at y. Writes harness/reports/M3/<YYYYMMDD>_cascade.md.

    python -m harness.cascade [--preset VOL0.2 ...] [--jobs N]
"""
import argparse
import datetime
import json
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
from .presets import PRESETS  # noqa: E402
from .render_spice import block_input  # noqa: E402
from .run_engine import _ampwav  # noqa: E402
from .tolerances import status  # noqa: E402
from .wavio import read_wav, write_wav  # noqa: E402

PRESET_IDS = ('VOL0.2', 'DEFAULT', 'VOL1.0')
NODES = ('p1b', 'k2b', 'y')
SUPPLIES = ('bpa', 'bpx')
PAIRS = (('P', 'F'), ('E', 'F'), ('E', 'P'))
LOADED = ('y', 'out')  # P56 vs F
BANDS = ((20.0, 10e3), (10e3, 20e3))


def signal_set():
    return signals.standard_set(BLOCKS['b12'].levels)


def _texts():
    t = {k: BLOCKS[k].path.read_text() for k in ('full', 'b12', 'b34', 'b56')}
    # the constant supplies become recorded sources (spice.tran Vrec_<name>), driven by F's waveforms
    t['b12'] = t['b12'].replace('Vsup_bpA bpA 0 DC', 'Vrec_bpa bpA 0 DC')
    t['b34'] = t['b34'].replace('Vsup_bpX bpX 0 DC', 'Vrec_bpx bpX 0 DC')
    assert 'Vrec_bpa' in t['b12'] and 'Vrec_bpx' in t['b34']
    return t


def render_f(texts, knobs, sig):
    return spice.tran(texts['full'], knobs, block_input(BLOCKS['full'], sig), list(NODES + SUPPLIES + ('out',)))


P56_RETRY_RELTOL = 1e-7


def render_p56(texts, knobs, p_res):
    """Returns (result, note). If ngspice stops with 'timestep too small' at the configured RELTOL, the run
    is repeated once at P56_RETRY_RELTOL and the note says so (the report lists it)."""
    try:
        return spice.tran(texts['b56'], knobs, p_res['y'], list(LOADED)), None
    except spice.SpiceError as e:
        if 'Timestep too small' not in str(e):
            raise
        res = spice.tran(texts['b56'], knobs, p_res['y'], list(LOADED), reltol=P56_RETRY_RELTOL)
        return res, f'P56 rendered at RELTOL {P56_RETRY_RELTOL:g} (ngspice: timestep too small at {config.SPICE_RELTOL:g})'


def render_chain(texts, knobs, sig):
    f = render_f(texts, knobs, sig)
    p = render_p(texts, knobs, sig, f)
    p56, note = render_p56(texts, knobs, p)
    return {'F': f, 'P': p, 'P56': p56, 'note': note}


def render_p(texts, knobs, sig, f_res):
    voc_p1b = spice.tran(texts['b12'], knobs, block_input(BLOCKS['b12'], sig), ['p1b'],
                         recorded={'bpa': f_res['bpa']})['p1b']
    return spice.tran(texts['b34'], knobs, voc_p1b, list(NODES), recorded={'bpx': f_res['bpx']})


def run_e(knobs, sig, workdir, f_res):
    d = Path(workdir) / sig.id
    d.mkdir(parents=True, exist_ok=True)
    write_wav(d / 'in.wav', block_input(BLOCKS['b12'], sig))
    args = ['--block', 'preamp', '--in', str(d / 'in.wav'), '--out', str(d / 'out.wav'), '--rate', str(config.FS),
            '--stats', str(d / 'stats.json')]
    for n in SUPPLIES:
        write_wav(d / f'supply_{n}.wav', f_res[n])
        args += ['--supply', f'{n}={d / f"supply_{n}.wav"}']
    args += [a for k, v in knobs.items() for a in ('--param', f'{k}={v!r}')]
    args += [a for n in NODES for a in ('--dump-node', f'{n}={d / n}.wav')]
    _ampwav(args)
    out = {n: read_wav(d / f'{n}.wav')[0] for n in NODES}
    out['stats'] = json.loads((d / 'stats.json').read_text())
    return out


def small_signal(texts, knobs):
    """|H_P / H_F| at y: product of the block transfers (each at its own junction source) vs the full netlist."""
    f, h12 = spice.ac(texts['b12'], knobs, 'p1b', 'in')
    _, h34 = spice.ac(texts['b34'], knobs, 'y', 'j2s')
    _, hf = spice.ac(texts['full'], knobs, 'y', 'in')
    return f, 20 * np.log10(np.abs(h12 * h34) / np.abs(hf))


def analyse(sig, res):
    row = {'id': sig.id, 'kind': sig.kind, 'amp': sig.meta.get('amp'), 'esr': {}, 'note': res.get('note')}
    ac = {k: {n: metrics.ac_part(res[k][n]) for n in NODES} for k in ('F', 'P', 'E')}
    for a, b in PAIRS:
        for n in NODES:
            row['esr'][(a, b, n)] = metrics.esr_db(ac[b][n], ac[a][n])
    for n in LOADED:
        row['esr'][('P56', 'F', n)] = metrics.esr_db(metrics.ac_part(res['F'][n]), metrics.ac_part(res['P56'][n]))
    if sig.kind == 'SIN':
        f0 = sig.meta['freq']
        h = {k: metrics.harmonics(res[k]['y'], f0) for k in ('F', 'P', 'E')}
        for a, b in PAIRS:
            diff, scored = metrics.harmonic_diff_db(h[b], h[a])
            sel = scored.copy()
            sel[0] = False
            sel[5:] = False
            row[('h1', a, b)] = float(diff[0])
            row[('h2_5', a, b)] = float(np.max(np.abs(diff[sel]))) if sel.any() else None
    st = res['E']['stats']
    row['newton'] = (st['newton_failures'], st['median_iterations'], st['max_iterations_used'])
    return row


def run(preset_ids, jobs=None, log=print):
    texts = _texts()
    sigs = signal_set()
    out = {}
    with ThreadPoolExecutor(max_workers=jobs or os.cpu_count()) as ex, \
            tempfile.TemporaryDirectory(prefix='amp2203_cascade_') as tmp:
        futures = {}
        for pid in preset_ids:
            knobs = PRESETS[pid].all_knobs()
            for s in sigs:
                # one task per signal (F -> P -> P56 depend on each other), so no worker waits on another
                futures[(pid, s.id)] = ex.submit(render_chain, texts, knobs, s)
        for pid in preset_ids:
            knobs = PRESETS[pid].all_knobs()
            rows, traces = [], {}
            for s in sigs:
                res = futures[(pid, s.id)].result()
                res['E'] = run_e(knobs, s, Path(tmp) / pid, res['F'])
                rows.append(analyse(s, res))
                traces[s.id] = {k: res[k]['y'] for k in ('F', 'P', 'E')}
                log(f'  cascade {pid}/{s.id}: y ESR P-F {rows[-1]["esr"][("P", "F", "y")]:.1f} dB, '
                    f'E-F {rows[-1]["esr"][("E", "F", "y")]:.1f} dB, E-P {rows[-1]["esr"][("E", "P", "y")]:.1f} dB, '
                    f'P56-F loaded y {rows[-1]["esr"][("P56", "F", "y")]:.1f} dB, out {rows[-1]["esr"][("P56", "F", "out")]:.1f} dB')
            f, ss = small_signal(texts, knobs)
            out[pid] = {'knobs': knobs, 'rows': rows, 'traces': traces, 'f': f, 'small_signal': ss}
    return out


def _check(row):
    return 'partition_esr_db_di' if row['kind'] == 'DI' else 'partition_esr_db_sines'


def score(res, milestone):
    """Scored checks (D-036): [(check, item, value, tolerance text, status)]."""
    od = BLOCKS['b12'].levels['overdrive']
    rows = []

    def add(check, item, value):
        tol, st = status(milestone, 'preamp', check, value)
        rows.append((check, item, value, tol, st))

    for pid, d in res.items():
        for r in d['rows']:
            item = f'{pid} {r["id"]}'
            if r['kind'] in ('SIN', 'DI'):
                add(_check(r), item + ' (loaded y, P56 vs F)', r['esr'][('P56', 'F', 'y')])
            esr = 'nonlinear_esr_db_overdrive' if r['kind'] == 'SIN' and r['amp'] == od else 'nonlinear_esr_db'
            add(esr, item + ' (y, E vs P)', r['esr'][('E', 'P', 'y')])
            if r.get(('h2_5', 'E', 'P')) is not None:
                add('harmonics_2_5_db', item + ' (y, E vs P)', r[('h2_5', 'E', 'P')])
            fail, median, mx = r['newton']
            add('newton_failures', item, int(fail))
            add('newton_median_iterations', item, int(median))
            add('newton_max_iterations', item, int(mx))
    return rows


def plot(res, outdir, stem):
    names = []
    fig, axes = plt.subplots(1, len(res), figsize=(4.5 * len(res), 3.6), sharey=True)
    for ax, (pid, d) in zip(np.atleast_1d(axes), res.items()):
        ax.semilogx(d['f'], d['small_signal'])
        ax.set_title(f'{pid} (Preamp Volume {d["knobs"]["preampVolume"]:g})')
        ax.set_xlabel('frequency [Hz]')
        ax.grid(True, which='both', alpha=0.3)
    np.atleast_1d(axes)[0].set_ylabel('|H_P / H_F| at y [dB]')
    fig.suptitle('Small-signal partition error at y (J2, J4)')
    fig.tight_layout()
    fig.savefig(outdir / f'{stem}_small_signal.png', dpi=110)
    plt.close(fig)
    names.append(f'{stem}_small_signal.png')

    fig, axes = plt.subplots(len(res), 1, figsize=(10, 3.0 * len(res)))
    for ax, (pid, d) in zip(np.atleast_1d(axes), res.items()):
        sines = [r for r in d['rows'] if r['kind'] == 'SIN']
        worst = max(sines, key=lambda r: r['esr'][('E', 'F', 'y')])
        tr = d['traces'][worst['id']]
        n = int(0.005 * config.FS)
        t = np.arange(n) / config.FS * 1e3
        for k, style in (('F', '-'), ('P', '--'), ('E', ':')):
            ax.plot(t, metrics.ac_part(tr[k])[-n:], style, label=k)
        ax.set_title(f'{pid}: {worst["id"]} at y (last 5 ms; worst E vs F sine)')
        ax.set_xlabel('time [ms]')
        ax.set_ylabel('V (minus DC)')
        ax.grid(True, alpha=0.3)
        ax.legend(fontsize=8)
    fig.tight_layout()
    fig.savefig(outdir / f'{stem}_worst.png', dpi=110)
    plt.close(fig)
    names.append(f'{stem}_worst.png')
    return names


def write_report(res, milestone='M3'):
    outdir = config.REPORT_DIR / milestone
    outdir.mkdir(parents=True, exist_ok=True)
    stem = f'{datetime.date.today().strftime("%Y%m%d")}_cascade'
    pngs = plot(res, outdir, stem)
    lines = [f'# {milestone} report: preamp cascade B12 → B34, partition and total error at y', '',
             f'- Date: {datetime.date.today().isoformat()}, engine commit `{_git()}`, ngspice {spice.ngspice_version()}, '
             f'fs = {config.FS} Hz, SPICE reltol {config.SPICE_RELTOL}',
             f'- F: `full.cir` (sha256 `{_sha(BLOCKS["full"].path)}`), P: b12 → b34 testbenches chained through '
             'the J2 source, E: `ampwav --block preamp`. At y, P and E are v_oc(y) (no PI load), F has '
             'the PI load; at p1b and k2b all three are the loaded nodes. P and E run on F\'s recorded bpA / bpX.',
             '- ESR on each signal minus its own DC operating point (D-018). Scored (D-036): partition error = P56 vs F '
             'at the loaded y (P\'s v_oc(y) through the J4 source into the b56 testbench), sines ≤ −26 dB, DI ≤ −20 dB; '
             'engine = E vs P at y with the block-exact limits by level (≤ −40 dB, 2 V sines ≤ −30 dB, harmonics 2–5 '
             '≤ 0.5 dB) and the Newton checks. Everything else is information.', '']
    scored = score(res, milestone)
    counts = {k: sum(1 for r in scored if r[4] == k) for k in ('PASS', 'FAIL', 'INFO')}
    summary = ['## Summary (worst over the signal set; ESR in dB)', '',
               '| Preset | loaded y P56–F (sines) | out P56–F | y E–P | y P–F (open) | y E–F (open) | k2b P–F | p1b P–F |',
               '|---|---|---|---|---|---|---|---|']
    for pid, d in res.items():
        rows = d['rows']
        sines = [r for r in rows if r['kind'] == 'SIN']
        w = lambda key, rs=rows: max(r['esr'][key] for r in rs)  # noqa: E731
        summary.append(f'| {pid} | {w(("P56", "F", "y"), sines):.1f} | {w(("P56", "F", "out")):.1f} | '
                       f'{w(("E", "P", "y")):.1f} | {w(("P", "F", "y")):.1f} | {w(("E", "F", "y")):.1f} | '
                       f'{w(("P", "F", "k2b")):.1f} | {w(("P", "F", "p1b")):.1f} |')
    lines += [f'Scored checks: PASS {counts["PASS"]} · FAIL {counts["FAIL"]}', ''] + summary
    fails = [r for r in scored if r[4] == 'FAIL']
    lines += ['', '## Failed checks', '']
    lines += (['| Check | Item | Measured | Tolerance |', '|---|---|---|---|'] +
              [f'| {c} | {i} | {v:.4g} | {t} |' for c, i, v, t, _ in fails]) if fails else ['None.']
    lines += ['', '## Small-signal partition error at y', '',
              '| Preset | max \\|H_P/H_F\\| 20 Hz–10 kHz | 10–20 kHz |', '|---|---|---|']
    for pid, d in res.items():
        f, e = d['f'], d['small_signal']
        band = [float(np.max(np.abs(e[(f >= lo) & (f <= hi)]))) for lo, hi in BANDS]
        lines.append(f'| {pid} | {band[0]:.3f} dB | {band[1]:.3f} dB |')
    lines += ['', f'![small-signal partition error]({pngs[0]})', '', '## Per signal (ESR in dB)', '',
              '| Preset | Signal | y P–F | y E–F | y E–P | loaded y P56–F | out P56–F | k2b P–F | k2b E–F | p1b P–F | p1b E–F | '
              'h1 E–F | h2–5 E–F | Newton fail / median / max |', '|---|---|---|---|---|---|---|---|---|---|---|---|---|---|']
    for pid, d in res.items():
        for r in d['rows']:
            e = r['esr']
            h1 = r.get(('h1', 'E', 'F'))
            h25 = r.get(('h2_5', 'E', 'F'))
            lines.append(f'| {pid} | {r["id"]} | {e[("P", "F", "y")]:.1f} | {e[("E", "F", "y")]:.1f} | '
                         f'{e[("E", "P", "y")]:.1f} | {e[("P56", "F", "y")]:.1f} | {e[("P56", "F", "out")]:.1f} | '
                         f'{e[("P", "F", "k2b")]:.1f} | {e[("E", "F", "k2b")]:.1f} | '
                         f'{e[("P", "F", "p1b")]:.1f} | {e[("E", "F", "p1b")]:.1f} | '
                         f'{"—" if h1 is None else f"{h1:+.2f} dB"} | {"—" if h25 is None else f"{h25:.2f} dB"} | '
                         f'{r["newton"][0]} / {r["newton"][1]:g} / {r["newton"][2]:g} |')
    notes = [f'- {pid} {r["id"]}: {r["note"]}' for pid, d in res.items() for r in d['rows'] if r.get('note')]
    if notes:
        lines += ['', '## Notes', ''] + notes
    lines += ['', f'![worst sine at y]({pngs[1]})', '']
    path = outdir / f'{stem}.md'
    path.write_text('\n'.join(lines) + '\n')
    return path, counts


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--preset', action='append', choices=PRESET_IDS)
    ap.add_argument('--jobs', type=int)
    a = ap.parse_args()
    res = run(a.preset or list(PRESET_IDS), a.jobs)
    path, counts = write_report(res)
    print(f'cascade: wrote {path.relative_to(config.ROOT)} ({counts["FAIL"]} FAIL)')


if __name__ == '__main__':
    main()
