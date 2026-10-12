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


## D-011 Dependency pins
Date: 2026-10-12 · Status: accepted
Context: SPEC §2 requires every third-party dependency to be pinned and recorded here.
Decision: FetchContent from GitHub source archives of fixed commits, verified by SHA-256
(`cmake/Dependencies.cmake`):
| Dependency | Pin | Commit | Archive SHA-256 |
|---|---|---|---|
| chowdsp_wdf | main (no tag) | 43bcd295e8403de913f9966f4fd7ff9d17c9f1a5 (2026-07-02) | 4fa7919de8ee893ea3810e5a46728a8fdf04d319dea51359670bed90a29eeb70 |
| Catch2 | v3.16.1 | 5010924c4d8e912e64fd64a11819eecacf572c06 | 66e15ad3f8896fb1a4ef9a868b1de573be766f45c9495eb3c4ba4cc944ea4e22 |
| dr_libs (dr_wav) | wav-0.14.5 | fa931f3285ced10ace628f7f1ac951e1951e7ea6 | b56be5930399b9ea81591a837e0c9ee959a5d57e90dac96e0af37dde1fa0bb7e |
| JUCE (plugin only) | 8.0.15 | 91ad83ae34a81e0833b1a2b0866f54846370ae53 | 04f8d5055382582c757be9da069ea98338005f98248facd9c2804435ac853e70 |
Eigen is not fetched; whether it is used is decided in M1.
Alternatives: chowdsp_wdf tag v1.0.0 (2022-12-15, 17 commits older). It lacks the 2026-07-02 change to
the adapted R-type up-port reflection (computed from the current incident waves instead of returned
from `b_vec`), SIMD comparison fixes, the open/short compile fix and the user-supplied Omega option.
The owner chose the main commit. JUCE 9.x exists (9.0.4) but SPEC §2 specifies 8.x, so the latest 8.x
tag is used. Git clones were not used because the JUCE history is large; archives with a hash give
the same pinning.
Consequences: If GitHub ever regenerates an archive with different bytes, the hash check fails loudly;
the fix is to switch that dependency to `GIT_REPOSITORY` + `GIT_TAG <commit>` (same pin).

## D-012 Engine API additions to SPEC §3.2
Date: 2026-10-12 · Status: accepted
Decision: `Amp2203` additionally has `process(double*, int) noexcept` (ampwav and tests run in double
end to end) and `probes()`, returning `{name, const double*}` pairs for `ampwav --dump-node`. Probes
point at members the blocks update anyway, so they cost nothing in the plugin; the class is
non-copyable because of these pointers. The M0 pass-through is the identity in volts
(v_speaker = v_in), so normalised output = input · inputVoltsPerFullScale / outputVoltsPerFullScale.
Consequences: SPEC §3.2 remains the minimum API; the plugin uses only the float overload.

## D-013 ampwav I/O conventions
Date: 2026-10-12 · Status: accepted
Context: The harness compares engine output with SPICE node voltages; the engine API works in
full-scale samples (SPEC §3.3).
Decision: `ampwav` WAV samples are volts at the block's input and output by default (`--units volts`),
for every block including `full`; `--units fs` (full amp only) uses full-scale samples as the plugin
sees them. Output and node dumps are mono 64-bit float WAV. The input must be mono and its rate must
equal `--rate` (no resampling, D-007). Block ids follow VERIFICATION §1: `full`, `b1`..`b7`, `b56`
(case-insensitive). `--stats` writes timing and Newton statistics as JSON for the reports. When nodes
are dumped, the engine is called one sample at a time; otherwise in `--block-size` chunks (default
512), so timing figures should come from runs without dumps.
Alternatives: Full-scale samples everywhere (the harness would have to know each block's scaling).
Consequences: SPICE renders and engine outputs are directly comparable without scaling.
