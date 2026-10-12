"""Junction Thevenin resistances (SPEC §4.1, CIRCUIT.md §4) used by the block testbenches and the engine.
Values: |Z(1 kHz)| of the upstream output impedance at default parameters, measured with
`python -m harness.rth_measure` (owner decision, PROGRESS.md Q6b, DECISIONS D-018). J5 is provisional
until its partition error is measured in M4. None = not measured; testbenches then carry a PENDING
marker and transient runs refuse them. There is no J1 (B12, D-017). J4 is not listed: its resistance is
a knob-dependent formula in the testbench (WDF_DESIGN §6)."""

RTH = {
    'j2': 89731.8,     # p1b  (B12 -> B3); DC 90106.6
    'j3': 359.917,     # k2b  (B3 -> B4); DC 359.874
    'j5a': 58062.8,    # p3a  (B5 -> B6); DC 41396.3; provisional
    'j5b': 56928.4,    # p3b  (B5 -> B6); DC 45505.7; provisional
}
