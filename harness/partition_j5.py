"""M4 step 1: SPICE partition error of J5 (PI plates -> power stage), closed loop (VERIFICATION §1,
SPEC §4.1). Reference: b56 (B5 + B6 + B7 joined, instantaneous NFB). Partitioned: b56j5, the same
netlist with C14 / C16 fed by J5 Thevenin sources (VCVS copies of the unloaded plates p3a / p3b behind
R_thA / R_thB from harness/junctions.py) instead of the plates, NFB still closed. The difference is the J5
junction alone (both have the J4 source and the constant preamp loads). B56 standard signal set
(SIN, SWP-lo, TT, BURST, STEP) at the B56 levels; Presence 0 / 0.5 / 1. Writes
harness/reports/M4/<YYYYMMDD>_partition_j5.md.

Variants of the J5 source (--variants; the report has one column group per variant):
- `self`: R_thA / R_thB = |Z_aa|, |Z_bb| at 1 kHz (harness/junctions.py, D-018), the provisional J5;
- `diff`: R_th = Z_self − Z_mutual at 1 kHz (the push-pull source resistance: with i_b = −i_a,
  v_a = v_oc,a − (Z_aa − Z_ab)·i_a);
- `2port`: the full 2×2 Thevenin source v = v_oc − Z·i with the measured 1 kHz matrix (real parts; the
  cross terms as CCVS sensing the other side's current).
Z is measured on b5open with harness/rth_measure.py (CIRCUIT §4).

    python -m harness.partition_j5 [--jobs N] [--variants self,diff,2port]
"""
import argparse
import datetime
import functools
import os
import re
from concurrent.futures import ThreadPoolExecutor

import matplotlib

matplotlib.use('Agg')
import matplotlib.pyplot as plt  # noqa: E402
import numpy as np  # noqa: E402

from . import config, metrics, recordings, rth_measure, signals, spice  # noqa: E402
from .blocks import BLOCKS  # noqa: E402
from .compare import _git  # noqa: E402
from .junctions import RTH  # noqa: E402
from .presets import PRESETS  # noqa: E402
from .render_spice import block_input  # noqa: E402

PRESET_IDS = ('PRES0', 'DEFAULT', 'PRES1')
NODES = recordings.B56_NODES
BANDS = ((20.0, 10000.0), (10000.0, 20000.0))
KINDS = ('SIN', 'SWP-lo', 'TT', 'BURST', 'STEP')


VARIANTS = ('self', 'diff', '2port')


@functools.lru_cache(maxsize=1)
def z_matrix():
    """PI output impedance matrix at 1 kHz, real parts: {'aa', 'ab', 'ba', 'bb'} (v_row per i_col)."""
    r = rth_measure.measure()
    return {'aa': r['j5a']['ac'].real, 'ba': r['j5a']['mutual_ac']['p3b'].real,
            'bb': r['j5b']['ac'].real, 'ab': r['j5b']['mutual_ac']['p3a'].real}


def variant_text(name):
    """The b56j5 testbench with the J5 source of the given variant."""
    text = (config.TESTBENCH_DIR / 'b56j5.cir').read_text()
    if name == 'self':
        return text
    z = z_matrix()

    def rth(t, a, b):
        t = re.sub(r'^\.param rth_j5a=\S+', f'.param rth_j5a={a!r}', t, flags=re.M)
        return re.sub(r'^\.param rth_j5b=\S+', f'.param rth_j5b={b!r}', t, flags=re.M)
    if name == 'diff':
        return rth(text, z['aa'] - z['ab'], z['bb'] - z['ba'])
    if name == '2port':
        t = rth(text, z['aa'], z['bb'])
        t = t.replace('Rth_j5a j5a p3a6 {rth_j5a}', 'Hj5ab j5a j5a2 Vsense_j5b {z_ab}\nRth_j5a j5a2 p3a7 {rth_j5a}\n'
                      'Vsense_j5a p3a7 p3a6 0')
        t = t.replace('Rth_j5b j5b p3b6 {rth_j5b}', 'Hj5ba j5b j5b2 Vsense_j5a {z_ba}\nRth_j5b j5b2 p3b7 {rth_j5b}\n'
                      'Vsense_j5b p3b7 p3b6 0')
        t = t.replace('.param rth_j5a=', f'.param z_ab={z["ab"]!r} z_ba={z["ba"]!r}\n.param rth_j5a=', 1)
        assert 'Vsense_j5a p3a7' in t and 'Vsense_j5b p3b7' in t
        return t
    raise KeyError(name)


def _render(text, knobs, sig):
    """Returns (result, note); repeats once at RELTOL 1e-7 if ngspice stops with 'timestep too small'."""
    x = block_input(BLOCKS['b56'], sig)
    try:
        return spice.tran(text, knobs, x, list(NODES)), None
    except spice.SpiceError as e:
        if 'Timestep too small' not in str(e):
            raise
        return spice.tran(text, knobs, x, list(NODES), reltol=1e-7), 'rendered at RELTOL 1e-7 (timestep too small)'


def _row(sig, ref, p):
    a = metrics.ac_part
    row = {'signal': sig.id, 'kind': sig.kind, 'esr': {n: metrics.esr_db(a(ref[n]), a(p[n])) for n in NODES if n != 'y'},
           'harm': None, 'h1': None}
    if sig.kind == 'SIN':
        f0 = sig.meta['freq']
        diff, scored = metrics.harmonic_diff_db(metrics.harmonics(a(ref['out']), f0), metrics.harmonics(a(p['out']), f0))
        row['h1'] = float(diff[0])
        sel = scored.copy()
        sel[0] = False
        sel[9:] = False
        row['harm'] = float(np.max(np.abs(diff[sel]))) if sel.any() else None
    return row


def run(variants=('self',), jobs=None, log=print):
    texts = {'b56': BLOCKS['b56'].path.read_text()} | {v: variant_text(v) for v in variants}
    sigs = signals.standard_set(BLOCKS['b56'].levels, include=KINDS)
    out = {v: {} for v in variants}
    with ThreadPoolExecutor(max_workers=jobs or os.cpu_count()) as ex:
        fut = {(pid, s.id, k): ex.submit(_render, texts[k], PRESETS[pid].all_knobs(), s)
               for pid in PRESET_IDS for s in sigs for k in texts}
        for pid in PRESET_IDS:
            knobs = PRESETS[pid].all_knobs()
            f, h_ref = spice.ac(texts['b56'], knobs, 'out', 'j4s')
            for v in variants:
                rows, notes = [], []
                for s in sigs:
                    (ref, n1), (p, n2) = fut[(pid, s.id, 'b56')].result(), fut[(pid, s.id, v)].result()
                    rows.append(_row(s, ref, p))
                    notes += [f'{s.id} {k}: {n}' for k, n in (('b56', n1), (v, n2)) if n]
                    log(f'  partition j5 {v} {pid}/{s.id}: out {rows[-1]["esr"]["out"]:.1f} dB')
                _, h_p = spice.ac(texts[v], knobs, 'out', 'j4s')
                out[v][pid] = {'rows': rows, 'notes': notes, 'f': f, 'ac_err': 20 * np.log10(np.abs(h_p) / np.abs(h_ref)),
                               'ac_phase': np.degrees(np.angle(h_p / h_ref))}
    return out


def write_report(res):
    out = config.REPORT_DIR / 'M4'
    out.mkdir(parents=True, exist_ok=True)
    stem = f'{datetime.date.today().strftime("%Y%m%d")}_partition_j5'
    variants = list(res)
    fig, ax = plt.subplots(1, 2, figsize=(10, 3.6))
    for v in variants:
        for pid, d in res[v].items():
            ax[0].semilogx(d['f'], d['ac_err'], label=f'{v} {pid}')
            ax[1].semilogx(d['f'], d['ac_phase'], label=f'{v} {pid}')
    ax[0].set_ylabel('|H_P / H_ref| at out [dB]')
    ax[1].set_ylabel('phase difference [deg]')
    for a_ in ax:
        a_.set_xlabel('frequency [Hz]')
        a_.grid(True, which='both', alpha=0.3)
        a_.legend(fontsize=7)
    fig.suptitle('J5 partition error, small-signal (J4 source -> out, closed loop)')
    fig.tight_layout()
    fig.savefig(out / f'{stem}.png', dpi=110)
    plt.close(fig)
    z = z_matrix() if any(v != 'self' for v in variants) else None
    cols = [n for n in NODES if n != 'y']
    lines = [
        '# M4 study: J5 partition error (PI plates -> power stage), closed loop', '',
        f'- Date: {datetime.date.today().isoformat()}, commit `{_git()}`, ngspice {spice.ngspice_version()}, '
        f'fs = {config.FS} Hz, SPICE reltol {config.SPICE_RELTOL}',
        '- Reference: `b56` (B5 + B6 + B7 joined, instantaneous NFB). Partitioned: `b56j5`, C14 / C16 fed by '
        'J5 Thevenin sources (VCVS copies of the unloaded p3a / p3b), NFB closed. Both driven by the J4 source '
        f'at the B56 levels ({", ".join(f"{v:g} V" for v in BLOCKS["b56"].levels.values())}), constant preamp loads.',
        f'- Variants: `self` R_thA / R_thB = {RTH["j5a"]:.6g} / {RTH["j5b"]:.6g} Ω (|Z_aa|, |Z_bb| at 1 kHz, '
        'junctions.py)' + ('' if z is None else
                           f'; `diff` R_th = Z_aa − Z_ab = {z["aa"] - z["ab"]:.6g} Ω, Z_bb − Z_ba = {z["bb"] - z["ba"]:.6g} Ω; '
                           f'`2port` v = v_oc − Z·i with Z = [[{z["aa"]:.6g}, {z["ab"]:.6g}], [{z["ba"]:.6g}, {z["bb"]:.6g}]] Ω '
                           '(real parts at 1 kHz, b5open, rth_measure)') + '.',
        '- ESR on each node minus its DC operating point (D-018). At p3a / p3b the partitioned value is the '
        'unloaded plate (v_oc) against the loaded plate, so those columns include the load itself. '
        'Harmonics 2–9 at out where above −80 dBc.',
        '- Tolerance context (not scored here): M4 total DRIVE / CRUNCH ESR ≤ −20 dB, harmonics 2–9 within '
        '1.5 dB; M3 partition target at y ≤ −26 dB sines (VERIFICATION §7).', '',
        '## Summary (worst over the signal set)', '',
        '| Variant | Preset | out, all | out, SIN | out, SIN ≤ 5 V | ngn | pa | worst harm. 2–9 diff | small-signal max dB 20 Hz–10 kHz | 10–20 kHz |',
        '|---|---|---|---|---|---|---|---|---|---|']
    for v in variants:
        for pid, d in res[v].items():
            rows = d['rows']
            sin = [r for r in rows if r['kind'] == 'SIN']
            low = [r for r in sin if not r['signal'].endswith(f'-{BLOCKS["b56"].levels["overdrive"]:g}V')]
            h = [r['harm'] for r in sin if r['harm'] is not None]
            band = [float(np.max(np.abs(d['ac_err'][(d['f'] >= lo) & (d['f'] <= hi)]))) for lo, hi in BANDS]
            lines.append(f'| {v} | {pid} | {max(r["esr"]["out"] for r in rows):.1f} dB | {max(r["esr"]["out"] for r in sin):.1f} dB | '
                         f'{max(r["esr"]["out"] for r in low):.1f} dB | '
                         f'{max(r["esr"]["ngn"] for r in rows):.1f} dB | {max(r["esr"]["pa"] for r in rows):.1f} dB | '
                         f'{max(h):.2f} dB | {band[0]:.3f} dB | {band[1]:.3f} dB |')
    lines += ['', f'![small-signal error]({stem}.png)', '']
    for v in variants:
        lines += [f'## Per signal, variant `{v}` (ESR, dB)', '',
                  '| Preset | Signal | ' + ' | '.join(cols) + ' | h1 diff | harm. 2–9 |', '|---|---|' + '---|' * (len(cols) + 2)]
        for pid, d in res[v].items():
            for r in d['rows']:
                lines.append(f'| {pid} | {r["signal"]} | ' + ' | '.join(f'{r["esr"][n]:.1f}' for n in cols) + ' | '
                             + ('—' if r['h1'] is None else f'{r["h1"]:.2f}') + ' | '
                             + ('—' if r['harm'] is None else f'{r["harm"]:.2f}') + ' |')
        lines.append('')
    notes = [f'- {v} {pid} {n}' for v in variants for pid, d in res[v].items() for n in d['notes']]
    if notes:
        lines += ['## Notes', ''] + notes
    path = out / f'{stem}.md'
    path.write_text('\n'.join(lines) + '\n')
    return path


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--jobs', type=int)
    ap.add_argument('--variants', default='self', help='comma-separated: ' + ', '.join(VARIANTS))
    a = ap.parse_args()
    variants = tuple(a.variants.split(','))
    if not set(variants) <= set(VARIANTS):
        ap.error(f'unknown variant in {a.variants}')
    path = write_report(run(variants, a.jobs))
    print(f'partition_j5: wrote {path.relative_to(config.ROOT)}')


if __name__ == '__main__':
    main()
