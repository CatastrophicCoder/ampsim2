# wdf-amp-2203

Real-time circuit model of the Marshall JCM800 2203 (100 W master-volume head) using Wave Digital
Filters (`chowdsp_wdf`), verified block by block against an ngspice reference model, and wrapped as a
JUCE plugin (AU / VST3 / Standalone).

**Status:** specification and SPICE reference complete; implementation starts at milestone M0.
See `docs/PROGRESS.md`.

## Repository contents (initial)

```
CLAUDE.md                     instructions for Claude Code
docs/
  SPEC.md                     scope, architecture, engine API, milestones + acceptance criteria
  CIRCUIT.md                  component tables per block, reference DC values, open items
  WDF_DESIGN.md               equations and algorithms (tube roots, Newton, R-type scattering, supply)
  VERIFICATION.md             SPICE harness, test signals, metrics, tolerances, report format
  DECISIONS.md                decision log
  PROGRESS.md                 milestone tracking
reference/
  spice/jcm800_2203.cir       full-amp ngspice netlist (ground truth)
  spice/tubes.lib             Koren 12AX7 / EL34 models (with screen-current modification)
  spice/spice_tools.py        Python helpers: op / ac / tran / THD+power
  schematics/                 Marshall 2203 STD preamp and output stage/PSU drawings
  plots/                      reference tone-stack sweeps and overdrive stage waveforms
  di/                         place DI guitar recordings here (not included)
  README.md                   notes on the reference model, assumptions and sanity results
requirements.txt              Python tooling dependencies
```

Planned layout after M0 (`engine/`, `tests/`, `tools/`, `plugin/`, `harness/`) is described in
`docs/SPEC.md` §3.1.

## Quick start

```bash
brew install ngspice cmake ninja
python3 -m venv .venv && source .venv/bin/activate
pip install -r requirements.txt
python3 reference/spice/spice_tools.py      # prints the reference DC operating point
```

Then open the repository in Claude Code and start with: "Read CLAUDE.md and the docs, then plan M0."

## Notes
- Third-party licences: chowdsp_wdf is BSD-3-Clause; check JUCE's current licence terms for your use.
- "Marshall" and "JCM800" are trademarks of their owner; this is a personal modelling project.
