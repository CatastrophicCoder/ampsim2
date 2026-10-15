# M4 acceptance: full amplifier vs the full netlist

- Date: 2026-10-14, engine commit `f1f3778 (dirty)`, ngspice 47, fs = 384000 Hz, J4 exact, SPICE reltol 1e-08
- Netlist sha256 `7bea8385e4adb04e`, tubes.lib sha256 `9ec33154be8127d7`
- Engine: `ampwav --block full` (volts at the jack and across the 16 Ω load), the checks of SPEC §7 M4.

## Summary

PASS 17 · FAIL 4

| Section | Check | Item | Measured | Tolerance | Status |
|---|---|---|---|---|---|
| hot | total_esr_db | HOT SIN-220Hz-0.3V | -58.9 dB | ≤ -20 | PASS |
| hot | total_harmonics_2_9_db | HOT SIN-220Hz-0.3V | 0.01274 dB | ≤ 1.5 | PASS |
| hot | newton_failures | HOT SIN-220Hz-0.3V | 0 | ≤ 0 | PASS |
| hot | newton_unrescued | HOT SIN-220Hz-0.3V | 0 | — | INFO |
| hot | newton_median_iterations | HOT SIN-220Hz-0.3V | 2 | ≤ 3 | PASS |
| hot | newton_max_iterations | HOT SIN-220Hz-0.3V | 7 | ≤ 16 | PASS |
| hot | total_esr_db | HOT DI-GTR L | -41.45 dB | ≤ -20 | PASS |
| hot | newton_failures | HOT DI-GTR L | 5 | ≤ 0 | FAIL |
| hot | newton_unrescued | HOT DI-GTR L | 0 | — | INFO |
| hot | newton_median_iterations | HOT DI-GTR L | 2 | ≤ 3 | PASS |
| hot | newton_max_iterations | HOT DI-GTR L | 16 | ≤ 16 | PASS |
| hot | total_esr_db | HOT DI-GTR LL | -42.13 dB | ≤ -20 | PASS |
| hot | newton_failures | HOT DI-GTR LL | 6 | ≤ 0 | FAIL |
| hot | newton_unrescued | HOT DI-GTR LL | 0 | — | INFO |
| hot | newton_median_iterations | HOT DI-GTR LL | 2 | ≤ 3 | PASS |
| hot | newton_max_iterations | HOT DI-GTR LL | 16 | ≤ 16 | PASS |
| hot | total_esr_db | HOT DI-GTR R | -42.66 dB | ≤ -20 | PASS |
| hot | newton_failures | HOT DI-GTR R | 6 | ≤ 0 | FAIL |
| hot | newton_unrescued | HOT DI-GTR R | 0 | — | INFO |
| hot | newton_median_iterations | HOT DI-GTR R | 2 | ≤ 3 | PASS |
| hot | newton_max_iterations | HOT DI-GTR R | 16 | ≤ 16 | PASS |
| hot | total_esr_db | HOT DI-GTR RR | -41.83 dB | ≤ -20 | PASS |
| hot | newton_failures | HOT DI-GTR RR | 8 | ≤ 0 | FAIL |
| hot | newton_unrescued | HOT DI-GTR RR | 0 | — | INFO |
| hot | newton_median_iterations | HOT DI-GTR RR | 2 | ≤ 3 | PASS |
| hot | newton_max_iterations | HOT DI-GTR RR | 16 | ≤ 16 | PASS |

## Signals

| Preset | Signal | ESR | best lag | harm. 2–9 | THD F / E | Power F / E | min v(ct) F / E |
|---|---|---|---|---|---|---|---|
| HOT | SIN-220Hz-0.3V | -58.9 dB | 0 | 0.01 dB | 46.17 / 46.17 % | 183.4 / 183.4 W | — |
| HOT | DI-GTR L | -41.5 dB | 0 | — | — | — | — |
| HOT | DI-GTR LL | -42.1 dB | 0 | — | — | — | — |
| HOT | DI-GTR R | -42.7 dB | 0 | — | — | — | — |
| HOT | DI-GTR RR | -41.8 dB | 0 | — | — | — | — |

## Delayed-NFB break

Not applicable: since D-041 the global NFB (R22) is part of the B56 tree and has no delay (SPEC §7 M4 asked for a 1- vs 2-sample comparison of the delayed design).
