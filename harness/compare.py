"""Compares the engine with the SPICE reference for a block and preset and writes the report
(VERIFICATION.md §6, §8) to harness/reports/M<n>/<YYYYMMDD>_<block>_<preset>.md plus PNGs.
Renders SPICE (cached) and runs the engine first, so this is the one command to run.

    python -m harness.compare --block full --preset CRUNCH --milestone M0
"""
import argparse
import datetime
import hashlib
import subprocess

import matplotlib

matplotlib.use('Agg')
import matplotlib.pyplot as plt  # noqa: E402
import numpy as np  # noqa: E402

from . import config, metrics, run_engine, spice  # noqa: E402
from .blocks import BLOCKS  # noqa: E402
from .presets import PRESETS  # noqa: E402
from .render_spice import render, select_signals, write_renders  # noqa: E402
from .tolerances import status  # noqa: E402


def _sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()[:16]


def _git():
    try:
        rev = subprocess.run(['git', 'rev-parse', '--short', 'HEAD'], cwd=config.ROOT, capture_output=True,
                             text=True, check=True).stdout.strip()
        dirty = subprocess.run(['git', 'status', '--porcelain', '--', 'engine', 'tools', 'harness', 'reference'],
                               cwd=config.ROOT, capture_output=True, text=True).stdout.strip()
        return rev + (' (dirty)' if dirty else '')
    except (OSError, subprocess.CalledProcessError):
        return 'unknown'


def _fmt(v, unit=''):
    if v is None:
        return '—'
    if isinstance(v, (int, np.integer)):
        return f'{v}{unit}'
    return f'{v:.4g}{unit}'


class Report:
    def __init__(self, milestone, block):
        self.milestone, self.block = milestone, block
        self.rows = []      # (check, item, measured, tolerance, status)

    def add(self, check, item, value, unit=''):
        tol, st = status(self.milestone, self.block.id, check, value)
        self.rows.append((check, item, _fmt(value, unit), tol, st))


def analyse(block, signals, spice_res, engine_res, report):
    per_signal = []
    for sig in signals:
        ref = spice_res['tran'][sig.id][block.output_node]
        y = engine_res[sig.id]['out']
        n = min(len(ref), len(y))
        ref, y = ref[:n], y[:n]
        ref_ac, y_ac = metrics.ac_part(ref), metrics.ac_part(y)   # each minus its own DC op (D-018)
        row = {'id': sig.id, 'kind': sig.kind, 'peak_ref': float(np.max(np.abs(ref_ac)))}
        row['esr'] = metrics.esr_db(ref_ac, y_ac)
        row['lag'], row['lag_esr'] = metrics.best_lag(ref_ac, y_ac)
        esr_check = 'total_esr_db' if block.id == 'full' else 'linear_esr_db' if block.linear else 'nonlinear_esr_db'
        if esr_check == 'nonlinear_esr_db' and sig.kind == 'SIN' and sig.meta['amp'] == block.levels.get('overdrive'):
            esr_check = 'nonlinear_esr_db_overdrive'
        report.add(esr_check, sig.id, row['esr'], ' dB')
        if sig.kind == 'SIN':
            f0 = sig.meta['freq']
            h_ref, h = metrics.harmonics(ref, f0), metrics.harmonics(y, f0)
            diff, scored = metrics.harmonic_diff_db(h_ref, h)
            sel = scored.copy()
            sel[0] = False
            sel[5:] = False       # harmonics 2..5
            row['h_ref'], row['h'] = h_ref, h
            row['harm_2_5'] = float(np.max(np.abs(diff[sel]))) if sel.any() else None
            row['thd_ref'], row['thd'] = metrics.thd_percent(h_ref), metrics.thd_percent(h)
            report.add('harmonics_2_5_db', sig.id, row['harm_2_5'], ' dB')
            report.add('total_thd_pp' if block.id == 'full' else 'thd_pp', sig.id, abs(row['thd'] - row['thd_ref']), ' pp')
            if block.output_load:
                row['p_ref'] = metrics.mean_power(ref, block.output_load)
                row['p'] = metrics.mean_power(y, block.output_load)
                rel = abs(row['p'] / row['p_ref'] - 1) if row['p_ref'] > 0 else None
                report.add('total_power_rel', sig.id, rel)
        if sig.kind == 'SWP-lo' and spice_res['ac'] is not None:
            f, h_ac = spice_res['ac']
            ac_db = 20 * np.log10(np.abs(h_ac))
            x = sig.x[:n]
            e_db = metrics.sweep_response_db(x, y_ac, f)
            s_db = metrics.sweep_response_db(x, ref_ac, f)
            lo, hi = (f >= 20) & (f <= 10e3), (f > 10e3) & (f <= 20e3)
            row['mag'] = {'f': f, 'ac': ac_db, 'engine': e_db, 'spice_tran': s_db}
            report.add('linear_mag_db_20_10k', sig.id, float(np.max(np.abs(e_db - ac_db)[lo])), ' dB')
            report.add('linear_mag_db_10k_20k', sig.id, float(np.max(np.abs(e_db - ac_db)[hi])), ' dB')
            report.add('method_mag_db_spice_tran_vs_ac', sig.id, float(np.max(np.abs(s_db - ac_db)[lo | hi])), ' dB')
        stats = engine_res[sig.id]['stats']
        row['ns_per_sample'] = stats['ns_per_sample']
        row['newton_failures'] = stats['newton_failures']
        row['max_iter'] = stats['max_iterations_used']
        report.add('newton_failures', sig.id, int(stats['newton_failures']))
        if stats.get('newton_solves'):
            row['median_iter'] = stats['median_iterations']
            report.add('newton_median_iterations', sig.id, int(stats['median_iterations']))
            report.add('newton_max_iterations', sig.id, int(stats['max_iterations_used']))
        per_signal.append(row)
    return per_signal


def dc_table(block, spice_res, engine_res, report):
    rows = []
    first = next(iter(engine_res.values()))
    for node, v_ref in spice_res['op'].items():
        trace = first['nodes'].get(node)
        v_e = float(trace[0]) if trace is not None else None
        rel = abs(v_e - v_ref) / max(abs(v_ref), 1e-3) if v_e is not None else None
        rows.append((node, v_ref, v_e, rel))
        if v_e is not None:
            report.add('dc_rel_error', f'v({node})', rel)
    return rows


def plots(block, signals, spice_res, engine_res, per_signal, outdir, stem):
    figs = []
    by_id = {s.id: s for s in signals}
    cand = [r for r in per_signal if r['kind'] in ('SIN', 'DI')] or per_signal
    worst = max(cand, key=lambda r: r['peak_ref'])
    ref = spice_res['tran'][worst['id']][block.output_node]
    y = engine_res[worst['id']]['out'][: len(ref)]
    t = np.arange(len(ref)) / config.FS * 1e3
    m = t >= t[-1] - (3e3 / by_id[worst['id']].meta['freq'] if 'freq' in by_id[worst['id']].meta else 20.0)
    fig, ax = plt.subplots(2, 1, figsize=(10, 6), sharex=True)
    ax[0].plot(t[m], ref[m], label='F (SPICE)', lw=1.2)
    ax[0].plot(t[m], y[m], '--', label='E (engine)', lw=1.0)
    ax[0].set_ylabel(f'v({block.output_node}) [V]')
    ax[0].legend(loc='upper right')
    ax[0].set_title(f'{block.id}: {worst["id"]} (end of signal)')
    ax[1].plot(t[m], y[m] - ref[m], color='C3', lw=1.0)
    ax[1].set_ylabel('E − F [V]')
    ax[1].set_xlabel('time [ms]')
    fig.tight_layout()
    figs.append(('Waveform overlay and error (most demanding signal)', _save(fig, outdir, stem, 'waveform')))

    sines = [r for r in per_signal if r['kind'] == 'SIN']
    if sines:
        r = max(sines, key=lambda r: r['peak_ref'])
        k = np.arange(1, 11)
        fig, ax = plt.subplots(figsize=(10, 4))
        ref_dbc = 20 * np.log10(np.maximum(r['h_ref'], 1e-300) / r['h_ref'][0])
        e_dbc = 20 * np.log10(np.maximum(r['h'], 1e-300) / max(r['h_ref'][0], 1e-300))
        ax.bar(k - 0.2, np.maximum(ref_dbc, -140), 0.4, label='F (SPICE)')
        ax.bar(k + 0.2, np.maximum(e_dbc, -140), 0.4, label='E (engine)')
        ax.axhline(-80, color='k', lw=0.8, ls=':')
        ax.set_xticks(k)
        ax.set_xlabel('harmonic')
        ax.set_ylabel('dBc (re SPICE fundamental)')
        ax.set_title(f'Harmonics: {r["id"]}')
        ax.legend()
        fig.tight_layout()
        figs.append(('Harmonics', _save(fig, outdir, stem, 'harmonics')))

    mags = [r for r in per_signal if 'mag' in r]
    if mags:
        mg = mags[0]['mag']
        fig, ax = plt.subplots(figsize=(10, 4))
        ax.semilogx(mg['f'], mg['ac'], label='SPICE AC', lw=1.5)
        ax.semilogx(mg['f'], mg['spice_tran'], ':', label='SPICE transient (SWP-lo)', lw=1.2)
        ax.semilogx(mg['f'], mg['engine'], '--', label='engine (SWP-lo)', lw=1.2)
        ax.set_xlabel('frequency [Hz]')
        ax.set_ylabel(f'|v({block.output_node}) / v({block.input_node})| [dB]')
        ax.grid(True, which='both', alpha=0.3)
        ax.legend()
        fig.tight_layout()
        figs.append(('Magnitude response', _save(fig, outdir, stem, 'magnitude')))

    for r in [r for r in per_signal if r['kind'] == 'DI'][:1]:
        ref = spice_res['tran'][r['id']][block.output_node]
        y = engine_res[r['id']]['out'][: len(ref)]
        fig, ax = plt.subplots(1, 2, figsize=(12, 4), sharey=True)
        for a, sig, name in ((ax[0], ref, 'F (SPICE)'), (ax[1], y, 'E (engine)')):
            a.specgram(sig, NFFT=8192, Fs=config.FS, noverlap=4096, cmap='magma', vmin=-160)
            a.set_ylim(0, 20000)
            a.set_title(f'{name}: {r["id"]}')
            a.set_xlabel('time [s]')
        ax[0].set_ylabel('frequency [Hz]')
        fig.tight_layout()
        figs.append(('Spectrogram (DI)', _save(fig, outdir, stem, 'spectrogram')))
    return figs


def _save(fig, outdir, stem, name):
    outdir.mkdir(parents=True, exist_ok=True)
    path = outdir / f'{name}.png'
    fig.savefig(path, dpi=110)
    plt.close(fig)
    return f'{stem}/{name}.png'


def write_report(milestone, block, preset, signals, report, per_signal, dc_rows, figs, notes):
    date = datetime.date.today().strftime('%Y%m%d')
    stem = f'{date}_{block.id}_{preset.id}'
    outdir = config.REPORT_DIR / milestone
    lines = [
        f'# {milestone} report: {block.id} / {preset.id}', '',
        f'- Block: {block.title}', f'- Preset: {preset.id} — {preset.description}',
        '- Knobs: ' + ', '.join(f'{k}={v:g}' for k, v in preset.all_knobs().items()),
        f'- Date: {datetime.date.today().isoformat()}',
        f'- Engine commit: `{_git()}`',
        f'- Testbench: `harness/testbenches/{block.testbench}` sha256 `{_sha(block.path)}`; netlist sha256 '
        f'`{_sha(config.NETLIST)}`; tubes.lib sha256 `{_sha(config.TUBES)}`',
        f'- ngspice {spice.ngspice_version()}, fs = {config.FS} Hz, reference: F (full netlist)'
        if block.id == 'full' else f'- ngspice {spice.ngspice_version()}, fs = {config.FS} Hz',
        '',
    ]
    lines += [f'> {n}' for n in notes] + ([''] if notes else [])
    counts = {s: sum(1 for r in report.rows if r[4] == s) for s in ('PASS', 'FAIL', 'INFO')}
    lines += ['## Summary', '',
              f'PASS {counts["PASS"]} · FAIL {counts["FAIL"]} · INFO {counts["INFO"]} (INFO = measured, not scored '
              f'in {milestone})', '',
              '| Check | Item | Measured | Tolerance | Status |', '|---|---|---|---|---|']
    lines += [f'| {c} | {i} | {m} | {t} | {s} |' for c, i, m, t, s in report.rows]

    lines += ['', '## Signals', '',
              '| Signal | Peak F [V] | ESR [dB] | Best lag | ESR at lag [dB] | THD F / E [%] | Power F / E [W] | '
              'ns/sample | Newton fail / median / max it |',
              '|---|---|---|---|---|---|---|---|---|']
    for r in per_signal:
        thd = f'{r["thd_ref"]:.3g} / {r["thd"]:.3g}' if 'thd' in r else '—'
        pw = f'{r["p_ref"]:.4g} / {r["p"]:.4g}' if 'p' in r else '—'
        lines.append(f'| {r["id"]} | {r["peak_ref"]:.4g} | {r["esr"]:.2f} | {r["lag"]} | {r["lag_esr"]:.2f} | {thd} | '
                     f'{pw} | {r["ns_per_sample"]:.1f} | {r["newton_failures"]} / {r.get("median_iter", "—")} / {r["max_iter"]:g} |')

    lines += ['', '## DC operating point', '', '| Node | SPICE [V] | Engine [V] | Rel. error |', '|---|---|---|---|']
    lines += [f'| {n} | {v:.4f} | {"—" if e is None else f"{e:.4f}"} | {_fmt(rel)} |' for n, v, e, rel in dc_rows]
    lines += ['', 'Engine values are the probe values after the first sample (zero input) following `reset()`. '
              'Nodes the engine does not probe yet show —.', '']

    lines += ['## Figures', '']
    for title, rel in figs:
        lines += [f'### {title}', '', f'![{title}]({rel})', '']
    path = outdir / f'{stem}.md'
    path.write_text('\n'.join(lines))
    return path


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--block', required=True, choices=sorted(BLOCKS))
    ap.add_argument('--preset', default='DEFAULT', choices=sorted(PRESETS))
    ap.add_argument('--milestone', required=True)
    ap.add_argument('--signal', help='only signals whose id contains this text')
    ap.add_argument('--jobs', type=int)
    ap.add_argument('--note', action='append', default=[], help='text added to the report header')
    a = ap.parse_args()

    block, preset = BLOCKS[a.block], PRESETS[a.preset]
    sigs = select_signals(block, preset, a.signal)
    print(f'compare: {block.id} / {preset.id} / {a.milestone}: {len(sigs)} signals')
    spice_res = render(block, preset, sigs, a.jobs)
    write_renders(block, preset, spice_res)
    engine_res = run_engine.run(block, preset, sigs)

    report = Report(a.milestone, block)
    dc_rows = dc_table(block, spice_res, engine_res, report)
    per_signal = analyse(block, sigs, spice_res, engine_res, report)
    stem = f'{datetime.date.today().strftime("%Y%m%d")}_{block.id}_{preset.id}'
    figs = plots(block, sigs, spice_res, engine_res, per_signal, config.REPORT_DIR / a.milestone / stem, stem)
    path = write_report(a.milestone, block, preset, sigs, report, per_signal, dc_rows, figs, a.note)
    fails = sum(1 for r in report.rows if r[4] == 'FAIL')
    print(f'compare: wrote {path.relative_to(config.ROOT)} ({fails} FAIL)')
    raise SystemExit(1 if fails else 0)


if __name__ == '__main__':
    main()
