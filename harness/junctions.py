"""Junction Thevenin resistances (SPEC §4.1, CIRCUIT.md §4) used by the block testbenches.
None = not measured yet (M0 step 5); testbenches then carry a PENDING marker and transient runs refuse
them. J4 is not listed: its resistance is a knob-dependent formula in the testbench (WDF_DESIGN §6)."""

RTH = {
    'j1': None,    # p1a  (B1 -> B2)
    'j2': None,    # p1b  (B2 -> B3)
    'j3': None,    # k2b  (B3 -> B4)
    'j5a': None,   # p3a  (B5 -> B6)
    'j5b': None,   # p3b  (B5 -> B6)
}
