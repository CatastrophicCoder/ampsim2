# M4 acceptance: full amplifier vs the full netlist

- Date: 2026-10-18, engine commit `b193fc7 (dirty)`, ngspice 47, fs = 768000 Hz, SPICE reltol 1e-08
- Netlist sha256 `ffd9b2812204248f`, tubes.lib sha256 `c9c9c88346c69010`
- Engine: `ampwav --block full` (volts at the jack and across the 16 Ω load), the checks of SPEC §7 M4.

## Summary

PASS 107 · FAIL 0

| Section | Check | Item | Measured | Tolerance | Status |
|---|---|---|---|---|---|
| dc | dc_rel_error | v(ct) | 4.176e-07 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(nscr) | 4.492e-07 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(bpx) | 3.649e-06 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(bpa) | 5.679e-06 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(nbias) | 4.636e-08 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(p1a) | 1.994e-05 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(k1a) | 3.976e-05 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(p1b) | 1.686e-05 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(k1b) | 0.0001011 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(p2a) | 2.195e-05 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(k2a) | 2.104e-05 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(k2b) | 2.14e-05 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(p3a) | 1.576e-05 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(p3b) | 1.707e-05 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(k3) | 2.137e-05 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(nf) | 2.137e-05 | ≤ 0.005 | PASS |
| dc | dc_rel_error | v(ng) | 2.137e-05 | ≤ 0.005 | PASS |
| dc | el34_plate_ma | ip_tube_a | 8.688e-06 mA | ≤ 1 | PASS |
| dc | el34_plate_ma | ip_tube_b | 8.695e-06 mA | ≤ 1 | PASS |
| dc | el34_screen_ma | is_tube_a | 1.951e-07 mA | ≤ 0.3 | PASS |
| dc | el34_screen_ma | is_tube_b | 1.918e-07 mA | ≤ 0.3 | PASS |
| dc | ct_rel | v(ct) | 4.176e-07 | ≤ 0.01 | PASS |
| dc | nbias_v | v(nbias) | 2.15e-06 V | ≤ 0.2 | PASS |
| nfb | nfb_gain_db | closed 100 Hz | 0.001766 dB | ≤ 0.5 | PASS |
| nfb | newton_failures | closed 100 Hz | 0 | ≤ 0 | PASS |
| nfb | newton_unrescued | closed 100 Hz | 0 | — | INFO |
| nfb | newton_median_iterations | closed 100 Hz | 1 | ≤ 3 | PASS |
| nfb | newton_max_iterations | closed 100 Hz | 2 | ≤ 64 | PASS |
| nfb | nfb_gain_db | closed 1000 Hz | 0.00536 dB | ≤ 0.5 | PASS |
| nfb | newton_failures | closed 1000 Hz | 0 | ≤ 0 | PASS |
| nfb | newton_unrescued | closed 1000 Hz | 0 | — | INFO |
| nfb | newton_median_iterations | closed 1000 Hz | 1 | ≤ 3 | PASS |
| nfb | newton_max_iterations | closed 1000 Hz | 2 | ≤ 64 | PASS |
| nfb | nfb_gain_db | closed 5000 Hz | 0.01479 dB | ≤ 0.5 | PASS |
| nfb | newton_failures | closed 5000 Hz | 0 | ≤ 0 | PASS |
| nfb | newton_unrescued | closed 5000 Hz | 0 | — | INFO |
| nfb | newton_median_iterations | closed 5000 Hz | 2 | ≤ 3 | PASS |
| nfb | newton_max_iterations | closed 5000 Hz | 2 | ≤ 64 | PASS |
| nfb | nfb_gain_db | open 100 Hz | 0.00227 dB | ≤ 0.5 | PASS |
| nfb | newton_failures | open 100 Hz | 0 | ≤ 0 | PASS |
| nfb | newton_unrescued | open 100 Hz | 0 | — | INFO |
| nfb | newton_median_iterations | open 100 Hz | 1 | ≤ 3 | PASS |
| nfb | newton_max_iterations | open 100 Hz | 2 | ≤ 64 | PASS |
| nfb | nfb_gain_db | open 1000 Hz | 0.007856 dB | ≤ 0.5 | PASS |
| nfb | newton_failures | open 1000 Hz | 0 | ≤ 0 | PASS |
| nfb | newton_unrescued | open 1000 Hz | 0 | — | INFO |
| nfb | newton_median_iterations | open 1000 Hz | 1 | ≤ 3 | PASS |
| nfb | newton_max_iterations | open 1000 Hz | 2 | ≤ 64 | PASS |
| nfb | nfb_gain_db | open 5000 Hz | 0.05701 dB | ≤ 0.5 | PASS |
| nfb | newton_failures | open 5000 Hz | 0 | ≤ 0 | PASS |
| nfb | newton_unrescued | open 5000 Hz | 0 | — | INFO |
| nfb | newton_median_iterations | open 5000 Hz | 2 | ≤ 3 | PASS |
| nfb | newton_max_iterations | open 5000 Hz | 2 | ≤ 64 | PASS |
| clean | total_esr_db | CLEAN SIN-1000Hz-0.025V | -30.3 dB | ≤ -20 | PASS |
| clean | total_harmonics_2_9_db | CLEAN SIN-1000Hz-0.025V | 0.7255 dB | ≤ 1.5 | PASS |
| clean | newton_failures | CLEAN SIN-1000Hz-0.025V | 0 | ≤ 0 | PASS |
| clean | newton_unrescued | CLEAN SIN-1000Hz-0.025V | 0 | — | INFO |
| clean | newton_median_iterations | CLEAN SIN-1000Hz-0.025V | 2 | ≤ 3 | PASS |
| clean | newton_max_iterations | CLEAN SIN-1000Hz-0.025V | 3 | ≤ 64 | PASS |
| clean | total_power_rel | CLEAN SIN-1000Hz-0.025V | 0.03927 | ≤ 0.05 | PASS |
| clean | total_thd_pp | CLEAN SIN-1000Hz-0.025V | 0.2173 pp | ≤ 0.5 | PASS |
| stability | stability_out_v | max |out| | 4.267e-05 V | ≤ 0.001 | PASS |
| stability | stability_growth | RMS last 10 s − first 10 s | 8.08e-12 V | ≤ 1e-06 | PASS |
| stability | newton_failures | 60 s silence | 0 | ≤ 0 | PASS |
| stability | newton_unrescued | 60 s silence | 0 | — | INFO |
| stability | newton_median_iterations | 60 s silence | 1 | ≤ 3 | PASS |
| stability | newton_max_iterations | 60 s silence | 1 | ≤ 64 | PASS |
| overdrive | total_esr_db | OVERDRIVE SIN-220Hz-0.3V | -67.99 dB | ≤ -20 | PASS |
| overdrive | total_harmonics_2_9_db | OVERDRIVE SIN-220Hz-0.3V | 0.008614 dB | ≤ 1.5 | PASS |
| overdrive | sag_min_v | OVERDRIVE SIN-220Hz-0.3V | 0.003537 V | ≤ 2 | PASS |
| overdrive | newton_failures | OVERDRIVE SIN-220Hz-0.3V | 0 | ≤ 0 | PASS |
| overdrive | newton_unrescued | OVERDRIVE SIN-220Hz-0.3V | 0 | — | INFO |
| overdrive | newton_median_iterations | OVERDRIVE SIN-220Hz-0.3V | 1 | ≤ 3 | PASS |
| overdrive | newton_max_iterations | OVERDRIVE SIN-220Hz-0.3V | 5 | ≤ 64 | PASS |
| crunch_drive | total_esr_db | CRUNCH SIN-220Hz-0.3V | -70.25 dB | ≤ -20 | PASS |
| crunch_drive | total_harmonics_2_9_db | CRUNCH SIN-220Hz-0.3V | 0.007954 dB | ≤ 1.5 | PASS |
| crunch_drive | newton_failures | CRUNCH SIN-220Hz-0.3V | 0 | ≤ 0 | PASS |
| crunch_drive | newton_unrescued | CRUNCH SIN-220Hz-0.3V | 0 | — | INFO |
| crunch_drive | newton_median_iterations | CRUNCH SIN-220Hz-0.3V | 2 | ≤ 3 | PASS |
| crunch_drive | newton_max_iterations | CRUNCH SIN-220Hz-0.3V | 5 | ≤ 64 | PASS |
| crunch_drive | total_esr_db | CRUNCH DI-GTR L | -60.8 dB | ≤ -20 | PASS |
| crunch_drive | newton_failures | CRUNCH DI-GTR L | 0 | ≤ 0 | PASS |
| crunch_drive | newton_unrescued | CRUNCH DI-GTR L | 0 | — | INFO |
| crunch_drive | newton_median_iterations | CRUNCH DI-GTR L | 2 | ≤ 3 | PASS |
| crunch_drive | newton_max_iterations | CRUNCH DI-GTR L | 7 | ≤ 64 | PASS |
| crunch_drive | total_esr_db | CRUNCH DI-GTR LL | -61.36 dB | ≤ -20 | PASS |
| crunch_drive | newton_failures | CRUNCH DI-GTR LL | 0 | ≤ 0 | PASS |
| crunch_drive | newton_unrescued | CRUNCH DI-GTR LL | 0 | — | INFO |
| crunch_drive | newton_median_iterations | CRUNCH DI-GTR LL | 2 | ≤ 3 | PASS |
| crunch_drive | newton_max_iterations | CRUNCH DI-GTR LL | 7 | ≤ 64 | PASS |
| crunch_drive | total_esr_db | CRUNCH DI-GTR R | -62.11 dB | ≤ -20 | PASS |
| crunch_drive | newton_failures | CRUNCH DI-GTR R | 0 | ≤ 0 | PASS |
| crunch_drive | newton_unrescued | CRUNCH DI-GTR R | 0 | — | INFO |
| crunch_drive | newton_median_iterations | CRUNCH DI-GTR R | 2 | ≤ 3 | PASS |
| crunch_drive | newton_max_iterations | CRUNCH DI-GTR R | 7 | ≤ 64 | PASS |
| crunch_drive | total_esr_db | CRUNCH DI-GTR RR | -60.94 dB | ≤ -20 | PASS |
| crunch_drive | newton_failures | CRUNCH DI-GTR RR | 0 | ≤ 0 | PASS |
| crunch_drive | newton_unrescued | CRUNCH DI-GTR RR | 0 | — | INFO |
| crunch_drive | newton_median_iterations | CRUNCH DI-GTR RR | 2 | ≤ 3 | PASS |
| crunch_drive | newton_max_iterations | CRUNCH DI-GTR RR | 7 | ≤ 64 | PASS |
| crunch_drive | total_esr_db | DRIVE SIN-220Hz-0.3V | -63.49 dB | ≤ -20 | PASS |
| crunch_drive | total_harmonics_2_9_db | DRIVE SIN-220Hz-0.3V | 0.03171 dB | ≤ 1.5 | PASS |
| crunch_drive | newton_failures | DRIVE SIN-220Hz-0.3V | 0 | ≤ 0 | PASS |
| crunch_drive | newton_unrescued | DRIVE SIN-220Hz-0.3V | 0 | — | INFO |
| crunch_drive | newton_median_iterations | DRIVE SIN-220Hz-0.3V | 1 | ≤ 3 | PASS |
| crunch_drive | newton_max_iterations | DRIVE SIN-220Hz-0.3V | 5 | ≤ 64 | PASS |
| crunch_drive | total_esr_db | DRIVE DI-GTR L | -60.24 dB | ≤ -20 | PASS |
| crunch_drive | newton_failures | DRIVE DI-GTR L | 0 | ≤ 0 | PASS |
| crunch_drive | newton_unrescued | DRIVE DI-GTR L | 0 | — | INFO |
| crunch_drive | newton_median_iterations | DRIVE DI-GTR L | 2 | ≤ 3 | PASS |
| crunch_drive | newton_max_iterations | DRIVE DI-GTR L | 7 | ≤ 64 | PASS |
| crunch_drive | total_esr_db | DRIVE DI-GTR LL | -61.54 dB | ≤ -20 | PASS |
| crunch_drive | newton_failures | DRIVE DI-GTR LL | 0 | ≤ 0 | PASS |
| crunch_drive | newton_unrescued | DRIVE DI-GTR LL | 0 | — | INFO |
| crunch_drive | newton_median_iterations | DRIVE DI-GTR LL | 2 | ≤ 3 | PASS |
| crunch_drive | newton_max_iterations | DRIVE DI-GTR LL | 7 | ≤ 64 | PASS |
| crunch_drive | total_esr_db | DRIVE DI-GTR R | -61.01 dB | ≤ -20 | PASS |
| crunch_drive | newton_failures | DRIVE DI-GTR R | 0 | ≤ 0 | PASS |
| crunch_drive | newton_unrescued | DRIVE DI-GTR R | 0 | — | INFO |
| crunch_drive | newton_median_iterations | DRIVE DI-GTR R | 2 | ≤ 3 | PASS |
| crunch_drive | newton_max_iterations | DRIVE DI-GTR R | 7 | ≤ 64 | PASS |
| crunch_drive | total_esr_db | DRIVE DI-GTR RR | -60.27 dB | ≤ -20 | PASS |
| crunch_drive | newton_failures | DRIVE DI-GTR RR | 0 | ≤ 0 | PASS |
| crunch_drive | newton_unrescued | DRIVE DI-GTR RR | 0 | — | INFO |
| crunch_drive | newton_median_iterations | DRIVE DI-GTR RR | 2 | ≤ 3 | PASS |
| crunch_drive | newton_max_iterations | DRIVE DI-GTR RR | 8 | ≤ 64 | PASS |

## DC operating point

| Node / current | SPICE | Engine |
|---|---|---|
| ct | 491.3466 V | 491.3468 V |
| nscr | 488.8805 V | 488.8807 V |
| bpx | 404.9982 V | 404.9997 V |
| bpa | 391.9215 V | 391.9237 V |
| nbias | -46.3636 V | -46.3636 V |
| p1a | 298.3260 V | 298.3319 V |
| k1a | 2.5271 V | 2.5270 V |
| p1b | 354.7502 V | 354.7562 V |
| k1b | 3.7171 V | 3.7167 V |
| p2a | 232.5771 V | 232.5822 V |
| k2a | 1.4139 V | 1.4138 V |
| k2b | 233.1404 V | 233.1454 V |
| p3a | 272.6113 V | 272.6156 V |
| p3b | 263.9512 V | 263.9557 V |
| k3 | 45.2502 V | 45.2492 V |
| nf | 43.8285 V | 43.8275 V |
| ng | 13.5790 V | 13.5787 V |
| ip_tube_a | 28.7718 mA | 28.7718 mA |
| ip_tube_b | 28.7715 mA | 28.7715 mA |
| is_tube_a | 3.2641 mA | 3.2641 mA |
| is_tube_b | 3.2643 mA | 3.2643 mA |

## NFB (gain in → out, dB)

| f | SPICE closed | engine closed | SPICE open | engine open | NFB SPICE | NFB engine |
|---|---|---|---|---|---|---|
| 100 Hz | 45.93 | 45.93 | 56.34 | 56.34 | 10.42 | 10.42 |
| 1000 Hz | 53.54 | 53.55 | 57.86 | 57.86 | 4.32 | 4.32 |
| 5000 Hz | 69.59 | 69.61 | 77.50 | 77.55 | 7.91 | 7.95 |

## Signals

| Preset | Signal | ESR | best lag | harm. 2–9 | THD F / E | Power F / E | min v(ct) F / E |
|---|---|---|---|---|---|---|---|
| CLEAN | SIN-1000Hz-0.025V | -30.3 dB | 2 | 0.73 dB | 7.37 / 7.59 % | 81.1 / 77.9 W | — |
| OVERDRIVE | SIN-220Hz-0.3V | -68.0 dB | 0 | 0.01 dB | 19.62 / 19.62 % | 32.2 / 32.2 W | 474.38 / 474.37 V |
| CRUNCH | SIN-220Hz-0.3V | -70.3 dB | 0 | 0.01 dB | 12.01 / 12.01 % | 31.6 / 31.6 W | — |
| CRUNCH | DI-GTR L | -60.8 dB | 0 | — | — | — | — |
| CRUNCH | DI-GTR LL | -61.4 dB | 0 | — | — | — | — |
| CRUNCH | DI-GTR R | -62.1 dB | 0 | — | — | — | — |
| CRUNCH | DI-GTR RR | -60.9 dB | 0 | — | — | — | — |
| DRIVE | SIN-220Hz-0.3V | -63.5 dB | 0 | 0.03 dB | 22.53 / 22.53 % | 34.5 / 34.6 W | — |
| DRIVE | DI-GTR L | -60.2 dB | 0 | — | — | — | — |
| DRIVE | DI-GTR LL | -61.5 dB | 0 | — | — | — | — |
| DRIVE | DI-GTR R | -61.0 dB | 0 | — | — | — | — |
| DRIVE | DI-GTR RR | -60.3 dB | 0 | — | — | — | — |

## Stability

60 s silence at Preamp / Master / Presence 1.0: max |out| 4.27e-05 V, RMS first / last 10 s 4.24e-05 / 4.24e-05 V.

## Delayed-NFB break

Not applicable: since D-041 the global NFB (R22) is part of the B56 tree and has no delay (SPEC §7 M4 asked for a 1- vs 2-sample comparison of the delayed design).
