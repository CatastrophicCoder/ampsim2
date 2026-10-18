# M4 acceptance: full amplifier vs the full netlist

- Date: 2026-10-18, engine commit `d37baf0 (dirty)`, ngspice 47, fs = 384000 Hz, J4 exact, SPICE reltol 1e-08
- Netlist sha256 `ffd9b2812204248f`, tubes.lib sha256 `c9c9c88346c69010`
- Engine: `ampwav --block full` (volts at the jack and across the 16 Ω load), the checks of SPEC §7 M4.

## Summary

PASS 20 · FAIL 1

| Section | Check | Item | Measured | Tolerance | Status |
|---|---|---|---|---|---|
| hot | total_esr_db | HOT SIN-220Hz-0.3V | -54.53 dB | ≤ -20 | PASS |
| hot | total_harmonics_2_9_db | HOT SIN-220Hz-0.3V | 0.008763 dB | ≤ 1.5 | PASS |
| hot | newton_failures | HOT SIN-220Hz-0.3V | 0 | ≤ 0 | PASS |
| hot | newton_unrescued | HOT SIN-220Hz-0.3V | 0 | — | INFO |
| hot | newton_median_iterations | HOT SIN-220Hz-0.3V | 2 | ≤ 3 | PASS |
| hot | newton_max_iterations | HOT SIN-220Hz-0.3V | 6 | ≤ 32 | PASS |
| hot | total_esr_db | HOT DI-GTR L | -40.27 dB | ≤ -20 | PASS |
| hot | newton_failures | HOT DI-GTR L | 0 | ≤ 0 | PASS |
| hot | newton_unrescued | HOT DI-GTR L | 0 | — | INFO |
| hot | newton_median_iterations | HOT DI-GTR L | 2 | ≤ 3 | PASS |
| hot | newton_max_iterations | HOT DI-GTR L | 15 | ≤ 32 | PASS |
| hot | total_esr_db | HOT DI-GTR LL | -41 dB | ≤ -20 | PASS |
| hot | newton_failures | HOT DI-GTR LL | 0 | ≤ 0 | PASS |
| hot | newton_unrescued | HOT DI-GTR LL | 0 | — | INFO |
| hot | newton_median_iterations | HOT DI-GTR LL | 2 | ≤ 3 | PASS |
| hot | newton_max_iterations | HOT DI-GTR LL | 13 | ≤ 32 | PASS |
| hot | total_esr_db | HOT DI-GTR R | -41.5 dB | ≤ -20 | PASS |
| hot | newton_failures | HOT DI-GTR R | 0 | ≤ 0 | PASS |
| hot | newton_unrescued | HOT DI-GTR R | 0 | — | INFO |
| hot | newton_median_iterations | HOT DI-GTR R | 2 | ≤ 3 | PASS |
| hot | newton_max_iterations | HOT DI-GTR R | 12 | ≤ 32 | PASS |
| hot | total_esr_db | HOT DI-GTR RR | -40.86 dB | ≤ -20 | PASS |
| hot | newton_failures | HOT DI-GTR RR | 1 | ≤ 0 | FAIL |
| hot | newton_unrescued | HOT DI-GTR RR | 0 | — | INFO |
| hot | newton_median_iterations | HOT DI-GTR RR | 2 | ≤ 3 | PASS |
| hot | newton_max_iterations | HOT DI-GTR RR | 32 | ≤ 32 | PASS |

## Signals

| Preset | Signal | ESR | best lag | harm. 2–9 | THD F / E | Power F / E | min v(ct) F / E |
|---|---|---|---|---|---|---|---|
| HOT | SIN-220Hz-0.3V | -54.5 dB | 0 | 0.01 dB | 37.86 / 37.86 % | 178.4 / 178.4 W | — |
| HOT | DI-GTR L | -40.3 dB | 0 | — | — | — | — |
| HOT | DI-GTR LL | -41.0 dB | 0 | — | — | — | — |
| HOT | DI-GTR R | -41.5 dB | 0 | — | — | — | — |
| HOT | DI-GTR RR | -40.9 dB | 0 | — | — | — | — |

## Delayed-NFB break

Not applicable: since D-041 the global NFB (R22) is part of the B56 tree and has no delay (SPEC §7 M4 asked for a 1- vs 2-sample comparison of the delayed design).
