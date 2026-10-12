"""SPICE partition-error study for one junction (VERIFICATION §1: P chained vs a reference at the same
node). For J2: the P chain b12 -> b3 (b3's J2 source driven by b12's open-circuit p1b, through each
candidate R_th2) is compared with b123, where B12 and B3 are joined directly. Both have the same constant
supplies, so the difference is the J2 junction alone. Writes
harness/reports/M0/<YYYYMMDD>_partition_<junction>.md. (The J1 study of 2026-10-12, which led to merging
B1 and B2, ran at commit 4838919; DECISIONS D-016, D-017.)

    python -m harness.partition --junction j2
"""
import argparse
import datetime
import os
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass

import matplotlib

matplotlib.use('Agg')
import matplotlib.pyplot as plt  # noqa: E402
import numpy as np  # noqa: E402

from . import config, metrics, rth_measure, signals, spice, testbench_gen  # noqa: E402
from .blocks import BLOCKS  # noqa: E402
from .presets import PRESETS  # noqa: E402


@dataclass(frozen=True)
class Study:
    junction: str        # RTH key
    upstream: str        # testbench of the upstream block (output open)
    up_node: str         # its output node = junction node
    up_input: str        # its input node (AC transfer)
    downstream: str      # testbench of the downstream block (contains Rth_<junction>)
    down_input: str      # node of its Vin (junction source)
    out_node: str        # compared node
    reference: str       # joined testbench (no junction)
    levels: str          # BLOCKS key whose level table drives the upstream input
    presets: tuple       # knob sets


STUDIES = {
    'j2': Study('j2', 'b12', 'p1b', 'in', 'b3', 'j2s', 'k2b', 'b123', 'b12', ('CLEAN', 'DEFAULT', 'DRIVE')),
}
BANDS = ((20.0, 10000.0), (10000.0, 20000.0))


def candidates(study):
    """Candidate R_th values from the junction measurement (rth_measure)."""
    r = rth_measure.measure()[study.junction]
    return {'DC': float(r['dc'].real), '|Z| 1 kHz': float(abs(r['ac'])), 'Re Z 1 kHz': float(r['ac'].real)}


def _texts(study, rth_values):
    files = testbench_gen.generate()
    texts = {'up': files[f'{study.upstream}.cir'], 'ref': files[f'{study.reference}.cir']}
    for name, r in rth_values.items():
        rth = dict(testbench_gen.RTH)
        rth[study.junction] = r
        texts[name] = testbench_gen.generate(rth)[f'{study.downstream}.cir']
    return texts


def _ac(study, texts, knobs, cands):
    f, h_up = spice.ac(texts['up'], knobs, study.up_node, study.up_input)
    _, h_ref = spice.ac(texts['ref'], knobs, study.out_node, study.up_input)
    err = {}
    for name in cands:
        _, h_dn = spice.ac(texts[name], knobs, study.out_node, study.down_input)
        err[name] = 20 * np.log10(np.abs(h_up * h_dn) / np.abs(h_ref))
    return f, err


def run(study, jobs=None, log=print):
    cands = candidates(study)
    texts = _texts(study, cands)
    sigs = signals.standard_set(BLOCKS[study.levels].levels, include=('SIN', 'SWP-lo', 'TT', 'BURST', 'STEP'))
    out = {'candidates': cands, 'presets': {}}
    with ThreadPoolExecutor(max_workers=jobs or os.cpu_count()) as ex:
        for pid in study.presets:
            knobs = PRESETS[pid].all_knobs()
            first = {s.id: (ex.submit(spice.tran, texts['up'], knobs, s.x, [study.up_node]),
                            ex.submit(spice.tran, texts['ref'], knobs, s.x, [study.out_node, study.up_node]))
                     for s in sigs}
            second = {(s.id, c): ex.submit(lambda s=s, c=c: spice.tran(
                texts[c], knobs, first[s.id][0].result()[study.up_node], [study.out_node, study.up_node]))
                for s in sigs for c in cands}
            rows = []
            for s in sigs:
                ref = first[s.id][1].result()
                for c in cands:
                    p = second[(s.id, c)].result()
                    rows.append(_metrics(study, s, c, ref, p))
            f, ac_err = _ac(study, texts, knobs, cands)
            out['presets'][pid] = {'rows': rows, 'f': f, 'ac_err': ac_err, 'knobs': knobs}
            log(f'  partition {study.junction} / {pid}: {len(sigs)} signals x {len(cands)} candidates')
    return out


def _metrics(study, sig, cand, ref, p):
    # node voltages minus the DC operating point (first sample; every signal starts at 0 V)
    r = {n: ref[n] - ref[n][0] for n in (study.out_node, study.up_node)}
    y = {n: p[n] - p[n][0] for n in (study.out_node, study.up_node)}
    row = {'signal': sig.id, 'kind': sig.kind, 'cand': cand,
           'esr': metrics.esr_db(r[study.out_node], y[study.out_node]),
           'esr_junction_node': metrics.esr_db(r[study.up_node], y[study.up_node]),
           'harm_2_5': None, 'h1_db': None}
    if sig.kind == 'SIN':
        f0 = sig.meta['freq']
        h_ref, h = metrics.harmonics(r[study.out_node], f0), metrics.harmonics(y[study.out_node], f0)
        diff, scored = metrics.harmonic_diff_db(h_ref, h)
        row['h1_db'] = float(diff[0])
        sel = scored.copy()
        sel[0] = False
        sel[5:] = False
        row['harm_2_5'] = float(np.max(np.abs(diff[sel]))) if sel.any() else None
    return row


def _fmt(v, unit=''):
    return '—' if v is None else f'{v:.3g}{unit}'


def plot(study, res, path):
    pids = list(res['presets'])
    fig, axes = plt.subplots(1, len(pids), figsize=(4.5 * len(pids), 3.6), sharey=True)
    for ax, pid in zip(np.atleast_1d(axes), pids):
        d = res['presets'][pid]
        for c, e in d['ac_err'].items():
            ax.semilogx(d['f'], e, label=f'{c} ({res["candidates"][c] / 1e3:.4g} k)')
        ax.set_title(f'{pid} (vol {d["knobs"]["preampVolume"]:g})')
        ax.set_xlabel('frequency [Hz]')
        ax.grid(True, which='both', alpha=0.3)
    np.atleast_1d(axes)[0].set_ylabel(f'|H_P / H_ref| at {study.out_node} [dB]')
    np.atleast_1d(axes)[0].legend(fontsize=8)
    fig.suptitle(f'{study.junction.upper()} partition error, small-signal magnitude')
    fig.tight_layout()
    fig.savefig(path, dpi=110)
    plt.close(fig)


def write_report(study, res):
    out = config.REPORT_DIR / 'M0'
    out.mkdir(parents=True, exist_ok=True)
    stem = f'{datetime.date.today().strftime("%Y%m%d")}_partition_{study.junction}'
    plot(study, res, out / f'{stem}.png')
    cands = res['candidates']
    lines = [
        f'# M0 partition-error study: {study.junction.upper()}', '',
        f'- Date: {datetime.date.today().isoformat()}, ngspice {spice.ngspice_version()}, '
        f'reltol {config.SPICE_RELTOL}',
        f'- P chain: {study.upstream} (output {study.up_node} open) -> {study.downstream} (J-Thevenin source '
        f'driven by the recorded open-circuit {study.up_node}, through R_th). Reference: {study.reference} '
        f'({study.upstream} and {study.downstream} joined, no junction). Same constant supply in both.',
        f'- Compared node: {study.out_node}; also the junction node {study.up_node} (loaded) as information. '
        'All time-domain metrics on node voltage minus its DC operating point.',
        '- Candidates: ' + ', '.join(f'{c} = {v:.6g} Ω' for c, v in cands.items()),
        '- Tolerance context (not scored here): partition error at y (M3) ≤ −26 dB sines, ≤ −20 dB DI '
        '(VERIFICATION §7).', '',
        '## Summary (worst case over the signal set)', '',
        '| Preset | Candidate | Worst ESR | Worst ESR (SIN only) | Worst h1 diff | Worst harm. 2–5 diff | '
        'Small-signal max dB error 20 Hz–10 kHz | 10–20 kHz |', '|---|---|---|---|---|---|---|---|']
    for pid, d in res['presets'].items():
        for c in cands:
            rows = [r for r in d['rows'] if r['cand'] == c]
            sin = [r for r in rows if r['kind'] == 'SIN']
            h2 = [r['harm_2_5'] for r in sin if r['harm_2_5'] is not None]
            band = [float(np.max(np.abs(d['ac_err'][c][(d['f'] >= lo) & (d['f'] <= hi)]))) for lo, hi in BANDS]
            lines.append(f'| {pid} | {c} | {max(r["esr"] for r in rows):.1f} dB | '
                         f'{max(r["esr"] for r in sin):.1f} dB | {max(abs(r["h1_db"]) for r in sin):.3f} dB | '
                         f'{_fmt(max(h2) if h2 else None, " dB")} | {band[0]:.3f} dB | {band[1]:.3f} dB |')
    lines += ['', f'![small-signal magnitude error]({stem}.png)', '', '## Per signal (ESR at '
              f'{study.out_node} / at {study.up_node}, dB)', '',
              '| Preset | Signal | ' + ' | '.join(cands) + ' |', '|---|---|' + '---|' * len(cands)]
    for pid, d in res['presets'].items():
        by = {}
        for r in d['rows']:
            by.setdefault(r['signal'], {})[r['cand']] = r
        for sid, rc in by.items():
            lines.append(f'| {pid} | {sid} | ' + ' | '.join(
                f'{rc[c]["esr"]:.1f} / {rc[c]["esr_junction_node"]:.1f}' for c in cands) + ' |')
    path = out / f'{stem}.md'
    path.write_text('\n'.join(lines) + '\n')
    return path


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--junction', default='j2', choices=sorted(STUDIES))
    ap.add_argument('--jobs', type=int)
    a = ap.parse_args()
    study = STUDIES[a.junction]
    res = run(study, a.jobs)
    path = write_report(study, res)
    print(f'partition: wrote {path.relative_to(config.ROOT)}')


if __name__ == '__main__':
    main()
