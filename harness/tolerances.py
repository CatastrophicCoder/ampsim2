"""Tolerances (VERIFICATION.md §7) and which checks each milestone scores. Changing a tolerance needs
owner approval and a DECISIONS.md entry; this file only transcribes the document.

A check without an entry for the milestone/block is reported as INFO (measured, not scored)."""

# check name -> (limit, 'max' | 'min'); 'max' means measured value must be <= limit
TOLERANCES = {
    'dc_rel_error': (0.005, 'max'),                 # DC operating point, any block (0.5 %)
    'tb_dc_rel_error': (0.005, 'max'),              # M0 testbench sanity vs F (0.5 %)
    'linear_mag_db_20_10k': (0.1, 'max'),
    'linear_mag_db_10k_20k': (0.3, 'max'),
    'linear_esr_db': (-60.0, 'max'),
    'nonlinear_esr_db': (-40.0, 'max'),             # Small / Nominal / Hot
    'nonlinear_esr_db_overdrive': (-30.0, 'max'),
    'harmonics_2_5_db': (0.5, 'max'),
    'total_power_rel': (0.05, 'max'),
    'total_thd_pp': (0.5, 'max'),
    'total_esr_db': (-20.0, 'max'),
    'total_harmonics_2_9_db': (1.5, 'max'),
    'newton_failures': (0, 'max'),
}

# milestone -> block -> set of scored checks. M0 tests the pipeline only: everything is INFO, except
# the testbench DC sanity check (added with the block testbenches in M0 step 4).
SCORED = {
    'M0': {},
}


def status(milestone, block_id, check, value):
    """Returns (tolerance text, 'PASS' | 'FAIL' | 'INFO')."""
    scored = SCORED.get(milestone, {}).get(block_id, set()) | SCORED.get(milestone, {}).get('*', set())
    if check not in TOLERANCES:
        return '—', 'INFO'
    limit, kind = TOLERANCES[check]
    text = f'{"≤" if kind == "max" else "≥"} {limit:g}'
    if check not in scored or value is None:
        return text, 'INFO'
    ok = value <= limit if kind == 'max' else value >= limit
    return text, 'PASS' if ok else 'FAIL'
