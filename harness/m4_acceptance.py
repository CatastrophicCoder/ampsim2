"""M4 acceptance for the full amplifier (SPEC §7 M4, VERIFICATION §4, §7), engine (`ampwav --block full`)
against the full netlist F:
1. DC: every CIRCUIT §5 node within 0.5 %, EL34 plate / screen current per tube ±1 mA / ±0.3 mA, v(ct)
   within 1 %, nbias within 0.2 V.
2. NFB: small-signal gain in → out at 100 Hz / 1 kHz / 5 kHz with R22 and with R22 → 1 TΩ (SPICE: AC on F
   and on a copy with R22 = 1T in harness/cache; engine: 1 mV sines, `--nfb open`), each within 0.5 dB.
3. CLEAN (Preamp 0.2, Master 1.0, 25 mV 1 kHz): power into 16 Ω ±5 %, THD ±0.5 pp.
4. Stability: 60 s of silence at Preamp / Master / Presence 1.0: |out| < 1 mV, no growth (engine only).
5. Overdrive (Preamp 0.8, Master 0.3, 300 mV 220 Hz): ESR ≤ −20 dB, harmonics 2–9 within 1.5 dB, B+ sag
   minimum v(ct) within 2 V.
6. Delayed-NFB break (SPEC §7 M4): not applicable since D-041, R22 closes the loop inside the B56 tree
   without a delay.
7. CRUNCH / DRIVE (VERIFICATION §4): 300 mV 220 Hz and the DI clips, ESR ≤ −20 dB, harmonics 2–9 within
   1.5 dB (sines).
F renders of 7 share harness/cascade.py's node list (its cache), so the DI references serve both.
Writes harness/reports/M4/<YYYYMMDD>_full_acceptance.md.

    python -m harness.m4_acceptance [--only dc,nfb,...] [--jobs N]
"""
import argparse
import datetime
import json
import os
import re
import tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

import numpy as np

from . import cascade, config, metrics, signals, spice
from .blocks import BLOCKS, FULL_DC_NODES
from .compare import _git, _sha
from .presets import PRESETS
from .run_engine import _ampwav
from .tolerances import status
from .wavio import read_wav, write_wav

SECTIONS = ('dc', 'nfb', 'clean', 'stability', 'overdrive', 'crunch_drive')
# run only with --only: HOT (Master, Presence 1.0) as CRUNCH / DRIVE, same tolerances (M6, owner request)
EXTRA_SECTIONS = ('hot',)
NFB_FREQS = (100.0, 1000.0, 5000.0)
NFB_AMP = 0.001
EL34 = {'ip_tube_a': '@b.x4.bp[i]', 'ip_tube_b': '@b.x6.bp[i]', 'is_tube_a': '@b.x4.bs[i]', 'is_tube_b': '@b.x6.bs[i]'}
SINE_NODES = ('out', 'ct', 'y', 'p3a', 'p3b', 'pa', 'pb')
STABILITY_SECONDS = 60.0


def _full_text():
    return BLOCKS['full'].path.read_text()


def open_loop_text():
    """F with R22 = 1 TΩ (the NFB check's open loop; the reference netlist is not changed)."""
    text, n = re.subn(r'^(R22\s+nG\s+out\s+)100k\s*$', r'\g<1>1T', _full_text(), flags=re.M | re.I)
    assert n == 1, 'R22 line changed in the netlist; update harness/m4_acceptance.py'
    return text


def engine(knobs, x, nodes=(), nfb=None, seconds=None):
    """Runs ampwav --block full on x (volts at the jack). Returns {'out', node..., 'stats'}."""
    with tempfile.TemporaryDirectory(prefix='amp2203_m4_') as d:
        d = Path(d)
        write_wav(d / 'in.wav', x)
        args = ['--block', 'full', '--in', str(d / 'in.wav'), '--out', str(d / 'out.wav'), '--rate', str(config.FS),
                '--stats', str(d / 'stats.json')]
        args += [a for k, v in knobs.items() for a in ('--param', f'{k}={v!r}')]
        args += [a for n in nodes for a in ('--dump-node', f'{n}={d / (n + ".wav")}')]
        if nfb:
            args += ['--nfb', nfb]
        if config.POWER_DIVISOR != 1:
            args += ['--power-rate-divisor', str(config.POWER_DIVISOR)]
        if config.NEWTON_TOL:
            args += ['--newton-tol', config.NEWTON_TOL]
        _ampwav(args)
        res = {'out': read_wav(d / 'out.wav')[0], 'stats': json.loads((d / 'stats.json').read_text())}
        for n in nodes:
            res[n] = read_wav(d / f'{n}.wav')[0]
        return res


class Report:
    def __init__(self):
        self.rows = []

    def add(self, section, check, item, value, unit=''):
        tol, st = status('M4', 'full', check, value)
        self.rows.append((section, check, item, value, unit, tol, st))


def _harm_2_9(ref, y, f0):
    h_ref, h = metrics.harmonics(ref, f0), metrics.harmonics(y, f0)
    diff, scored = metrics.harmonic_diff_db(h_ref, h)
    sel = scored.copy()
    sel[0] = False
    sel[9:] = False
    return (float(np.max(np.abs(diff[sel]))) if sel.any() else None), h_ref, h


def _newton(rep, section, item, stats):
    rep.add(section, 'newton_failures', item, int(stats['newton_failures']))
    rep.add(section, 'newton_unrescued', item, int(stats.get('newton_unrescued', 0)))
    if stats.get('newton_solves'):
        rep.add(section, 'newton_median_iterations', item, int(stats['median_iterations']))
        rep.add(section, 'newton_max_iterations', item, int(stats['max_iterations_used']))


def check_dc(rep, notes):
    knobs = PRESETS['DEFAULT'].all_knobs()
    ref = spice.op(_full_text(), knobs, list(FULL_DC_NODES) + list(EL34.values()))
    e = engine(knobs, np.zeros(64), nodes=list(FULL_DC_NODES) + list(EL34))
    rows = []
    for n in FULL_DC_NODES:
        v_e = float(e[n][0])
        rel = abs(v_e - ref[n]) / max(abs(ref[n]), 1e-3)
        rep.add('dc', 'dc_rel_error', f'v({n})', rel)
        rows.append((n, ref[n], v_e))
    for probe_name, vec in EL34.items():
        d_ma = abs(float(e[probe_name][0]) - ref[vec]) * 1e3
        rep.add('dc', 'el34_plate_ma' if probe_name.startswith('ip') else 'el34_screen_ma', probe_name, d_ma, ' mA')
        rows.append((probe_name, ref[vec] * 1e3, float(e[probe_name][0]) * 1e3))
    rep.add('dc', 'ct_rel', 'v(ct)', abs(float(e['ct'][0]) / ref['ct'] - 1))
    rep.add('dc', 'nbias_v', 'v(nbias)', abs(float(e['nbias'][0]) - ref['nbias']), ' V')
    return {'rows': rows}


def check_nfb(rep, notes, jobs):
    knobs = PRESETS['DEFAULT'].all_knobs()
    texts = {'closed': _full_text(), 'open': open_loop_text()}
    out = {}
    for mode, text in texts.items():
        for f in NFB_FREQS:
            _, h = spice.ac(text, knobs, 'out', 'in', fstart=f, fstop=f * 1.0001, ppd=1)
            sig = signals.sine(f, NFB_AMP)
            e = engine(knobs, sig.x, nfb=mode)
            h1 = metrics.harmonics(metrics.ac_part(e['out']), f)[0] / NFB_AMP
            s_db, e_db = 20 * np.log10(abs(h[0])), 20 * np.log10(h1)
            out[(mode, f)] = (s_db, e_db)
            rep.add('nfb', 'nfb_gain_db', f'{mode} {f:g} Hz', abs(e_db - s_db), ' dB')
            _newton(rep, 'nfb', f'{mode} {f:g} Hz', e['stats'])
    return out


def _sine_compare(rep, section, pid, sig, f_res, e_res, sag=False):
    ref, y = metrics.ac_part(f_res['out']), metrics.ac_part(e_res['out'])
    n = min(len(ref), len(y))
    ref, y = ref[:n], y[:n]
    row = {'preset': pid, 'signal': sig.id, 'esr': metrics.esr_db(ref, y)}
    row['lag'], _ = metrics.best_lag(ref, y)
    rep.add(section, 'total_esr_db', f'{pid} {sig.id}', row['esr'], ' dB')
    if sig.kind == 'SIN':
        f0 = sig.meta['freq']
        row['harm'], h_ref, h = _harm_2_9(f_res['out'][:n], e_res['out'][:n], f0)
        rep.add(section, 'total_harmonics_2_9_db', f'{pid} {sig.id}', row['harm'], ' dB')
        row['thd'] = (metrics.thd_percent(h_ref), metrics.thd_percent(h))
        row['power'] = (metrics.mean_power(f_res['out'], 16.0), metrics.mean_power(e_res['out'], 16.0))
    if sag:
        row['sag'] = (float(np.min(f_res['ct'])), float(np.min(e_res['ct'])))
        rep.add(section, 'sag_min_v', f'{pid} {sig.id}', abs(row['sag'][1] - row['sag'][0]), ' V')
    _newton(rep, section, f'{pid} {sig.id}', e_res['stats'])
    return row


def check_clean(rep, notes):
    pid = 'CLEAN'
    knobs = PRESETS[pid].all_knobs()
    sig = PRESETS[pid].signals(BLOCKS['full'])[0]
    f_res = spice.tran(_full_text(), knobs, sig.x, list(SINE_NODES))
    e_res = engine(knobs, sig.x, nodes=['ct'])
    row = _sine_compare(rep, 'clean', pid, sig, f_res, e_res)
    rep.add('clean', 'total_power_rel', f'{pid} {sig.id}', abs(row['power'][1] / row['power'][0] - 1))
    rep.add('clean', 'total_thd_pp', f'{pid} {sig.id}', abs(row['thd'][1] - row['thd'][0]), ' pp')
    return [row]


def check_stability(rep, notes):
    knobs = {**config.DEFAULT_KNOBS, 'preampVolume': 1.0, 'master': 1.0, 'presence': 1.0}
    e = engine(knobs, np.zeros(int(STABILITY_SECONDS * config.FS)))
    y = e['out']
    w = int(10.0 * config.FS)
    first, last = float(np.sqrt(np.mean(y[:w] ** 2))), float(np.sqrt(np.mean(y[-w:] ** 2)))
    peak = float(np.max(np.abs(y)))
    rep.add('stability', 'stability_out_v', 'max |out|', peak, ' V')
    rep.add('stability', 'stability_growth', 'RMS last 10 s − first 10 s', last - first, ' V')
    _newton(rep, 'stability', '60 s silence', e['stats'])
    return {'peak': peak, 'first': first, 'last': last}


def check_overdrive(rep, notes):
    pid = 'OVERDRIVE'
    knobs = PRESETS[pid].all_knobs()
    sig = PRESETS[pid].signals(BLOCKS['full'])[0]
    f_res = spice.tran(_full_text(), knobs, sig.x, list(SINE_NODES))
    e_res = engine(knobs, sig.x, nodes=['ct'])
    return [_sine_compare(rep, 'overdrive', pid, sig, f_res, e_res, sag=True)]


def check_crunch_drive(rep, notes, jobs, pids=('CRUNCH', 'DRIVE'), section='crunch_drive'):
    texts = cascade._texts()
    work = []
    for pid in pids:
        for sig in PRESETS[pid].signals(BLOCKS['full']):
            work.append((pid, sig))

    def one(item):
        pid, sig = item
        knobs = PRESETS[pid].all_knobs()
        return pid, sig, cascade.render_f(texts, knobs, sig), engine(knobs, sig.x)

    rows = []
    with ThreadPoolExecutor(max_workers=jobs or os.cpu_count()) as ex:
        for pid, sig, f_res, e_res in ex.map(one, work):
            rows.append(_sine_compare(rep, section, pid, sig, f_res, e_res))
    return rows


def write_report(rep, res, notes, only=None):
    # rate studies (AMP2203_FS, M5) go to the M5 folder with the rate in the name; power-amp rate studies
    # (AMP2203_POWER_DIVISOR, M6) to M6 with the divisor too
    m6 = config.POWER_DIVISOR != 1 or config.NEWTON_TOL or (only and 'hot' in only)
    out = config.REPORT_DIR / ('M6' if m6 else 'M4' if config.FS == config.DEFAULT_FS else 'M5')
    out.mkdir(parents=True, exist_ok=True)
    stem = f'{datetime.date.today().strftime("%Y%m%d")}_full_acceptance'
    if config.FS != config.DEFAULT_FS or m6:
        stem += f'_fs{config.FS}'
    if config.POWER_DIVISOR != 1:
        stem += f'_pdiv{config.POWER_DIVISOR}'
    if config.NEWTON_TOL:
        stem += f'_tol{config.NEWTON_TOL}'
    if only:  # a subset of sections gets its own file
        stem += '_' + '-'.join(only)
    npass = sum(r[6] == 'PASS' for r in rep.rows)
    nfail = sum(r[6] == 'FAIL' for r in rep.rows)
    lines = [f'# M4 acceptance: full amplifier vs the full netlist', '',
             f'- Date: {datetime.date.today().isoformat()}, engine commit `{_git()}`, ngspice {spice.ngspice_version()}, '
             f'fs = {config.FS} Hz' + (f' (power amp at fs / {config.POWER_DIVISOR})' if config.POWER_DIVISOR != 1 else '') +
             (f', Newton residual tolerance {config.NEWTON_TOL} V' if config.NEWTON_TOL else '') +
             f', SPICE reltol {config.SPICE_RELTOL}',
             f'- Netlist sha256 `{_sha(config.NETLIST)}`, tubes.lib sha256 `{_sha(config.TUBES)}`',
             '- Engine: `ampwav --block full` (volts at the jack and across the 16 Ω load), the checks of SPEC §7 M4.',
             ''] + [f'> {n}' for n in notes] + ([''] if notes else []) + [
             '## Summary', '', f'PASS {npass} · FAIL {nfail}', '',
             '| Section | Check | Item | Measured | Tolerance | Status |', '|---|---|---|---|---|---|']
    for sec, check, item, value, unit, tol, st in rep.rows:
        v = '—' if value is None else (f'{value:.4g}{unit}' if isinstance(value, float) else f'{value}{unit}')
        lines.append(f'| {sec} | {check} | {item} | {v} | {tol} | {st} |')
    if 'dc' in res:
        lines += ['', '## DC operating point', '', '| Node / current | SPICE | Engine |', '|---|---|---|']
        for n, a, b in res['dc']['rows']:
            unit = ' mA' if n.startswith(('ip_', 'is_')) else ' V'
            lines.append(f'| {n} | {a:.4f}{unit} | {b:.4f}{unit} |')
    if 'nfb' in res:
        lines += ['', '## NFB (gain in → out, dB)', '', '| f | SPICE closed | engine closed | SPICE open | engine open | '
                  'NFB SPICE | NFB engine |', '|---|---|---|---|---|---|---|']
        for f in NFB_FREQS:
            sc, ec = res['nfb'][('closed', f)]
            so, eo = res['nfb'][('open', f)]
            lines.append(f'| {f:g} Hz | {sc:.2f} | {ec:.2f} | {so:.2f} | {eo:.2f} | {so - sc:.2f} | {eo - ec:.2f} |')
    sine_rows = [r for k in ('clean', 'overdrive', 'crunch_drive', 'hot') for r in res.get(k, [])]
    if sine_rows:
        lines += ['', '## Signals', '', '| Preset | Signal | ESR | best lag | harm. 2–9 | THD F / E | Power F / E | '
                  'min v(ct) F / E |', '|---|---|---|---|---|---|---|---|']
        for r in sine_rows:
            th = f'{r["thd"][0]:.2f} / {r["thd"][1]:.2f} %' if 'thd' in r else '—'
            pw = f'{r["power"][0]:.1f} / {r["power"][1]:.1f} W' if 'power' in r else '—'
            sg = f'{r["sag"][0]:.2f} / {r["sag"][1]:.2f} V' if 'sag' in r else '—'
            hm = '—' if r.get('harm') is None else f'{r["harm"]:.2f} dB'
            lines.append(f'| {r["preset"]} | {r["signal"]} | {r["esr"]:.1f} dB | {r["lag"]} | {hm} | {th} | {pw} | {sg} |')
    if 'stability' in res:
        s = res['stability']
        lines += ['', '## Stability', '', f'60 s silence at Preamp / Master / Presence 1.0: max |out| {s["peak"]:.3g} V, '
                  f'RMS first / last 10 s {s["first"]:.3g} / {s["last"]:.3g} V.']
    lines += ['', '## Delayed-NFB break', '', 'Not applicable: since D-041 the global NFB (R22) is part of the B56 tree and '
              'has no delay (SPEC §7 M4 asked for a 1- vs 2-sample comparison of the delayed design).']
    path = out / f'{stem}.md'
    path.write_text('\n'.join(lines) + '\n')
    return path


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--only', help='comma-separated sections: ' + ', '.join(SECTIONS + EXTRA_SECTIONS))
    ap.add_argument('--jobs', type=int)
    a = ap.parse_args()
    sections = a.only.split(',') if a.only else list(SECTIONS)
    rep, res, notes = Report(), {}, []
    for sec in sections:
        print(f'm4_acceptance: {sec}')
        if sec == 'dc':
            res[sec] = check_dc(rep, notes)
        elif sec == 'nfb':
            res[sec] = check_nfb(rep, notes, a.jobs)
        elif sec == 'clean':
            res[sec] = check_clean(rep, notes)
        elif sec == 'stability':
            res[sec] = check_stability(rep, notes)
        elif sec == 'overdrive':
            res[sec] = check_overdrive(rep, notes)
        elif sec == 'crunch_drive':
            res[sec] = check_crunch_drive(rep, notes, a.jobs)
        elif sec == 'hot':
            res[sec] = check_crunch_drive(rep, notes, a.jobs, ('HOT',), 'hot')
        else:
            ap.error(f'unknown section {sec}')
    path = write_report(rep, res, notes, sections if a.only else None)
    nfail = sum(r[6] == 'FAIL' for r in rep.rows)
    print(f'm4_acceptance: wrote {path.relative_to(config.ROOT)} ({nfail} FAIL)')


if __name__ == '__main__':
    main()
