# M4 acceptance: full amplifier vs the full netlist

- Date: 2026-10-16, engine commit `3cb4124 (dirty)`, ngspice 47, fs = 384000 Hz, J4 exact, SPICE reltol 1e-08
- Netlist sha256 `83f01b6fd8b64a06`, tubes.lib sha256 `5767afe208240bfa`
- Engine: `ampwav --block full` (volts at the jack and across the 16 Ω load), the checks of SPEC §7 M4.

## Summary

PASS 17 · FAIL 4

| Section | Check | Item | Measured | Tolerance | Status |
|---|---|---|---|---|---|
| hot | total_esr_db | HOT SIN-220Hz-0.3V | -54.54 dB | ≤ -20 | PASS |
| hot | total_harmonics_2_9_db | HOT SIN-220Hz-0.3V | 0.008907 dB | ≤ 1.5 | PASS |
| hot | newton_failures | HOT SIN-220Hz-0.3V | 0 | ≤ 0 | PASS |
| hot | newton_unrescued | HOT SIN-220Hz-0.3V | 0 | — | INFO |
| hot | newton_median_iterations | HOT SIN-220Hz-0.3V | 2 | ≤ 3 | PASS |
| hot | newton_max_iterations | HOT SIN-220Hz-0.3V | 7 | ≤ 32 | PASS |
| hot | total_esr_db | HOT DI-GTR L | -33.49 dB | ≤ -20 | PASS |
| hot | newton_failures | HOT DI-GTR L | 56 | ≤ 0 | FAIL |
| hot | newton_unrescued | HOT DI-GTR L | 0 | — | INFO |
| hot | newton_median_iterations | HOT DI-GTR L | 2 | ≤ 3 | PASS |
| hot | newton_max_iterations | HOT DI-GTR L | 32 | ≤ 32 | PASS |
| hot | total_esr_db | HOT DI-GTR LL | -35.1 dB | ≤ -20 | PASS |
| hot | newton_failures | HOT DI-GTR LL | 45 | ≤ 0 | FAIL |
| hot | newton_unrescued | HOT DI-GTR LL | 0 | — | INFO |
| hot | newton_median_iterations | HOT DI-GTR LL | 2 | ≤ 3 | PASS |
| hot | newton_max_iterations | HOT DI-GTR LL | 32 | ≤ 32 | PASS |
| hot | total_esr_db | HOT DI-GTR R | -35.73 dB | ≤ -20 | PASS |
| hot | newton_failures | HOT DI-GTR R | 37 | ≤ 0 | FAIL |
| hot | newton_unrescued | HOT DI-GTR R | 0 | — | INFO |
| hot | newton_median_iterations | HOT DI-GTR R | 2 | ≤ 3 | PASS |
| hot | newton_max_iterations | HOT DI-GTR R | 32 | ≤ 32 | PASS |
| hot | total_esr_db | HOT DI-GTR RR | -34.89 dB | ≤ -20 | PASS |
| hot | newton_failures | HOT DI-GTR RR | 49 | ≤ 0 | FAIL |
| hot | newton_unrescued | HOT DI-GTR RR | 0 | — | INFO |
| hot | newton_median_iterations | HOT DI-GTR RR | 2 | ≤ 3 | PASS |
| hot | newton_max_iterations | HOT DI-GTR RR | 32 | ≤ 32 | PASS |

## Signals

| Preset | Signal | ESR | best lag | harm. 2–9 | THD F / E | Power F / E | min v(ct) F / E |
|---|---|---|---|---|---|---|---|
| HOT | SIN-220Hz-0.3V | -54.5 dB | 0 | 0.01 dB | 37.87 / 37.87 % | 178.4 / 178.4 W | — |
| HOT | DI-GTR L | -33.5 dB | 0 | — | — | — | — |
| HOT | DI-GTR LL | -35.1 dB | 0 | — | — | — | — |
| HOT | DI-GTR R | -35.7 dB | 0 | — | — | — | — |
| HOT | DI-GTR RR | -34.9 dB | 0 | — | — | — | — |

## Delayed-NFB break

Not applicable: since D-041 the global NFB (R22) is part of the B56 tree and has no delay (SPEC §7 M4 asked for a 1- vs 2-sample comparison of the delayed design).
