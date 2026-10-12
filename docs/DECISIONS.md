# Decision Log

Format: one entry per decision. Never delete entries; supersede them with a new entry that references
the old one. Claude Code adds entries for implementation choices (dependency pins, DC-init method,
parameter update interval, junction upgrades, tolerance changes approved by the owner).

Template:
```
## D-NNN Title
Date: YYYY-MM-DD · Status: proposed | accepted | superseded by D-MMM
Context: why a decision was needed.
Decision: what was decided.
Alternatives: what else was considered, and why not.
Consequences: what this affects (code, verification, sound).
```

---

## D-001 Start with the JCM800 2203 before the Peavey 5150
Date: 2026-10-12 · Status: accepted
Context: Both amps were transcribed to SPICE. The 5150 has 10 triode sections, relay-switched channels,
special stages and no power-supply sheet; the 2203 has 6 triode sections, one channel and a complete
schematic including the supply.
Decision: Build the first circuit-based engine on the 2203. Every 2203 block type also appears in the
5150, so components and the harness carry over.
Alternatives: Start directly with the 5150 (target sound) and absorb the extra complexity.
Consequences: The 5150 becomes a later project reusing this engine's building blocks.

## D-002 Wave Digital Filters with chowdsp_wdf
Date: 2026-10-12 · Status: accepted
Context: Candidate methods were nodal DK, WDF, static nonlinearities + filters, and neural networks
trained on SPICE renders. DK and WDF have equal accuracy potential when coupled nonlinearities are
solved jointly.
Decision: WDF using chowdsp_wdf (header-only, BSD-3-Clause), compile-time `wdft` API in the audio path.
Alternatives: DK via ACME.jl (netlist-driven derivation, matrix updates on knob changes); static
nonlinearities (loses blocking, loading and sag behaviour); NN (needs a training pipeline per setting
space).
Consequences: Custom multiport tube roots and R-type scattering computation must be written (no tube
elements in the library).

## D-003 SPICE reference model is the ground truth
Date: 2026-10-12 · Status: accepted
Decision: `reference/spice/jcm800_2203.cir` + `tubes.lib` define correct behaviour. Changes to either
require owner approval, a new entry here, regenerated reference values in `CIRCUIT.md`, and re-run
reports for affected blocks.

## D-004 Device models: Koren equations with a screen-current modification
Date: 2026-10-12 · Status: accepted
Decision: 12AX7 and EL34 use Koren's equations and parameters as in `tubes.lib`. The EL34 screen current
is computed from the plate equation's E1 term plus diversion of plate current to the screen at low plate
voltage (Koren's original screen equation gives ~0 mA at this bias).
Consequences: The engine must reproduce this exact modification; tube parameter fitting is deferred (M6).

## D-005 Partition, junction model and joint roots
Date: 2026-10-12 · Status: accepted
Decision: Seven blocks (SPEC §4.2); J-Thevenin junctions; joint multiport roots for V2a+V2b, the phase
inverter pair, and the power stage; matched power-tube pairs modelled as single equivalents with doubled
current.
Consequences: Partition error is measured explicitly (VERIFICATION §1) and junctions are upgraded only
with evidence.

## D-006 Global NFB closed with a one-sample delay (v1)
Date: 2026-10-12 · Status: accepted
Decision: B5 uses the previous sample's speaker voltage at the oversampled rate (≈ 2.6 µs at 384 kHz).
Consequences: Small HF phase error in the feedback loop; checked in M4 against a two-sample variant.

## D-007 Verify at 384 kHz without resampling
Date: 2026-10-12 · Status: accepted
Decision: Harness signals are generated at 384 kHz; SPICE output is sampled at the same fixed step; the
engine runs at 384 kHz. Resampling and aliasing are verified separately at the plugin level (M5).

## D-008 Engine library is JUCE-independent
Date: 2026-10-12 · Status: accepted
Decision: Only `plugin/` includes JUCE. The engine can later be integrated into the owner's existing
amp-sim plugin project next to its NAM engine.

## D-009 Signal level calibration defaults
Date: 2026-10-12 · Status: superseded by D-010 (input side)
Decision: 0 dBFS input = 1.0 V peak at the guitar jack; 0 dBFS output = 60 V peak across 16 Ω. Both are
configurable; the plugin exposes input calibration (±24 dB) and output level.

## D-010 Input calibration follows the +12 dBu amp-sim convention
Date: 2026-10-12 · Status: accepted · Supersedes: D-009 (input side; the output default is unchanged)
Context: The plugin will be used with many interfaces, so per-setup measurement is not a sensible
default. Amp-sim tools converge on 0 dBFS ≈ +12 dBu at the instrument input (NAM expects 12 dBu, Neural
DSP calibrates at 12.2 dBu, Focusrite Scarlett +12.5 dBu, UAD Apollo Twin X +12.2 dBu, Audient iD4 MkII
lineup 12 dBu = 0 dBFS). The previous default (1.0 V peak) assumed a signal ~13 dB hotter at the jack.
Decision: Default interface level +12.0 dBu at 0 dBFS → inputVoltsPerFullScale = 4.361 V peak. The
plugin exposes it as "Interface Level (dBu at 0 dBFS)" (0..+24 dBu), NAM-style, plus a separate Input Trim.
Alternatives: Measuring the owner's own interface (multimeter + sine); kept as an optional refinement for
comparing against real hardware later.
Consequences: SPICE/harness tests are in volts and unaffected; only the scaling of DI recordings and of
plugin input changes.

