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

## D-014 SPICE transient setup: sample-aligned input, sweep tail, configurable RELTOL
Date: 2026-10-12 · Status: accepted (RELTOL 1e-6: owner decision 2026-10-12, PROGRESS.md Q4)
Context: M0 step 3 measurements (ngspice 47):
- An XSPICE filesource creates no breakpoints, so ngspice's internal steps drift off the 384 kHz grid
  and the input is interpolated twice (filesource, then `option interp`). The circuit input was
  low-passed by 5.4e-5 at 1 kHz and 1.35e-3 at 5 kHz (load-independent, ∝ f²).
- PWL sources are exact but their lookup is linear in the number of points (0.5 s at 384 kHz on a
  single resistor took 52 s).
- Even with an exact input, ngspice's default RELTOL (1e-3) limits the reference accuracy: an RC
  low-pass vs its exact continuous response gives −104/−76/−52 dB ESR at 1/5/15 kHz without
  breakpoints and −92/−78/−67 dB with them; with RELTOL 1e-6, −92/−88/−97 dB. On the full amp
  (5 kHz, 0.1 V, 50 ms), compared with a RELTOL 1e-7 run: default −46 dB at `out`, −35 dB at `y`;
  breakpoints −60/−37 dB; +RELTOL 1e-5 −73/−61 dB; +RELTOL 1e-6 −87/−76 dB. Run times 5 / 8 / 23 /
  41 / 74 s (default / breakpoints / 1e-5 / 1e-6 / 1e-7).
- Fractional-octave averaging of Y·X* biases the sweep magnitude estimate (0.044 dB on a known
  filter, because the phase rotates within each band).
Decision:
- Keep the filesource and add a dummy PULSE triangle with a corner at every sample (breakpoints), so
  every sample instant is a simulator time point; the input is then exact (< 1e-10 V).
- The transient stops half a sample after the last sample and the input file has two hold points.
- Sweeps (SWP-lo/hi) carry 50 ms of silence after the 2 s sweep; the magnitude response is the per-bin
  ratio Y/X, complex-interpolated, without smoothing (error 1.3e-4 dB on the known filter).
- RELTOL for reference transients is one harness setting, `harness/config.py: SPICE_RELTOL`, part of
  the cache key. Owner decision (Q4): RELTOL = 1e-6, about 16 dB below the strictest time-domain
  tolerance (−60 dB) at `y`, at about 5x the run time of the default. op and ac are unaffected.
Alternatives: PWL input (too slow); inline PWL chunks; accepting the input low-pass (would appear as
engine error at high frequencies).
Consequences: Breakpoints cost about 1.6x run time on the full amp. Harness unit tests cover the input
alignment (1 and 5 kHz, 1e-9 V) and the reference accuracy at RELTOL 1e-6.

## D-015 Block testbench construction
Date: 2026-10-12 · Status: accepted
Context: VERIFICATION §1–2 define the testbench set and leave the construction details open.
Decision (`harness/testbench_gen.py`, generated files in `harness/testbenches/`):
- The reference netlist is parsed and every element is assigned to exactly one of B1–B7 (SPEC §4.2);
  generation fails if an element is missing or assigned twice. Testbenches copy the element lines
  verbatim and add only harness sources, so a netlist change propagates on regeneration.
- Inputs: `Vin` is the J-Thevenin source voltage behind `Rth_jN` (or the guitar source for B1). Its DC
  value is the F operating-point value of the upstream output, so `op` on a testbench reproduces F.
  Transient runs replace `Vin` with the signal.
- Supply rails in B1–B4: ideal constant sources at the F operating point (`Vsup_bpA`, `Vsup_bpX`).
- B56+B7: the preamp loads on bpA (1.302 mA) and bpX (4.039 mA) are constant current sinks from the F
  operating point (measured with a 0 V ammeter in a copy of the netlist for the V2b plate). Recorded
  waveforms from F replace them in M4.
- B5-open and B6: recordings from another run (`Vrec_<name>`: out and bpX for B5-open; p3a, p3b, ct
  and nScr for B6), with F operating-point DC values. The harness renders them only from M4 on.
- R_th4 (J4): DC resistance into the master wiper with J3 shorted and capacitors open,
  `R_VR2b ‖ (R_VR2a + R_VR3b + R_VR5 + R_VR4)`, as a knob-dependent `.param` (WDF_DESIGN §6; reactive
  part ignored). Its adequacy is checked by the partition error in M3/M4.
- R_th1/2/3/5a/5b come from `harness/junctions.py` (M0 step 5). Unmeasured values are written as
  `PENDING` placeholders and transient runs refuse such a testbench.
Consequences: DC sanity check results in `harness/reports/M0/20261012_testbench_dc.md`: b1–b5open
match F to ≤ 1.2e-9 relative. b6 differs at out/sl by 40 µV because R22 (B5) is not in B6. Owner
decision (Q5, 2026-10-12): accepted as a known partition effect. The check, its 1 mV floor and the
testbench stay unchanged; the DC report keeps listing it as FAIL with the explanation, and any other
deviation remains a failure.

## D-016 Junction resistance measurement and the SPICE partition-error study
Date: 2026-10-12 · Status: accepted (method); R_th values used: |Z(1 kHz)| (D-018)
Context: SPEC §7 M0.5 asks for R_th1/2/3/A/B by AC current injection at 1 kHz plus the DC small-signal
value. The measurements (CIRCUIT.md §4) show that J1 and J5 differ strongly between DC and 1 kHz, so the
owner asked for a partition-error measurement before choosing.
Decision:
- `harness/rth_measure.py`: 1 A AC into the open-circuited output node of the upstream block testbench,
  every other independent source AC-grounded (`spice.impedance`), decade sweep 1 mHz–100 kHz. "DC" is
  the 1 mHz point, cross-checked by a ±1 µA finite difference of two DC operating points (agree to
  < 1e-4). Upstream junctions inside a testbench use junctions.py values, or the DC value measured
  while unset; the measured sensitivity to that choice is < 4e-4.
- `harness/partition.py`: SPICE-only partition error for one junction. P chain = upstream testbench
  (output open) → downstream testbench driven by the recorded open-circuit voltage through each
  candidate R_th. Reference = a junction reference testbench joining the two blocks directly with the
  same constant supply (`b12` for J1, generated like the others and DC-checked), so the difference is
  the junction alone. Metrics on node voltage minus its DC operating point: ESR, h1 and harmonics 2–5,
  and the small-signal magnitude error from AC analyses. A unit test checks the AC error against the
  Thevenin prediction |Z_in + Z_th| / |Z_in + R| (agreement < 0.01 dB).
- J1 signal set: the B1 standard set without DI and SWP-hi, at the CLEAN, DEFAULT and DRIVE knob sets
  (Preamp Volume 0.2 / 0.6 / 1.0). J5 cannot be studied this way before M4 (it needs B56 recordings).
Consequences: Results in CIRCUIT.md §4 and `harness/reports/M0/20261012_partition_j1.md`. No constant
R_th1 gives better than −11 dB (CLEAN) / −17.6 dB (DEFAULT) worst sine ESR at p1b.

## D-017 Merge B1 and B2 into B12 (joint 4-port root)
Date: 2026-10-12 · Status: accepted (owner decision, PROGRESS.md Q6a)
Context: D-016's SPICE study of J1 (V1a plate → V1b input network) gave a worst sine ESR at p1b of
−8.7 to −22.6 dB for every constant R_th1 (DC 74.8 kΩ, |Z(1 kHz)| 39.3 kΩ, Re Z 37.9 kΩ) at Preamp
Volume 0.2 / 0.6 / 1.0, with up to 2.3 dB small-signal magnitude error. V1a's output impedance has no
flat region in the audio band (cathode bypass C1, plate capacitances), and B2's input impedance falls
to 54–177 kΩ at 10 kHz (C5, C4, V1b Miller). Options presented: constant R_th1 (|Z| or DC), SPEC §4.1
upgrade (a) (a fitted Z(f) network), upgrade (b) (merge).
Decision: upgrade (b). B1 and B2 become one block B12 (`StageV1aV1b`): V1a and V1b solved together by
a 4-port triode root (like B3), the coupling and Preamp Volume network inside the same tree, p1a an
internal node. There is no J1. The harness partition entry is `B12`, its testbench `b12.cir` (guitar
input, output p1b), ampwav block id `b12`; b1/b2 testbenches and block ids are removed. J2 is measured
on b12. `harness/partition.py` now studies J2 (b12 → b3 vs `b123`, B12 and B3 joined).
Consequences: No J1 partition error by construction. One 4-unknown Newton solve per sample instead of
two 2-unknown solves (cost measured in M2). SPEC §7: M2 is now B12 (triode model, multiport Newton, the
4-port tree); M3 is B3 and the preamp cascade. Tolerances unchanged; the M2 block-exact checks are
scored at p1b. V1a cannot be checked block-exact on its own in the engine; the triode model keeps its
device unit tests (VERIFICATION §5.1). The J1 study is reproducible at commit a64bf9b.

## D-018 Junction resistance basis and ESR on the signal minus its own DC operating point
Date: 2026-10-12 · Status: accepted (owner decisions, PROGRESS.md Q6b and Q7)
Context: D-016 measured R_th as the DC small-signal value and the complex value at 1 kHz. For J2 and
J3 the choice makes no measurable difference (J2 partition study: within 0.1 dB); for J5 the DC value
is about 30 % below the 57–59 kΩ the PI plates show from 10 Hz to 5 kHz. Separately, `metrics.esr_db`
was applied to raw node voltages; at DC-biased nodes (plates at ~250 V) the DC term dominates Σy², so a
1 % AC error on a 1 V signal read as about −88 dB.
Decision:
- junctions.py holds |Z(1 kHz)| at default parameters: J2 89.7318 kΩ, J3 359.917 Ω, J5a 58.0628 kΩ,
  J5b 56.9284 kΩ. J5 is provisional until its partition error is measured in M4 (below ~5 Hz the true
  value falls toward 41–46 kΩ). A unit test re-measures and fails on a > 0.1 % change.
- ESR, best lag and the SWP-lo magnitude response use each signal minus its own DC operating point (its
  value at t = 0; every test signal starts at 0 V): `metrics.ac_part`, used by `compare` and
  `partition`. Static DC offsets are scored only by the DC error check (0.5 %); bias shifts that develop
  during a signal stay in ESR. VERIFICATION §6 wording updated; tolerance values unchanged.
Alternatives: DC small-signal or Re Z(1 kHz) for R_th (Re Z differs from |Z| by < 0.02 % here);
subtracting the reference's operating point from both signals (a static engine DC offset inside the
0.5 % DC tolerance, up to ~1.25 V at a 250 V plate, could push ESR to ~0 dB); subtracting the window
mean (hides the average bias shift); raw voltages.
Consequences: All committed testbenches are free of PENDING junctions, so every non-recorded block can
be rendered in transient. Harmonics already removed the mean of their analysis window (unchanged);
power into the load still uses the raw output voltage.

## D-019 Device DC sweep tables
Date: 2026-10-12 · Status: accepted
Context: VERIFICATION §5.1 defines the 12AX7 and EL34 DC sweeps for the device unit tests (M2, M4).
It leaves the plate and screen voltages of the EL34 grid-current sweep open, and the engine model
(WDF_DESIGN §3) omits tubes.lib's 1 GΩ Rcp, whose current (up to 0.6 µA at 600 V) exceeds the 1 nA
tolerance near cutoff.
Decision:
- `harness/device_sweeps.py` runs one ngspice DC sweep per curve on a single tube with ideal sources on
  every electrode (cathode at ground) and tight DC options (reltol 1e-12, abstol 1e-18, vntol 1e-12,
  gmin 1e-18). Tables in `tests/data/` (committed, `--check` for freshness): voltages, then `ip`
  (terminal current − Vpk / 1 GΩ, what the engine must match), `is`, `ig`, and `ip_terminal`.
- EL34 grid-current sweep at Vpk = Vsk = 400 V (the model's grid current does not depend on them; only
  the ip/is columns do).
- A harness test recomputes every row from the WDF_DESIGN §3 equations to 1e-7 relative / 1e-13 A. It
  established three SPICE details, written into WDF_DESIGN §3: Vt from ngspice's constants
  (25.8649170 mV); tubes.lib's 1.5707963 for π/2 in the screen current (1.4e-7 relative); ngspice's
  reverse-bias diode form below −3·Vt (≤ 5e-11 A, not replicated in the engine).
Consequences: The C++ device tests (M2: 12AX7, M4: EL34) read these tables with the VERIFICATION §5.1
tolerance (0.1 % or 1 nA).

## D-020 CI workflow
Date: 2026-10-12 · Status: accepted; first GitHub run green (run 38304157627 on f311402)
Context: SPEC §7 M0.6 asks for a minimal GitHub Actions workflow on a macOS runner; the owner chose to
leave the JUCE plugin out of CI (PROGRESS.md Q3).
Decision: `.github/workflows/ci.yml`, on pushes to main and on pull requests: `macos-15` runner,
`actions/checkout` pinned to the v7.0.1 commit (3d3c42e5aac5ba805825da76410c181273ba90b1), Ninja from
Homebrew only if the image lacks it, then the documented commands: configure Release with
`-DAMP2203_BUILD_PLUGIN=OFF`, build (warnings are errors on our targets), `ctest --output-on-failure`.
Read-only token permissions, 30 min timeout, superseded runs cancelled. No dependency cache: the three
pinned archives download in a few seconds. The Python harness stays out of CI (it needs ngspice and its
SPICE runs take minutes).
Consequences: Verified by running the same steps on a fresh clone locally (macOS 15.7, Apple clang 17,
CMake 4.4.3): configure 4.4 s, build 8 s, 0 warnings, ctest 14/14. First GitHub run (macos-15,
2026-10-12): configure 6 s, build 38 s, tests passed, 52 s in total.

## D-021 Publication: drawings removed from history, AGPL-3.0 licence
Date: 2026-10-12 · Status: accepted (owner decisions before the first GitHub publication)
Context: `reference/schematics/` held Marshall's own drawings (2203_preamp.gif,
2203_output_stage_psu.gif) in every commit; the repository had no licence and no trademark notice.
Decision:
- The drawings were removed from all commits with `git filter-branch --index-filter` before the first
  push (refs/original, reflog and unreachable objects purged; their blobs are no longer in the object
  store). They stay on the owner's machine, ignored by git (`.gitignore`). CIRCUIT.md, reference/README.md
  and the README name the drawings by title and issue instead. Every commit hash changed; the only hash
  cited in the docs (the J1 study, formerly 4838919) is now a64bf9b.
- Licence: GNU AGPL-3.0 (`LICENSE`, the FSF text), matching JUCE 8's open-source licence option.
  Dependencies keep their own licences (README "Licence").
- README rewritten for the current state, with a non-affiliation / trademark disclaimer.
Consequences: A pre-rewrite git bundle was kept outside the repository during the change. Contributions
are under AGPL-3.0.

## D-022 Harness fixes from M0 step 8: junction DC level, vector selection
Date: 2026-10-12 · Status: accepted
Context: The first `compare` runs on every block found two harness defects.
1. `spice.tran` replaced a testbench's `Vin` with the signal itself, so for blocks driven by a junction
   source (b3, b4, b56) the transient started from an operating point with that source at 0 V instead
   of its F value (e.g. k2b at 0 V instead of 232 V in b4). Capacitor coupling and linearity kept the
   AC results of these blocks unaffected, but internal node DC levels were wrong. The P chain (partition
   studies) was not affected: it drives `Vin` with the absolute upstream voltage.
2. `tran` and `impedance` named their output vectors in the control-language `write` command, and `ac`
   built `v(out)/v(in)` there. Node names that are operators in that language (`ne`, `gt`, `and`, ...)
   then fail; B4's tone-stack node nE stopped the b4 render.
Decision:
1. Standalone signals ride on the DC value of the block's `Vin` (`render_spice.block_input`): the
   junction source's operating point, 0 V for the guitar input. The engine gets the same samples through
   `run_engine`, so a block's input WAV is the absolute junction source voltage (VERIFICATION §4).
   A test checks that b3, b4 and b56 transients start at the F operating point.
2. Vectors are selected with a netlist `.save` line and `write <file>` without arguments; `ac` divides
   in Python. A test uses nodes named ne, gt and and.
Consequences: SPICE cache keys changed (renders before this fix are not reused). The junction
measurement report is byte-identical after the change. The M0 step 8 compare runs were restarted.

## D-023 Output transformer from the Hammond 1750U datasheet (netlist change)
Date: 2026-10-12 · Status: accepted (owner approval of the netlist change; PROGRESS.md Q8)
Context: The OT values in the reference netlist were assumptions (A2: 25 H, k = 0.999, 40 Ω per
half-primary, 0.3 Ω secondary). The owner supplied the datasheet of the Hammond 1750U, a drop-in
replacement for the JMP / JCM800 100 W OT: DCR 15.36 Ω (Blue–White), 16.56 Ω (White–Red), 0.320 Ω
(Violet–Green, 16 Ω tap); 8.85 H primary and 88.86 mH secondary (16 Ω tap) at 1 V / 1 kHz; leakage
7.97 mH (secondary shorted); response and phase curves at 27 dBu in a test circuit with RS/2 = 850 Ω at
each primary end. In that test circuit the old values gave −1.6 dB at 10 kHz and −7.0 dB at 30 kHz,
against ≈ −0.2 dB and ≈ −0.85 dB measured.
Decision (owner choices: adopt and fit to the curves; nominal turns ratio; asymmetric DCRs with
Blue–White on the pA side):
- `reference/spice/jcm800_2203.cir`: Rwp1 15.36 Ω (pA side), Rwp2 16.56 Ω (pB side), Rws 0.320 Ω;
  `npr` = sqrt(1700/16) unchanged; new `.param kot=0.9997925` for K1–K3 (from the 7.97 mH leakage);
  `lpt` = 25.61 H fitted to the datasheet's 16 Ω response and phase curves (readings in
  `harness/ot_fit.py`, ±0.05 dB / ±1°). The fit also allowed a plate-to-plate capacitance and set it to
  zero, so no element was added. Residuals: magnitude ≤ 0.15 dB, phase ≤ 1.3°.
- `python -m harness.ot_fit` reproduces the fit; `--check` (and a harness test) verifies the netlist
  against the datasheet (DCRs, shorted-secondary inductance within 0.1 %, curve residuals ≤ 0.2 dB /
  1.5°).
- New open item O7: which half-primary goes to which EL34 pair.
Alternatives: the 1 V inductances as given (8.85 H; the 27 dBu curves imply ≈ 25.6 H, so the low end
would roll off too early); the measured inductance ratio for n² (99.6); averaged DCRs; keeping the
assumptions.
Consequences: DC changes are small: ct 488.444 V (was 488.445), plates pA/pB 487.38 / 487.30 V (were
485.67 V), EL34 currents unchanged to 4 digits, preamp supplies change by 1e-9 relative. b6's known
out/sl deviation is now 42.4 µV (was 39.8 µV). NFB unchanged (6.2 dB at 1 kHz). Clean power at the same
inputs rises by 2–6 % (≈ 103 / 136 / 154 W at 25 / 30 / 40 mV). CIRCUIT §3, §5 and §6, reference/README,
README and the testbenches are regenerated; all SPICE renders are redone. The model remains a linear
transformer: no level-dependent inductance, core loss, winding capacitance or saturation.
