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
- [ ] `PhaseInverter` (joint 4-port, delayed NFB)
- [x] OT linear network verified alone vs SPICE (coupled-inductor element, D-040)
- [ ] `PowerStage` (6-port root, bias network, J5)
- [ ] `Supply` and current bookkeeping
- [ ] `Amp2203` full chain; DC, NFB, clean power, stability, overdrive checks
- [ ] M4 report
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

## M5 — JUCE plugin
- [ ] Processor, parameters, oversampling, latency
- [ ] Minimal editor
- [ ] `amp2203_bench` real-time factor
- [ ] pluginval (VST3, AU), auval
- [ ] Wrapper null test vs `ampwav`
- [ ] M5 report
Plan:
Notes:

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

