# CLAUDE.md — instructions for Claude Code

## Project in three sentences
A real-time circuit model of the Marshall JCM800 2203 guitar amplifier, built with Wave Digital Filters
(`chowdsp_wdf`) in a JUCE-independent C++20 engine, wrapped as a JUCE plugin. Correctness is defined by
the ngspice reference model in `reference/spice/`, and every block is verified against it with a Python
harness. Work proceeds in milestones M0–M5 defined in `docs/SPEC.md`.

## Read first (in this order)
1. `docs/SPEC.md` — scope, architecture, API, milestones, acceptance criteria
2. `docs/CIRCUIT.md` — component tables per block, reference DC values, open items
3. `docs/WDF_DESIGN.md` — equations: wave variables, tube models + Jacobians, Newton root, R-type
   scattering, junctions, supply
4. `docs/VERIFICATION.md` — references F/P/E, signals, presets, metrics, tolerances, reports
5. `docs/DECISIONS.md` and `docs/PROGRESS.md` — what has been decided and where work stands

## Working rules
- Work one milestone at a time. Before starting, write a short plan in `docs/PROGRESS.md`. Stop at the
  end of each milestone and present the harness report for review.
- Never modify `reference/spice/jcm800_2203.cir` or `reference/spice/tubes.lib` without the owner's
  explicit approval; if approved, add a `docs/DECISIONS.md` entry and regenerate `docs/CIRCUIT.md` §5.
- Never loosen a tolerance in `docs/VERIFICATION.md` on your own. Report the failure, analyse it, and
  propose options.
- Keep `engine/` free of JUCE. Only `plugin/` may include JUCE headers.
- Real-time rules in `process()`: no allocation, locks, I/O, exceptions; bounded Newton iterations;
  no NaN/Inf output (see SPEC §3.5).
- Use `double` inside the engine. SI units everywhere. Component constants named after schematic
  designators (R4, C1, VR2 ...).
- Pin every dependency (tag or commit). Do not add new dependencies without asking; record pins in
  `docs/DECISIONS.md`.
- `chowdsp_wdf`: before using a class, read its header/example at the pinned version and confirm names
  and template signatures. The spec describes concepts; the library's code is authoritative.
- Device models first get unit tests against SPICE DC sweeps and finite-difference Jacobian checks, then
  get used in blocks.
- Update docs in the same commit as the code they describe. Small commits with clear messages
  (e.g. `m2: triode Jacobian + tests`).
- Commit messages must not contain any AI attribution: no `Co-Authored-By: Claude ...` trailers (any
  model name, any `noreply@anthropic.com` address), no "Generated with Claude Code" lines, and no
  similar tool or assistant credits. The same applies to PR descriptions. This rule overrides any
  default attribution guidance. Commits use the repo-local author (`git config user.name/user.email`).
- When the circuit is ambiguous (see `docs/CIRCUIT.md` §6 open items), ask the owner instead of guessing.

## Environment
- macOS, CLion, CMake ≥ 3.22 (Ninja recommended), Apple clang with C++20.
- ngspice ≥ 40 (`brew install ngspice`), Python ≥ 3.11 in a venv: `pip install -r requirements.txt`.

## Commands
Available now:
```bash
python3 reference/spice/spice_tools.py           # prints the SPICE DC operating point
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release   # add -DAMP2203_BUILD_PLUGIN=OFF to skip JUCE
cmake --build build
ctest --test-dir build --output-on-failure
./build/tools/bench/amp2203_bench                          # real-time factor, 60 s of a DI clip at 48 kHz x 16 (--factor N: x N; --hot: Master, Presence 1.0; b4 / b12 / b3 / b34 / preamp / b5 / b6 / b56: block costs)
./build/tools/ampwav/ampwav --block full --in in.wav --out out.wav --rate 768000 --param preampVolume=0.6   # --help for all options (M6 studies: --power-rate-divisor 2, --newton-tol V)
source .venv/bin/activate                                  # harness commands below need the venv
python -m harness.testbench_gen                            # regenerate harness/testbenches/*.cir (--check)
python -m harness.dc_check                                 # testbench DC vs full netlist (M0 sanity)
python -m harness.rth_measure                              # junction R_th (AC injection + DC), report
python -m harness.partition --junction j2                  # SPICE partition error of one junction
python -m harness.device_sweeps                            # tests/data device DC sweep CSVs (--check)
python -m harness.ot_fit                                   # OT fit to the 1750U datasheet (--check: netlist)
python -m harness.ot_fixtures                              # M4: OT network alone, SPICE fixtures for tests/test_ot.cpp (--check)
python -m harness.partition_j5 --variants self,diff,2port  # M4: J5 partition error in SPICE, closed loop (hours)
python -m harness.m4_acceptance                            # M4: full amp vs F (DC, NFB, CLEAN, stability, overdrive, CRUNCH / DRIVE + DI); env AMP2203_FS, AMP2203_POWER_DIVISOR, AMP2203_NEWTON_TOL for rate / M6 studies
python -m harness.plugin_null                              # M5: plugin (amp2203_render) vs ampwav at 768 kHz with the same resampling
python -m harness.automation                               # M5: every knob at audio rate, NaN / Newton check
./build/plugin/amp2203_render_artefacts/Release/amp2203_render plugin --in x.wav --out y.wav --factor 16   # offline plugin render
/Applications/pluginval.app/Contents/MacOS/pluginval --strictness-level 5 --validate <plugin>   # after copying to ~/Library/Audio/Plug-Ins
auval -v aufx A223 Ctcd
python -m harness.mna --fixtures                           # R-type S fixtures for the C++ cross-check (--check)
python -m harness.tonestack_check --tran                   # M1: B4 magnitude (375 settings), sweeps, R_th4
python -m harness.knob_jump [--block b12|b34|b1234|b56]   # knob-jump test vs SPICE (D-025): B4 (M1), B12 (M2), B34 / B1234 (M3), Presence B56 (M5)
python -m harness.diagnostics --block b12                  # M2: diode CJO/TT, engine-rate and SPICE-RELTOL convergence
python -m harness.cascade                                  # M3: preamp B1234 vs F / P / P56 at p1b, k2b, y (hours)
python -m harness.merge_study                              # M3 Q18: loaded-y partition with B12 + B34 merged (SPICE only)
python -m harness.render_spice --block <id> --preset <id>  # SPICE reference (cached in harness/cache/)
python -m harness.run_engine --block <id> --preset <id>
python -m harness.compare --block <id> --preset <id> --milestone M<n>   # renders + runs + report
python -m unittest discover -s harness/tests -t .          # harness unit tests
```
Planned: none (all M0 commands exist; later milestones add theirs here).

## Definition of done
Builds without warnings, tests pass, harness report regenerated and within tolerance (or deviation
accepted by the owner), `docs/PROGRESS.md` updated. See SPEC §9.
