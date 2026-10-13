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
    'partition_esr_db_sines': (-26.0, 'max'),        # at y (M3), VERIFICATION §7
    'partition_esr_db_di': (-20.0, 'max'),
    'newton_median_iterations': (3, 'max'),        # SPEC §7 M2 acceptance
    'newton_max_iterations': (12, 'max'),          # the configured limit (WDF_DESIGN §4 maxIter)
}

# milestone -> block -> set of scored checks. M0 tests the pipeline only: everything is INFO, except
# the testbench DC sanity check (added with the block testbenches in M0 step 4).
SCORED = {
    'M0': {},
    # M1: B4 ToneStackMaster (SPEC §7 M1). The 375-setting magnitude sweep and the knob-jump test run in
    # harness/tonestack_check.py and harness/knob_jump.py with the same tolerances.
    'M1': {'b4': {'linear_mag_db_20_10k', 'linear_mag_db_10k_20k', 'linear_esr_db', 'dc_rel_error'}},
    # M2: B12 StageV1aV1b (SPEC §7 M2). ESR: every signal at the small/nominal/hot levels ≤ −40 dB, the
    # overdrive-level (2 V) sines ≤ −30 dB; harmonics 2–5 where above −80 dBc; Newton statistics.
    'M2': {'b12': {'dc_rel_error', 'nonlinear_esr_db', 'nonlinear_esr_db_overdrive', 'harmonics_2_5_db',
                   'newton_failures', 'newton_median_iterations', 'newton_max_iterations'}},
    # M3: B3 StageV2aV2b block-exact with the M2 targets at B3's own levels (SPEC §7 M3); the cascade
    # partition / total error at y is scored by harness/cascade.py.
    'M3': {'b3': {'dc_rel_error', 'nonlinear_esr_db', 'nonlinear_esr_db_overdrive', 'harmonics_2_5_db',
                  'newton_failures', 'newton_median_iterations', 'newton_max_iterations'},
           'b34': {'dc_rel_error', 'nonlinear_esr_db', 'nonlinear_esr_db_overdrive', 'harmonics_2_5_db',
                   'newton_failures', 'newton_median_iterations', 'newton_max_iterations'},
           # cascade (harness/cascade.py, D-036): partition at the loaded y, engine vs the SPICE chain
           'preamp': {'partition_esr_db_sines', 'partition_esr_db_di', 'nonlinear_esr_db', 'nonlinear_esr_db_overdrive',
                      'harmonics_2_5_db', 'newton_failures', 'newton_median_iterations', 'newton_max_iterations'}},
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
