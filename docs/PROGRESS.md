# Progress

Claude Code keeps this file current: tick tasks when done, link the milestone report, and note blockers
or questions for the owner. Before starting a milestone, write a short plan under it (steps, files,
risks) and wait for the owner's go-ahead if the plan deviates from SPEC.md.

Legend: [ ] todo · [~] in progress · [x] done · [!] blocked (see note)

## M0 — Skeleton and verification harness
- [x] Top-level CMake, pinned FetchContent (chowdsp_wdf, Catch2, JUCE, Eigen if used); targets build
- [x] `ampwav` CLI (pass-through engine), 64-bit float WAV I/O
- [x] `harness/signals.py`, `render_spice.py`, `run_engine.py`, `compare.py`
- [~] Block testbenches B1–B4, B56(+B7), B5-open, B6, full; DC sanity check vs F (≤ 0.5 %) — generated;
  R_th pending step 5; b6 DC deviation at out/sl accepted as known (Q5)
- [ ] Junction Thevenin resistances measured and entered in CIRCUIT.md §4
- [ ] Device DC sweep CSVs generated into `tests/data/`
- [ ] CI workflow (macOS): build + tests
- [ ] M0 report: `harness/reports/M0/...`
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

Step 5 — Junction resistances: measure R_th1, R_th2, R_th3, R_thA and R_thB with a 1 kHz AC current
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

## M1 — B4 tone stack + master
- [ ] MNA scattering builder (C++) + numpy reference + cross-check test
- [ ] Adaptor sanity tests (series/parallel equivalence, losslessness, reflection-free up-port)
- [ ] `ToneStackMaster` block with smoothing and rate-limited updates
- [ ] Magnitude (5×5×5×3 settings) and time-domain checks; knob-jump test
- [ ] M1 report
Plan:
Notes:

## M2 — B1 V1a with triode root
- [ ] 12AX7 model + Wright omega grid current + Jacobian; device unit tests
- [ ] Generic multiport Newton root
- [ ] `InputStageV1a` tree, DC initialisation
- [ ] Block-exact checks, Newton statistics
- [ ] M2 report
Plan:
Notes:

## M3 — B2, B3, preamp cascade
- [ ] `StageV1b`
- [ ] `StageV2aV2b` (joint 4-port root)
- [ ] Cascade B1→B4, node dumps
- [ ] Block-exact and partition reports; junction upgrades if needed (DECISIONS entries)
- [ ] M3 report
Plan:
Notes:

## M4 — B5, B6, B7, full chain
- [ ] EL34 model (+ screen modification) and Jacobian; device unit tests
- [ ] `PhaseInverter` (joint 4-port, delayed NFB)
- [ ] OT linear network verified alone vs SPICE
- [ ] `PowerStage` (6-port root, bias network, J5)
- [ ] `Supply` and current bookkeeping
- [ ] `Amp2203` full chain; DC, NFB, clean power, stability, overdrive checks
- [ ] M4 report
Plan:
Notes:

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
