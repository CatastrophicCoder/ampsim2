# M0 report: full / CLEAN

- Block: Full amplifier (reference F)
- Preset: CLEAN — 25 mV peak 1 kHz (about 100 W in SPICE)
- Knobs: preampVolume=0.2, treble=0.5, middle=0.5, bass=0.5, master=1, presence=0.5
- Date: 2026-10-12
- Engine commit: `f311402 (dirty)`
- Testbench: `harness/testbenches/full.cir` sha256 `9c21536867160578`; netlist sha256 `5b0b8095ad124450`; tubes.lib sha256 `9ec33154be8127d7`
- ngspice 47, fs = 384000 Hz, reference: F (full netlist)

## Summary

PASS 0 · FAIL 0 · INFO 5 (INFO = measured, not scored in M0)

| Check | Item | Measured | Tolerance | Status |
|---|---|---|---|---|
| total_esr_db | SIN-1000Hz-0.025V | 0.0002935 dB | ≤ -20 | INFO |
| harmonics_2_5_db | SIN-1000Hz-0.025V | 209.2 dB | ≤ 0.5 | INFO |
| total_thd_pp | SIN-1000Hz-0.025V | 3.197 pp | ≤ 0.5 | INFO |
| total_power_rel | SIN-1000Hz-0.025V | 1 | ≤ 0.05 | INFO |
| newton_failures | SIN-1000Hz-0.025V | 0 | ≤ 0 | INFO |

## Signals

| Signal | Peak F [V] | ESR [dB] | Best lag | ESR at lag [dB] | THD F / E [%] | Power F / E [W] | ns/sample | Newton fail / max it |
|---|---|---|---|---|---|---|---|---|
| SIN-1000Hz-0.025V | 69.24 | 0.00 | -2 | 0.00 | 3.2 / 4.14e-06 | 99.57 / 1.953e-05 | 0.9 | 0 / 0 |

## DC operating point

| Node | SPICE [V] | Engine [V] | Rel. error |
|---|---|---|---|
| ct | 488.4450 | — | — |
| nscr | 486.8281 | — | — |
| bpx | 403.3073 | — | — |
| bpa | 390.2881 | — | — |
| nbias | -46.3636 | — | — |
| p1a | 297.1044 | — | — |
| k1a | 2.5160 | — | — |
| p1b | 353.2796 | — | — |
| k1b | 3.7008 | — | — |
| p2a | 231.6577 | — | — |
| k2a | 1.4075 | — | — |
| k2b | 232.2166 | — | — |
| p3a | 271.5123 | — | — |
| p3b | 262.8838 | — | — |
| k3 | 45.0490 | — | — |
| nf | 43.6336 | — | — |
| ng | 13.5186 | — | — |

Engine values are the probe values after the first sample (zero input) following `reset()`. Nodes the engine does not probe yet show —.

## Figures

### Waveform overlay and error (most demanding signal)

![Waveform overlay and error (most demanding signal)](20261012_full_CLEAN/waveform.png)

### Harmonics

![Harmonics](20261012_full_CLEAN/harmonics.png)
