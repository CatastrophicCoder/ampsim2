"""M0 testbench sanity check (VERIFICATION §2): each block testbench's DC operating point at its nodes
must match the full netlist (F) within 0.5 %. Writes harness/reports/M0/<YYYYMMDD>_testbench_dc.md.

    python -m harness.dc_check
"""
import argparse
import datetime

from . import config, netlist, spice
from .testbench_gen import f_operating_point
from .tolerances import TOLERANCES

# relative error denominator floor: nodes at 0 V in F (tone stack, NFB nodes) are judged on 1 mV
ABS_FLOOR = 1e-3
TESTBENCHES = ('b12', 'b3', 'b4', 'b56', 'b5open', 'b6', 'b123')

# Deviations whose cause is known. They keep their PASS/FAIL status; the owner decides on acceptance.
# b6 out/sl: accepted by the owner as a known partition effect (PROGRESS Q5, DECISIONS D-015).
KNOWN = {
    ('b6', 'out'): 'Accepted known partition effect (DECISIONS D-015), not a transcription error: in F, R22 (B5) injects '
                   'v(nG)/100 kΩ ≈ 135 µA into node out, giving 135 µA × (16 Ω ‖ 0.32 Ω) ≈ 42 µV. '
                   'B6 does not contain R22 (SPEC §4.2), so out = 0 V there.',
    ('b6', 'sl'): 'Same cause as out (sl is out through the secondary inductance, DC short).',
}


def check(name):
    text = (config.TESTBENCH_DIR / f'{name}.cir').read_text()
    v_f, _ = f_operating_point()
    nodes = [n for n in netlist.parse(text).nodes() if n in v_f]
    v_tb = spice.op(text, {}, nodes)
    rows = []
    for n in nodes:
        err = abs(v_tb[n] - v_f[n]) / max(abs(v_f[n]), ABS_FLOOR)
        rows.append((n, v_f[n], v_tb[n], err))
    return rows


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--no-report', action='store_true')
    a = ap.parse_args()
    limit = TOLERANCES['tb_dc_rel_error'][0]
    lines = ['# M0 testbench DC sanity check', '',
             f'- Date: {datetime.date.today().isoformat()}, ngspice {spice.ngspice_version()}, default parameters',
             f'- Check: every node of each block testbench vs the full netlist (F) operating point; relative '
             f'error |Δ| / max(|v_F|, {ABS_FLOOR * 1e3:g} mV) ≤ {limit:.1%} (VERIFICATION §2)', '',
             '| Testbench | Nodes | Max rel. error | Worst node | Status |', '|---|---|---|---|---|']
    detail, notes = [], []
    fails = 0
    for name in TESTBENCHES:
        rows = check(name)
        worst = max(rows, key=lambda r: r[3])
        ok = worst[3] <= limit
        fails += not ok
        lines.append(f'| {name} | {len(rows)} | {worst[3]:.2e} | {worst[0]} | {"PASS" if ok else "FAIL"} |')
        detail += ['', f'### {name}', '', '| Node | F [V] | Testbench [V] | Rel. error | Note |',
                   '|---|---|---|---|---|']
        detail += [f'| {n} | {vf:.6g} | {vt:.6g} | {e:.2e} | {KNOWN.get((name, n), "") if e > limit else ""} |'
                   for n, vf, vt, e in rows]
        notes += [f'- **{name} / {n}**: {e:.2e} — {KNOWN.get((name, n), "unexplained")}'
                  for n, vf, vt, e in rows if e > limit]
        print(f'{name:8s} {len(rows):3d} nodes  max rel error {worst[3]:.2e} at {worst[0]:6s} '
              f'{"PASS" if ok else "FAIL"}')
    if notes:
        lines += ['', '## Deviations above tolerance', ''] + notes
    lines += ['', '## Per-node values'] + detail
    if not a.no_report:
        out = config.REPORT_DIR / 'M0'
        out.mkdir(parents=True, exist_ok=True)
        path = out / f'{datetime.date.today().strftime("%Y%m%d")}_testbench_dc.md'
        path.write_text('\n'.join(lines) + '\n')
        print(f'dc_check: wrote {path.relative_to(config.ROOT)}')
    raise SystemExit(1 if fails else 0)


if __name__ == '__main__':
    main()
