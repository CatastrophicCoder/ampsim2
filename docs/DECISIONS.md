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
Decision: B5 uses the previous sample's speaker voltage at the oversampled rate (≈ 2.6 µs at 384 kHz;
1.3 µs at 768 kHz, D-035).
Consequences: Small HF phase error in the feedback loop; checked in M4 against a two-sample variant.

## D-007 Verify at 384 kHz without resampling
Date: 2026-10-12 · Status: superseded by D-035 (768 kHz)
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
Addition (2026-10-13, M2): `Stats` also has `newtonSolves` and `medianIterations` (SPEC §7 M2 acceptance);
ampwav `--stats` writes them as `newton_solves` and `median_iterations`.

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
Addition (2026-10-13, M2): dr_wav rejects rates above 384 kHz by default; ampwav sets
`DRWAV_MAX_SAMPLE_RATE` to 1.536 MHz so it also runs the 16x rates of SPEC §3.4 (768 kHz at 48 kHz),
which the M2 discretisation check uses.

## D-014 SPICE transient setup: sample-aligned input, sweep tail, configurable RELTOL
Date: 2026-10-12 · Status: accepted (RELTOL 1e-6: owner decision 2026-10-12, PROGRESS.md Q4); RELTOL
superseded by D-031 (1e-8)
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

## D-024 Choke from the Hammond 194F datasheet (netlist change)
Date: 2026-10-13 · Status: accepted (owner approval of the netlist change; PROGRESS.md Q9)
Context: The choke was assumed at 5 H / 60 Ω (A3). The owner supplied the datasheet of the Hammond 194F,
a replacement choke "for JCP and JCM800": 5 H @ 120 mA DC, DCR 115 Ω ±15 % at 20 °C, no curves.
Decision: `Rch` 60 → 115 Ω; `Lch` stays 5 H (the inductance at the amp's ≈ 27 mA choke current is not
specified).
Consequences (vs 60 Ω): nScr −1.18 V (485.65 V), bpX / bpA −0.97 / −0.94 V, preamp plates −0.16 to
−0.24 V, EL34 plate current −1.7 % (34.08 mA per tube), the choke/C22 resonance (ct → nScr, ≈ 9.7 Hz)
+6.4 dB instead of +10.2 dB, 100 Hz attenuation unchanged (−39.8 dB), clean power at 30 mV −1.8 %
(133.2 W, 3.54 % THD), NFB 6.2 dB at 1 kHz. CIRCUIT §1 (B7), §5, §6 and reference/README updated;
testbenches, DC check and junction measurements regenerated (junctions.py re-recorded: J2 89.7328 kΩ,
J3 360.137 Ω, J5a 58.0676 kΩ, J5b 56.9374 kΩ; changes ≤ 0.06 %); all SPICE renders redone. Remaining
limits: constant inductance, DCR at 20 °C (copper rises ≈ 0.4 %/°C when warm).

## D-025 Knob-jump test criterion
Date: 2026-10-13 · Status: accepted (owner decision, PROGRESS.md Q10)
Context: SPEC §6.2 requires that full-range knob jumps produce no NaN/Inf and no clicks above the real
circuit's transient, without a measurable criterion.
Decision: Each knob is swept 0→1 in 10 ms during a 1 kHz tone. The reference is SPICE with
time-varying pot resistances (ngspice `R='expr(time)'`) following the engine's smoothed rotation
trajectory (one-pole, `paramSmoothingMs`). Pass: no NaN/Inf, and engine vs SPICE ESR ≤ −40 dB over the
sweep and the 20 ms after it. Added to VERIFICATION §7.
Alternatives: peak error ≤ 1 % of the reference peak; error energy above 5 kHz ≤ −60 dB relative to the
tone; report only.
Consequences: The ESR measures what the engine's rate-limited matrix updates (every
`kParamUpdateInterval` samples) and the discretisation add to the real transient. A brief click carries
little energy, so peak error and HF error energy are reported alongside as information.

## D-026 MNA scattering builder: double-double setup arithmetic, exact Python reference
Date: 2026-10-13 · Status: accepted (owner decision, PROGRESS.md Q11)
Context: WDF_DESIGN §5.5 requires the adapted up-port of an R-type adaptor to be reflection-free to
1e-12. For B4 with every knob at maximum (pot ends of 1–22 Ω next to 1 MΩ, capacitor ports of 59 Ω) the
nodal matrix has a condition number near 1e5, and double precision gives |S[up][up]| ≈ 2–4e-12;
iterative refinement and diagonal scaling do not improve it. Options presented: zero S[up][up] and
check 1e-10; keep S and check 1e-10; higher precision.
Decision:
- `engine/src/mna/` (header-only, templated on port and node count, fixed-size arrays, no heap) solves
  the nodal equations in double-double arithmetic (`DD`: error-free sums and fma products, ≈ 32
  digits) with partial pivoting; results are rounded to double. Functions: `upPortResistance`,
  `scattering` (definition-based, WDF_DESIGN §5.1), `solveDc` (for `reset()`).
- `harness/mna.py` solves the same equations in exact rational arithmetic (`fractions`), so its fixtures
  (`tests/data/rtype_fixtures.txt`: B4 at four knob sets and a random bridge) are the correctly rounded
  exact results. No Eigen and no new dependency.
- Tests: C++ vs fixtures within 1e-12 (R_up within 1e-15 relative), reflection-free < 1e-12,
  losslessness 1e-9, 3-port R-types vs the library's parallel and series adaptors within 1e-12 (which
  also pins the port orientation: the library's series adaptor presents −(v1 + v2) upwards).
Consequences: The WDF_DESIGN §5.5 tolerances stay as written. A matrix update costs more than in plain
double; the cost is measured with the ToneStackMaster block (M1 step 2).

## D-027 ToneStackMaster (B4) structure, DC start, smoothing and matrix-update interval
Date: 2026-10-13 · Status: accepted
Context: SPEC §7 M1.2, WDF_DESIGN §5.2–5.4, SPEC §3.6 and §6.2.
Decision:
- Tree: `IdealVoltageSourceT` (J3 source voltage, absolute volts) → `PolarityInverterT` →
  `WDFSeriesT`(R_th3, R-type) → `RtypeAdaptor` with up-port index 0 (k2b–ground, reflection-free) and
  the 11 B4 elements as down-ports (port order and node order = `engine/src/tonestack/ToneStackNetwork.h`
  = `harness/mna.py` B4_PORTS). The library's `RootRtypeAdaptor` is not used: its `compute()` scatters
  the incident waves stored by the previous call, a one-sample delay of the source. R_th3 = 360.137 Ω
  (junctions.py). Pots exactly as the netlist (SPEC §6.1).
- `reset(vInDc)`: DC node voltages from `mna::solveDc` (J3 source as a Norton equivalent, capacitors
  open) written into the capacitor states. The capacitors are `detail::DcCapacitor`, the library's
  bilinear capacitor equations plus a settable DC state (a test checks it against `CapacitorT`
  sample for sample). Knobs jump to their targets.
- Smoothing: one-pole on each knob rotation (`paramSmoothingMs`, 20 ms), settled within 1e-7. While a
  knob moves, the pots and the scattering matrix are recomputed every `kParamUpdateInterval` = 32
  samples (one R-type computation per update via `ScopedDeferImpedancePropagation`), plus one final
  update on the targets.
- R_th4 for J4: the D-015 formula from the smoothed knobs, also exposed as probe `rth4` (ohms).
- ampwav `--block b4` runs the block; it resets on the first input sample (signals start at the
  operating point).
Cost (amp2203_bench b4, Apple M-series, Release, 384 kHz): 31.7 ns/sample with static knobs (1.2 % of
the 2.6 µs/sample budget); one matrix update ≈ 7.3 µs (double-double, D-026); with knobs moving
continuously 262 ns/sample (≈ 10 % of the budget, only while a knob moves). This fits the audio thread,
so no worker thread (WDF_DESIGN §5.4); the interval stays at 32.

## D-028 StageV1aV1b (B12): joint 4-port root in Thevenin form, device model, DC start
Date: 2026-10-13 · Status: accepted
Context: SPEC §7 M2, WDF_DESIGN §3.1, §4 and §5.3. WDF_DESIGN §4 writes the root equation as
f_k = v_k + R_k·i_k(v) − a_k with one port resistance per root port. That holds when the adaptor's
root-port block S_rr is zero, which K port resistances cannot achieve for K > 1 coupled ports: in B12
V1a's grid and plate ports share k1a (R1‖C1) and Cgp couples grid and plate, so the incident waves
depend on all reflected root waves.
Decision:
- Root equation in exact K-port Thevenin form: the linear network seen from the root ports gives
  v = v_oc − Z·i(v), with v_oc a linear function of the element waves and Z the K×K resistance matrix;
  f(v) = v + Z·i(v) − v_oc, J = I + Z·∂i/∂v (`engine/src/solver/MultiportNewton.h`, template on K).
  This is the R-type adaptor with the root ports eliminated; WDF_DESIGN §4's diagonal form is the
  special case of uncoupled ports, and the root-port resistances of §5.3 cancel out. The matrices come
  from `mna::rootTransfer` (double-double, D-026): node voltages = Xe·b − Xr·i for element waves b and
  root currents i, with Z = the root-port rows of Xr.
- B12 linear network: 15 element ports between 8 nodes (`engine/src/stage12/Stage12Network.h`):
  (Vin, R3), (bpA, R4), (bpA, R7) as `ResistiveVoltageSourceT`; R1‖C1, R5‖C4, C5‖VR1a as library
  `WDFParallelT`; C3, VR1b, R6; the six inter-electrode capacitances of tubes.lib. R2 sits across the
  ideal input source and changes no node voltage, so it is not modelled. Per sample: element reflected
  waves → open-root node voltages → v_oc → Newton → node voltages with the root currents → incident
  waves 2·v − b to the elements.
- 12AX7 (`engine/src/devices/Triode.h`): tubes.lib equations with the analytic Jacobian; the grid diode
  in closed form with an own double-precision Wright omega (initial guess plus Fritsch–Shafer–Crowley
  iterations; chowdsp's `Omega::omega4` is a fast approximation). Omitted as in WDF_DESIGN §3.1 and
  SPEC §5: Re1/Rcp (1 GΩ), the diode's CJO/TT. Measured in M2 (scratch copy of tubes.lib with
  CJO = TT = 0, reference files unchanged): CJO accounts for −57 dB ESR at 5 kHz, −70 dB at 1 kHz at
  p1b; the 1 GΩ Rcp is about 1e-4 of the 100 kΩ plate loads (≈ −80 dB).
- `reset(vInDc)`: DC operating point with the capacitors open, from the same network (open ports for
  the capacitors) and the same root solved with up to 500 iterations from Vgk = −1 V, Vpk = bpA/2,
  written into the capacitor states and the warm start (SPEC §3.6 option 1).
- Preamp Volume: one-pole smoothing (`paramSmoothingMs`), matrices recomputed every
  kParamUpdateInterval = 32 samples while it moves (as D-027). Supply bpA is a settable ideal source
  (default 389.349603 V, the F operating point); `supplyCurrent()` reports the current through R4 + R7
  for B7 (M4). A non-finite solution keeps the previous state and is counted (`sanitizeEvents()`).
- Newton statistics (`detail::NewtonStats`: histogram, median, max, failures) are reported through
  ampwav `--stats` (D-012 addition).
Cost (amp2203_bench b12, Apple M-series, Release, 384 kHz): 323 ns/sample at 0.01 V, 420 ns at 0.1 V,
468 ns at 2 V (1 kHz), 617 ns at 5 kHz 2 V (12–24 % of the 2.6 µs/sample budget); one matrix update
≈ 8.4 µs; 683 ns/sample with Preamp Volume moving continuously. Fits the audio thread; no worker thread.
Alternatives: R-type adaptor with chosen root-port resistances and the root solved in wave variables
(equivalent, but the S_rr block makes the root equation implicit in the same way, with more arithmetic);
a chowdsp `RtypeAdaptor` (single up-port only).
Consequences: B3, B5 and B6 reuse the solver and `rootTransfer`. WDF_DESIGN §4 updated.

## D-029 Newton predictor: linear extrapolation (WDF_DESIGN §4 step 1)
Date: 2026-10-13 · Status: accepted
Context: With the warm start of WDF_DESIGN §4 (previous solution), B12 at 5 kHz 2 V (DEFAULT) had 4213
Newton failures and ESR −7.9 dB. V1b leaves cutoff at 6–7 V/sample on its grid; there the plate current
is exponential in Vgk (scale s/(ex·kp) ≈ 0.46 V), the full Newton step overshoots, the halving rule
cuts each step to 1/16 and 12 iterations do not reach the solution. Measured variants (DEFAULT,
failures / median / max iterations; ESR identical where converged):

| Variant | 5 kHz 2 V | 5 kHz 0.5 V | 1 kHz 2 V |
|---|---|---|---|
| §4 as written (warm start, halving) | 4213 / 4 / 12 (−7.9 dB) | 0 / 3 / 11 | 0 / 3 / 6 |
| grid step limit 5 V or 20 V instead of 2 V | 7155–7492 failures | — | — |
| warm start, no halving | 0 / 4 / 8 | 0 / 3 / 5 | 0 / 3 / 4 |
| warm start, maxIter 100 (diagnostic) | 0 / 4 / 23 | 0 / 3 / 11 | 0 / 3 / 6 |
| linear extrapolation, halving (chosen) | 0 / 3 / 10 | 0 / 3 / 5 | 0 / 2 / 4 |
| linear extrapolation, no halving | 0 / 3 / 10 | 0 / 3 / 5 | 0 / 2 / 4 |

Decision: initial guess v[n] = 2·v[n−1] − v[n−2] (the two previous solutions); everything else in §4 is
unchanged (step limits 2 V / 20 V, halving up to 4 times, maxIter 12). Of the two variants that meet
SPEC §7 M2 (median ≤ 3, zero failures) this one changes only step 1. WDF_DESIGN §4 listed the predictor
as a later optimisation "only with harness evidence"; the table is that evidence. It also lowers the
cost (e.g. 1 kHz 0.01 V: median 2 → 1).
Consequences: The blocks keep the previous two root solutions; `reset()` sets both to the DC solution.

## D-030 STEP test signal with band-limited edges
Date: 2026-10-13 · Status: accepted (owner decision, PROGRESS.md Q12)
Context: B12 at Preamp Volume 0.2 scored STEP ESR −34.5 dB (≤ −40 dB). The error lay above 20 kHz
(−50.7 dB after a 20 kHz low-pass on both signals) and shrank with the engine rate (−44.4 / −52.6 dB at
768 kHz / 1.536 MHz): the bilinear discretisation at 384 kHz of an ideal step, whose spectrum reaches
Nyquist, passed by the C5 bright cap at low volume. Options presented: accept as known; band-limit the
STEP edges; score STEP after a 20 kHz low-pass; run B12 at 2× internally.
Decision: the STEP pulse train (VERIFICATION §3) is filtered by a causal 4th-order Bessel low-pass with
−3 dB at 20 kHz (`harness/signals.py: STEP_EDGE_HZ`, `STEP_EDGE_ORDER`), for every block. The signal
still starts at exactly 0 V and stays DC-free; overshoot is about 1 %; energy above 80 kHz is below
−85 dB of the total.
Consequences: STEP tests the audio band; content above about 20 kHz is no longer exercised by STEP. M1
(B4) and M2 (B12) STEP references re-rendered.

## D-031 SPICE RELTOL 1e-8 for all reference transients
Date: 2026-10-13 · Status: accepted (owner decision, PROGRESS.md Q13); supersedes the RELTOL part of D-014
Context: B12 at Preamp Volume 0.2, 5 kHz 0.1 V: H3 (−70.7 dBc) differed by 0.54 dB (≤ 0.5 dB). The
engine gives −71.23 dBc at 384 kHz, 768 kHz and 1.536 MHz; SPICE gives −71.28 / −70.68 / −71.03 /
−71.13 dBc at RELTOL 1e-5 / 1e-6 / 1e-7 / 1e-8, so the reference was not converged at 1e-6 (difference
at 1e-8: 0.10 dB). Options presented: 1e-8 for B12; 1e-8 everywhere; accept as known; skip unconverged
harmonics.
Decision: `harness/config.py: SPICE_RELTOL = 1e-8` for every reference transient. SPICE run time is
2–3× that at 1e-6 (B12 5 kHz sine 8 s → 25 s, 2 s sweep 58 s → 121 s). op and ac are unaffected.
Consequences: All M1 and M2 transient references re-rendered and their reports regenerated. The M0
reports and the M0 J1/J2 partition studies stay as recorded at 1e-6 (the M0 report and the partition
reports state it; `compare` reports state the RELTOL from M2 on).


## D-032 StageV2aV2b (B3) and the shared root-network code
Date: 2026-10-13 · Status: accepted
Context: SPEC §7 M3.1. B3 has the same structure as B12 (two triodes on a 4-port root), and B5 / B6 follow.
V2b's plate is the supply node bpX, an ideal source rather than a network node.
Decision:
- The per-sample algorithm of D-028 (element waves → open-root node voltages → Newton → incident waves),
  the matrix update and the DC solve live in `engine/src/stage/RootNetwork.h` (template on nodes, element
  ports and root ports); the state is `detail::RootNetworkState`. The two-triode device and its solver
  limits are `devices::TwoTriodes`. B12 was moved onto it; its outputs are bit-identical to M2 (five
  renders compared, including a Preamp Volume ramp).
- Ports with a terminal on an ideal supply node are stored against ground with an exact offset:
  v_port = v(node+) − v(node−) + offset, offset = (supply at +) − (supply at −); an element's Thevenin
  source b becomes b − offset, a root port's Thevenin voltage gains + offset. In B3: V2b's plate port
  (bpX − k2b) and V2b's Cgp (p2a − bpX) and Cpk (bpX − k2b). A Catch2 test checks KCL at p2a, k2a, k2b at
  three supply voltages to 1e-12 A.
- B3 network (`engine/src/stage3/Stage3Network.h`): 6 nodes, 13 element ports (J2 source with
  R_th2 = 89732.8 Ω, C7, R10‖C8, R11, R9, (bpX, R12), R13, six inter-electrode capacitances); output
  v_oc(k2b); supply current through R12 and V2b's plate; DC start as B12.
Consequences: B3 block-exact vs its testbench passes (M3 report). B3 is kept as a verified block; the engine
chain uses B34 (D-033).

## D-033 Merge B3 and B4 into B34 (no J3 junction)
Date: 2026-10-13 · Status: accepted (owner decision, PROGRESS.md Q14); the engine chain became one tree in
D-038 (B34 stays as a verified block)
Context: First M3 cascade runs: the SPICE chain b12 → b3 → b4 (J-Thevenin J3 with R_th3 = 360 Ω, B3's
output k2b unloaded) differed from the full netlist at y by −2.2 to −7.5 dB ESR from 0.1 V input upwards
(−18 dB even at 0.01 V with Preamp Volume 1.0), while the engine matched that chain to −60 dB. On large
swings V2b cuts off into the tone stack; its output resistance jumps from 360 Ω to R13 = 100 kΩ and F's
k2b swing is 179 V pp against 248 V pp unloaded (1 kHz 0.1 V, Volume 0.6). SPICE variants at y (1 kHz
0.01 / 0.1 / 0.5 / 2 V at Volume 0.2 / 0.6 / 1.0, plus 100 Hz and 5 kHz at 0.1 V):
J3 as is −2.2 … −64.6 dB (−2.2 to −7.5 from 0.1 V up); B3 with an AC-coupled 43.6 kΩ load (|Z_in(1 kHz)|
of the tone stack) driving B4 −10.7 to −18.5 dB from 0.1 V up; B3 and B4 joined −27.7 to −42.5 dB.
SPEC §4.1 step (a), the reactive part of the source impedance, does not address a cutoff nonlinearity.
Options presented: merge; keep J3 and report; fixed load; analyse more first.
Decision: B3 and B4 form one block B34, `StageV2aV2bToneStack`: the tone stack and master are part of B3's
linear network under the V2a/V2b root (`engine/src/stage34/Stage34Network.h`: 13 nodes, 24 element ports).
Treble, Middle, Bass and Master are smoothed as in D-027, and the matrices are recomputed every
kParamUpdateInterval = 32 samples while one moves. Output v_oc(y); R_th4 by the D-015 formula (unchanged).
The engine cascade is `Preamp`: B12 → B34. `StageV2aV2b` and `ToneStackMaster` stay as verified blocks
with their own testbenches (b3, b4); the SPICE testbench `b34` (B3 and B4 joined, J2 source, y open) is
the block-exact reference of B34, and the P chain of the cascade study is b12 → b34.
Consequences: Remaining junctions upstream of the PI: J2 (p1b) and J4 (y). A tone-knob update recomputes
the larger matrices (cost in the M3 report). SPEC §4.2 / §4.3, CIRCUIT.md §1 / §4 and VERIFICATION §4
updated.

## D-034 Newton predictor clamp, trend reset and asymmetric step limits
Date: 2026-10-13 · Status: accepted; refines D-029 and the WDF_DESIGN §4 step limits
Context: B34 (D-033) at 5 kHz 10 V / 50 V (B3's hot / overdrive levels) had 142 / 13853 Newton failures and
diverged at 50 V (ESR +220 dB). Traces: (1) at V2b's exit from cutoff the extrapolated grid voltage
overshot to +13.7 V against a solution of −1.7 V (the grid port had risen 17 V/sample while the cathode
was held by the tone stack); (2) repeated failures fed the extrapolation, which grew without bound; (3) in
deep cutoff V2b's grid port has to fall by tens of volts per sample, and the 2 V grid limit (applied as a
uniform step scale) shrank every port's step. With maxIter 100 the 50 V case converges (−37.7 dB, max 30
iterations), so the loss was the solver giving up, not the discretisation alone. Measured (failures /
max iterations; ESR):

| Variant | B34 5 kHz 10 V | B34 5 kHz 50 V | B3 5 kHz 50 V | B12 5 kHz 2 V |
|---|---|---|---|---|
| D-029 as in M2 | 142 / 12 | 13853 / 12 (+220 dB) | 0 / 9 | 0 / 10 |
| start from the lower-residual of extrapolation and warm start | 691 / 12 | 3415 / 12 (−24.7 dB) | 4 / 12 | 4213 / 12 (−7.9 dB) |
| extrapolation clamped to the step limits, trend reset after a failure | 27 / 12 | 6433 / 12 (−15.8 dB) | 0 / 8 | 0 / 11 |
| + grid down-limit unlimited | 27 / 12 | 4483 / 12 (−17.2 dB) | 0 / 8 | 0 / 11 |
| + grid up-limit 5 V | 0 / 9 | 566 / 12 (−37.7 dB) | 0 / 6 | 0 / 9 |
| + grid up-limit 10 V (chosen) | 0 / 8 | 0 / 11 (−37.7 dB) | 0 / 6 | 0 / 10 |
| grid up 2 V, no step halving | 92 / 12 | 4372 / 12 (−16.7 dB) | 0 / 6 | 0 / 7 |

Decision: the extrapolated step of each port is clamped to its step limit; after a failed solve the trend
restarts at zero; `MultiportNewton` has separate upward / downward limits (`maxStep`, `maxStepDown`); the
triode blocks use grid ports +10 V / −1000 V and plate ports ±20 V per iteration. maxIter stays 12.
The remaining B34 error at 5 kHz 50 V (−37.7 dB at 384 kHz; −46.0 / −55.0 dB with the engine at 2× / 4×
the rate on the same input) is the fixed-step discretisation of V2b's fast cutoff transitions into the
tone stack (p2a: a few pF against R12, τ ≈ 0.5 µs < T = 2.6 µs).
Consequences: B12, B3 and B34 reports regenerated with these settings (M2 values change only in the Newton
statistics; converged solutions are the same).

## D-035 Verification and default internal rate 768 kHz
Date: 2026-10-13 · Status: accepted (owner decision, PROGRESS.md Q16); supersedes D-007
Context: In the M3 cascade at 384 kHz, 5 kHz sines at 0.5–2 V guitar input (B12's output slewing up to
57 V per sample) gave E vs P −21 to −33 dB at y with 1376–2630 Newton failures per signal. With up to 200
iterations everything converged (max 31) but the error stayed (−24 dB): the fixed-step discretisation
did not resolve V2a / V2b cutoff transitions (p2a: a few pF against R12, τ ≈ 0.5 µs). The engine at 2× /
4× the rate gave −31 to −39 / −33 to −44 dB. Options presented: maxIter 32 and report; oversample the
preamp 2× internally; keep the limit and report; verify at 768 kHz.
Decision: `harness/config.py: FS = 768000`; signals, SPICE references (step 1/768000 s, breakpoints at
every sample) and engine runs at 768 kHz. `EngineConfig::sampleRate` defaults to 768 kHz; the plugin's
default oversampling becomes 16x at 44.1/48 kHz, 8x at 88.2/96 kHz, 4x at 176.4/192 kHz (SPEC §3.4, M5).
kParamUpdateInterval stays 32 samples (≈ 42 µs). ampwav accepts up to 3.072 MHz (D-013) for the 4×
discretisation checks. All M1–M3 references re-rendered; the M0 reports stay as recorded (384 kHz).
Consequences: SPICE time about doubles. The per-sample budget halves to 1.3 µs at 48 kHz × 16: the M3
report gives the block costs against it; the M6 optimisations (WDF_DESIGN §4) become necessary for real
time. The R-type fixtures are regenerated at 768 kHz. The harness RC reference-accuracy test uses the
configured RELTOL (1e-8: −127 / −104 dB at 5 / 15 kHz at 768 kHz; RELTOL 1e-6 would give −73 dB).

## D-036 M3 partition scoring at the loaded master wiper
Date: 2026-10-13 · Status: accepted (owner decision, PROGRESS.md Q15)
Context: SPEC §7 M3 compares the cascade's v_oc(y) (no PI load) with F's y, which carries the PI load
behind the master pot's high source impedance. At 5 kHz the load alone gives −19 to −24 dB. Driving the
b56 testbench (J4 source R_th4 into the PI, power stage and supply) with the SPICE chain's v_oc(y) and
comparing its loaded y with F gave −25.1 to −64.4 dB on 15 tested points (384 kHz). Options presented:
score the loaded y; score v_oc(y) as in SPEC; defer y to M4.
Decision: `python -m harness.cascade` scores (tolerances.py, block `preamp`):
- partition error: P56 vs F at the loaded y (P = b12 → b34 SPICE chain on F's recorded supplies; P56 =
  b56 driven by P's v_oc(y)), ESR ≤ −26 dB on sines, ≤ −20 dB on DI (VERIFICATION §7);
- engine: E (ampwav `preamp` on F's recorded supplies) vs P at y with the block-exact limits by level
  (≤ −40 dB; 2 V sines ≤ −30 dB; harmonics 2–5 within 0.5 dB) and the Newton checks.
P vs F and E vs F at the open-circuit y, the speaker voltage of P56 and the loaded p1b / k2b are reported.
Consequences: The engine's own loaded y (with B5) is compared with F in M4. VERIFICATION §7 updated.

## D-037 Triode step limits at 768 kHz: grid +50 V / −1000 V, plate ±50 V
Date: 2026-10-13 · Status: accepted; refines D-034
Context: At 768 kHz (D-035) the preamp cascade still had Newton failures at 5 kHz 2 V (B34; 919 at
Volume 0.6, 442 at Volume 1.0 in 0.15 s). Trace: when V2a cuts off, p2a rises 136 V within one sample and
V2b's plate port falls by about 100 V; with ±20 V plate steps (and V2b's grid port climbing out of deep
cutoff in +10 V steps) the iteration needed more than 12 steps. Measured (preamp cascade, total
failures over 5 kHz 2 V at Volume 0.6 and 1.0, 5 kHz 0.5 V, SWP-hi and 1 kHz 2 V; converged results equal):
plate ±20 V: 1361; ±50 V: 371; ±200 V / unlimited: 412; plate ±50 V with grid up +20 V: 25, +50 V: 0,
unlimited: 0.
Decision: grid ports +50 V / −1000 V, plate ports ±50 V per iteration (`devices::TwoTriodes::solver`).
Checked on the hard block cases (B12 5 kHz 2 V, B3 / B34 5 kHz 50 V): 0 failures, max 7 iterations,
ESR unchanged. Preamp cascade: 0 failures, max 12 (Volume 1.0, 5 kHz 2 V).
Consequences: B12, B3, B34 reports and the cascade regenerated with these limits.

## D-038 Merge B12 and B34 into one preamp tree B1234 (no J2 junction)
Date: 2026-10-13 · Status: accepted (owner decision, PROGRESS.md Q18)
Context: M3 cascade at 768 kHz (D-035, D-036): the partition error at the PI-loaded y failed once, Preamp
Volume 1.0, 5 kHz 2 V, −22.1 dB (target −26 dB), with margins of 1–2 dB elsewhere at 5 kHz. SPEC §4.1
upgrade path: (a) the reactive part of R_th2 (|Z| 89.7 → 85.6 kΩ over 1–10 kHz, not a large-signal
change); (b) merge. SPICE study of (b) (`python -m harness.merge_study`,
`harness/reports/M3/20261013_merge_study.md`, 36 sines, recorded supplies): worst loaded-y error with J2
−30.0 / −27.0 / −22.1 dB, merged −45.9 / −36.5 / −36.1 dB at Preamp Volume 0.2 / 0.6 / 1.0; speaker output
−31.2 / −28.0 / −27.3 → −47.1 / −41.3 / −41.0 dB.
Decision: the engine's preamp `Preamp` is one tree B1234 (`engine/src/stage1234/Stage1234Network.h`: 20
nodes, 38 element ports = B12's 15 + B34's 23 without J2) under an 8-port root of V1a, V1b, V2a, V2b
(`devices::FourTriodes`, the D-037 step limits per port). Preamp Volume and the tone knobs are smoothed;
the matrices are recomputed every kParamUpdateInterval = 32 samples while one moves. Supplies bpA and bpX
are settable (B7 in M4); R_th4 by D-015. SPICE testbench `b1234` (B12 + B3 + B4 joined, y open) is its
block-exact reference and the cascade study's P. `StageV1aV1b`, `StageV2aV2b`, `ToneStackMaster` and
`StageV2aV2bToneStack` stay as verified blocks with their testbenches.
Consequences: Only J4 (y) remains before the PI. Cost: 1.25–1.83 µs per sample at 768 kHz (above the 1.3 µs
budget on its own; B12 + B34 separately were 0.81–1.23 µs); see the M3 report. SPEC §4.2 / §4.3, CIRCUIT
§4, VERIFICATION §4 updated.

## D-039 Engine vs SPICE at 5 kHz 0.5 V in the preamp: accepted as a known deviation
Date: 2026-10-13 · Status: accepted (owner decision, PROGRESS.md Q17)
Context: In the M3 cascade at 768 kHz the engine missed the block-exact "hot" limit (≤ −40 dB) at y on the
5 kHz 0.5 V sine: −39.3 dB (Volume 0.6) and −38.4 dB (Volume 1.0). The cause is the fixed-step
discretisation of V2a / V2b cutoff transitions (as Q16): in the 384 kHz study the same case went
−33.1 / −39.4 / −44.2 dB at 1× / 2× / 4× the rate; Newton converges. Options presented: run the preamp at
2× the rate internally (about −44 dB, double cost) or accept and report.
Decision: accept and report. The tolerance stays as written; the cascade report keeps the check as an
explained FAIL (the numbers after the B1234 merge, D-038, are in the M3 report).
Consequences: none on the engine. Revisit if the M6 optimisations make a higher internal rate affordable.

## D-040 EL34 model, OT as a coupled-inductor element, supply discretisation
Date: 2026-10-13 · Status: accepted
Context: M4 steps 2, 3, 6 (PROGRESS.md plan): SPEC §5 EL34, CIRCUIT §3 OT, WDF_DESIGN §7 supply.
Decision:
- EL34 (`devices/Pentode.h`): WDF_DESIGN §3.2 literally (Vs' = max(Vsk, 1 mV) with d/dVsk = 0 below the
  clamp, atan in Ip unclamped, tubes.lib's 1.5707963); grid diode Rs = 1001 Ω; `count` = 2 for the matched
  pair. `tests/test_pentode.cpp`: 1464 plate / screen points and 5 grid points within 0.1 % / 1 nA, the
  analytic Jacobian within 1e−5 of central differences (∂Is/∂Vpk skipped at the Vpk = 0 kink of
  max(Vpk, 0)).
- OT: the three windings are one coupled element (WDF_DESIGN §5.6): wave elements b = −a[n−1] with the
  port conductance matrix (T/2)·L⁻¹ in closed form, stamped in the MNA with `mna::Mutual` (no ideal
  transformer). `python -m harness.ot_fixtures` writes SPICE AC and transient fixtures of the transformer
  network alone (CIRCUIT §3); `tests/test_ot.cpp` passes the linear-block limits at out, pA, pB (AC) and
  −123 dB at out (transient). The transient fixture fades its drive in over 0.5 ms: a drive with a di/dt
  step at t = 0 is inconsistent with the zero initial state and the trapezoidal rule alternates at fs/2
  at the plates.
- Supply (`Supply`, B7): five states, trapezoidal with the previous sample's load currents held over the
  step (SPEC §4.3); DC by solving A·x = −(B·u + c). Matches the SPICE operating point to 1 µV
  (`tests/test_supply.cpp`).
Consequences: B6 uses `CoupledInductors<3>`; the coupling of B6 and B7 through the centre tap is checked
by the M4 clean-power and sag acceptance (WDF_DESIGN §7).


## D-041 Merge B5 and B6 into one tree B56 (no J5 junction, NFB without delay)
Date: 2026-10-13 · Status: accepted (owner decision, PROGRESS.md Q19)
Context: M4 step 1 (`python -m harness.partition_j5`, testbench `b56j5`, Presence 0): with J5 as J-Thevenin
sources the speaker voltage against the joined b56 was −23 to −26 dB on 0.1–1 V sines for R_th = |Z_self|
(the PI plates are cross-coupled, Z_ab = 32.1 kΩ, Z_ba = 25.4 kΩ at 1 kHz; small-signal gain 0.45 dB low),
−46 to −68 dB for R_th = Z_self − Z_mutual, but −13.7 to −21 dB at 20 V for both (a PI half cuts off, the
EL34 grids conduct). The 2×2 source could not be rendered (ngspice "timestep too small" at node y).
Options presented: keep J5 with Z_self − Z_mutual; 2×2 source; merge; finish the study.
Decision: `PowerAmp` (B56): the PhaseInverter and PowerStage elements in one tree (`engine/src/stage56/`: 21
nodes, 42 element ports, R22 a resistor nG – out inside the tree) under a 10-port root (V3a, V3b, both EL34
pair equivalents; `devices::TriodesPentodes`). `Amp2203` = Preamp → PowerAmp, Supply once per sample.
`PhaseInverter` (b5open) and `PowerStage` (b6) stay as verified sub-blocks. The delayed-NFB check of SPEC
§7 M4 no longer applies (no delay; D-006 superseded for the engine chain).
Consequences: full amp at default settings, idle machine, measured in the M4 report. First acceptance run
(engine vs F): NFB gain closed / open within 0.02 dB, overdrive ESR −68.0 dB, sag 4 mV, CLEAN 97.3 W vs
100.2 W, THD +0.16 pp.

## D-042 Power-stage Newton: free plate / screen steps, two halvings, source-stepping fallback
Date: 2026-10-13 · Status: accepted
Context: at Master 1.0 / Presence 1.0 with 1.9–2.2 V guitar peaks the OT flyback moves an EL34 plate by
hundreds of volts per sample (SPICE: plates +1588 V / −462 V; the engine reproduces them). With the triode
step limits (±50 V) and four halvings the power-stage solve stalled (1/16 steps) and 1469–3157 of 23040
samples (30 ms) did not converge in 12 iterations. Measured (merged B56, 30 ms, 2.18 / 1.0 V at max
settings): limits ±50 V, 4 halvings: 3157 / 2805 failures; 2 halvings: 3136 / 2782; plate / screen free,
2 halvings: 77 / 12; free, 3 halvings: 187 / 74.
Decision: EL34 ports: grid +50 V / −1000 V, screen and plate unlimited, at most 2 halvings
(`devices::TwoPentodes::solver`, also used for the pentode ports of B56; the triode blocks keep D-037).
`stage::RootNetwork` gains a fallback for any failed solve: from the previous sample's solution the
Thevenin voltages move to the new ones in 2, then 4, then 8 substeps (bounded: at most 14 more solves).
The first solve's failure is still counted in the Newton statistics (`newton_failures`, scored as before);
`newton_rescued` / `newton_unrescued` report the fallback (ampwav --stats, Amp2203::Stats; D-012 addition).
Consequences: blocks that never fail are unchanged bit for bit. At the max settings above all failed
solves were rescued (0 unrescued); those samples cost more (6.3–6.6 µs per sample for the full amp there).

## D-043 Adaptive half-steps in the power amp (B56)
Date: 2026-10-13 · Status: accepted (owner decision, PROGRESS.md Q20)
Context: M4 block-exact at 5 kHz 20 V (J4 source): B56 Newton median 5 (≤ 3); B6 harmonics 2–5 1.48 dB
(≤ 0.5), median 4. The engine at 2× / 4× the rate: B56 median 5 → 3 → 2, ESR −37.8 → −46.5 → −52.5 dB; B6 H5
difference 1.48 → 1.08 → 0.33 dB; predictor variants (none / linear / quadratic) did not bring the median to
≤ 3. Options presented: accept; B56 at 2× throughout (≈ +1.6–2.0 µs per sample); the whole engine at
1.536 MHz (≈ 2×); adaptive half-steps (unmeasured).
Decision: `PowerAmp` computes a sample as two steps of T/2 when any root Thevenin voltage moves by more
than kDefaultHalfStepTrigger = 100 V from the previous sample. A second set of root matrices at T/2 is kept
(capacitor port conductances ×2, the OT's (T/2)·L⁻¹ ×1/2; updated with every knob update). The trapezoidal
states are converted exactly when the step changes, keeping each port's voltage and current: z' = v + r·(z
− v), r = R'/R (`DcCapacitor::rescale`, `CoupledInductors::rescaleState`; the reflected waves of the sample
are restored first, since reflected() has overwritten them). The J4 source takes the linearly
interpolated midpoint (as SPICE sees the input); the supplies are held. Every solve is counted in the Newton
statistics. Measured, b56 SIN-5000Hz-20V, trigger off / 1e−9 (every sample) / 100 / 50 / 20 / 10 V: ESR
−37.8 / −46.5 / −45.6 / −45.9 / −46.2 / −46.4 dB, median 5 / 3 / 3 / 3 / 3 / 3, solves per sample 1.00 / 2.00
/ 1.20 / 1.24 / 1.41 / 1.62, 4024 / 5650 / 3573 / 3718 / 4192 / 4743 ns per sample; other signals: 100 V
triggers on 0–37 % of the samples (5 kHz 5 V: 1.37 solves per sample, −65.0 → −66.3 dB), ≈ 6 % overhead
where it never triggers. Every-sample half-steps reproduce the engine at 2× the rate (−46.5 dB both).
Consequences: b56 block-exact 106 PASS, 0 FAIL; full-amp acceptance unchanged (107 PASS); full amp 2.67 µs
per sample (was 2.48). Max settings (Master / Presence 1.0, 2.18 / 1.0 V input): first-solve failures 77 →
18 / 12 → 0 in 30 ms, all rescued, 4.0–4.3 µs per sample there (was 6.3–6.6). The standalone `PowerStage`
(B6, a verified sub-block not used in the chain) has no half-steps; its three 5 kHz 20 V FAILs remain,
explained as discretisation by the rate study.

## D-044 JUCE plugin structure, offline render tool, M5 checks
Date: 2026-10-13 · Status: accepted
Context: SPEC §7 M5, §6.3; PROGRESS.md M5 plan.
Decision:
- `Amp2203Processor` (plugin/src): APVTS parameters per SPEC §6.3 (knobs 0–10 shown, rotation = value / 10;
  Interface Level 0..+24 dBu, default +12, sets `Amp2203::setInputVoltsPerFullScale` (new real-time setter,
  D-012 addition); Input Trim ±24 dB and Output Level −48..+12 dB as smoothed gains in the wrapper);
  Oversampling Auto / 2x / 4x / 8x / 16x (Auto = SPEC §3.4 by host rate; replaced by D-048); Oversampling Filter linear-phase
  FIR or polyphase IIR (`juce::dsp::Oversampling<double>`, maximum quality, integer latency). Latency via
  `setLatencySamples` (FIR 67, IIR 6 samples at 16x / 48 kHz). Mono: the left (or only) input channel, the
  output on every channel. A factor or filter change rebuilds on the message thread (10 Hz timer,
  `suspendProcessing`). Double-precision processing supported. State through the APVTS.
- Minimal editor: nine rotary sliders, two combo boxes (APVTS attachments).
- `amp2203_render` (JUCE console app, plugin/render): the processor offline, and the oversampler's up / down
  stages alone; `python -m harness.plugin_null` compares the plugin with down(ampwav full(up(x))).
- `amp2203_bench` (default mode): 60 s of the first DI clip, interpolated linearly to 768 kHz while running.
- pluginval 1.0.4 (the owner's installed app, /Applications/pluginval.app) and macOS auval; the plugins are
  installed in ~/Library/Audio/Plug-Ins/{VST3,Components} by copying the build artefacts (owner decision:
  install and keep).
- `python -m harness.automation`: every knob at audio rate (10 Hz triangles, 0 ↔ 1 jumps) on the engine at
  768 kHz. ampwav `--knob-ramp` now follows each knob's latest started ramp (piecewise-linear trajectories;
  single ramps unchanged).
Consequences: see the M5 report (`harness/reports/M5/20261013_M5_report.md`).

## D-045 Power-amp Newton iteration limit 16
Date: 2026-10-13 · Status: accepted (owner decision, PROGRESS.md Q21)
Context: M5 automation (every knob at audio rate, `harness/reports/M5/20261013_automation.md`): 6367 power-amp
first solves did not converge in 12 iterations (all rescued by the D-042 fallback); with static knobs at high
Master too (5 s of DI with every knob at 1.0: 865). A limit of 16 / 20 gave 10 / 7 at the same cost. Options
presented: raise the limit; count rescued solves as converged; both; keep.
Decision: `PowerAmp` uses `TriodesPentodes::solver(16)` (maxIter 16; the DC solve keeps 500, every other block
12). The Newton maximum check ("≤ the configured limit") is 16 for b56 and the full amp
(`harness/tolerances.py` OVERRIDES); first-solve failures still count as failures.
Consequences: M4 / M5 reports for b56 and the full amp regenerated with the new limit.

## D-046 Solver and device optimisations without accuracy change (M5, Q22)
Date: 2026-10-14 · Status: accepted (owner decision Q22: optimise the solver first)
Context: M5 real-time factor 0.42 at 48 kHz × 16 (target ≥ 10); profile in the M5 report.
Decision (each step checked against the previous engine on fixed inputs: DI 2 s at default / every knob 1.0,
full amp 5 kHz 0.5 V, b56 5 kHz 20 V):
- `RootNetwork::openCircuit` / `solve`: the power amp's half-step trigger reuses the open-circuit product
  instead of computing it twice (bit-identical).
- Block-diagonal device Jacobians (`solver::BlockDevice`: per column the rows that can be nonzero): J = I +
  Z·∂i/∂v from those entries only, and the devices no longer clear the whole matrix (bit-identical).
- `solveLinear`: one reciprocal per pivot, no update of the eliminated column (outputs within 2.3e-8 V at
  default knobs, 1.2e-6 V at every knob 1.0; ESR against the previous engine −169 to −231 dB).
- Devices: the grid diode's constant part of z precomputed (`GridDiode` constructor; the device instances
  are `inline const` instead of `constexpr`), softplus and sigmoid from one exponential (same tolerance as
  above; the device unit tests pass unchanged).
- Tried and rejected: chord Newton (the first iteration's factorisation reused within the sample): 858 to
  61994 first-solve failures on the four cases, ESR −15 dB at every knob 1.0.
Consequences: `amp2203_bench` on 60 s of DI 3114 → 2436 ns per engine sample (real-time factor 0.42 → 0.53);
Preamp 1263–1857 → 1111–1586 ns, PowerAmp 1699–2286 → 1181–1673 ns (5 kHz 20 V 3558 → 2593 ns).

## D-047 SIMD-friendly network products; rate study at 384 kHz
Date: 2026-10-14 · Status: accepted (owner direction after Q22: SIMD + tables, also study lower rates)
Decision: `RootNetwork` computes the open-circuit and node products with a four-accumulator dot product
(vectorised by the compiler; rounding differs at the last bit: ESR vs the previous engine −169 to −223 dB on
the fixed checks). Tried and not kept: two-accumulator residual (no gain at K = 8–10), full-row LU updates
(no gain). The harness rate is overridable (`AMP2203_FS`); `AMP2203_FS=384000 python -m harness.m4_acceptance`
writes `harness/reports/M5/20261014_full_acceptance_fs384000.md`: 107 PASS, 0 FAIL (DI −60.8 to −62.7 dB;
768 kHz −61.0 to −63.0 dB). Plugin real-time factor on 20 s of DI (idle machine): 16x 0.55–0.61, 8x
0.97–1.17, 4x 1.71–2.27. At 192 kHz (`20261014_full_acceptance_fs192000.md`): 102 PASS, 5 FAIL (1–2 Newton
first-solve failures on five DI runs, 2 of them not rescued by the fallback); DI ESR −54.4 to −57.1 dB, sines and NFB as at 768 kHz.
Consequences: the earlier "8x vs 16x render" comparison (−42 to −47 dB) mixed two oversampling filter chains
and is not a measure of the engine's accuracy; the SPICE comparison at each rate is.

## D-048 Plugin oversampling 2x / 4x / 8x / 16x, default 8x
Date: 2026-10-14 · Status: accepted (owner decision after D-047)
Context: rate study D-047: at 48 kHz, 8x (384 kHz) passes the full-amp acceptance as 16x does (107 PASS, DI
−60.8 to −62.7 dB) at about twice the real-time factor (0.97–1.17 vs 0.55–0.61); 4x (192 kHz) 102 PASS /
5 FAIL, 1.71–2.27.
Decision: the Oversampling parameter offers 2x, 4x, 8x and 16x (choice index 0 / 1 / 2 / 3), default 8x; Auto
is removed, and the factor no longer depends on the host rate. `amp2203_render --factor` accepts 2 / 4 / 8 / 16
(default 8). (2x was first removed with Auto and restored the same day at the owner's request.)
Consequences: at host rates above 48 kHz the engine runs above the verified rates (8x at 96 kHz = 768 kHz,
real-time cost doubled; at 192 kHz 1.536 MHz, not verified). 2x (96 kHz inside at 48 kHz) has no SPICE
verification; the M5 measurement at 2x showed Newton failures not rescued at Master + Presence 10 (M5 report,
Q22 table). States saved with the old choice list restore to the next higher factor (old index 0 Auto → 2x,
1 2x → 4x, 2 4x → 8x, 3 8x → 16x; 4 clamps to 16x).

## D-049 M6 performance: exact-output changes, half-rate power amp as a plugin option, Newton tolerance kept
Date: 2026-10-14 · Status: accepted (owner decisions Q23, Q24)
Context: M6 scope performance (owner). Baseline at the 8x default (384 kHz): 2270 ns per engine sample,
real-time factor 1.15 (Master + Presence 1.0: 0.83). Measurements in PROGRESS.md M6 notes.
Decision:
- Kept: the elimination in `solver::solveLinear` unrolled at compile time (bit-identical); plain-resistor
  ports (wave always 0, no supply offset) left out of the open-circuit product (`stage::plainResistors`,
  `RootNetwork::updateMatrices` zeroWave; rounding-level change). Together −2 to −4 %.
- Newton residual tolerance stays 1e-9 V (WDF_DESIGN §4.3). Studied (`--newton-tol`, acceptance at 384 kHz
  107 PASS at 1e-7…1e-4 V; −12 / −22 / −30 % time at 1e-6 / 1e-5 / 1e-4); owner: keep 1e-9.
- Power amp at half rate (`EngineConfig::powerAmpRateDivisor`, halfband FIR 35 taps / Kaiser β 10, 34 samples
  delay at the engine rate reported by `Amp2203::latencySamples()`): a plugin parameter "Power Amp Rate"
  (Full / Half, default Full); the plugin adds the delay to its latency. At 384 kHz: −24 to −26 % time;
  acceptance 107 PASS, DI ESR −55.2 to −57.6 dB (full rate −60.8 to −62.7).
- Not adopted: build flags (≤ 3 %), LU reuse after small steps (≤ 5 %, output moves from a 1 V threshold),
  inline polynomial exp / log (+24 % time), two threads (owner: not chosen).
Consequences: SPEC §6.3 lists Power Amp Rate. The HOT preset study (PROGRESS.md M6) found the J4 partition
error at high Master, independent of these changes.

## D-050 PyTorch 2.14.1 for the neural-model experiment (X3)
Date: 2026-10-14 · Status: accepted (owner decision: "PyTorch 2.14.1")
Context: ARCHITECTURE_STUDY.md §6 X3 trains a small recurrent network on the engine's output.
Decision: `torch==2.14.1`, pinned in `requirements-ml.txt` (separate from `requirements.txt`; the harness and
the engine do not need it). Inference cost is measured with hand-written C++ in the experiment, so the engine and
plugin have no new dependency.
Consequences: `pip install -r requirements-ml.txt` in the venv before `python -m harness.x3_neural`.


## D-051 One ESR tolerance of −20 dB for every ESR check, engine and real-time plugin model
Date: 2026-10-15 · Status: accepted (owner decisions: "change the acceptance level to what the literature say
−17 to −27 dB ESR"; threshold "−20 dB"; engine block tolerances "loosen them too")
Context: M6 experiments X3 / X4 (neural models trained on the engine) and the owner's aim of "good enough" tonal
accuracy at a low CPU and memory cost. The literature on real-time amp models reports ESR −17 … −27 dB with good
listening scores (Wright et al. 2019/2020; ARCHITECTURE_STUDY §2.4, measured there against hardware recordings,
here against SPICE). The whole-amp DRIVE / CRUNCH check was already −20 dB; the engine's per-block checks were
−40 / −30 / −60 dB.
Decision: every ESR tolerance in VERIFICATION §7 is −20 dB: linear block (was −60), nonlinear block Small / Nominal
/ Hot (was −40), Overdrive (was −30), knob jump (was −40), partition at y, sines (was −26); DI partition and the
whole-amp DRIVE / CRUNCH check stay −20. New check `plugin_model_esr_db`: a real-time plugin model (any
architecture) vs SPICE, held-out DI clips at CRUNCH, DRIVE and HOT, 48 kHz band, first 0.1 s excluded, ≤ −20 dB.
Tolerances that are not ESR (DC, magnitude, harmonics, power / THD, NFB, stability, Newton) are unchanged.
Consequences: the engine's per-block checks flag only errors above −20 dB; an error such as the J4 partition error
at HOT before X1 (−13 dB) is still caught, smaller ones (e.g. −25 dB) are not. Earlier reports keep their recorded
tolerances; reports generated from now on score against −20 dB. Against the new check: X3 (CRUNCH only) −16.5 …
−17.1 dB, X4 −3.0 … −7.1 dB, the engine −39.6 … −56.1 dB.

## D-052 Newton iteration limit 32 in the full amp's roots (preamp B1234, power amp B56)
Date: 2026-10-15 · Status: accepted (owner: "run 1 and 2", Newton convergence at 2x / 4x)
Context: M6 X2 / A1: at 2x / 4x of 48 kHz the preamp root failed to converge in 12 iterations (DI: up to 80
first-solve failures and 29 unrescued per 5 s clip at 2x; loud 5 / 9 kHz sines up to ≈ 140 000 per 2 s), the power
amp at HOT in 16 (rescued). Diagnosis (preamp, scratch builds): step limits ×4 left the DI failures unchanged; maxIter
64: every case converged, the DI clips in ≤ 21 iterations at 2x and ≤ 17 at 4x (median 2), the sines in ≤ 47–60.
Failing at 12 was also the expensive path: the fallback runs up to 14 more solves.
Decision: `Preamp` and `PowerAmp` use maxIter 32 (`FourTriodes::solverK(32)`, `TriodesPentodes::solverK(32)`); the
standalone sub-blocks keep 12. VERIFICATION §7 "max ≤ configured limit" follows (32 for b1234, b56, full).
Consequences: samples that converged in ≤ 12 / 16 iterations are unchanged bit for bit. Full acceptance at 384 kHz,
exact J4: 107 PASS (`…_20261015_full_acceptance_fs384000_j4exact.md`); HOT 21 PASS, 0 FAIL (was 4 FAIL on
newton_failures), DI ESR within 0.08 dB, max 17–19 iterations. At 2x / 4x the DI clips have 0 failures; only loud
9 kHz sines (and 5 kHz at 2x) still fail. Worst case per sample rises (32 iterations before the fallback).

## D-053 Exact J4 with the power amp at half rate: coupling at the power-amp samples, no decimation filter
Date: 2026-10-15 · Status: implemented as an engine option (owner: "run 1 and 2"); not used by the plugin
Context: with `powerAmpRateDivisor = 2` the exact J4 coupling (X1) was switched off (`Amp2203::exactJ4()`), so A1's
half-rate runs used the Thevenin J4 (HOT −13 dB). A same-sample coupling cannot pass through the halfband
decimator (17 engine samples of delay in the loop).
Decision: with exact J4 and divisor 2 the junction is solved exactly at the samples where the power amp runs (the
preamp's linearised Thevenin source at y taken directly, no decimation filter); in between the preamp solves with
the held load current; the power amp output goes through the halfband interpolator as before. Latency 17 engine
samples (was 34 with the decimator). Divisor 1 is unchanged bit for bit.
Consequences (`harness/reports/M6/20261015_x2_rates_a1_div2_exact.md`, maxIter 32): vs SPICE worst HOT −22.5 / −26.3 /
−33.2 dB at 2x / 4x / 8x (PASS; was −13.0), CRUNCH / DRIVE ≤ −38.0 dB; unseen settings vs 16x worst −18.9 / −20.6 /
−26.8 dB; cost 24 / 46 / 89 % of one core (HOT 36 / 63 / 115 %), full-rate power amp 34 / 67 / 129 % (HOT 52 / 92 /
166 %). Without the decimation filter the preamp content above a quarter of the engine rate folds into the power amp:
non-harmonic power on 5 / 9 kHz sines −4.6 … −29.1 dB (full rate at the same engine rate: −6.4 … −36.6 dB).

## D-054 Plugin defaults: 2x oversampling, power amp at half rate, exact J4
Date: 2026-10-15 · Status: accepted (owner: "put half-rate power amp at 2x into the plugin"; listening: half rate 2x
sounds identical to the 16x reference)
Decision: Oversampling default 2x (was 8x, D-048), Power Amp Rate default Half (was Full, D-049); the plugin's
engine always runs the exact J4 coupling (D-053). The other factors and Full stay selectable.
Consequences: plugin CPU (`amp2203_render` now reports processBlock time) on 60 s DI GTR L at 48 kHz: 25.1 % / 24.8 %
of one core (FIR / IIR), HOT 39.2 % / 39.1 %; latency 58 (FIR) / 13 (IIR) samples. Null test at 2x Half: first 25 PASS,
1 FAIL; corrected in D-055 (the FAIL came from the null test's own reference, not from the plugin). Separately measured:
with the power amp at half rate a 1.5e-5 change of the input level (4.361 vs the plugin's exact 4.361064 V per full
scale) flips some D-043 half-step decisions, and the two discretisations then differ locally by up to −27 dB (5 kHz
−40 dBFS); with the trigger off the same change stays at −105 dB. Full rate: 26 PASS. Automation (96 kHz, Half, exact J4): 56 runs,
0 non-finite, 0 Newton failures; 5.5–6.2 µs per engine sample while knobs move (static ≈ 2.5).

## D-055 Plugin: Newton tolerance 1e-4 V; no adaptive half-steps with the power amp at half rate
Date: 2026-10-15 · Status: accepted (owner: "adopt both, use 1e-4"; revises D-049's tolerance for the plugin and D-043
for the half-rate power amp)
Context: half rate 2x (`harness/reports/M6/20261015_x2_rates_half*.md`): Newton tolerance 1e-9 → 1e-4 V leaves every
accuracy figure unchanged (HOT −22.6 dB vs SPICE, unseen settings −19.7 dB) and cuts first-solve failures 40 → 12
(unrescued 31 → 2) and engine cost 24 → 19.4 % (HOT 32 → 27.1 %); half-steps off at 2x: accuracy unchanged, HOT
36 → 32 %, no switch sensitivity (D-054).
Decision: the plugin sets the Newton residual tolerance to 1e-4 V (`kNewtonTolerance`) and turns the adaptive
half-steps off when the power amp runs at half rate (`Amp2203::setAdaptiveHalfSteps`); full rate keeps the D-043
trigger. The engine defaults (1e-9 V, 100 V) stay for verification. ampwav: `--volts-per-fs` (the plugin's exact input
scale, full-scale samples reach the engine unscaled) and `--keep-delay` (keep the half-rate output delay).
Consequences: null test at 2x Half / Full with the plugin's settings, input scale and delay: 26 / 26 PASS, worst
−138.5 dB (the earlier FAIL was the reference's padding of the removed delay). Measured after the decision (static,
every knob 1.0, 5 s DI, half 2x): the power amp's first solves fail 656 times without half-steps (0 with them), all
rescued, 5371 vs 3501 ns per engine sample (tolerance 1e-4); automation at every knob 1.0: up to 1732 rescued
failures per run and up to 10.4 µs per engine sample (was 0 and ≈ 6 µs). Open: owner review of the half-step choice
at extreme settings.

## D-056 Plugin: adaptive half-steps back on with the power amp at half rate
Date: 2026-10-15 · Status: accepted (owner: "turn half-steps back on at half rate"; revises the half-step part of D-055)
Context: after D-055, every knob at 1.0 showed that without half-steps the half-rate power amp's first solves fail
(656 per 5 s, all rescued) and the sample costs more (5371 vs 3501 ns, engine).
Decision: the plugin keeps the D-043 half-steps at both power-amp rates (`setAdaptiveHalfSteps(true)`); the Newton
tolerance stays 1e-4 V (D-055).
Consequences: plugin CPU at 2x Half, 60 s DI GTR L, FIR: default 20.6 %, HOT 32.9 % (28.5 % without half-steps), every
knob 10: 35.6 %, 0 Newton failures (48.8 %, 5777 rescued without). Null test 26 / 26 at Half and Full (worst
−138.5 dB). Automation (96 kHz, Half, exact J4, 1e-4 V, one run at a time): 56 runs, 0 non-finite, 0 failures; ampwav
cost 4.9 / 6.3 / 9.6 / 13.0 µs per engine sample (min / median / 90 % / max, all six knobs jumping 0 ↔ 1 every 50 ms),
which includes ampwav setting every knob each sample (the plugin sets them once per block); the plugin's cost under
automation is not measured yet.

## D-057 Full amp roots: matrix updates in double precision, every 128 samples while a knob moves
Date: 2026-10-15 · Status: accepted (owner: "run 3", both changes; refines D-026 for `Preamp` / `PowerAmp`)
Context: plugin CPU under knob automation (`harness/reports/M6/20261015_plugin_cpu.md` before this change): one knob
moving 45.8 … 67.7 %, all six 84.6 … 87.1 % (static 18.9 %); 76 % of the time in the double-double MNA solve of the
matrix updates (every 32 samples while a knob is smoothed).
Decision: `mna::rootTransfer` and its helpers are templated on the arithmetic (DD default, or double);
`RootNetwork::updateMatrices` takes it as a flag. `Preamp` and `PowerAmp` (the full amp's roots) update in double and
every 128 samples (`kParamUpdateInterval`, was 32). DC solves, the standalone sub-blocks and the R-type fixtures keep
double-double (D-026 unchanged there).
Consequences: plugin CPU (2x Half, plugin_cpu): automation of any knob or all six 18.7 … 24.6 % from the default knobs,
22.8 … 33.5 % from every knob 10 (static 32.5 %), 0 Newton failures. Static accuracy unchanged: full acceptance at
384 kHz exact J4 107 PASS (ESR identical), HOT 21 PASS (DI ESR within 0.14 dB); half 2x vs SPICE HOT −22.4 dB, unseen
settings −18.9 dB; null test 26 / 26 at Half and Full (−138.5 dB); automation 0 failures. Knob jump vs SPICE (768 kHz)
still PASS with less margin: B1234 −39.2 … −55.1 dB (was −50.0 … −59.4), B56 Presence −63.2 dB (was −77.0); which of
the two changes accounts for it is not separated.

## D-058 EL34 model parameters fitted to the Ei-RC data sheet curves (reference model change)
Date: 2026-10-15 · Status: accepted (owner: "Adopt the fitted EL34 in the reference model"; changes `tubes.lib`, A5)
Context: owner listening ("dark", "lacks gain"); M6 EL34 study (`harness/reports/M6/20261015_el34_curve_fit.md`): Koren's
EL34 set deviated from the data sheet by a median 11.5 % on the plate curves and 28.2 % on the transfer curves.
Decision: `tubes.lib` EL34 mu 14.77, ex 1.046, kg1 194.3, kg2 2243, kp 53.82, kvb 40.43 (was 11 / 1.35 / 650 / 4200 / 60 /
24), fitted to 81 digitised curve points (median error 3.7 / 2.7 %) and the sheet's screen currents; the engine's
`devices::Pentode` / `kEL34Pair` take the same values. The bias network (A4) is unchanged.
Consequences: DC (CIRCUIT.md §5): EL34 idle 28.77 mA plate / 3.26 mA screen per tube (was 34.08 / 4.57), B+ 491.3 V (was
488.7), total HT 136.5 mA (162.9). tests/data EL34 sweeps, the block testbenches and the C++ DC fixtures regenerated; J3
re-measured (359.535 Ω). Every SPICE reference that contains the power amp is re-rendered (netlist hash); the
verification reports from before this entry describe the Koren EL34. Expected from the study: output impedance ≈ 20–23 Ω
(was 14–16), less power (clean 87 vs 101 W at 25 mV, clipped ≈ 156–163 vs 182–194 W).

## D-059 Speaker load: representative nominal 16 Ω 4x12 impedance instead of the 16 Ω resistor (reference model change)
Date: 2026-10-15 · Status: accepted (owner: "put the typical speaker load into the reference and engine")
Context: owner listening ("dark", needs Treble / Presence 7); speaker-load study (`harness/reports/M6/20261015_speaker_load_study.md`):
with the D-058 EL34 the closed-loop output impedance is ≈ 20–23 Ω, so a speaker's rising impedance raises the speaker
voltage +3 … +4.7 dB at 100 Hz and +1.8 … +4.7 dB at 5 kHz relative to a resistor. No published Thiele–Small data for the
Celestion G12T-75 was found.
Decision: `Rload out 0 16` → Rspk_e 13 Ω + (Lspk_e 1.0 mH ‖ Rspk_le 45 Ω) + (Rspk_m 37 Ω ‖ Lspk_m 29.4437 mH ‖ Cspk_m
86.0297 µF; 100 Hz, Q 2), the study's "typical" variant. Engine: B56 (`PowerAmp`) and B6 (`PowerStage`) get nodes spk1 / spk2
and six elements; the inductors are one-winding `CoupledInductors` (shorts at DC, trapezoidal, half-step rescaling in B56),
the capacitor a `DcCapacitor`. The plugin output stays the speaker voltage / 60 V. Power metrics keep V²/16 Ω as a nominal
figure. The parameters are representative, not measured.
Consequences: DC unchanged except v(out) 42.4 µV (was 42.6). Every SPICE reference that contains the power amp is
re-rendered; reports before this entry describe the 16 Ω resistor.

## D-060 Plugin default Power Amp Rate: Full (with the speaker load)
Date: 2026-10-17 · Status: accepted (owner: "switch the power amp to full rate at 2x"; revises D-054's Half default)
Context: with the speaker load (D-059) the plugin model check at half rate 2x fails at HOT (−18.8 dB vs SPICE; half-steps off
−19.2 dB), the full-rate power amp at 2x passes (−24.1 dB, unseen settings −20.8 dB vs 16x; engine 26.8 / 40.8 % default /
HOT). The owner's NAM comparison with a real 2203 (PROGRESS 2026-10-17) is at Master 10 / Presence 8, the HOT region.
Decision: Power Amp Rate default Full; Half stays selectable. Oversampling stays 2x; exact J4, Newton 1e-4 V and half-steps
as D-055 / D-056.
Consequences: no rate-change delay (latency = the oversampler's: 49 samples FIR at 2x, IIR less); CPU as measured in
`harness/reports/M6/20261017_plugin_cpu.md`: default knobs 26.4 % (Half 18.9 %), every knob 10 50.2 % (33.2 %), knob automation
32–35 % (≈ 24 %); at the owner's NAM-matched settings 47.3 % (Half 31.5 %). Null test 26 / 26; automation (96 kHz, Full, exact J4,
1e-4 V) 0 non-finite, 20 rescued first-solve failures. The null test and the CPU tool default to Full.

## D-061 EL34: no plate current below the cathode (5 V softplus of the plate voltage; reference model change)
Date: 2026-10-17 · Status: accepted (owner: "approve the tubes.lib change")
Context: with the speaker load (D-059) the power amp at HOT had 37–56 first-solve Newton failures per DI clip at 384 kHz
(more at 768 kHz, 43 unrescued) and the b56 / b6 SPICE references stopped with "timestep too small". Diagnosis
(PROGRESS 2026-10-17): every failure at pair B's plate port during a flyback (pair A +1100 … +3400 V, pair B −700 …
−2600 V), where Koren's unclamped atan(Vpk/kvb) gives a reverse plate current and the screen knee switches at Vpk = 0;
not the iteration budget, tolerance, matrix precision, halvings or the half-step code (every-sample half-steps at fs
reproduce 2·fs to −67.6 dB).
Decision: in `tubes.lib` (EL34 Bp, Bs) and `devices::Pentode`, Vpk enters atan through P = max(Vpk, 0) + 5·ln(1 +
exp(−|Vpk|/5)): the plate current vanishes below the cathode, the cut-off current goes to the screen, and both are smooth
at 0 V. Above +20 V, P = Vpk within 0.1 V (DC operating point unchanged).
Consequences: engine diagnostic: HOT DI 0 power-amp failures at 8x and 16x; CRUNCH / DRIVE unchanged (−218 … −224 dB vs
the previous engine), HOT differs by −35 … −37 dB. tests/data EL34 sweeps regenerated; the pentode test asserts the new
below-cathode behaviour. SPICE references with the power amp are re-rendered.

## D-062 C2 100 pF (V1a plate–cathode) added to the netlist (reference model change)
Date: 2026-10-18 · Status: accepted (owner: "add C2 to the netlist")
Context: the drawing (2203 STD Preamp iss. 4) shows C2 100 pF from V1a's plate to its cathode; it was missing from the
transcription (open item O8, found 2026-10-15). SPICE study: −0.3 dB at 5 kHz, −0.7 … −0.9 dB at 10 kHz into V1b.
Decision: `C2 p1a k1a 100p` in the netlist (B12 partition). The engine adds it to V1a's plate–cathode capacitance port
(`cpk1a_` = Cpk + C2 in `Preamp` and `StageV1aV1b`): the same two nodes, so the parallel combination is exact.
Consequences: DC unchanged. Every SPICE reference containing V1a is re-rendered; earlier reports are without C2.
