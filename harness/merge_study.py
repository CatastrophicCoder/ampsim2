"""M3 J2 study (PROGRESS.md Q18): would merging B12 and B34 into one block (no J2 junction) meet the
partition target at the loaded master wiper y?

For each Preamp Volume preset and the sines of the standard set at the B12 levels:
- F: the full netlist (cached renders of harness/cascade.py),
- P56: the current partition, SPICE chain b12 -> b34 (J2) -> b56 through the J4 source,
- M56: the merged partition, B12 + B3 + B4 joined in one testbench (no J2, no J3; y open) -> b56 through
  the J4 source.
Both chains run on F's recorded supplies bpA / bpX. Compared with F at the loaded y and the speaker output.
Writes harness/reports/M3/<YYYYMMDD>_merge_study.md.

    python -m harness.merge_study [--jobs N]
"""
import argparse
import datetime
import os
from concurrent.futures import ThreadPoolExecutor

from . import cascade, config, metrics, signals, spice
from .blocks import BLOCKS
from .compare import _git
from .presets import PRESETS
from .render_spice import block_input
from .tolerances import TOLERANCES


def merged_text():
    """b123 (B12 + B3 joined) plus the B4 elements, y open, supplies as recorded sources."""
    b123 = (config.TESTBENCH_DIR / 'b123.cir').read_text()
    b4 = BLOCKS['b4'].path.read_text().split('*** B4 (from the reference netlist)')[1].split('*** J3')[0]
    text = b123.replace('*** supply (inserted', '*** B4 (from the reference netlist; joined, no J3)\n' +
                        b4.strip() + '\n\n*** supply (inserted', 1)
    text = text.replace('Vsup_bpA bpA 0 DC', 'Vrec_bpa bpA 0 DC').replace('Vsup_bpX bpX 0 DC', 'Vrec_bpx bpX 0 DC')
    assert 'RVR2b' in text and 'Vrec_bpa' in text and 'Vrec_bpx' in text
    return text


def one(texts, merged, knobs, sig):
    f = cascade.render_f(texts, knobs, sig)
    p = cascade.render_p(texts, knobs, sig, f)
    p56, _ = cascade.render_p56(texts, knobs, p)
    m = spice.tran(merged, knobs, block_input(BLOCKS['b12'], sig), ['y'], recorded={'bpa': f['bpa'], 'bpx': f['bpx']})
    m56, note = cascade.render_p56(texts, knobs, m)
    a, e = metrics.ac_part, metrics.esr_db
    return {'id': sig.id, 'p56_y': e(a(f['y']), a(p56['y'])), 'p56_out': e(a(f['out']), a(p56['out'])),
            'm56_y': e(a(f['y']), a(m56['y'])), 'm56_out': e(a(f['out']), a(m56['out'])),
            'j2_y': e(a(m56['y']), a(p56['y'])), 'note': note}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--jobs', type=int)
    a = ap.parse_args()
    texts = cascade._texts()
    merged = merged_text()
    sigs = [s for s in cascade.signal_set() if s.kind == 'SIN']
    limit = TOLERANCES['partition_esr_db_sines'][0]
    res = {}
    with ThreadPoolExecutor(max_workers=a.jobs or os.cpu_count()) as ex:
        futures = {(pid, s.id): ex.submit(one, texts, merged, PRESETS[pid].all_knobs(), s)
                   for pid in cascade.PRESET_IDS for s in sigs}
        for pid in cascade.PRESET_IDS:
            res[pid] = [futures[(pid, s.id)].result() for s in sigs]
            for r in res[pid]:
                print(f'  merge_study {pid}/{r["id"]}: loaded y P56 {r["p56_y"]:.1f} dB, merged {r["m56_y"]:.1f} dB; '
                      f'out {r["p56_out"]:.1f} / {r["m56_out"]:.1f} dB')
    lines = ['# M3 study: merging B12 and B34 (J2 removed), partition at the loaded y', '',
             f'- Date: {datetime.date.today().isoformat()}, commit `{_git()}`, ngspice {spice.ngspice_version()}, '
             f'fs = {config.FS} Hz, SPICE reltol {config.SPICE_RELTOL}',
             '- P56: b12 → b34 (J2) → b56 (J4 source); M56: B12 + B3 + B4 joined (no J2 / J3) → b56 (J4 source). Both '
             "on F's recorded bpA / bpX; ESR vs F at the loaded y and the speaker output (out). Sines of the standard "
             'set at the B12 levels (PROGRESS.md Q18).', f'- Partition target: ≤ {limit:g} dB on sines at y (D-036).', '',
             '## Summary (worst over the sines, dB)', '',
             '| Preset | y P56 (current) | y M56 (merged) | out P56 | out M56 | sines failing (current → merged) |',
             '|---|---|---|---|---|---|']
    for pid, rows in res.items():
        nf_p = sum(r['p56_y'] > limit for r in rows)
        nf_m = sum(r['m56_y'] > limit for r in rows)
        lines.append(f'| {pid} | {max(r["p56_y"] for r in rows):.1f} | {max(r["m56_y"] for r in rows):.1f} | '
                     f'{max(r["p56_out"] for r in rows):.1f} | {max(r["m56_out"] for r in rows):.1f} | {nf_p} → {nf_m} |')
    lines += ['', '## Per sine (ESR in dB)', '',
              '| Preset | Signal | y P56 | y M56 | out P56 | out M56 | M56 vs P56 at y (J2 effect) |', '|---|---|---|---|---|---|---|']
    for pid, rows in res.items():
        for r in rows:
            lines.append(f'| {pid} | {r["id"]} | {r["p56_y"]:.1f} | {r["m56_y"]:.1f} | {r["p56_out"]:.1f} | '
                         f'{r["m56_out"]:.1f} | {r["j2_y"]:.1f} |')
    notes = [f'- {pid} {r["id"]}: {r["note"]} (merged chain)' for pid, rows in res.items() for r in rows if r['note']]
    if notes:
        lines += ['', '## Notes', ''] + notes
    out = config.REPORT_DIR / 'M3'
    path = out / f'{datetime.date.today().strftime("%Y%m%d")}_merge_study.md'
    path.write_text('\n'.join(lines) + '\n')
    print(f'merge_study: wrote {path.relative_to(config.ROOT)}')


if __name__ == '__main__':
    main()
