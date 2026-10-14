# M4 acceptance: full amplifier vs the full netlist

- Date: 2026-10-14, engine commit `46721e2 (dirty)`, ngspice 47, fs = 384000 Hz, SPICE reltol 1e-08
- Netlist sha256 `7bea8385e4adb04e`, tubes.lib sha256 `9ec33154be8127d7`
- Engine: `ampwav --block full` (volts at the jack and across the 16 Ω load), the checks of SPEC §7 M4.

## Summary

PASS 11 · FAIL 10

| Section | Check | Item | Measured | Tolerance | Status |
|---|---|---|---|---|---|
| hot | total_esr_db | HOT SIN-220Hz-0.3V | -12.61 dB | ≤ -20 | FAIL |
| hot | total_harmonics_2_9_db | HOT SIN-220Hz-0.3V | 11.56 dB | ≤ 1.5 | FAIL |
| hot | newton_failures | HOT SIN-220Hz-0.3V | 0 | ≤ 0 | PASS |
| hot | newton_unrescued | HOT SIN-220Hz-0.3V | 0 | — | INFO |
| hot | newton_median_iterations | HOT SIN-220Hz-0.3V | 2 | ≤ 3 | PASS |
| hot | newton_max_iterations | HOT SIN-220Hz-0.3V | 6 | ≤ 16 | PASS |
| hot | total_esr_db | HOT DI-GTR L | -13.07 dB | ≤ -20 | FAIL |
| hot | newton_failures | HOT DI-GTR L | 8 | ≤ 0 | FAIL |
| hot | newton_unrescued | HOT DI-GTR L | 0 | — | INFO |
| hot | newton_median_iterations | HOT DI-GTR L | 2 | ≤ 3 | PASS |
| hot | newton_max_iterations | HOT DI-GTR L | 16 | ≤ 16 | PASS |
| hot | total_esr_db | HOT DI-GTR LL | -13.32 dB | ≤ -20 | FAIL |
| hot | newton_failures | HOT DI-GTR LL | 6 | ≤ 0 | FAIL |
| hot | newton_unrescued | HOT DI-GTR LL | 0 | — | INFO |
| hot | newton_median_iterations | HOT DI-GTR LL | 2 | ≤ 3 | PASS |
| hot | newton_max_iterations | HOT DI-GTR LL | 16 | ≤ 16 | PASS |
| hot | total_esr_db | HOT DI-GTR R | -13.1 dB | ≤ -20 | FAIL |
| hot | newton_failures | HOT DI-GTR R | 6 | ≤ 0 | FAIL |
| hot | newton_unrescued | HOT DI-GTR R | 0 | — | INFO |
| hot | newton_median_iterations | HOT DI-GTR R | 2 | ≤ 3 | PASS |
| hot | newton_max_iterations | HOT DI-GTR R | 16 | ≤ 16 | PASS |
| hot | total_esr_db | HOT DI-GTR RR | -12.9 dB | ≤ -20 | FAIL |
| hot | newton_failures | HOT DI-GTR RR | 3 | ≤ 0 | FAIL |
| hot | newton_unrescued | HOT DI-GTR RR | 0 | — | INFO |
| hot | newton_median_iterations | HOT DI-GTR RR | 2 | ≤ 3 | PASS |
| hot | newton_max_iterations | HOT DI-GTR RR | 16 | ≤ 16 | PASS |

## Signals

| Preset | Signal | ESR | best lag | harm. 2–9 | THD F / E | Power F / E | min v(ct) F / E |
|---|---|---|---|---|---|---|---|
| HOT | SIN-220Hz-0.3V | -12.6 dB | 2 | 11.56 dB | 46.17 / 44.73 % | 183.4 / 190.1 W | — |
| HOT | DI-GTR L | -13.1 dB | 2 | — | — | — | — |
| HOT | DI-GTR LL | -13.3 dB | 2 | — | — | — | — |
| HOT | DI-GTR R | -13.1 dB | 2 | — | — | — | — |
| HOT | DI-GTR RR | -12.9 dB | 2 | — | — | — | — |

## Delayed-NFB break

Not applicable: since D-041 the global NFB (R22) is part of the B56 tree and has no delay (SPEC §7 M4 asked for a 1- vs 2-sample comparison of the delayed design).
