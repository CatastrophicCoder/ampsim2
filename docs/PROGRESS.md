# Progress

Claude Code keeps this file current: tick tasks when done, link the milestone report, and note blockers
or questions for the owner. Before starting a milestone, write a short plan under it (steps, files,
risks) and wait for the owner's go-ahead if the plan deviates from SPEC.md.

Legend: [ ] todo · [~] in progress · [x] done · [!] blocked (see note)

## M0 — Skeleton and verification harness
- [x] Top-level CMake, pinned FetchContent (chowdsp_wdf, Catch2, JUCE, Eigen if used); targets build
- [x] `ampwav` CLI (pass-through engine), 64-bit float WAV I/O
- [x] `harness/signals.py`, `render_spice.py`, `run_engine.py`, `compare.py`
- [x] Block testbenches B12, B3, B4, B56(+B7), B5-open, B6, full; DC sanity check vs F (≤ 0.5 %) — b6
  DC deviation at out/sl accepted as known (Q5)
- [x] Junction Thevenin resistances measured and entered in CIRCUIT.md §4 — B1+B2 merged into B12
  (Q6a); |Z(1 kHz)| in junctions.py (Q6b); J5 provisional until M4
- [x] Device DC sweep CSVs generated into `tests/data/`
- [x] CI workflow (macOS): build + tests — first GitHub run green (run 38304157627 on f311402, 52 s)
- [x] M0 report: `harness/reports/M0/20261013_M0_report.md` — awaiting owner review
Plan (2026-10-12; follows SPEC §7 M0 and VERIFICATION §1–8, no deviations intended):

Step 0 — Environment and repo
- Prerequisites: `brew install ngspice` (not installed yet), `.venv` with `requirements.txt` (not created
  yet), and `git init` (the directory is not a repository yet; CLAUDE.md asks for small commits).
- Check `ngspice -v` for XSPICE (`filesource`) support. This decides how recorded signals are driven
  (filesource or PWL include, VERIFICATION §2).

Step 1 — CMake skeleton (`CMakeLists.txt`, `cmake/Dependencies.cmake`, `engine/`, `tests/`, `tools/`, `plugin/`)
- Pin chowdsp_wdf, Catch2 v3, JUCE 8.x and dr_wav (dr_libs) with FetchContent at the latest release tag
  or commit, resolved with `git ls-remote` when I implement this step. Record each pin in DECISIONS.md
  (one entry, D-011). Eigen is not fetched in M0; whether to use it is decided in M1.
- Targets: `amp2203_engine` (static library; pass-through `Amp2203` with the SPEC §3.2 API plus the
  block-class stubs and a debug probe interface for `--dump-node`), `amp2203_tests` (placeholder tests
  plus a WAV round-trip test), `ampwav`, `amp2203_bench` (stub), and `amp2203_plugin` (JUCE, behind
  `AMP2203_BUILD_PLUGIN`, default ON on macOS; the processor just calls the pass-through engine).
- Warnings: `-Wall -Wextra -Wpedantic -Werror` on our own targets only. Third-party code is included
  as SYSTEM.

Step 2 — `ampwav` (`tools/ampwav/`)
- Implement the full SPEC §7 M0.2 command line: `--block`, `--in`, `--out`, `--rate`, repeatable `--param`,
  repeatable `--dump-node`. Reads and writes 64-bit float WAV. Prints timing per sample to stderr
  for the reports.

Step 3 — Python harness (`harness/`, run as a package with `python -m harness.<module>`)
- `signals.py`: generates SIN, SWP-lo/hi, TT, BURST, STEP and DI at 384 kHz. DI is skipped with a
  message when `reference/di/` is empty.
- `netlist.py`: parses the reference netlist and generates testbenches from per-block descriptions
  (elements kept, J-Thevenin sources inserted, supply rails as sources, probes).
- `render_spice.py`: fixed-step transient (`tran` + `linearize` at 1/384000 s), plus `op` and `ac`.
  Renders are cached in `harness/cache/`, keyed by a hash of testbench text, parameters and signal.
- `run_engine.py`: calls `ampwav` on the same signals.
- `compare.py`: computes the VERIFICATION §6 metrics and writes the §8 report (markdown + PNGs, git hash,
  netlist hash) to `harness/reports/M0/`.
- `spice_tools.py` stays as it is. The harness imports it and does not edit it.

Step 4 — Testbenches (`harness/testbenches/`, generated and committed)
- B1, B2, B3, B4, B56+B7, B5-open, B6 and full (= the reference netlist unchanged). This is the
  VERIFICATION §1 set; SPEC §7 says "B1–B7", and B7 lives inside B56+B7.
- Supply rails in B1–B4 are constant sources taken from the F operating point (as allowed for M1–M2).
- DC sanity check: every internal node of every testbench within 0.5 % of F. The result goes in the M0
  report.

Step 5 — Junction resistances: measure R_th1 (later removed, D-017), R_th2, R_th3, R_thA and R_thB with a 1 kHz AC current
  injected at the open-circuited output node (input AC-grounded), plus the DC small-signal value.
  Enter the results in CIRCUIT.md §4.

Step 6 — Device DC sweep CSVs: generate the 12AX7 and EL34 sweeps of VERIFICATION §5.1 into `tests/data/`
  with single-tube SPICE testbenches. They are used from M2 onward.

Step 7 — CI: `.github/workflows/ci.yml` on a macOS runner: configure, build engine + tests + CLI
  (plugin OFF, to keep CI time down), run ctest. Harness steps stay out of CI for now.

Step 8 — M0 report: `render_spice --block full`, then `compare` against the pass-through engine. Large
  errors are expected; this step tests the pipeline. The report also holds the testbench DC
  sanity table and the R_th table. Update CLAUDE.md "Commands" and README as things appear.

Risks / things to watch:
- CMake 4.4.3 is installed. CMake 4 rejects `cmake_minimum_required` < 3.5 in fetched projects. If any
  pinned dependency hits this, I'll use `CMAKE_POLICY_VERSION_MINIMUM` scoped to that dependency
  and note it in DECISIONS.
- B5-open and B6 need waveforms recorded from a B56 run (speaker voltage, PI outputs, supply rails). M0
  only builds and DC-checks them; driving them with recorded inputs is exercised in M4.
- Generating testbenches from the netlist depends on the line-oriented netlist format staying
  stable. The generator fails loudly on any element it does not account for.
- Measuring R_thA/R_thB on the long-tail pair: the injection is done with the other half and the tail
  connected as in F. The method is described in CIRCUIT.md §4.
Notes:
- 2026-10-12 Step 0 done: git repo initialised (branch `main`), ngspice 47 (KLU) installed, `.venv`
  created from `requirements.txt`. `spice_tools.py` operating point reproduces CIRCUIT.md §5 exactly.
  XSPICE `filesource` works in this build, so recorded signals are driven with `filesource` (no PWL
  fallback needed).
- 2026-10-12 Step 1 done: CMake skeleton with all five targets; pins in DECISIONS D-011 (chowdsp_wdf
  at main 43bcd29 by owner's choice, JUCE 8.0.15, Catch2 v3.16.1, dr_wav 0.14.5). A clean configure +
  build (Release, plugin ON) takes ~34 s with zero warnings and produces AU, VST3 and Standalone.
  ctest: 3/3 pass (pass-through silence/identity, chowdsp_wdf divider with the wdft API).
  `ampwav` is a stub until step 2; `amp2203_bench` runs the pass-through (RTF ≈ 990, meaningless
  until real blocks exist). API additions (double `process`, probes) in DECISIONS D-012.
- 2026-10-12 Step 2 done: `ampwav` with the full SPEC §7 M0.2 command line plus `--units`,
  `--block-size`, `--stats` and `--list-nodes` (conventions in DECISIONS D-013). Code lives in
  `ampwav_core` so it is unit-tested by `ampwav_tests` (11 Catch2 cases: parsing, WAV round trips,
  end-to-end runs, error paths) plus two CLI smoke tests; ctest 14/14 pass, no warnings. Checked
  that Python `soundfile` reads and writes the same 64-bit float WAVs (round trip within 2e-15 V).
- 2026-10-12 Step 3 done: harness package (`signals`, `blocks`, `presets`, `spice`, `render_spice`,
  `run_engine`, `metrics`, `tolerances`, `compare`, `testbench_gen` with `full` only) and 14 unittest
  cases (`python -m unittest discover -s harness/tests -t .`). `compare --block full --preset PIPE`
  runs end to end (SPICE render, engine run, metrics, markdown report with plots) in ~11 s.
  Findings, all in DECISIONS D-014: (1) filesource input drifted off the sample grid, a 1.35e-3
  low-pass at 5 kHz; fixed with per-sample breakpoints (input now exact). (2) The reference's own
  accuracy at ngspice's default RELTOL is about −35 dB at `y` and −46 dB at `out` on the full amp at
  5 kHz, i.e. worse than the −40 dB block-exact tolerance; RELTOL is now a harness setting (set to
  1e-6 after Q4). (3) Octave smoothing biased the sweep magnitude; replaced by per-bin Y/X
  with a 50 ms sweep tail. Also observed: on the full amp at default knobs the 10 mV SWP-lo is not
  small-signal (about 1 dB compression around 3–15 kHz; 0.18 dB at 1 mV); this only affects INFO
  magnitude plots for `full`, the scored magnitude check is B4's.
- 2026-10-12 Step 4 done (with two open items): netlist parser, partition check and generator for
  b1, b2, b3, b4, b56 (+B7), b5open, b6 and full (DECISIONS D-015); `python -m harness.dc_check`
  writes `harness/reports/M0/20261012_testbench_dc.md`. b1–b5open match F within 1.2e-9 relative at
  every node. b6 FAILs at out/sl: 0 V vs 39.8 µV in F (4 % against the 1 mV floor), because R22's
  135 µA NFB current into out belongs to B5 (Q5: accepted as known by the owner). R_th1/2/3/5a/5b are PENDING placeholders until
  step 5, and transient runs refuse those testbenches. `compare` now runs on b1; b5open/b6 need
  B56 recordings (M4) and say so. 21 harness tests pass.
- 2026-10-12 Owner decisions: Q4 RELTOL = 1e-6 (`harness/config.py`; cached renders re-render because
  RELTOL is part of the cache key); Q5 b6 out/sl deviation accepted as a known partition effect.
- 2026-10-12 Step 5 measured (DECISIONS D-016, CIRCUIT.md §4): `python -m harness.rth_measure` gives
  R_th DC / |Z(1 kHz)|: J1 74.8 k / 39.3 k, J2 90.1 k / 89.8 k, J3 360 / 360 Ω, J5a 41.4 k / 58.1 k,
  J5b 45.5 k / 56.9 k. On the owner's request, `python -m harness.partition --junction j1` measured the
  J1 partition error in SPICE (b1 → b2 chain vs b12 = B1+B2 joined): worst sine ESR at p1b −11 dB
  (CLEAN) to −17.6 dB (DEFAULT) for the best constant R_th1, caused by B2's input impedance falling to
  54–177 kΩ at 10 kHz. 25 harness tests pass. Open: Q6 (R_th basis, J1 handling), Q7 (ESR and DC).
- 2026-10-12 Q6a: B1 and B2 merged into B12 (`StageV1aV1b`, joint 4-port root; DECISIONS D-017).
  Partition, testbenches (b12 replaces b1/b2; b123 = B12+B3 joined, the J2 reference), block registry,
  ampwav block ids, SPEC §3.2/§4.2/§7 (M2 is now B12, M3 is B3 + cascade), CIRCUIT §1/§4,
  VERIFICATION §1/§4 and WDF_DESIGN §4/§5 updated. J2 re-measured on b12 (90.11 k DC / 89.73 k at
  1 kHz). `harness.partition` now studies J2 (b12 → b3 vs b123): the three R_th2 candidates give the
  same result within 0.1 dB; worst ESR at k2b −30.6 / −27.0 / −26.2 dB (CLEAN / DEFAULT / DRIVE, all
  from 0.5–2 V 5 kHz sines), small-signal error ≤ 0.18 dB (CIRCUIT.md §4).
- 2026-10-12 Step 5 done. Q6b: junctions.py = |Z(1 kHz)| (J2 89.73 k, J3 359.9 Ω, J5a 58.06 k, J5b
  56.93 k; J5 provisional); testbenches regenerated, none PENDING. Q7: `metrics.ac_part` (signal minus
  its own value at t = 0) before ESR, best lag and sweep response in `compare` and `partition`
  (VERIFICATION §6, D-018). 27 harness tests pass; `compare --block b12` runs end to end.
- 2026-10-12 Step 6 done: `python -m harness.device_sweeps` writes tests/data/12ax7_plate.csv (567
  rows), 12ax7_grid.csv (8), el34_plate.csv (1464), el34_grid.csv (5) from single-tube ngspice DC
  sweeps (DECISIONS D-019). A harness test recomputes every row from the WDF_DESIGN §3 equations
  (Koren, Wright-omega grid diode) to 1e-7 relative / 1e-13 A; this found three SPICE details, now in
  WDF_DESIGN §3: ngspice's Vt constants (25.8649170 mV), tubes.lib's 1.5707963 for π/2 in the EL34
  screen current, and ngspice's reverse-bias diode form below −3·Vt (≤ 5e-11 A, not replicated). The
  1 GΩ Rcp is removed from `ip` (kept as `ip_terminal`). 31 harness tests pass.
- 2026-10-12 Step 7: `.github/workflows/ci.yml` (macos-15, checkout pinned by commit, plugin OFF,
  configure / build / ctest; DECISIONS D-020). The same steps on a fresh local clone: configure 4.4 s,
  build 8 s, no warnings, ctest 14/14. The repository has no remote yet, so no GitHub run so far.
- 2026-10-12 Published on GitHub; first CI run green (52 s). Publication prep: drawings removed from
  history, AGPL-3.0, README (D-021).
- 2026-10-12 Step 8 started: the first compare runs found two harness defects, fixed in D-022 (junction
  DC level for standalone signals; vector names that are control-language operators, e.g. nE).
- 2026-10-12 OT from the Hammond 1750U datasheet (Q8, D-023): netlist change approved by the owner;
  `python -m harness.ot_fit` (lpt 25.61 H fitted, kot 0.9997925 from the leakage, DCRs from the
  datasheet, no capacitance). DC moves by ≤ 1.7 V at the plates; NFB unchanged; clean power +2–6 %.
  Testbenches, DC check and junction measurements regenerated; step 8 renders restarted.
- 2026-10-13 Choke from the Hammond 194F datasheet (Q9, D-024): Rch 60 → 115 Ω. Screens −1.2 V,
  preamp supplies −0.24 %, EL34 current −1.7 %, screen-supply resonance better damped, clean power −1.8 %.
  Regenerated again; step 8 renders restarted again.
- 2026-10-13 Step 8 done on 3a47496: compare reports for full (CLEAN, CRUNCH, DRIVE, DEFAULT; DEFAULT
  took 734 s at reltol 1e-6) and PIPE on b12, b3, b4, b56, all 0 FAIL; J2 partition study rerun on the
  final netlist (unchanged: −30.6 / −27.1 / −26.2 dB). M0 report written. M0 stops here for review.

## M1 — B4 tone stack + master
- [x] MNA scattering builder (C++) + numpy reference + cross-check test — C++ in double-double, Python
  reference in exact rationals (Q11, D-026)
- [x] Adaptor sanity tests (series/parallel equivalence, losslessness, reflection-free up-port)
- [x] `ToneStackMaster` block with smoothing and rate-limited updates (D-027)
- [x] Magnitude (5×5×5×3 settings) and time-domain checks; knob-jump test
- [x] M1 report: `harness/reports/M1/20261013_M1_report.md` — awaiting owner review
Plan (2026-10-13; follows SPEC §7 M1, WDF_DESIGN §2, §5, §6 and VERIFICATION §6–7):

Step 1 — MNA scattering builder
- Python reference `harness/mna.py` (numpy): ports as (node+, node−) pairs plus port resistances;
  definition-based S (WDF_DESIGN §5.1) and the reflection-free up-port resistance (§5.2); DC node
  solve with capacitors open (for `reset()`).
- C++ `engine/src/mna/` (header templates on port and node count, fixed-size arrays, Gaussian
  elimination with partial pivoting, no heap): the same three functions. No Eigen: SPEC §2 allows a
  small hand-written LU, so no new dependency.
- Cross-check: the Python side writes S fixtures (B4 at several knob sets, plus a random network) to
  `tests/data/`, a Catch2 test compares within 1e-12; freshness test in the harness.
- WDF_DESIGN §5.5 sanity tests in C++: R-type for a pure series / parallel 3-port vs the library
  adaptors (1e-12), losslessness (1e-9), up-port reflection-free (1e-12).

Step 2 — `ToneStackMaster` block (`engine/include/amp2203/ToneStackMaster.h`, `engine/src/`)
- Tree: `IdealVoltageSourceT` (J3 source voltage, absolute, as in the testbench) at the root →
  `WDFSeriesT`(R_th3, R-type) → `RtypeAdaptor` adapted at its up-port (k2b–ground) with the 11 B4
  elements as down-ports (C10, R15, C11, C12, VR3a/b, VR5, VR4a/b, VR2a/b). The library's
  `RootRtypeAdaptor` is not used: its `compute()` scatters the incident waves of the previous call, which
  would delay the source by one sample. Bilinear capacitors (WDF_DESIGN §2).
- Pot values exactly as the netlist (linear Treble/Middle without +1 Ω, log Bass/Master with +1 Ω).
- `reset()`: DC operating point from the MNA DC solve, written into the capacitor states (a small
  custom capacitor element with a settable state, tested against the library's `CapacitorT`).
- Smoothing: one-pole on knob rotations (`paramSmoothingMs`); S and port resistances recomputed every
  `kParamUpdateInterval` = 32 samples while a value moves; `reset()` snaps to the targets. Cost per
  sample and per update measured; interval recorded in DECISIONS.
- Outputs: v_oc(y) (unloaded wiper) and R_th4 (the D-015 formula from the smoothed knobs). Probes for
  k2b, tt, tw, nd, ne, nbb, mw, y. `ampwav --block b4` runs it; `Amp2203` (full) stays a pass-through
  until the chain exists.

Step 3 — Harness checks
- `python -m harness.tonestack_check`: engine SWP-lo (1 V) vs SPICE AC for 5×5×5 Treble/Middle/Bass
  at Master ∈ {0.2, 0.5, 1.0} (375 settings): max |dB error| 20 Hz–10 kHz ≤ 0.1, 10–20 kHz ≤ 0.3.
- Time-domain ESR vs SPICE transient for the log sweep ≤ −60 dB at DEFAULT and the 8 tone-control
  corners at Master 0.5 (SPICE transient sweeps cost ~1 min each at reltol 1e-6).
- `compare --block b4 --preset DEFAULT --milestone M1` with M1 tolerances active in `tolerances.py`.
- Knob-jump test (SPEC §6.2): each knob 0→1 in 10 ms during a 1 kHz tone. Engine: no NaN/Inf
  (Catch2). Reference: SPICE with time-varying pot resistances following the engine's smoothed rotation
  trajectory (ngspice `R='expr(time)'` works, checked). Pass criterion (Q10): ESR ≤ −40 dB over the
  sweep and 20 ms after it, plus no NaN/Inf.

Step 4 — M1 report (`harness/reports/M1/`), cost figures, PROGRESS/DECISIONS updates.

Risks: R-type conditioning with 1 Ω pot ends next to 1 MΩ (check S accuracy against Python at the
extremes); S-update cost in the audio thread (measure before considering a worker thread, WDF_DESIGN
§5.4).
Notes:
- 2026-10-13 Step 1 done: `harness/mna.py` (exact rationals), `engine/src/mna/` (double-double),
  fixtures `tests/data/rtype_fixtures.txt` (`python -m harness.mna --fixtures`, `--check`), Catch2
  tests `test_mna.cpp` (fixtures 1e-12, §5.5 checks, library parallel/series equivalence).
- 2026-10-13 Step 2 done: `ToneStackMaster` (`engine/include/amp2203/ToneStackMaster.h`), `DcCapacitor`,
  tapers and knob smoother, ampwav `--block b4`, `amp2203_bench b4`. First SPICE comparison (PIPE,
  1 kHz 1 V): ESR −90.3 dB, best lag 0, DC exact. Cost: 31.7 ns/sample static, 7.3 µs per matrix
  update, 262 ns/sample with knobs moving (D-027). Catch2: 21 tests.
- 2026-10-13 Step 3 done: `python -m harness.tonestack_check [--tran]` (375-setting magnitude: worst
  0.018 / 0.078 dB; 9 log sweeps: worst ESR −78.5 dB, lag 0; R_th4 vs the testbench formula 2e-16),
  `python -m harness.knob_jump` (worst ESR −46.1 dB with all four knobs; −88 to −92 dB with an update
  every sample, so the residual is the 32-sample staircase), `compare --block b4 --milestone M1`
  (17 PASS, 0 FAIL). ampwav `--knob-ramp`; `spice.op` reads device parameters (v(@dev[param])).
  B4 is scored with the linear-block ESR limit (`Block.linear`).
- 2026-10-13 Step 4: M1 report written. M1 stops here for review.

## M2 — B12: V1a and V1b with a joint triode root
- [x] 12AX7 model + Wright omega grid current + Jacobian; device unit tests
- [x] Generic multiport Newton root (first user N = 4) — Thevenin form (D-028), predictor (D-029)
- [x] `StageV1aV1b` tree (Preamp Volume inside), DC initialisation (D-028)
- [x] Block-exact checks, Newton statistics
- [x] M2 report: `harness/reports/M2/20261013_M2_report.md` — awaiting owner review
Plan (2026-10-13; follows SPEC §7 M2, WDF_DESIGN §3.1, §4, §5.3 and VERIFICATION §5–7):

Step 1 — 12AX7 device model (`engine/src/devices/`)
- Wright omega ω(z) in double precision (initial guess plus Fritsch–Shafer–Crowley iterations to
  machine precision). chowdsp's `Omega::omega4` is a fast approximation (its accuracy is not enough
  for the 0.1 % / 1 nA device tolerance across the grid-current sweep), so it is not used.
- Triode: Koren plate current and the closed-form grid diode (Rs = rgi + 1 Ω = 2001 Ω), with the
  analytic Jacobian (WDF_DESIGN §3.1). Header-only, `noexcept`, no allocation.
- Catch2: plate and grid sweeps vs `tests/data/12ax7_{plate,grid}.csv` (0.1 % or 1 nA), analytic vs
  central-difference Jacobian at every sweep point (1e-5 relative, VERIFICATION §5.2), ω identity
  ω + ln ω = z over a wide range.

Step 2 — Generic multiport Newton root (`engine/src/solver/`)
- Template on the port count K. The linear part of the block seen from the K root ports is an exact
  K-port Thevenin equivalent: v = v_oc − Z·i(v), with v_oc a linear function of the element waves and Z
  the K×K resistance matrix (both from the MNA, double-double setup). Root equation
  f(v) = v + Z·i(v) − v_oc = 0, J = I + Z·∂i/∂v. This is the R-type adaptor with the root ports
  eliminated: WDF_DESIGN §4's form with diag(R) is the special case of uncoupled root ports. In B12 the
  ports share nodes (V1a's grid and plate ports share k1a), so Z has off-diagonal terms; the full matrix
  is used. The root-port resistances of §5.3 cancel out of this form. Recorded in DECISIONS and in
  WDF_DESIGN §4.
- §4 algorithm otherwise unchanged: warm start, maxIter 12, step limits 2 V (grid) / 20 V (plate),
  convergence ‖Δv‖∞ < 1e-9·max(1, ‖v‖∞) or ‖f‖∞ < 1e-9 V, step halving up to 4×, best iterate and a
  failure counter, fixed-size Gaussian elimination. Statistics: iteration histogram (median, max),
  failures.
- Catch2: a 2-port test network against a direct solve, step limiting, failure counting.

Step 3 — `StageV1aV1b` (B12)
- Linear network (CIRCUIT.md §1 B12) as 15 element ports between 8 nodes plus the 4 root ports:
  (Vin, R3) and (bpA, R4), (bpA, R7) as resistive voltage sources; R1‖C1, R5‖C4, C5‖VR1a as library
  parallel adaptors; C3, VR1b, R6; the six inter-electrode capacitances (Cgk 2.3 pF, Cgp 2.4 pF,
  Cpk 0.9 pF per triode, SPEC §4.2). R2 sits across the ideal input source and does not change any node
  voltage, so it is left out. Bilinear capacitors (`DcCapacitor`).
- Per sample: element reflected waves → node voltages of the open-root network → v_oc at the
  root ports → Newton → node voltages with the root currents → incident waves to the elements.
- Preamp Volume (VR1, log, +1 Ω per side as the netlist) smoothed like the B4 knobs; the matrices are
  recomputed every kParamUpdateInterval = 32 samples while it moves (cost measured).
- Supply bpA: a settable ideal source, default the F operating point (389.349603 V, as the testbench);
  the block reports its supply current for B7 (M4).
- `reset()`: DC operating point with the capacitors open, solved with the same Newton (more iterations,
  not real-time), written into the capacitor states and the warm start (SPEC §3.6 option 1).
- Output v_oc(p1b) (unloaded); probes for in, g1a, k1a, p1a, na, vtop, w1, k1b, p1b, the four device
  currents and the Newton iterations. `ampwav --block b12`; `amp2203_bench b12`.

Step 4 — Harness checks and report
- `compare --block b12 --preset DEFAULT --milestone M2` with M2 scoring in `tolerances.py`: DC of p1a,
  k1a, p1b, k1b (0.5 %); ESR ≤ −40 dB for every signal at the small/nominal/hot levels and ≤ −30 dB for
  the 2 V sines; harmonics 2–5 within 0.5 dB where above −80 dBc; Newton median ≤ 3 iterations,
  max ≤ the limit, zero failures (the median and max checks are added to `tolerances.py` as transcribed
  from SPEC §7 M2).
- Same signal set at Preamp Volume 0.2 and 1.0 (new presets), reported.
- Knob-jump test for Preamp Volume (SPEC §6.2, D-025 criterion) added to `harness/knob_jump.py`.
- M2 report `harness/reports/M2/`, with costs (ns/sample, µs per matrix update).

Risks: Newton conditioning at grid-current onset (2 V input); bias shift of C3 / the Miller capacitance
in BURST; the ignored diode CJO/TT (SPEC §5) shows up as a small high-frequency error at 2 V. Each is
reported, not tuned away.
Notes:
- 2026-10-13 Step 1 done: `engine/src/devices/` (WrightOmega.h, Triode.h); Catch2 plate curves (567
  points) and grid current (8) within 0.1 % / 1 nA, Jacobian at 568 points within 1e-5 (Vpk = 0 skipped,
  VERIFICATION §5.2), ω identity to 4e-16. The CSV reader uses strtod (the tables hold subnormals).
- 2026-10-13 Step 2 done: `engine/src/solver/MultiportNewton.h`, `detail/NewtonStats.h`,
  `mna::rootTransfer`; the §4 root equation in K-port Thevenin form (D-028). Catch2: closed-form one-port,
  coupled two-port, step limit, failure handling, median.
- 2026-10-13 Step 3 done: `StageV1aV1b` (D-028), ampwav `--block b12` (Newton fields in `--stats`),
  `amp2203_bench b12`. First DEFAULT run: everything within tolerance except 5 kHz 2 V (4213 Newton
  failures, ESR −7.9 dB): V1b leaving cutoff at 6–7 V/sample with the plain warm start. Linear
  extrapolation as the predictor fixes it (0 failures, median 3, max 10; D-029, variants measured).
- 2026-10-13 Step 4: `compare` scores M2 (`tolerances.py`: overdrive sines at −30 dB, Newton median /
  max checks), presets VOL0.2 / VOL1.0, `knob_jump --block b12` (−49.7 dB, PASS),
  `python -m harness.diagnostics` (diode CJO/TT, engine rate convergence, SPICE RELTOL convergence).
  SPICE cache keys now include the content of included libraries. ampwav accepts up to 1.536 MHz (D-013).
  DEFAULT and VOL1.0: all PASS. VOL0.2: two FAILs raised with the owner (Q12 STEP ESR −34.5 dB from
  > 20 kHz content, converges with the engine rate; Q13 H3 at 5 kHz 0.1 V 0.54 dB, the SPICE reference is
  not converged at RELTOL 1e-6).
- 2026-10-13 Owner decisions Q12 / Q13: STEP edges band-limited (D-030), RELTOL 1e-8 for every reference
  transient (D-031). M1 and M2 re-rendered: B12 DEFAULT / VOL0.2 / VOL1.0 all PASS (VOL0.2 STEP −48.3 dB,
  H3 0.09 dB); B4 17 PASS, sweeps worst −79.9 dB, knob jumps unchanged. M2 stops here for review.

## M3 — B3, preamp cascade
- [x] `StageV2aV2b` (joint 4-port root) — D-032; B3 + B4 merged into B34 (Q14, D-033)
- [x] Cascade B12→B34, then one tree B1234 (`Preamp`, Q18, D-038), node dumps
- [x] Block-exact and partition reports; junction upgrades (J3 → merge); 768 kHz (Q16); loaded-y scoring (Q15)
- [x] M3 report: `harness/reports/M3/20261013_M3_report.md` — awaiting owner review
Plan (2026-10-13; follows SPEC §7 M3, §4.1, WDF_DESIGN §4, §6 and VERIFICATION §1, §7):

Step 1 — Shared root-network code
- Move the per-sample machinery of B12 (element waves → open-root node voltages → Newton → incident
  waves, matrix updates, DC solve) into a header template reused by B3 and later B5 / B6. Ports may have
  a terminal on an ideal supply node (V2b's plate is bpX, a fixed voltage, not a network node): the port
  is stored against ground and the supply voltage enters as an exact offset of its wave / Thevenin
  voltage. B12 must reproduce its M2 results bit for bit (renders compared).

Step 2 — `StageV2aV2b` (B3, CIRCUIT.md §1 B3)
- Network: 6 nodes (p1b, nB, g2a, k2a, p2a, k2b), 13 element ports: J2 source (v_oc(p1b), R_th2 =
  89732.8 Ω, junctions.py), C7, R10‖C8, R11, R9, (bpX, R12), R13 and the six inter-electrode
  capacitances (V2b's Cgp and Cpk end on bpX). Root: V2a (g2a−k2a, p2a−k2a), V2b (p2a−k2b, bpX−k2b).
- Output v_oc(k2b) (unloaded) for J3; supply current for B7; DC start as B12. No knobs.
- ampwav `--block b3`, `amp2203_bench b3`. Catch2: DC start / silence, robustness at 50 V+.
- Block-exact: `compare --block b3 --preset DEFAULT --milestone M3` with the M2 targets (ESR ≤ −40 dB at
  0.1 / 1 / 10 V, ≤ −30 dB at the 50 V overdrive sines, harmonics 2–5 ≤ 0.5 dB, Newton median ≤ 3,
  max ≤ 12, no failures, DC 0.5 %).

Step 3 — Preamp cascade
- `Preamp` engine class: B12 → B3 (J2 driven by v_oc(p1b)) → B4 (J3 driven by v_oc(k2b)), output
  v_oc(y); `reset()` passes each block's DC output to the next; probes from all three blocks.
  ampwav `--block preamp`. `Amp2203` stays a pass-through until M4.

Step 4 — Partition and total error at y (`python -m harness.cascade`)
- Standard signal set at the B12 levels, Preamp Volume ∈ {0.2, 0.6, 1.0}, other knobs default:
  F = full netlist (PI load at y present), P = SPICE chain b12 → b3 → b4 (each J-Thevenin source driven
  by the upstream open-circuit voltage), E = engine cascade. Reported at p1b, k2b and y: P vs F
  (partition error, VERIFICATION §1), E vs F (total, SPEC §7 M3 wording), E vs P (block-exact chain).
- Scored at y: ESR ≤ −26 dB on sines for P vs F and for E vs F (VERIFICATION §7 / SPEC §7 M3); DI
  ≤ −20 dB (no DI clips present: skipped with the harness message). Other signals reported.
- If a junction fails: apply the SPEC §4.1 upgrade path and ask the owner before changing the partition.

Step 5 — M3 report (`harness/reports/M3/`), costs, PROGRESS / DECISIONS updates.

Risks: B3 at 50 V (hard grid conduction in V2a, V2b grid current through R12) stresses Newton (B12 already
used all 12 iterations once); J2's R_th2 varies with Preamp Volume (88.4–90.1 kΩ) and frequency; the
M0 J2 study already gave −26.2 dB at k2b for DRIVE, so y may fail the −26 dB target. Full-netlist renders
at RELTOL 1e-8 take a long time; they run in the background.
Notes:
- 2026-10-13 Step 1–2: shared `stage/RootNetwork.h` with exact supply offsets; B12 bit-identical after the
  move; B3 block-exact 87 PASS (D-032).
- 2026-10-13 Cascade study: J3 failed as a J-Thevenin junction (−2 to −8 dB at y); merged B3 + B4 into B34
  (Q14, D-033). B34 Newton fixes (D-034). Recorded F supplies for P and E (VERIFICATION §1). Scoring at the
  loaded y (Q15, D-036). 384 kHz too coarse for the cutoff transitions: verification rate 768 kHz (Q16,
  D-035), all M1–M3 references re-rendered; step limits for 768 kHz (D-037).
- 2026-10-13 M3 report: B3 / B34 block-exact PASS, knob jumps PASS, cascade 273 PASS / 3 FAIL (Q17: E vs P
  at 5 kHz 0.5 V −39.3 / −38.4 dB; Q18: loaded-y partition at Volume 1.0, 5 kHz 2 V −22.1 dB). M3 stops
  here for review.
- 2026-10-13 Q17 / Q18 answered: B12 + B34 merged into one tree B1234 (`Preamp`, 8-port root, D-038);
  5 kHz 0.5 V deviation accepted (D-039). Cascade: partition at the loaded y 36 / 36 sines pass (worst
  −36.1 dB), E vs P all pass except the accepted 5 kHz 0.5 V cases (−39.4 / −39.0 dB); B1234 block-exact
  102 PASS / 1 FAIL (the same case); knob jumps PASS. Cost 1.27–1.86 µs/sample (above the 1.3 µs budget),
  4.3 µs/sample with all knobs moving (94 µs per matrix update). M3 stops here for review.

## M4 — B5, B6, B7, full chain
- [x] EL34 model (+ screen modification) and Jacobian; device unit tests (D-040)
- [x] `PhaseInverter` (joint 4-port) — sub-block of B56 (D-041)
- [x] OT linear network verified alone vs SPICE (coupled-inductor element, D-040)
- [x] `PowerStage` (6-port root, bias network, J5) — sub-block; B5 + B6 merged into `PowerAmp` (Q19, D-041)
- [x] `Supply` and current bookkeeping (D-040)
- [x] `Amp2203` full chain; DC, NFB, clean power, stability, overdrive checks (107 PASS)
- [x] M4 report: `harness/reports/M4/20261013_M4_report.md` — awaiting owner review
Plan (2026-10-13; follows SPEC §7 M4, §4.2–4.3, WDF_DESIGN §3.2, §4, §6, §7 and VERIFICATION §1, §7):

Step 1 — J5 partition in SPICE first (as J1–J3 were measured before their blocks were built)
- New testbench `b56j5`: B5 + B6 + B7 in one netlist, but B6's C14 / C16 are fed by J5 Thevenin
  sources (VCVS copies of B5's unloaded p3a / p3b behind R_thA / R_thB) instead of the PI plates; R22
  stays wired to B6's out (closed loop, instantaneous NFB). Same input as b56 (J4 source).
- `python -m harness.partition --junction j5`: b56j5 vs b56 at out, nGn / nOr, pA / pB, ct, with the B56
  standard signal set (0.1 / 1 / 5 / 20 V) and Presence 0 / 0.5 / 1. CIRCUIT §4 already shows the PI
  plates are strongly cross-coupled (transimpedance 0.44–0.56 of self), so J5 may fail as J1–J3 did; if
  it does, SPEC §4.1 upgrade options go to the owner before B6 is built (a merged B56 tree would also
  remove the NFB delay, at the cost of a 10-port root).

Step 2 — EL34 model (`devices/Pentode.h`, WDF_DESIGN §3.2)
- Koren pentode with the repository screen modification, grid diode (Rs = 1001 Ω), analytic Jacobian;
  matched-pair equivalent (×2). Catch2 against `tests/data/el34_{plate,grid}.csv` (0.1 % / 1 nA) and
  central finite differences (1e−5).

Step 3 — Output transformer network
- The OT's three coupled windings (same k for every pair) are a 3-port inductance matrix. Extend the MNA /
  root-network code with a coupled-inductor element (trapezoidal companion of the inductance matrix,
  setup-time matrices; Python reference in `harness/mna.py`). Verify the transformer network alone
  against SPICE: plates driven by ideal current sources, AC magnitude / phase and a transient, as
  CIRCUIT §3 requires.

Step 4 — `PhaseInverter` (B5)
- Tree: J4 source (v_oc(y), R_th4 from the preamp), C9, R17, R16, R19, C15, R20, R23, C17, VR6 Presence
  (smoothed), R22 as a resistive source driven by out[n−1], R18 / R21 to bpX, inter-electrode
  capacitances; 4-port root of V3a / V3b (shared cathode k3). Outputs v_oc(p3a), v_oc(p3b), bpX current.
- Block-exact vs `b5open` (R22 and bpX driven by recordings from a b56 run) at the B56 levels with the
  nonlinear block-exact targets (≤ −40 dB, overdrive ≤ −30 dB, harmonics 2–5 within 0.5 dB, Newton).

Step 5 — `PowerStage` (B6)
- J5 sources (or the step 1 outcome), C14, C16, C18, R24 / R25, bias Thevenin (−46.364 V, 11.59 kΩ) + C19,
  grid stoppers 2.8 kΩ, screen resistors 500 Ω to nScr, EL34 pair equivalents with their
  inter-electrode capacitances (×2), the OT network, 16 Ω load; 6-port root. Outputs v(out), centre-tap
  and screen currents.
- Block-exact vs `b6` (recorded p3a / p3b / ct / nScr) with the same targets; DC per tube within ±1 mA
  plate / ±0.3 mA screen.

Step 6 — `Supply` (B7, WDF_DESIGN §7)
- Five-state trapezoidal state-space, inputs the previous sample's currents from all blocks; DC by fixed
  point with the blocks' DC currents. Checked alone: driven by F's recorded supply currents vs F's
  recorded supply voltages.

Step 7 — `Amp2203` full chain (SPEC §4.3) and the M4 acceptance (vs F unless noted)
- ampwav `--block full` / `b56` / `b5` / `b6`; the `b56` testbench's constant preamp loads become F's
  recorded currents.
- DC (EL34 plate / screen per tube, v(ct) 1 %, nbias 0.2 V); NFB gain with and without R22 (SPICE copy
  with R22 = 1 TΩ in harness/cache, the reference unchanged; engine option to open the loop); CLEAN
  power and THD; 60 s stability at Preamp / Master / Presence 1.0; overdrive 220 Hz 300 mV (ESR,
  harmonics 2–9, B+ sag minimum); NFB delay check against a 2-sample delay variant; CRUNCH / DRIVE with
  the DI clips (ESR ≤ −20 dB).
- The M3 DI check that was skipped (loaded-y partition ≤ −20 dB on DI) runs now that clips exist.
- The SPEC §7 M4 numbers in parentheses (34.7 mA, 4.65 mA, 101 W, 469 V) predate the D-023 / D-024
  netlist changes (CIRCUIT §5 now: 34.08 mA, 4.57 mA). The checks compare with the current SPICE runs;
  the parentheses get the measured values. The tolerances do not change.

Step 8 — M4 report, costs, docs.

Risks:
- J5 partition (step 1).
- Real-time cost: SPEC §7 M5 asks for a real-time factor ≥ 10 at 48 kHz × 16, i.e. ≤ 130 ns per sample for
  the whole amp. The M3 report measured the preamp alone at 1.27–1.86 µs (the "1.3 µs budget" there is a
  real-time factor of 1). So the preamp is already 10–14× over the M5 target before B5 / B6 are added.
  SPEC M5 says to profile and report before optimising; M4 reports per-block costs.
- OT stiffness: leakage 8 mH against 25.6 H (k = 0.9997925) makes the inductance matrix ill-conditioned
  (≈ 5000); the setup arithmetic is double-double.
- Supply: the centre tap sees C23 through a one-sample delay (WDF_DESIGN §7), checked by the clean-power
  and sag acceptance.
- Full-netlist SPICE renders of 5 s DI clips at 768 kHz / RELTOL 1e-8 take hours; they run in the
  background.
- DI clips: `reference/di/` has four tracks of one song (`GTR L`, `GTR LL`, `GTR R`, `GTR RR`, 44.1 kHz,
  238 s). The harness uses the first 5 s of each (VERIFICATION §3; all four play from t ≈ 0 at about
  −31 dBFS RMS, peaks −7 to −9 dBFS, 1.6–1.9 V at the jack).
Notes:
- 2026-10-13 Step 1 started: testbench `b56j5`, `python -m harness.partition_j5`. First sines with the
  provisional J5 (R_th = |Z_self|): −23 to −26 dB at out on small sines, −18 to −21 dB at 20 V. AC: the
  small-signal gain is 0.43–0.47 dB low because the PI plates are cross-coupled (Z_ab = 32.1 kΩ,
  Z_ba = 25.4 kΩ at 1 kHz); with R_th = Z_self − Z_mutual or the full 2×2 source the small-signal error is
  ≤ 0.12 dB. The study now runs the three source variants (`self`, `diff`, `2port`) on the whole signal set.
- 2026-10-13 Steps 2, 3, 6: EL34, the OT coupled-inductor element (verified alone vs SPICE) and `Supply`
  (D-040).
- 2026-10-13 Steps 4, 5 (blocks): `PhaseInverter` (B5, 8 nodes, 18 ports, 4-port root) and `PowerStage` (B6,
  15 nodes, 26 ports, 6-port root, J5 sources with the provisional R_th) with Catch2 DC / silence /
  robustness tests; ampwav `--block b5` / `b6` (recorded inputs through `--supply`). The harness renders
  the recordings for b5open / b6 from b56 runs (`harness/recordings.py`); first b5open check (1 kHz 1 V):
  ESR −80.5 dB. The J5 structure (keep, change R_th, 2-port source, or merge B5 + B6) waits for step 1.
- 2026-10-13 Step 7 (chain): `Amp2203` = Preamp → PhaseInverter (NFB out[n−1]) → PowerStage, Supply once per
  sample; DC by fixed point; `--nfb open|delay2` diagnostics; `python -m harness.m4_acceptance`. With the
  provisional J5: DC PASS (EL34 34.078 mA per tube, 1.2e-5 mA from SPICE), overdrive ESR −25.7 dB,
  harmonics 2–9 0.74 dB, sag 1.66 V; CLEAN power 88.3 W vs 100.2 W and the open-loop gain 0.9 dB low (the
  J5 small-signal error). Newton at Master 1.0 / Presence 1.0 with 1.9 V guitar peaks: 1469 failures in
  30 ms in the power stage (slow convergence through OT flyback; 100 iterations converge, max 24–42); the
  acceptance signals converge in ≤ 5.
- 2026-10-13 Q19 answered: B5 + B6 merged into `PowerAmp` (B56, D-041). Power-stage solver and the
  source-stepping fallback (D-042). Acceptance (DC, NFB, CLEAN, overdrive): 60 PASS, 0 FAIL.
- 2026-10-13 M4 report: full-amp acceptance 107 PASS / 0 FAIL (DI clips −61 to −63 dB); M3 DI check PASS
  (−81 to −86 dB); block-exact B5 88 / 0, B56 105 / 1, B6 94 / 3 (all failures on SIN-5000Hz-20V: Q20).
  Full amp 2.48 µs per sample (real-time factor 0.5). M4 stops here for review.
- Q20 (M4): block-exact at 5 kHz 20 V (J4 source): B56 Newton median 5 (≤ 3); B6 (sub-block) harmonics 2–5
  1.48 dB (≤ 0.5), 1 Newton failure (rescued), median 4. — 2026-10-13 owner: analyse further first. Engine at
  2× / 4× rate: B6 H5 difference 1.48 → 1.08 → 0.33 dB, ESR −39.7 → −45.0 → −50.0 dB; B56 median 5 → 3 → 2.
  Predictors (none / linear / quadratic): B56 median 4 / 5 / 5, no variant ≤ 3; quadratic adds failures.
  Discretisation, as Q16 / Q17. Options: accept and report; run B56 at 2× internally; other. — Answered
  2026-10-13: try adaptive half-steps → adopted in `PowerAmp` (D-043): b56 106 PASS / 0 FAIL, full amp 2.67 µs
  per sample. The B6 sub-block keeps its three 5 kHz 20 V FAILs (no half-steps there; discretisation).

## M5 — JUCE plugin
- [x] Processor, parameters, oversampling, latency (D-044)
- [x] Minimal editor
- [x] `amp2203_bench` real-time factor: 0.42 (≥ 10 not met; profiled, Q22)
- [x] pluginval (VST3, AU), auval: pass
- [x] Wrapper null test vs `ampwav`: −92.6 to −110.9 dB
- [x] M5 report: `harness/reports/M5/20261013_M5_report.md` — closed by the owner 2026-10-14
Plan (2026-10-13; follows SPEC §7 M5, §3.3–3.5, §6.3, D-035):

Step 1 — Processor (`plugin/src/`, the only JUCE code)
- APVTS parameters per SPEC §6.3: Preamp Volume, Treble, Middle, Bass, Master, Presence (0–10 shown,
  rotation 0–1 inside), Interface Level (0..+24 dBu, default +12), Input Trim (±24 dB), Output Level
  (−48..+12 dB, default 0 dB), Oversampling (2x / 4x / 8x / 16x; default by host rate per SPEC §3.4: 16x at
  44.1 / 48 kHz, 8x at 88.2 / 96, 4x at 176.4 / 192; since D-048: 2x / 4x / 8x / 16x, default 8x), Oversampling filter (linear-phase FIR or polyphase IIR,
  task 2). State saved / restored through the APVTS.
- Mono: a mono bus uses its channel; on a stereo bus the left channel is processed (SPEC allows summed or
  left; guitar interfaces put the instrument on input 1, and summing a single-sided input halves it) and the
  output goes to every channel.
- `juce::dsp::Oversampling` around `Amp2203` at host rate × factor; latency of the oversampler reported via
  `setLatencySamples`. Changing the factor or the filter rebuilds off the audio thread (suspendProcessing,
  re-prepare, new latency). `juce::ScopedNoDenormals`; knob values go to `Amp2203::setParams` once per
  block (the engine smooths them). Interface Level sets the engine's input scale (a real-time setter on
  `Amp2203`, D-012 addition); trim and output level are gains in the wrapper.
- The processor code goes into a static library shared by the plugin and a console tool (step 4).

Step 2 — Minimal editor: six knobs, Interface Level, Input Trim, Output Level, Oversampling and filter
  selectors (JUCE sliders / combo boxes with APVTS attachments).

Step 3 — `amp2203_bench` (task 4): 60 s of a DI clip (`reference/di/`, its first 60 s; linear 16x
  interpolation in the bench, which costs little against the engine) at 48 kHz × 16, real-time factor, plus
  the per-block costs (SPEC: if the real-time factor is below 10, profile and report before optimising).

Step 4 — Wrapper null test: console tool `amp2203_render` (JUCE, links the processor library): renders a
  WAV through the processor offline at 16x, and runs the oversampler's up / down stages alone. Harness
  `python -m harness.plugin_null`: plugin(x) vs down(ampwav full(up(x))) at default levels, ESR ≤ −60 dB, for a
  sine set and a DI excerpt.

Step 5 — pluginval `--strictness-level 5` on the VST3 and the AU, `auval -v aufx A223 Ctcd`. These need the
  built plugins in `~/Library/Audio/Plug-Ins/{VST3,Components}`; the build copies them there only when asked
  (option or manual copy, documented). pluginval 1.0.4 is the owner's installed app (recorded in DECISIONS).

Step 6 — Automation: every knob over its full range at audio rate (fast LFO sweeps and 0 → 1 jumps) during a
  hot DI excerpt at 16x, engine-level (Catch2 / ampwav): no NaN / Inf, Newton statistics reported. Also the
  Presence knob-jump test against SPICE left from M4 (VERIFICATION §7, D-025).

Step 7 — M5 report, docs.

Risks:
- Real-time: the engine runs at 2.67 µs per sample at 768 kHz (real-time factor ≈ 0.5 at 48 kHz × 16, M4
  report); the M5 target is ≥ 10. The plugin will not run in real time at 16x; lower factors trade the
  accuracy established at 768 kHz (D-035). SPEC M5 prescribes profiling and reporting first.
- Newton under automation: at extreme settings (Master / Presence 1.0, hot input) first solves still fail
  occasionally and are rescued by the fallback (D-042, D-043), but are counted as failures; the automation
  check may report them.
- pluginval at strictness 5 runs many render passes; at 16x the engine is slow and tests may take long.
Notes:
- 2026-10-13 M5: processor, editor, render tool, null test 26 / 26 PASS, pluginval VST3 / AU and auval PASS
  (AU installed in ~/Library/Audio/Plug-Ins: owner decision), Presence knob jump −77 dB, automation: no NaN,
  6367 first-solve Newton failures all rescued (Q21), real-time factor 0.42 (Q22). M5 stops here for review.
- 2026-10-14 M5: owner decision after the rate study (D-047): Oversampling 2x / 4x / 8x / 16x, default 8x (D-048).
- 2026-10-14 M5 closed by the owner. Deviations from SPEC §7 M5 carried with the close: real-time factor
  0.53 at 16x (target ≥ 10; plugin at the 8x default 0.97–1.17), and under automation at high Master 90
  first-solve Newton failures at 768 kHz, all rescued (D-042, D-045). 2x / 4x have no SPICE pass (D-047).

## M6 — Performance (owner scope 2026-10-14: performance work only; the other SPEC §7 M6 items stay open)
Closed by the owner on 2026-10-19 (report accepted). Steps 2–4 were not finished; the CPU work continues in M7.
- [x] Step 0: baseline at the 8x default (384 kHz)
- [x] Step 1: build settings (no change adopted)
- [x] Step 2: device equations (studied; inline polynomials slower than libm, not adopted; rest moved to M7)
- [x] Step 3: dense linear algebra / SIMD (unrolled elimination adopted; rest moved to M7)
- [x] Step 4: linear parts at a lower rate (moved to M7)
- [x] Step 5: re-verification, M6 report (`harness/reports/M6/20261018_M6_report.md`, accepted 2026-10-19)
- 2026-10-18: owner-supplied measured 4x12 impedance curves (T1511, T1281, G12K-85) compared with D-059: match above ≈ 200 Hz,
  resonance within their range; D-059 kept (D-063).
- Owner 2026-10-18: handle the remaining Newton first-solve failures (HOT DI RR at 384 kHz: 2; plugin model check at 2x:
  102 over 28 runs, 3 unrescued, mostly extreme unseen settings). Plan: per-root / per-setting counts, temporary failure
  dump (ports, residuals, states), cause, fix, re-verify HOT and the plugin model check.
  Done (D-064): causes V2b grid conduction (preamp, 33 … 56 iterations) and an EL34 plate near 0 V with a positive grid
  (power amp, oscillating damped steps); preamp maxIter 64, EL34 pair 3 halvings. 0 failures at HOT / every knob 1.0 /
  CRUNCH (2x, 8x, 16x) and in the 2x plugin model check (`M6/20261018_x2_rates_newton_fix_full2x.md`). Re-verification below.
Plan (2026-10-14; SPEC §7 M6 "performance work", WDF_DESIGN §4; goal: real-time headroom at the 8x default
without loosening any tolerance):

Step 0 — Baseline. `amp2203_bench --factor N` (engine rate 48 kHz × N; default 16 as before). Profile at 8x
(macOS `sample`), per-block ns, Newton iteration distribution at 384 kHz.

Step 1 — Build settings (engine and plugin): LTO, `-mcpu` for Apple Silicon, FMA contraction. Results change at
the rounding level only; checked with the null test against the previous build.

Step 2 — Device equations (≈ 26 % in the M5 profile): exp / log1p / pow / atan / Wright omega replaced by
polynomial or rational approximations, or tables (WDF_DESIGN §4 M6 list), with a stated error bound per
function. Gate: device unit tests vs the SPICE DC sweeps and the finite-difference Jacobian checks unchanged in
tolerance; Newton iterations not increased.

Step 3 — Dense parts (solve 32 %, J build 19 %, network products 11 %): structure (J = I + Z·D, D block
diagonal), memory layout, NEON via the compiler. xsimd (SPEC lists it) would be a new dependency: asked before
use.

Step 4 — Linear parts at a lower rate: which parts are separable from the roots (the tone stack is inside the
preamp tree, D-033; supply B7). Analysis and a proposal before any change, as partitions moved results before
(Q17, Q19).

Step 5 — Full-amp acceptance at 384 and 768 kHz, M3 cascade, plugin null test, automation, pluginval / auval;
bench and plugin real-time factors at 2x–16x; M6 report.

Risks: approximations change results (bounded per function, verified end to end); speedups unknown until
measured; any tolerance failure is reported with options, not loosened.
Notes:
- 2026-10-14 Step 0: `amp2203_bench --factor 8`: 2270–2313 ns per engine sample, real-time factor 1.13–1.15;
  `--hot` (Master, Presence 1.0) 3130 ns, 0.83. At 16x 2221 / 2602 ns. Profile at 8x (default knobs): solveLinear
  28 %, Newton loop incl. J build 22 %, libm (pow, exp, log1p, log, Wright omega) 23 %, network products 13 %.
  Iterations per solve (default): preamp 2 in 91 %, power amp 2 in 95 %; hot power amp 2–4 in 77 %.
- Step 1: LTO −3 %, `-mcpu=apple-m4` 0 %, `-ffp-contract=off` +5 % (60 s bench, 8x); not adopted (timing noise
  level; LTO would apply to the plugin build only).
- Step 3: elimination unrolled at compile time (bit-identical output): −1 to −3 % end to end (10 s DI at 384 kHz,
  default / hot). Tried and not adopted (scratch builds): reusing the LU for a step after a small step (same
  convergence test): ≤ 5 % faster, and from a 1 V threshold the output moves (ESR −72 / −67 dB vs the current
  engine); a looser residual tolerance 1e-7 / 1e-6 / 1e-5 V: −6 / −12 / −22 % time, ESR vs the current engine
  −159 / −137 / −118 dB at default knobs, −124 / −114 / −62 dB hot (design criterion WDF_DESIGN §4.3; owner
  decision, not changed).
- 2026-10-14 Q23 answered (owner): pursue exact-output tuning, a looser Newton tolerance, and the power amp at
  half rate (two threads not chosen).
- Power amp at half rate (`EngineConfig::powerAmpRateDivisor` = 2, halfband FIR 35 taps, Kaiser β 10, 2 × 17
  samples delay = `Amp2203::latencySamples()`; ampwav `--power-rate-divisor`, removes the delay): full-amp
  acceptance at 384 kHz 107 PASS (`harness/reports/M6/20261014_full_acceptance_fs384000_pdiv2.md`), DI ESR
  −55.2 to −57.6 dB (full rate −60.8 to −62.7). 10 s DI at 384 kHz: −24 to −26 % time; vs the engine at 768 kHz
  (48 kHz band) −74.2 dB default / −38.6 dB hot (full rate −80.8 / −52.7). The loss is the power amp's rate:
  the filter alone (power amp at full rate on the filtered input) −95.6 / −60.9 dB vs the full-rate engine;
  delaying the supply currents by 17 samples alone −62.3 dB hot; half-step triggers 50 / 20 V do not change it.
- Newton residual tolerance (`ampwav --newton-tol`): acceptance at 384 kHz 107 PASS at 1e-7, 1e-6, 1e-5,
  1e-4 V (DI ESR within 0.03 dB of 1e-9). Time (10 s DI, default / hot): 1e-9 2263 / 3042 ns; 1e-6 1979 / 2664;
  1e-5 1759 / 2495; 1e-4 1573 / 2280; 1e-3 1483 / 2139. vs 768 kHz (48 kHz band): −80.8 / −52.7 dB for 1e-9
  to 1e-5, −80.4 / −52.8 at 1e-4, −72.3 / −52.4 at 1e-3.
- Exact-output tuning: plain resistors (wave always 0, no supply offset; 13 of 38 preamp, 14 of 42 power-amp
  ports) left out of the open-circuit product (`RootNetwork::updateMatrices` zeroWave, `stage::plainResistors`):
  −1 % (ESR vs before −230 / −204 dB, rounding). Inline polynomial exp / log / log1p / pow in the devices
  (≤ 2 ulp; pow 17 ulp) in a scratch build: +24 % time (Apple libm faster in these latency-bound chains); not
  adopted.
- 2026-10-14 Q24 answered (owner): Newton tolerance stays 1e-9 V; the half-rate power amp becomes a plugin
  choice, default off; first a SPICE reference at Master + Presence 1.0 ("render it first").
- HOT preset (Master, Presence 1.0; 220 Hz 0.3 V and the 4 DI clips; `m4_acceptance --only hot`, CRUNCH /
  DRIVE tolerances) at 384 kHz: full rate 11 PASS / 10 FAIL (`harness/reports/M6/20261014_full_acceptance_fs384000_hot.md`):
  ESR −12.6 to −13.3 dB (≤ −20), harmonics 2–9 11.6 dB (≤ 1.5), 3–8 first-solve Newton failures per DI clip
  (all rescued); half rate the same 10 FAIL (`…_pdiv2_hot.md`). Cause: the J4 partition (D-015 resistive
  R_th4, y open on the preamp side). The SPICE chain alone (P56 vs F, no engine) gives the same numbers:
  y −20.6 dB, out −12.8 dB. Master sweep (220 Hz 0.3 V, SPICE chain vs F, out): Master 0.3 −71, 0.5 −74,
  0.7 −48, 0.85 −31, 1.0 −15 dB (Presence 0.5); Presence 1.0 within 3 dB of these. The verified presets all
  use Master ≤ 0.3 except CLEAN (Master 1.0, −31 dB ESR at 25 mV).
- 2026-10-14 Owner paused the J4 fix question: asked for a comprehensive architecture study (playable,
  knob-robust, usable with other plugins). Written: `docs/ARCHITECTURE_STUDY.md` (literature, products,
  candidates A–E against latency / authenticity / CPU / knobs / verifiability / effort). M6 waits for the
  owner's direction.
- 2026-10-14 Owner: run experiments X1 and X3 (ARCHITECTURE_STUDY §6); PyTorch 2.14.1 approved for X3.
- X1 (branch `m6-x1-j4`): exact J4 coupling (`EngineConfig::exactJ4`, ampwav `--j4 exact`, harness
  `AMP2203_J4=exact`): a load port rY at y on both roots (preamp: given current; power amp: the preamp's
  linearised Thevenin source, rTh = Z_yy − Z_yp·D·(I + Z_pp·D)⁻¹·Z_py, every sample), preamp solved, power amp
  solved against it, preamp re-solved (warm) with the actual load current. At 384 kHz: HOT 220 Hz ESR −12.6 →
  −58.9 dB, harmonics 11.6 → 0.01 dB, HOT DI −13 → −41.5 … −42.7 dB (the HOT DI clips keep 5–8 rescued
  first-solve failures each: 4 FAIL on newton_failures, `…_j4exact_hot.md`); full acceptance 107 PASS, CLEAN
  (Master 1.0) −31.2 → −60.1 dB, the rest within 0.5 dB (`…_j4exact.md`). Cost at 384 kHz (10 s DI): 3352 / 4497
  ns default / hot (main 2242 / 3016; the extra open port alone +6 %).
- X3 (`harness/x3_neural.py`, report `harness/reports/M6/20261015_x3_neural.md`): LSTM-32 at 48 kHz trained on
  the engine (768 kHz, CRUNCH, four DI tracks × four input gains; CPU, Adam 1e-3 with gradient clipping 1.0 after
  5e-3 diverged at epoch ≈ 80; 1200 epochs, 78 min). Vs SPICE on the held-out 5 s CRUNCH DI clips: −16.5 …
  −17.1 dB (teacher −48.7 … −49.1 dB); validation vs teacher −14.5 dB. Non-harmonic power on 5 / 9 kHz sines:
  −6.3 … −17.0 dB (teacher −25.7 … −39.9 dB). Cost: 439 ns per 48 kHz sample in plain C++ (2.1 % of one core).
  Training is resumable (`train_state.pt`, written at every validation).
- X4 plan (owner 2026-10-15: knobs Preamp Volume, Master, Presence as inputs; tone stack at 0.5):
  `harness/x4_conditioned.py`. LSTM-32 as X3 with the three knob values as extra inputs (constant per segment).
  Teacher: ampwav full amp at 768 kHz with `--j4 exact` (X1; valid at high Master). Data: 96 training settings
  (each knob uniform in [0, 1], one in five at 0 or 1 so the corners are covered), each a 20 s segment of a random
  DI track (between the held-out first 5 s and last 20 s) at a random input gain; 16 validation settings on the
  last 20 s. Training as X3 (Adam 1e-3, clipping 1.0), 2400 epochs, resumable. Evaluation: vs SPICE on the
  held-out 5 s DI clips at CRUNCH, DRIVE and HOT (cached 384 kHz references, 48 kHz band), X3 alongside at
  CRUNCH; vs the teacher at the 16 unseen settings; aliasing at CRUNCH and HOT; C++ cost.
- X4 result (report `harness/reports/M6/20261015_x4_conditioned.md`): render 112 × 20 s (the Mac slept from 01:16;
  jobs now run under `caffeinate -s -i`), training 2400 epochs, 2 h 26 min, validation flat at −8.1 dB from epoch
  ≈ 1300. Vs SPICE (held-out 5 s DI, 48 kHz band): CRUNCH −3.0 … −3.8 dB (X3 at the same clips −16.5 … −17.1 dB),
  DRIVE −5.0 … −5.4 dB, HOT −6.8 … −7.1 dB; teacher −48.7 … −49.0 / −55.7 … −56.1 / −39.6 … −40.1 dB. Unseen
  settings vs teacher −8.4 dB overall (best −10.9 dB); at Preamp Volume 0 or Master 0 (near-silent output) +23.7 …
  +62.2 dB: the input skip (output = network + input) has to cancel the input there. Non-harmonic power on 5 / 9 kHz
  sines −18.7 … −23.3 dB (teacher −25.7 … −39.9 dB). Cost 442 ns per 48 kHz sample (static knobs folded into the bias).
- X6 plan, block boundaries for per-block models (owner 2026-10-15: "run the boundary measurement"):
  `harness/boundary_study.py`, engine only (ampwav full amp, 768 kHz, `--j4 exact`). Candidate boundaries p1a, p1b,
  p2a, k2b, y and the supply nodes bpa, bpx, dumped in one run per knob setting. Settings: Preamp Volume 0.2 / 0.6 /
  1.0, each with every downstream knob (Treble, Middle, Bass, Master, Presence) at 0 and at 1 one at a time, all at 0,
  all at 1, and the defaults (13 per Volume); signals: DI GTR L and GTR RR, first 5 s, at 0 and +12 dB. Metric: ESR
  in the 48 kHz band of a boundary node at a changed setting vs the same node at the default downstream knobs (each
  minus its value at t = 0). A boundary is clean for a knob when that change stays ≤ −20 dB (D-051) and is also
  reported against −40 dB.
- X6 result (`harness/reports/M6/20261015_x6_boundaries.md`, 156 renders, 5 min): worst ESR of each boundary vs
  downstream knobs, raw / above 20 Hz (20 Hz high-pass added: the raw figure is dominated by sub-audio B+ sag drift).
  p1a (no knobs upstream): Volume / tone / Presence ≤ −30.7 dB, Master −27.5 dB above 20 Hz (raw up to +15 dB).
  p1b: each knob ≤ −20.5 dB above 20 Hz, all downstream knobs at once −19.4 dB (raw up to +29 dB). p2a / k2b: Master
  −17.3 / −17.4 dB above 20 Hz (coupled, via the supply bpx); k2b Middle / Bass −20.5 / −22.5 dB (J3 loading);
  treble and Presence ≤ −35 dB. y: Presence −36.4 dB. Supply nodes bpa / bpx change by +3 … +26 dB with the knobs
  (they carry the coupling). Implication: a split at y and at p1a / p1b is within −20 dB in the audio band; any
  preamp block reaching p2a or later needs the supply (power-amp current draw) as an input, or Master as a proxy.
- Owner 2026-10-15: leave the per-block route; run X2, then A1.
- X2 / A1 plan: `harness/x2_rates.py` (A1 = the same script with `--power-rate-divisor 2` and / or `--newton-tol`).
  Full amp with exact J4 at 2x / 4x / 8x of 48 kHz (96 / 192 / 384 kHz). Scored as the plugin model check (D-051):
  vs SPICE (cached 384 kHz references, 48 kHz band) on the held-out 5 s DI clips at CRUNCH, DRIVE and HOT; vs the
  engine at 16x on the 16 X4 validation settings (10 s each); Newton failures (rescued / not) on every run;
  aliasing on 5 / 9 kHz sines vs 16x at CRUNCH and HOT; cost from `ampwav --stats` on 10 s DI (default and HOT
  knobs), run one at a time, as % of one core at 48 kHz (oversampling filters not included).
- X2 result (`harness/reports/M6/20261015_x2_rates.md`): vs SPICE worst CRUNCH / DRIVE / HOT −46.4 / −49.9 / −26.2 dB at
  2x, −48.5 / −55.6 / −33.2 at 4x, −48.7 / −55.8 / −38.6 at 8x (all PASS ≤ −20 dB); unseen settings vs 16x worst
  −20.6 / −26.9 / −39.1 dB. Newton: unrescued failures per 5 s DI clip 6 … 29 at 2x, 0 at 4x and 8x; on loud
  5 / 9 kHz sines thousands to ≈ 140 000 per 2 s at 2x / 4x (and up to 11 496 at 8x, 20 at 16x), which dominate the
  aliasing figures there. Cost ≈ 3.4–3.6 µs per engine sample (HOT 4.3–5.5): 34 / 67 / 129 % of one core at 2x / 4x
  / 8x (HOT 52 / 92 / 166 %). Limiting factor at lower rates: Newton convergence, not ESR.
- A1 result: (a) Newton tolerance 1e-5 V (`…_x2_rates_a1_tol1e-5.md`): accuracy as X2 (worst HOT −26.2 / −33.2 /
  −38.7 dB at 2x / 4x / 8x, PASS); cost 36 / 58 / 106 % of one core (HOT 45 / 77 / 157 %); Newton failures fewer
  (DI + validation runs: 6568 / 1380 / 22, unrescued 1048 / 138 / 3), loud 5 / 9 kHz sines still fail at 2x / 4x.
  (b) plus the power amp at half rate (`…_a1_div2_tol1e-5.md`): not a test of exact J4 — `Amp2203::exactJ4()` is off
  when the power amp runs at half rate, so these runs used the Thevenin J4: HOT −13.0 dB at every rate (FAIL), CRUNCH
  / DRIVE pass; cost 17 / 31 / 55 % (HOT 27 / 44 / 75 %). ampwav `--stats` now reports `exact_j4` as in effect.
  Exact J4 with the half-rate power amp is not implemented (open).
- Owner 2026-10-15: run (1) exact J4 with the half-rate power amp and (2) Newton convergence at 2x / 4x.
  Plan (2) first: per-block Newton counts in `ampwav --stats`; diagnose the 2x / 4x failures (which block, which
  ports, step-limited or not; diagnostic maxIter / step limits); then a rate-aware fix, e.g. D-043's adaptive
  substeps generalised to the preamp and to m > 2, with bounded cost; verify with `harness.x2_rates` and the 16x
  acceptance unchanged. Plan (1): the halfband filters put 34 engine samples of delay between the preamp and the
  half-rate power amp, so a same-sample (exact) J4 cannot span them; test the load current at y fed back with that
  delay (approximation) against the HOT SPICE references, report before adopting.
- (2) done: Newton failures at 2x / 4x come from the preamp's 12-iteration budget (not the step limits); maxIter 32 in
  both roots of the full amp (D-052). Acceptance at 384 kHz exact J4: 107 PASS; HOT 21 PASS / 0 FAIL. `ampwav --stats`
  reports Newton failures and max iterations per root.
- X2 with maxIter 32 (`…_x2_rates_maxiter32.md`): accuracy as before; Newton failures over 28 runs 15 (7 unrescued) /
  4 (0) / 3 (0) at 2x / 4x / 8x (was 8723 (1376) / 2058 (147) / 128 (3)); DI cost unchanged (34 / 67 / 129 %).
  Aliasing now measurable without solver noise: non-harmonic power on 5 / 9 kHz sines −6.4 … −19.6 dB at 2x,
  −14.9 … −26.1 at 4x, −24.8 … −36.6 at 8x (16x −25.7 … −39.9).
- (1) done (D-053): exact J4 with the half-rate power amp, coupled at the power-amp samples without a decimation
  filter (`…_x2_rates_a1_div2_exact.md`): HOT −22.5 / −26.3 / −33.2 dB vs SPICE at 2x / 4x / 8x (PASS), unseen settings
  worst −18.9 / −20.6 / −26.8 dB vs 16x, cost 24 / 46 / 89 % (HOT 36 / 63 / 115 %), latency 17 engine samples. The
  plugin still runs the Thevenin J4 (exact J4 not enabled there).
- Listening set (owner 2026-10-15): `python -m harness.listening` → `harness/cache/listening/20261015/` (not committed):
  DI GTR L / GTR RR 10–30 s at CRUNCH, DRIVE, HOT; engine 16x reference, full-rate power amp 2x / 4x / 8x, half rate
  2x / 4x, X4 network, X3 (CRUNCH); named / blind (KEY.txt) / difference; same gain per excerpt and preset, 32-bit
  float. Difference RMS vs the reference: CRUNCH / DRIVE −52 … −94 dBFS (engine variants), HOT −27 … −58 dBFS;
  networks −13 … −29 dBFS. No cabinet in the files.
- Owner listening result 2026-10-15: full-rate power amp at 4x and half-rate power amp at 2x sound identical to the
  16x reference; the X3 / X4 networks sound weaker, with clearly less gain, most of all at HOT.
- Owner 2026-10-15: (1) put half-rate power amp at 2x into the plugin and measure its CPU; (2) close the CPU gap.
  Plan (1): plugin engine with exact J4; defaults Oversampling 2x, Power Amp Rate Half (SPEC §6.3, DECISIONS);
  `amp2203_render` reports processing time; plugin CPU on 60 s DI at default and HOT knobs, FIR and IIR filters;
  re-run plugin_null and automation. Plan (2): Newton tolerance 1e-6 / 1e-5 / 1e-4 V at half rate 2x (accuracy vs
  SPICE, unseen settings, cost; adoption is the owner's decision, D-049); profile the engine at half rate 2x.
- (1) done (D-054): plugin defaults 2x / Half / exact J4; plugin CPU 25 % (HOT 39 %) incl. oversampling; null test
  25 / 26 (half-step decision sensitivity at half rate); automation 0 failures.
- (2) measured, not adopted (owner decisions): half rate 2x with the half-step trigger off
  (`…_x2_rates_half_trigger_off.md`): accuracy as with it (HOT −22.6 dB, unseen −19.7 dB), engine 24 / 32 %; plus
  Newton tolerance 1e-6 / 1e-5 / 1e-4 V (`…_half2x_off_tol*.md`): accuracy identical, failures 12 (2 unrescued),
  engine 22.3 / 21.5 / 19.4 % (HOT 29.4 / 28.3 / 27.1 %). Profile (half 2x, HOT): LU solves 29 %, libm pow / exp / log
  26 %, rest of the Newton loops 34 %, WDF 6 %.
- Owner 2026-10-15: adopt both (tolerance 1e-4 V, no half-steps at half rate), D-055. Null test 26 / 26 at Half and
  Full (ampwav `--volts-per-fs`, `--keep-delay`). Found after the decision: at every knob 1.0 the half-rate power amp
  without half-steps has 656 rescued first-solve failures per 5 s and costs 5371 vs 3501 ns per engine sample;
  automation at every knob 1.0 up to 10.4 µs. Raised with the owner.
- Owner 2026-10-15: half-steps back on at half rate (D-056). Plugin CPU 2x Half FIR: default 20.6 %, HOT 32.9 %, every
  knob 10 35.6 % (0 failures). Null test 26 / 26; automation 0 failures. Open: plugin CPU under knob automation.
- Plugin CPU under knob automation (`python -m harness.plugin_cpu`, `amp2203_render --automate`, report
  `harness/reports/M6/20261015_plugin_cpu.md`): static 18.9 % (every knob 10: 32.1 %); one knob moving 45.8 … 55.0 %
  (from every knob 10: 54.4 … 67.7 %); all six moving 84.6 … 87.1 %; host block 64 as 512; 1 rescued failure in 31
  runs. Profile with all knobs moving: 76 % in the double-double MNA solve of the matrix updates (D-026; every 32
  samples while a knob is smoothed), Newton + LU 14 %, libm 5 %.
- Owner 2026-10-15: run both: (a) plain double instead of double-double for the matrix updates and (b) a longer
  update interval. Plan: `mna` routines templated on the scalar (DD / double); `Preamp` and `PowerAmp` (the full amp's
  roots) update in double and every 128 samples (was 32); sub-blocks, DC solves and the fixtures keep DD. Verify:
  ctest, knob-jump vs SPICE (b1234, b56), full acceptance at 384 kHz exact J4 + HOT, X2 at half 2x, null test,
  automation, plugin CPU.
- Done (D-057): plugin CPU under automation 18.7 … 24.6 % (default base), 22.8 … 33.5 % (every knob 10); static
  ≈ 19 % / 32.5 %. Accuracy: acceptance and HOT unchanged; knob jump PASS (B1234 −39.2 … −55.1 dB, B56 −63.2 dB).
- 2026-10-15 (main after PR #1): plugin reinstalled to ~/Library/Audio/Plug-Ins (VST3, AU); pluginval strictness 5
  SUCCESS for VST3 and AU, auval PASS (`harness/reports/M6/20261015_pluginval_*.log`, `…_auval.log`).
- Owner listening in Logic (2026-10-15): "very dark", "lacks gain", Master "just adds volume"; cab IR after the plugin;
  compared with a real 2203. Review of O3 / A5 (owner request), SPICE studies on copies, reference unchanged:
  - O3: C5 1 nF top-to-wiper matches the drawing. New O8: C2 100 pF V1a plate–cathode is on the drawing but missing
    from the netlist; adding it darkens V1a → V1b by 0.3 dB at 5 kHz, 0.7 … 0.9 dB at 10 kHz.
  - A5 12AX7: Koren's set vs RCA datasheet: 250 V / −2 V 0.95 mA (1.2), gm 1.67 (1.6) mA/V, rp 53.7 (62.5) kΩ; 100 V /
    −1 V 0.10 mA (0.5), gm 0.59 (1.25), rp 141 (80) kΩ. Sarti et al. DAFx-07 set (mu 100.8, ex 1.4979, kg1 1890, kp 828,
    kvb 72, Vct 0.612) matches both points (1.22 mA / 1.62 / 61.4; 0.50 mA / 1.22 / 82.0). Full amp with it: DC −1.6 …
    −6.6 V, CRUNCH / DRIVE 220 Hz 0.3 V: output −0.3 dB, harmonics H2–H9 within 0.0 … 3.8 dB, spectral centroid +2 %.
  - A5 EL34 (Koren set) vs Philips class A (250 / 250 V, −12.2 V): Ia 113 (100) mA, Ig2 19.3 (14.9) mA, gm 13.1 (11)
    mA/V, ra 34 (15) kΩ; effect on the sound not studied.
  Neither O3 nor the 12AX7 set explains "dark / lacks gain"; open candidates: resistive 16 Ω load (README known
  limitation), EL34 model, input level.
- EL34 study (owner request; SPICE copies, reference unchanged): no published Koren EL34 fit found; JJ datasheet typical
  point Ua 250 V, Ug2 265 V, Ia 100 mA, S 11 mA/V, Ri 15 kΩ, mu g2-g1 11. Current set there: gm 12.2 mA/V, ri 38.8 kΩ.
  One-point fit (mu, ex, kg1, kg2 kept): kp 49.97, kvb 58.9 → gm 11.0, ri 15.0 kΩ; softer knee (Vg1 0 V, Va 50 V:
  159 vs 254 mA), not validated against curves. Full amp, current / fit: clean power (CLEAN 25 mV) 101 / 82.5 W
  (THD 3.2 / 1.9 %); clipped output at DRIVE Master 0.6–1.0 and HOT 182–194 / 132–137 W into 16 Ω; harmonics
  H2–H9 mostly within 0–2 dB (H2 at Master 0.6: −38.6 / −53.2 dB); spectral centroid −1 … −7 % with the fit.
  Closed-loop output impedance (16 Ω vs open load) 14.1 / 15.7 / 16.0 Ω at 100 Hz / 1 kHz / 5 kHz (fit: 14.6 / 16.2 /
  16.5 Ω): about equal to the load, so a speaker's rising impedance would raise the real amp's output at resonance
  and at high frequencies relative to the 16 Ω resistor (not yet simulated). Neither EL34 set explains "dark".
- EL34 curve fit (owner request; poppler installed for the data sheet; `harness/reports/M6/20261015_el34_curve_fit.md`):
  Koren parameters fitted to 81 digitised Ei-RC curve points: mu 14.77, ex 1.046, kg1 194.3, kp 53.82,
  kvb 40.4, kg2 2243; error vs the curves median 3.7 / 2.7 % (current set 11.5 / 28.2 %). Full amp: output impedance
  20–23 Ω (was 14–16), clean 87 W (101), clipped 156–163 W (182–194), harmonics mostly within 1 dB, centroid −1 … −2 %.
  Not a cause of "dark"; the higher output impedance makes the resistive-load difference larger.
- Owner 2026-10-15: adopt the fitted EL34 in the reference model (D-058). Plan: tubes.lib EL34 parameters (bias network A4
  unchanged); engine `devices::Pentode` defaults; regenerate tests/data el34 sweeps (`harness.device_sweeps`), CIRCUIT.md §5,
  README sanity figures; re-verify: b56 block-exact, M4 acceptance at 768 kHz and at 384 kHz exact J4 + HOT, X2 at half 2x
  (plugin config), B56 knob jump; SPICE references re-rendered (netlist hash changes).
- D-058 done: re-verified against the new references: M4 full acceptance (768 kHz) 107 PASS (CRUNCH / DRIVE DI −61.1 …
  −63.0 dB), 384 kHz exact J4 107 PASS, HOT 21 PASS (DI −42.2 … −43.3 dB); b56 106 PASS, b5open 88 PASS, b6 95 PASS / 2 FAIL
  (the known 5 kHz 20 V discretisation FAILs, D-043; SWP-lo reference rendered at RELTOL 1e-7 after "timestep too small"
  at 1e-8, noted in the report); half 2x plugin config vs SPICE HOT −23.1 dB, unseen −19.6 dB; B56 knob jump −64.0 dB;
  null test 26 / 26; plugin CPU static 31.6 % (every knob 10), automation 22.2 … 24.4 %. SPICE sanity: NFB 5.1 / 4.7 /
  4.6 dB (was 6.6 / 6.2 / 6.0), clean 87 / 112 / 126 W at 25 / 30 / 40 mV (was 101 / 133 / 151 W).
- Plugin with the fitted EL34 reinstalled (VST3, AU); pluginval strictness 5 SUCCESS for both, auval PASS.
- Owner listening after D-058: decent with Preamp 10, Interface Level +24 dBu, Bass 0, Treble 7, Middle 1, Presence 7,
  Master 3; "bass / mid heavy"; Master above 3 adds nothing to the driven sound. Audient iD4 MkII, DI gain ¼ up (DI lineup
  12 dBu = 0 dBFS, 40 dB DI gain range: the correct Interface Level is below +12 dBu, so +24 drives the model ≈ 20 dB
  above the real guitar). Engine at Preamp 10: 0.8 / 4.4 / 21.4 % THD at y for 10 / 20 / 100 mV, plateau 26–28 %.
- Speaker-load study (`harness/reports/M6/20261015_speaker_load_study.md`): with three 4x12-style impedances instead of
  16 Ω the speaker voltage gains +3 … +4.7 dB at 100 Hz and +1.8 … +4.7 dB at 5 kHz (+2.7 … +5.1 at 8 kHz), −0.3 … −0.9 dB
  at 400 Hz – 1 kHz; driven tones at the owner's settings: centroid +11 … +21 %, 5–10 kHz energy +1.7 … +3.5 dB.
- Owner 2026-10-15: put the "typical" speaker load into the reference and the engine (D-059). Plan: netlist Rload →
  Re 13 Ω + (Le 1.0 mH ‖ 45 Ω) + (Res 37 Ω ‖ Lm ‖ Cm at 100 Hz, Q 2); engine B56 (`PowerAmp`) and B6 (`PowerStage`): two
  nodes, six elements (inductors as one-winding CoupledInductors, the capacitor a DcCapacitor; DC states, half-step
  rescaling); testbench_gen / blocks / CLEAN power label; C++ DC fixtures, CIRCUIT.md §5, README; re-verify as after D-058.
- D-059 verification (new references): M4 full acceptance (768 kHz) 107 PASS (CRUNCH / DRIVE DI −60.3 … −62.1 dB); 384 kHz
  exact J4 107 PASS; HOT 17 PASS / 4 FAIL: DI ESR −33.5 … −35.7 dB (was −42.2 … −43.3) and 37–56 first-solve Newton failures
  per DI clip in the power amp, all rescued (newton_failures ≤ 0 FAIL); b5open 88 PASS; b56 and b6 not scored: their SPICE
  references stop with "timestep too small" at RELTOL 1e-8 (long sweeps, t ≈ 1.5–1.8 s). Presence knob jump −62.9 dB;
  null test 26 / 26; plugin CPU static 33.2 % (every knob 10), automation ≈ 23–24 %.
  Plugin model check (half 2x, 1e-4 V): HOT −18.8 dB vs SPICE (FAIL ≤ −20; was −23.1), unseen −16.5 dB; half-steps off
  −19.2 dB (FAIL); full-rate power amp at 2x −24.1 dB (PASS), unseen −20.8 dB, engine 26.8 / 40.8 % (default / HOT).
  Power-amp Newton at HOT (DI L): failures 49 / 420 at 8x / 16x (43 unrescued at 16x); maxIter 64 or a looser tolerance
  do not change them; half-steps off: 3 / 68; half-steps every sample: 64 / 466. Cause (half-step handling of the speaker
  inductors vs a stiffer discretised load) not yet established.
- Owner comparison 2026-10-17 against a NAM capture of a real 2203 (Tone3000, settings published with the capture), plugin
  build with D-058 + D-059 at its defaults (2x, Half, Interface Level +12 dBu, trim and output 0 dB): with the capture's
  settings (Preamp 10, Treble 8, Middle 7, Bass 4, Master 10, Presence 8) "the tone is pretty accurate"; the output at Master
  10 is ≈ 12 dB too loud for the IR loader. The earlier "missing gain" was at Master 3: the capture's sound includes the
  power stage at full Master. These settings are in the HOT region (plugin model check there: −18.8 dB vs SPICE at half 2x).
- Owner: power amp at full rate by default (D-060). Plugin CPU: default 26.4 %, every knob 10 50.2 %, NAM-matched settings
  47.3 %, knob automation 32–35 %; null test 26 / 26; automation 0 unrescued.
- Owner listening 2026-10-17 (NAM-matched settings, same IR): Power Amp Rate Full vs Half, "can't hear any real audible
  difference". The earlier NAM comparison ("pretty accurate") was made at Half (the restored instance setting).
- Owner 2026-10-17: keep Full as the default (D-060 confirmed after the listening test); Half stays selectable per instance.
- Owner 2026-10-17: fix the HOT convergence with the speaker load. Plan: (1) check the half-step handling of the speaker
  inductors (every-sample half-steps at fs must reproduce 2·fs without half-steps, D-043); (2) characterise the failing
  power-amp solves (ports, residuals, iterations); (3) fix (Newton step control / fallback / discretisation) and re-verify
  HOT, the plugin model check and the acceptance.
- Diagnosis: every-sample half-steps at 8x reproduce 16x to −67.6 dB (half-step code correct); failures grow with the step
  rate (384 kHz: 1, 768 kHz: 34–37) and are unchanged by maxIter 64, tolerances 1e-6 … 1e-8, double-double matrices or 4 / 8
  halvings (8: stall). All at pair B's plate in flybacks below the cathode (−700 … −2600 V): Koren's unclamped
  atan(Vpk/kvb) gives reverse plate current there. Hard clamp: mixed; 5 V softplus with a matching screen knee: 0 failures
  at 8x and 16x. Owner approved the tubes.lib change (D-061).
- D-061 verification (new references): M4 full acceptance (768 kHz) 107 PASS; 384 kHz exact J4 107 PASS; HOT 20 PASS / 1 FAIL:
  DI ESR −40.3 … −41.3 dB (D-059 alone −33.5 … −35.7), Newton failures 0 / 0 / 0 / 2 (rescued; the FAIL); b56 106 PASS; b6
  96 PASS / 1 FAIL (the known 5 kHz 20 V Newton median, D-043); b5open 88 PASS. The b56 / b6 SWP-lo-0.1V references still stop
  with "timestep too small" at RELTOL 1e-8 (node y / the recorded nScr source, same times as before D-061: not the tube
  model) and are now rendered once more at RELTOL 1e-7 by `render_spice` (as the M3 cascade), noted in the reports. Plugin
  model check: Full 2x HOT −24.2 dB (PASS), unseen −20.8 dB, 94 first-solve failures (2 unrescued) over 28 runs, nearly all in
  the extreme validation settings (presets: 0 / 0 / 2); Half 2x HOT −18.9 dB. Presence knob jump −62.9 dB; null test 26 / 26.
- Owner 2026-10-18: add C2 to the netlist (D-062). Plan: `C2 p1a k1a 100p`; the engine folds it into V1a's Cpk port (Cpk + C2,
  B1234 and B12: the same nodes, exact); B12 partition; re-verify b12 (DEFAULT, VOL0.2, VOL1.0), b1234, knob jumps b12 / b1234,
  M4 acceptance at 768 kHz and 384 kHz exact J4 + HOT, plugin model check (Full 2x), null test, CPU.
- D-062 done. b12 / b1234 first failed on DC (≈ 0.65 %): the standalone blocks' default supplies were pre-D-058; fixed
  (engine headers + block DC fixtures). Then: b12 DEFAULT / VOL0.2 / VOL1.0 105 / 106 / 106 PASS, b1234 119 PASS, b3 87 PASS,
  b34 94 PASS (0 FAIL each); knob jumps b12 −55.9 dB, b1234 −39.1 … −55.1 dB, b34 −53.1 … −67.6 dB; M4 full acceptance 107
  PASS (768 kHz and 384 kHz exact J4); HOT 20 PASS / 1 FAIL (2 rescued first-solves, as after D-061); null test 26 / 26.

## M7 — CPU optimisation (owner 2026-10-19: "close M6 and start on the CPU optimisation")
Goal: lower the plugin's CPU at its defaults (2x, Power Amp Rate Full, exact J4, Newton tol 1e-4 V, half-steps on;
D-054 – D-060, D-064), above all at high Master, toward the 20 % working budget, without loosening any tolerance and
without audible change. Owner criteria: no added latency, authentic at all knobs, CPU headroom.

Baseline (2026-10-19, main 0e70f81; `amp2203_render plugin --factor 2`, DI GTR L, FIR oversampling):
- CPU: default knobs 26.6 % (60 s), NAM settings (Preamp 10, Treble 8, Middle 7, Bass 4, Master 10, Presence 8)
  42.7 % (20 s), every knob 10 54.0 % (60 s). Master 10 alone 33.6 %, Presence 10 alone 26.4 %.
- Engine at 96 kHz (ampwav, plugin settings, 20 s): default 2491 ns / sample, NAM 4236 ns.
- Profile at NAM (macOS `sample`, audio thread): power amp ≈ 62 %, preamp (two solves per sample) ≈ 30 %. By kind:
  libm (pow, log1p, exp, log, atan) + Wright omega ≈ 29 %, LU elimination ≈ 23 %, Newton loop (J build, residual,
  inlined devices) ≈ 26 %, open-circuit products ≈ 7 %.
- Newton at NAM (temporary counters): power amp 1.15 solves per sample (half-steps), 3.46 iterations per solve
  (default knobs 1.36), 1.03 M step halvings on 7.65 M iterations, step limits rarely active (0.6 %); preamp 2 solves
  per sample, 1.41 iterations per solve. The power amp's iteration count is what grows with Master.

Plan (each step measured alone; adopted only if it pays):
1. Newton iterations in the power amp: initial guess (a predictor from the last Jacobian instead of linear
   extrapolation), the step-halving rule, Jacobian reuse within a solve. Gate: 0 Newton failures (HOT, every knob 1.0,
   plugin model check), output within the Newton tolerance of the current engine.
2. Fewer solves per sample: the half-step trigger at 2x Full; the preamp's second solve (exact J4) replaced by a
   linear correction from its last Jacobian. Gate: plugin model check vs SPICE and 16x, HOT acceptance.
3. Code level, results unchanged to rounding: memory layout / padding for NEON, the linear rY port folded out of
   the LU, derivatives only where used, vectorised device maths across tubes. Gate: null test vs the previous build.
4. Re-verification as M6 (full acceptance 768 / 384 kHz, HOT, block tests, knob jumps, automation, plugin model
   check, null test, pluginval / auval), plugin CPU, M7 report.
Tools: `ampwav --stats` now writes the per-root iteration histograms; `harness.plugin_cpu`; `amp2203_render`.

- [x] Step 1 (D-065): Jacobian predictor from the kept LU factors, preamp J4 Thevenin resistance from them. Plugin 2x Full
  (20 s DI): default 26.6 → 21.6 %, NAM 42.7 → 33.4 %, every knob 10 49.7 → 37.3 %; plugin model check unchanged
  (`M6/20261019_x2_rates_m7_step1_full2x.md`). Tried without gain: step-halving rule, EL34 step limits, chord iterations.
- Profile after step 1 (NAM, inclusive): half-steps 21 % (15 % of samples at 2x cross the 100 V trigger), LU 17 %,
  triode maths 17 %, pentode maths 14 %, open-circuit products 6 %.
- Half-step trigger at 2x Full (plugin model check): off: HOT −22.7 dB, unseen worst −19.8 dB, HOT cost 31.7 → 29.6 %;
  300 V: HOT −23.4 dB, unseen −20.2 dB, 30.3 % (`M6/20261019_x2_rates_m7_hst0_full2x.md`, `…_hst300_…`). Kept at 100 V
  (2 points of CPU for 1.3 dB of HOT margin and the unseen check).
- [x] Step 3a (D-066): fast Wright omega (series below z = −10, cubic Hermite table start + one Fritsch–Shafer–Crowley step,
  ≤ 1e-14 relative to the reference, 4.3× faster), E1^ex as exp(ex·ln E1) (≤ 1.2e-15 relative), no node voltages / waves
  for the preamp's trial J4 solve. Engine NAM 3464 → 3175 ns. Tried without gain: float LU for the Newton direction
  (3175 → 3163 ns: K = 9 / 11 is latency-bound; the double rule stays).
- After steps 1 + 3a: plugin (20 s DI, 2x Full) default 26.6 → 20.4 %, NAM 42.7 → 31.0 %, every knob 10 49.7 → 34.4 %
  (60 s DI: every knob 10 54.0 → 42.1 %); plugin model check unchanged (CRUNCH / DRIVE / HOT −45.3 / −48.5 / −24.0 dB,
  unseen −20.8 / −39.0 dB, 0 failures, `M6/20261019_x2_rates_m7_step3a_full2x.md`).
- Owner 2026-10-19: "do A and B". Plan: (B) the preamp's second J4 solve replaced by a linear correction from the first
  (dv = −J⁻¹·Z[:, y]·Δi_y, currents moved by the device conductances at the first solution), with a full re-solve when the
  correction is large; measured for time, output vs the current engine and the plugin model check. (A) the linear J4 port
  out of both Newton systems: the preamp's given-current port solved after the 8 triode ports, the power amp's source port
  folded into its Thevenin matrix per sample (results unchanged to rounding); null test vs the current engine.
- [x] B (D-067): linear load correction below 10 mV (vs the re-solve −115 dB default / −126 dB NAM; larger limits move the
  output: 30 mV −51.6 dB, unlimited −31 dB). [x] A (D-068): the linear port out of both Newton systems (−128 / −56.9 dB vs
  before). Plugin (20 s DI, 2x Full): default 20.4 → 18.5 %, NAM 31.0 → 31.1 %, every knob 10 34.4 → 34.5 %: the gain is
  at default knobs; at high Master the load steps exceed 10 mV.
- Options C / D measured (owner: "how much ... could affect CPU", then "re-run the Half model check"). C (half-step trigger
  300 V, engine): NAM −5 %, every knob 1.0 −8 %, default 0 %. D (Power Amp Rate Half, plugin 20 s): default 18.5 → 12.8 %,
  NAM 31.1 → 21.6 %, every knob 10 34.5 → 23.7 %. Half model check after M7 (`M6/20261019_x2_rates_m7_AB_half2x.md`):
  CRUNCH / DRIVE / HOT −37.7 / −37.2 / −18.9 dB (FAIL: HOT ≤ −20), unseen worst / median −16.6 / −28.0 dB, 0 failures
  (before M7: 12, 1 unrescued); non-harmonic power on 5 / 9 kHz sines −7.0 … −12.6 dB (Full −14.9 … −20.5 dB). Accuracy as
  before M7 (−18.9 / −16.7 dB).
- Owner 2026-10-19: Half becomes the plugin default, Full selectable (D-069; HOT −18.9 dB accepted). Latency 49 → 58 samples
  at 48 kHz.
- M7 verification (2026-10-19): M4 full acceptance 768 kHz 0 FAIL; 384 kHz exact J4 0 FAIL; HOT 0 FAIL; b1234, b56, b6, b5open
  0 FAIL (b6's 5 kHz 20 V Newton median now 3, D-043 item passes); knob jumps b1234 −39.1 … −55.1 dB, b56 −62.9 dB;
  automation Full 0 failures, Half 3 rescued first-solve failures → D-070 (power-amp maxIter 48), then Half and Full 0.
Risks: changes to the Newton path move the output within the tolerance (verified end to end); gains unknown until
measured; any tolerance failure is reported with options, not loosened. No new dependencies without asking.

## Questions for the owner
_(Claude Code: add questions here and also ask them in the session.)_
- Q1 (M0): git init + baseline commit? — Answered 2026-10-12: yes; repo-local author CatastrophicCoder.
- Q2 (M0): install ngspice? — Answered 2026-10-12: yes; ngspice 47 installed via Homebrew.
- Q3 (M0): build the JUCE plugin in CI? — Answered 2026-10-12: no, plugin OFF in CI.
- Q4 (M0): ngspice RELTOL for reference transients (`harness/config.py: SPICE_RELTOL`)? — Answered
  2026-10-12: 1e-6 (DECISIONS D-014).
- Q5 (M0): b6 testbench DC at out/sl is 0 V vs 39.8 µV in F (R22's NFB current is in B5, not B6).
  Accept as a known partition deviation, or change the check / testbench? — Answered 2026-10-12:
  accept as known; check and testbench unchanged, report keeps the explained FAIL (DECISIONS D-015).
- Q6a (M0): how to handle J1 (partition error, CIRCUIT.md §4)? — Answered 2026-10-12: merge B1 and
  B2 into one block B12 with a joint 4-port root (DECISIONS D-017).
- Q6b (M0): which R_th basis (DC, |Z(1 kHz)|, Re Z(1 kHz)) goes into junctions.py for J2, J3, J5
  (J5 provisional until M4)? — Answered 2026-10-12: |Z(1 kHz)| (DECISIONS D-018).
- Q7 (M0): `metrics.esr_db` uses raw node voltages; at plate nodes (~250 V DC) the DC term dominates
  Σy² and makes ESR look better than it is (`compare` scores b1 at p1a, b2 at p1b). Compute ESR on the
  signal minus the DC operating point? — Answered 2026-10-12: each signal minus its own DC operating
  point (value at t = 0) for ESR, best lag and the sweep response (DECISIONS D-018).
- Q8 (M0): use the Hammond 1750U OT datasheet? — Answered 2026-10-12: adopt it in the netlist and fit
  the primary inductance to the published curves; nominal turns ratio 1700/16; asymmetric DCRs with
  Blue–White on the pA side (DECISIONS D-023, open item O7).
- Q9 (M0): use the Hammond 194F choke datasheet? — Answered 2026-10-13: adopt now, Rch 115 Ω, L 5 H
  (DECISIONS D-024).
- Q10 (M1): knob-jump pass criterion? — Answered 2026-10-13: ESR ≤ −40 dB vs SPICE with time-varying
  pots on the same smoothed trajectory, over the sweep and 20 ms after it (VERIFICATION §7, D-025).
- Q11 (M1): the R-type up-port reaches only 2–4e-12 reflection-free in double precision at extreme B4
  settings (WDF_DESIGN §5.5: 1e-12)? — Answered 2026-10-13: higher precision (double-double setup
  arithmetic; exact-rational Python reference) (D-026).
- Q12 (M2): STEP at Preamp Volume 0.2 gives ESR −34.5 dB (≤ −40 dB); the error lies above 20 kHz
  (−50.7 dB after a 20 kHz low-pass) and shrinks with the engine rate (−44.4 / −52.6 dB at 2× / 4×):
  bilinear discretisation of an ideal step through the C5 bright-cap path. How to treat it? — Answered
  2026-10-13: band-limit the STEP edges for all blocks (causal 4th-order Bessel, −3 dB at 20 kHz; D-030).
- Q13 (M2): H3 of the 5 kHz 0.1 V sine at Preamp Volume 0.2 differs by 0.54 dB (≤ 0.5 dB); the SPICE
  reference is not converged at RELTOL 1e-6 (−70.68 dBc; −71.13 at 1e-8; engine −71.23). How to treat
  it? — Answered 2026-10-13: RELTOL 1e-8 for all reference transients; M1 and M2 re-rendered (D-031).
- Q14 (M3): J3 (cathode follower k2b → tone stack) fails as a J-Thevenin junction: SPICE partition ESR at
  y −2.2 to −7.5 dB from 0.1 V up (V2b cuts off into the tone stack). Merge B3 and B4, keep J3 and report,
  use a fixed load, or analyse more first? — Answered 2026-10-13: merge B3 and B4 into B34 (D-033).
- Q15 (M3): which comparison at y should M3 score (SPEC: v_oc(y) vs F's PI-loaded y)? — Answered 2026-10-13:
  score the loaded y (SPICE chain through the J4 source into the PI testbench) vs F; the engine vs the
  SPICE chain with the block-exact limits (D-036).
- Q16 (M3): engine discretisation at extreme slews (5 kHz, 0.5–2 V input) at 384 kHz? — Answered
  2026-10-13: verify at 768 kHz and make it the default internal rate (D-035).
- Q17 (M3): engine vs the SPICE chain at 5 kHz 0.5 V is −39.3 / −38.4 dB (≤ −40 dB) at 768 kHz
  (discretisation, as Q16). How to treat it? — Answered 2026-10-13: accept and report (D-039).
- Q18 (M3): partition at the loaded y, Volume 1.0, 5 kHz 2 V is −22.1 dB (≤ −26 dB), mostly J2. Upgrade
  path (a) reactive R_th2, (b) merge B12 and B34, or other? — Open. SPICE study of (b) first (owner request,
  `python -m harness.merge_study`, `harness/reports/M3/20261013_merge_study.md`): with B12 + B3 + B4
  joined, the loaded-y partition error over all 36 sines is worst −45.9 / −36.5 / −36.1 dB at Preamp
  Volume 0.2 / 0.6 / 1.0 (current: −30.0 / −27.0 / −22.1 dB); speaker output −47.1 / −41.3 / −41.0 dB.
  — Answered 2026-10-13: merge B12 and B34 into one tree B1234 (D-038).

- Q19 (M4): J5 (PI plates → power stage) as a J-Thevenin junction. SPICE closed-loop study
  (`harness/partition_j5.py`, Presence 0; DEFAULT / Presence 1 not finished), ESR at the speaker vs b56:
  provisional R_th = |Z_self|: −23 to −26 dB up to 1 V, −25 to −32 dB at 5 V, −18 to −21 dB at 20 V
  (small-signal gain 0.45 dB low: the PI plates are cross-coupled). R_th = Z_self − Z_mutual: −46 to −68 dB
  up to 1 V, −34 to −37 dB at 5 V, −13.7 to −18.6 dB at 20 V. Full 2×2 source: −75 / −63 dB on two small
  sines, then ngspice failed ("timestep too small" at node y, RELTOL 1e-8 and 1e-7). Options: keep J5 with
  Z_self − Z_mutual; 2×2 source; merge B5 + B6 into one tree (10-port root, R22 inside the tree, no NFB
  delay); finish the study first. — Answered 2026-10-13: merge B5 and B6 into one tree B56 (D-041).
- Q21 (M5): under automation at high Master the power amp's first solves exceed 12 iterations (6367 in 56 runs,
  all rescued by the D-042 fallback, 0 unrescued; static every-knob-at-1.0 DI: 865 / 5 s). A B56 limit of 16 /
  20 iterations: 10 / 7 at the same cost. Options: count rescued solves as converged; raise the B56
  iteration limit (and the "max ≤ configured limit" check with it); keep as reported; other. — Answered
  2026-10-13: raise the B56 limit to 16 (D-045).
- Q22 (M5): real-time factor 0.42 at 48 kHz × 16 (target ≥ 10); profile in the M5 report. Which direction:
  optimise the solver / device code first (structure-exploiting solve, chord Newton, tables), lower the internal
  rate (accuracy table in the report, unverified against SPICE), both, or other? — Answered 2026-10-13:
  optimise the solver first (accuracy unchanged; measured per step). Done (D-046): real-time factor 0.42 → 0.53;
  re-verification (B56, cascade, M4 acceptance, null test) unchanged; automation 90 first-solve failures, all
  rescued. M5 stops here for review.
- Q25 (M6): acceptance target for the real-time plugin? — Answered 2026-10-15: −20 dB ESR (literature −17 … −27 dB) for
  every ESR check, the engine's per-block checks included (D-051).
