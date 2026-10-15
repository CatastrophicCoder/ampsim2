# M4 acceptance: full amplifier vs the full netlist

- Date: 2026-10-14, engine commit `f1f3778 (dirty)`, ngspice 47, fs = 384000 Hz, J4 exact, SPICE reltol 1e-08
- Netlist sha256 `7bea8385e4adb04e`, tubes.lib sha256 `9ec33154be8127d7`
- Engine: `ampwav --block full` (volts at the jack and across the 16 Ω load), the checks of SPEC §7 M4.

## Summary

PASS 107 · FAIL 0

| Section | Check | Item | Measured | Tolerance | Status |
|---|---|---|---|---|---|
| dc | dc_rel_error | v(ct) | 4.141e-07 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(nscr) | 4.454e-07 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(bpx) | 3.65e-06 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(bpa) | 5.682e-06 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(nbias) | 4.636e-08 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(p1a) | 1.996e-05 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(k1a) | 3.983e-05 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(p1b) | 1.688e-05 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(k1b) | 0.0001012 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(p2a) | 2.198e-05 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(k2a) | 2.11e-05 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(k2b) | 2.143e-05 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(p3a) | 1.578e-05 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(p3b) | 1.709e-05 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(k3) | 2.143e-05 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(nf) | 2.143e-05 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(ng) | 2.143e-05 | ≤ 0.005 | PASS |
| dc | el34_plate_ma | ip_tube_a | 1.194e-05 mA | ≤ 1 | PASS |
| dc | el34_plate_ma | ip_tube_b | 1.195e-05 mA | ≤ 1 | PASS |
| dc | el34_screen_ma | is_tube_a | 1.046e-06 mA | ≤ 0.3 | PASS |
| dc | el34_screen_ma | is_tube_b | 1.044e-06 mA | ≤ 0.3 | PASS |
| dc | ct_rel | v(ct) | 4.141e-07 | ≤ 0.01 | PASS |
| dc | nbias_v | v(nbias) | 2.15e-06 V | ≤ 0.2 | PASS |
| nfb | nfb_gain_db | closed 100 Hz | 0.001954 dB | ≤ 0.5 | PASS |
| nfb | newton_failures | closed 100 Hz | 0 | ≤ 0 | PASS |
| nfb | newton_unrescued | closed 100 Hz | 0 | — | INFO |
| nfb | newton_median_iterations | closed 100 Hz | 1 | ≤ 3 | PASS |
| nfb | newton_max_iterations | closed 100 Hz | 2 | ≤ 16 | PASS |
| nfb | nfb_gain_db | closed 1000 Hz | 0.004996 dB | ≤ 0.5 | PASS |
| nfb | newton_failures | closed 1000 Hz | 0 | ≤ 0 | PASS |
| nfb | newton_unrescued | closed 1000 Hz | 0 | — | INFO |
| nfb | newton_median_iterations | closed 1000 Hz | 2 | ≤ 3 | PASS |
| nfb | newton_max_iterations | closed 1000 Hz | 2 | ≤ 16 | PASS |
| nfb | nfb_gain_db | closed 5000 Hz | 0.01361 dB | ≤ 0.5 | PASS |
| nfb | newton_failures | closed 5000 Hz | 0 | ≤ 0 | PASS |
| nfb | newton_unrescued | closed 5000 Hz | 0 | — | INFO |
| nfb | newton_median_iterations | closed 5000 Hz | 2 | ≤ 3 | PASS |
| nfb | newton_max_iterations | closed 5000 Hz | 2 | ≤ 16 | PASS |
| nfb | nfb_gain_db | open 100 Hz | 0.002342 dB | ≤ 0.5 | PASS |
| nfb | newton_failures | open 100 Hz | 0 | ≤ 0 | PASS |
| nfb | newton_unrescued | open 100 Hz | 0 | — | INFO |
| nfb | newton_median_iterations | open 100 Hz | 1 | ≤ 3 | PASS |
| nfb | newton_max_iterations | open 100 Hz | 2 | ≤ 16 | PASS |
| nfb | nfb_gain_db | open 1000 Hz | 0.0057 dB | ≤ 0.5 | PASS |
| nfb | newton_failures | open 1000 Hz | 0 | ≤ 0 | PASS |
| nfb | newton_unrescued | open 1000 Hz | 0 | — | INFO |
| nfb | newton_median_iterations | open 1000 Hz | 2 | ≤ 3 | PASS |
| nfb | newton_max_iterations | open 1000 Hz | 2 | ≤ 16 | PASS |
| nfb | nfb_gain_db | open 5000 Hz | 0.02095 dB | ≤ 0.5 | PASS |
| nfb | newton_failures | open 5000 Hz | 0 | ≤ 0 | PASS |
| nfb | newton_unrescued | open 5000 Hz | 0 | — | INFO |
| nfb | newton_median_iterations | open 5000 Hz | 2 | ≤ 3 | PASS |
| nfb | newton_max_iterations | open 5000 Hz | 2 | ≤ 16 | PASS |
| clean | total_esr_db | CLEAN SIN-1000Hz-0.025V | -60.06 dB | ≤ -20 | PASS |
| clean | total_harmonics_2_9_db | CLEAN SIN-1000Hz-0.025V | 0.02042 dB | ≤ 1.5 | PASS |
| clean | newton_failures | CLEAN SIN-1000Hz-0.025V | 0 | ≤ 0 | PASS |
| clean | newton_unrescued | CLEAN SIN-1000Hz-0.025V | 0 | — | INFO |
| clean | newton_median_iterations | CLEAN SIN-1000Hz-0.025V | 2 | ≤ 3 | PASS |
| clean | newton_max_iterations | CLEAN SIN-1000Hz-0.025V | 6 | ≤ 16 | PASS |
| clean | total_power_rel | CLEAN SIN-1000Hz-0.025V | 0.001105 | ≤ 0.05 | PASS |
| clean | total_thd_pp | CLEAN SIN-1000Hz-0.025V | 0.0006695 pp | ≤ 0.5 | PASS |
| stability | stability_out_v | max |out| | 4.428e-05 V | ≤ 0.001 | PASS |
| stability | stability_growth | RMS last 10 s − first 10 s | -3.968e-12 V | ≤ 1e-06 | PASS |
| stability | newton_failures | 60 s silence | 0 | ≤ 0 | PASS |
| stability | newton_unrescued | 60 s silence | 0 | — | INFO |
| stability | newton_median_iterations | 60 s silence | 1 | ≤ 3 | PASS |
| stability | newton_max_iterations | 60 s silence | 1 | ≤ 16 | PASS |
| overdrive | total_esr_db | OVERDRIVE SIN-220Hz-0.3V | -67.66 dB | ≤ -20 | PASS |
| overdrive | total_harmonics_2_9_db | OVERDRIVE SIN-220Hz-0.3V | 0.008332 dB | ≤ 1.5 | PASS |
| overdrive | sag_min_v | OVERDRIVE SIN-220Hz-0.3V | 0.004485 V | ≤ 2 | PASS |
| overdrive | newton_failures | OVERDRIVE SIN-220Hz-0.3V | 0 | ≤ 0 | PASS |
| overdrive | newton_unrescued | OVERDRIVE SIN-220Hz-0.3V | 0 | — | INFO |
| overdrive | newton_median_iterations | OVERDRIVE SIN-220Hz-0.3V | 2 | ≤ 3 | PASS |
| overdrive | newton_max_iterations | OVERDRIVE SIN-220Hz-0.3V | 6 | ≤ 16 | PASS |
| crunch_drive | total_esr_db | CRUNCH SIN-220Hz-0.3V | -69.99 dB | ≤ -20 | PASS |
| crunch_drive | total_harmonics_2_9_db | CRUNCH SIN-220Hz-0.3V | 0.01118 dB | ≤ 1.5 | PASS |
| crunch_drive | newton_failures | CRUNCH SIN-220Hz-0.3V | 0 | ≤ 0 | PASS |
| crunch_drive | newton_unrescued | CRUNCH SIN-220Hz-0.3V | 0 | — | INFO |
| crunch_drive | newton_median_iterations | CRUNCH SIN-220Hz-0.3V | 2 | ≤ 3 | PASS |
| crunch_drive | newton_max_iterations | CRUNCH SIN-220Hz-0.3V | 5 | ≤ 16 | PASS |
| crunch_drive | total_esr_db | CRUNCH DI-GTR L | -61.26 dB | ≤ -20 | PASS |
| crunch_drive | newton_failures | CRUNCH DI-GTR L | 0 | ≤ 0 | PASS |
| crunch_drive | newton_unrescued | CRUNCH DI-GTR L | 0 | — | INFO |
| crunch_drive | newton_median_iterations | CRUNCH DI-GTR L | 2 | ≤ 3 | PASS |
| crunch_drive | newton_max_iterations | CRUNCH DI-GTR L | 8 | ≤ 16 | PASS |
| crunch_drive | total_esr_db | CRUNCH DI-GTR LL | -61.57 dB | ≤ -20 | PASS |
| crunch_drive | newton_failures | CRUNCH DI-GTR LL | 0 | ≤ 0 | PASS |
| crunch_drive | newton_unrescued | CRUNCH DI-GTR LL | 0 | — | INFO |
| crunch_drive | newton_median_iterations | CRUNCH DI-GTR LL | 2 | ≤ 3 | PASS |
| crunch_drive | newton_max_iterations | CRUNCH DI-GTR LL | 8 | ≤ 16 | PASS |
| crunch_drive | total_esr_db | CRUNCH DI-GTR R | -62.34 dB | ≤ -20 | PASS |
| crunch_drive | newton_failures | CRUNCH DI-GTR R | 0 | ≤ 0 | PASS |
| crunch_drive | newton_unrescued | CRUNCH DI-GTR R | 0 | — | INFO |
| crunch_drive | newton_median_iterations | CRUNCH DI-GTR R | 2 | ≤ 3 | PASS |
| crunch_drive | newton_max_iterations | CRUNCH DI-GTR R | 8 | ≤ 16 | PASS |
| crunch_drive | total_esr_db | CRUNCH DI-GTR RR | -61.11 dB | ≤ -20 | PASS |
| crunch_drive | newton_failures | CRUNCH DI-GTR RR | 0 | ≤ 0 | PASS |
| crunch_drive | newton_unrescued | CRUNCH DI-GTR RR | 0 | — | INFO |
| crunch_drive | newton_median_iterations | CRUNCH DI-GTR RR | 2 | ≤ 3 | PASS |
| crunch_drive | newton_max_iterations | CRUNCH DI-GTR RR | 11 | ≤ 16 | PASS |
| crunch_drive | total_esr_db | DRIVE SIN-220Hz-0.3V | -63.6 dB | ≤ -20 | PASS |
| crunch_drive | total_harmonics_2_9_db | DRIVE SIN-220Hz-0.3V | 0.03526 dB | ≤ 1.5 | PASS |
| crunch_drive | newton_failures | DRIVE SIN-220Hz-0.3V | 0 | ≤ 0 | PASS |
| crunch_drive | newton_unrescued | DRIVE SIN-220Hz-0.3V | 0 | — | INFO |
| crunch_drive | newton_median_iterations | DRIVE SIN-220Hz-0.3V | 2 | ≤ 3 | PASS |
| crunch_drive | newton_max_iterations | DRIVE SIN-220Hz-0.3V | 6 | ≤ 16 | PASS |
| crunch_drive | total_esr_db | DRIVE DI-GTR L | -60.62 dB | ≤ -20 | PASS |
| crunch_drive | newton_failures | DRIVE DI-GTR L | 0 | ≤ 0 | PASS |
| crunch_drive | newton_unrescued | DRIVE DI-GTR L | 0 | — | INFO |
| crunch_drive | newton_median_iterations | DRIVE DI-GTR L | 2 | ≤ 3 | PASS |
| crunch_drive | newton_max_iterations | DRIVE DI-GTR L | 9 | ≤ 16 | PASS |
| crunch_drive | total_esr_db | DRIVE DI-GTR LL | -61.56 dB | ≤ -20 | PASS |
| crunch_drive | newton_failures | DRIVE DI-GTR LL | 0 | ≤ 0 | PASS |
| crunch_drive | newton_unrescued | DRIVE DI-GTR LL | 0 | — | INFO |
| crunch_drive | newton_median_iterations | DRIVE DI-GTR LL | 2 | ≤ 3 | PASS |
| crunch_drive | newton_max_iterations | DRIVE DI-GTR LL | 9 | ≤ 16 | PASS |
| crunch_drive | total_esr_db | DRIVE DI-GTR R | -61.32 dB | ≤ -20 | PASS |
| crunch_drive | newton_failures | DRIVE DI-GTR R | 0 | ≤ 0 | PASS |
| crunch_drive | newton_unrescued | DRIVE DI-GTR R | 0 | — | INFO |
| crunch_drive | newton_median_iterations | DRIVE DI-GTR R | 2 | ≤ 3 | PASS |
| crunch_drive | newton_max_iterations | DRIVE DI-GTR R | 9 | ≤ 16 | PASS |
| crunch_drive | total_esr_db | DRIVE DI-GTR RR | -60.57 dB | ≤ -20 | PASS |
| crunch_drive | newton_failures | DRIVE DI-GTR RR | 0 | ≤ 0 | PASS |
| crunch_drive | newton_unrescued | DRIVE DI-GTR RR | 0 | — | INFO |
| crunch_drive | newton_median_iterations | DRIVE DI-GTR RR | 2 | ≤ 3 | PASS |
| crunch_drive | newton_max_iterations | DRIVE DI-GTR RR | 9 | ≤ 16 | PASS |

## DC operating point

| Node / current | SPICE | Engine |
|---|---|---|
| ct | 488.7084 V | 488.7086 V |
| nscr | 485.6493 V | 485.6495 V |
| bpx | 402.3360 V | 402.3375 V |
| bpa | 389.3496 V | 389.3518 V |
| nbias | -46.3636 V | -46.3636 V |
| p1a | 296.4025 V | 296.4084 V |
| k1a | 2.5096 V | 2.5095 V |
| p1b | 352.4328 V | 352.4387 V |
| k1b | 3.6917 V | 3.6913 V |
| p2a | 231.1295 V | 231.1346 V |
| k2a | 1.4039 V | 1.4039 V |
| k2b | 231.6858 V | 231.6908 V |
| p3a | 270.8809 V | 270.8852 V |
| p3b | 262.2706 V | 262.2751 V |
| k3 | 44.9334 V | 44.9324 V |
| nf | 43.5216 V | 43.5207 V |
| ng | 13.4840 V | 13.4837 V |
| ip_tube_a | 34.0783 mA | 34.0783 mA |
| ip_tube_b | 34.0780 mA | 34.0780 mA |
| is_tube_a | 4.5674 mA | 4.5674 mA |
| is_tube_b | 4.5675 mA | 4.5675 mA |

## NFB (gain in → out, dB)

| f | SPICE closed | engine closed | SPICE open | engine open | NFB SPICE | NFB engine |
|---|---|---|---|---|---|---|
| 100 Hz | 43.44 | 43.45 | 50.07 | 50.07 | 6.63 | 6.63 |
| 1000 Hz | 55.58 | 55.59 | 61.74 | 61.74 | 6.16 | 6.16 |
| 5000 Hz | 68.07 | 68.09 | 74.12 | 74.14 | 6.04 | 6.05 |

## Signals

| Preset | Signal | ESR | best lag | harm. 2–9 | THD F / E | Power F / E | min v(ct) F / E |
|---|---|---|---|---|---|---|---|
| CLEAN | SIN-1000Hz-0.025V | -60.1 dB | 0 | 0.02 dB | 3.47 / 3.47 % | 100.2 / 100.3 W | — |
| OVERDRIVE | SIN-220Hz-0.3V | -67.7 dB | 0 | 0.01 dB | 23.35 / 23.35 % | 33.8 / 33.8 W | 469.82 / 469.81 V |
| CRUNCH | SIN-220Hz-0.3V | -70.0 dB | 0 | 0.01 dB | 15.04 / 15.04 % | 32.9 / 32.9 W | — |
| CRUNCH | DI-GTR L | -61.3 dB | 0 | — | — | — | — |
| CRUNCH | DI-GTR LL | -61.6 dB | 0 | — | — | — | — |
| CRUNCH | DI-GTR R | -62.3 dB | 0 | — | — | — | — |
| CRUNCH | DI-GTR RR | -61.1 dB | 0 | — | — | — | — |
| DRIVE | SIN-220Hz-0.3V | -63.6 dB | 0 | 0.04 dB | 25.98 / 25.99 % | 36.2 / 36.2 W | — |
| DRIVE | DI-GTR L | -60.6 dB | 0 | — | — | — | — |
| DRIVE | DI-GTR LL | -61.6 dB | 0 | — | — | — | — |
| DRIVE | DI-GTR R | -61.3 dB | 0 | — | — | — | — |
| DRIVE | DI-GTR RR | -60.6 dB | 0 | — | — | — | — |

## Stability

60 s silence at Preamp / Master / Presence 1.0: max |out| 4.43e-05 V, RMS first / last 10 s 4.23e-05 / 4.23e-05 V.

## Delayed-NFB break

Not applicable: since D-041 the global NFB (R22) is part of the B56 tree and has no delay (SPEC §7 M4 asked for a 1- vs 2-sample comparison of the delayed design).
