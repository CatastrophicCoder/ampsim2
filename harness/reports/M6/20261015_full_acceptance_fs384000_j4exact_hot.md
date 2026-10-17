# M4 acceptance: full amplifier vs the full netlist

- Date: 2026-10-15, engine commit `476d21f (dirty)`, ngspice 47, fs = 384000 Hz, J4 exact, SPICE reltol 1e-08
- Netlist sha256 `7bea8385e4adb04e`, tubes.lib sha256 `5767afe208240bfa`
- Engine: `ampwav --block full` (volts at the jack and across the 16 Ω load), the checks of SPEC §7 M4.

## Summary

PASS 21 · FAIL 0

| Section | Check | Item | Measured | Tolerance | Status |
|---|---|---|---|---|---|
| hot | total_esr_db | HOT SIN-220Hz-0.3V | -58.95 dB | ≤ -20 | PASS |
| hot | total_harmonics_2_9_db | HOT SIN-220Hz-0.3V | 0.01735 dB | ≤ 1.5 | PASS |
| hot | newton_failures | HOT SIN-220Hz-0.3V | 0 | ≤ 0 | PASS |
| hot | newton_unrescued | HOT SIN-220Hz-0.3V | 0 | — | INFO |
| hot | newton_median_iterations | HOT SIN-220Hz-0.3V | 2 | ≤ 3 | PASS |
| hot | newton_max_iterations | HOT SIN-220Hz-0.3V | 7 | ≤ 32 | PASS |
| hot | total_esr_db | HOT DI-GTR L | -42.16 dB | ≤ -20 | PASS |
| hot | newton_failures | HOT DI-GTR L | 0 | ≤ 0 | PASS |
| hot | newton_unrescued | HOT DI-GTR L | 0 | — | INFO |
| hot | newton_median_iterations | HOT DI-GTR L | 2 | ≤ 3 | PASS |
| hot | newton_max_iterations | HOT DI-GTR L | 16 | ≤ 32 | PASS |
| hot | total_esr_db | HOT DI-GTR LL | -42.63 dB | ≤ -20 | PASS |
| hot | newton_failures | HOT DI-GTR LL | 0 | ≤ 0 | PASS |
| hot | newton_unrescued | HOT DI-GTR LL | 0 | — | INFO |
| hot | newton_median_iterations | HOT DI-GTR LL | 2 | ≤ 3 | PASS |
| hot | newton_max_iterations | HOT DI-GTR LL | 17 | ≤ 32 | PASS |
| hot | total_esr_db | HOT DI-GTR R | -43.31 dB | ≤ -20 | PASS |
| hot | newton_failures | HOT DI-GTR R | 0 | ≤ 0 | PASS |
| hot | newton_unrescued | HOT DI-GTR R | 0 | — | INFO |
| hot | newton_median_iterations | HOT DI-GTR R | 2 | ≤ 3 | PASS |
| hot | newton_max_iterations | HOT DI-GTR R | 17 | ≤ 32 | PASS |
| hot | total_esr_db | HOT DI-GTR RR | -42.63 dB | ≤ -20 | PASS |
| hot | newton_failures | HOT DI-GTR RR | 0 | ≤ 0 | PASS |
| hot | newton_unrescued | HOT DI-GTR RR | 0 | — | INFO |
| hot | newton_median_iterations | HOT DI-GTR RR | 2 | ≤ 3 | PASS |
| hot | newton_max_iterations | HOT DI-GTR RR | 17 | ≤ 32 | PASS |

## Signals

| Preset | Signal | ESR | best lag | harm. 2–9 | THD F / E | Power F / E | min v(ct) F / E |
|---|---|---|---|---|---|---|---|
| HOT | SIN-220Hz-0.3V | -59.0 dB | 0 | 0.02 dB | 46.42 / 46.42 % | 157.6 / 157.6 W | — |
| HOT | DI-GTR L | -42.2 dB | 0 | — | — | — | — |
| HOT | DI-GTR LL | -42.6 dB | 0 | — | — | — | — |
| HOT | DI-GTR R | -43.3 dB | 0 | — | — | — | — |
| HOT | DI-GTR RR | -42.6 dB | 0 | — | — | — | — |

## Delayed-NFB break

Not applicable: since D-041 the global NFB (R22) is part of the B56 tree and has no delay (SPEC §7 M4 asked for a 1- vs 2-sample comparison of the delayed design).
