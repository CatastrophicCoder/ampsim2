"""Junction Thevenin resistances (SPEC §4.1, CIRCUIT.md §4) used by the block testbenches and the engine.
Values: |Z(1 kHz)| of the upstream output impedance at default parameters, measured with
`python -m harness.rth_measure` (owner decision, PROGRESS.md Q6b, DECISIONS D-018; re-measured after
D-024). J5 is provisional until its partition error is measured in M4. None = not measured; testbenches then carry a PENDING
marker and transient runs refuse them. There is no J1 (B12, D-017). J4 is not listed: its resistance is
a knob-dependent formula in the testbench (WDF_DESIGN §6)."""

RTH = {
    'j2': 89732.8,     # p1b  (B12 -> B3); DC 90107.6
    'j3': 360.137,     # k2b  (B3 -> B4); DC 360.094
    'j5a': 58067.6,    # p3a  (B5 -> B6); DC 41404.1; provisional
    'j5b': 56937.4,    # p3b  (B5 -> B6); DC 45515.6; provisional
}
