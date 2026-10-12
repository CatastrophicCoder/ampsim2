# wdf-amp-2203

Real-time circuit model of the Marshall JCM800 2203 (100 W master-volume head) in C++20, using Wave
Digital Filters (`chowdsp_wdf`). Every block is verified against an ngspice reference model of the full
amp with a Python harness, and the engine is wrapped as a JUCE plugin (AU / VST3 / Standalone).

**Status:** milestone M0 (build skeleton and verification harness) is in progress; the engine is still
a pass-through. Milestones M0–M5 are defined in `docs/SPEC.md`; current state and decisions are in
`docs/PROGRESS.md` and `docs/DECISIONS.md`.

## How it works
- **Reference:** `reference/spice/jcm800_2203.cir` is a full-amp ngspice netlist (preamp, tone stack,
  long-tail-pair phase inverter with negative feedback and presence, 4× EL34, output transformer, HT
  supply, bias) with Koren tube models. It is the ground truth.
- **Engine:** the circuit is split into blocks, each one WDF tree with at most one nonlinear (tube)
  root, solved with a bounded multiport Newton iteration. Blocks are joined by Thevenin junctions whose
  error is measured in SPICE before they are used. The engine is JUCE-free; only `plugin/` uses JUCE.
- **Verification:** the harness generates block testbenches from the netlist, renders SPICE references
  at 384 kHz, runs the engine on the same signals through the `ampwav` CLI, and writes reports with
  ESR, harmonics, frequency response and DC tables against fixed tolerances (`docs/VERIFICATION.md`).

## Repository layout
```
docs/          SPEC, CIRCUIT, WDF_DESIGN, VERIFICATION, DECISIONS, PROGRESS
reference/     SPICE netlist and tube models, reference plots, DI recordings folder (not included)
engine/        real-time engine (C++20, JUCE-free)
plugin/        JUCE plugin wrapper
tools/         ampwav (WAV in/out CLI for the harness) and a benchmark
tests/         Catch2 unit tests; tests/data/ holds SPICE device DC sweeps
harness/       Python verification harness, generated testbenches, committed reports
.github/       CI: build and unit tests on macOS (plugin off)
CLAUDE.md      working rules for Claude Code, which develops this project with the owner
```

## Quick start (macOS)
```bash
brew install ngspice cmake ninja
python3 -m venv .venv && source .venv/bin/activate
pip install -r requirements.txt

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release   # add -DAMP2203_BUILD_PLUGIN=OFF to skip JUCE
cmake --build build
ctest --test-dir build --output-on-failure

python3 reference/spice/spice_tools.py                     # reference DC operating point
python -m unittest discover -s harness/tests -t .          # harness tests (need ngspice)
python -m harness.compare --block b12 --preset DEFAULT --milestone M0
```
All commands are listed in `CLAUDE.md` ("Commands").

## Circuit source
The netlist was transcribed from Marshall's "2203 STD Preamp" (issue 4, 19-5-88) and "2203 STD Output
Stage & PSU" (issue 6) drawings. The drawings are not included in this repository. Values that are not on
the drawings are either taken from other sources or assumed: the output transformer uses the datasheet of
Hammond's drop-in replacement part (1750U), with its primary inductance fitted to the published response
curves; the choke, HT source and bias are assumptions. All are listed in `docs/CIRCUIT.md` §3 and §6 and in
`reference/README.md`.

## Licence
Copyright (C) 2026 CatastrophicCoder. This project is licensed under the GNU Affero General Public
License v3.0 (`LICENSE`).

Third-party dependencies are downloaded at build time (pinned in `cmake/Dependencies.cmake`, see
`docs/DECISIONS.md` D-011) and keep their own licences: chowdsp_wdf (BSD-3-Clause), Catch2 (BSL-1.0),
dr_libs (public domain / MIT-0) and JUCE 8 (AGPLv3 or a JUCE commercial licence; distributed plugin
builds must meet the terms of one of them).

## Disclaimer
This is an independent modelling project. It is not affiliated with, endorsed by or sponsored by Marshall
Amplification plc. "Marshall" and "JCM800" are trademarks of their owner and are used here only to
identify the circuit being modelled.
