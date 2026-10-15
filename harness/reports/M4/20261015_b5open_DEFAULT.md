# M4 report: b5open / DEFAULT

- Block: B5-open phase inverter, R22 driven by recorded speaker voltage
- Preset: DEFAULT — Default knobs; standard signal set at the block levels
- Knobs: preampVolume=0.6, treble=0.5, middle=0.5, bass=0.5, master=0.3, presence=0.5
- Date: 2026-10-15
- Engine commit: `476d21f (dirty)`
- Testbench: `harness/testbenches/b5open.cir` sha256 `b4224b865df6ebae`; netlist sha256 `7bea8385e4adb04e`; tubes.lib sha256 `5767afe208240bfa`
- ngspice 47, fs = 768000 Hz, SPICE reltol 1e-08

## Summary

PASS 88 · FAIL 0 · INFO 12 (INFO = measured, not scored in M4)

| Check | Item | Measured | Tolerance | Status |
|---|---|---|---|---|
| dc_rel_error | v(g3a) | 2.529e-05 | ≤ 0.005 | PASS |
| dc_rel_error | v(g3b) | 2.529e-05 | ≤ 0.005 | PASS |
| dc_rel_error | v(k3) | 2.526e-05 | ≤ 0.005 | PASS |
| dc_rel_error | v(nf) | 2.526e-05 | ≤ 0.005 | PASS |
| dc_rel_error | v(ng) | 2.526e-05 | ≤ 0.005 | PASS |
| dc_rel_error | v(p3a) | 1.223e-05 | ≤ 0.005 | PASS |
| dc_rel_error | v(p3b) | 1.353e-05 | ≤ 0.005 | PASS |
| dc_rel_error | v(y) | 4.441e-12 | ≤ 0.005 | PASS |
| nonlinear_esr_db | SIN-100Hz-0.1V | -84.13 dB | ≤ -20 | PASS |
| harmonics_2_5_db | SIN-100Hz-0.1V | 0.0005039 dB | ≤ 0.5 | PASS |
| thd_pp | SIN-100Hz-0.1V | 1.176e-06 pp | — | INFO |
| newton_failures | SIN-100Hz-0.1V | 0 | ≤ 0 | PASS |
| newton_median_iterations | SIN-100Hz-0.1V | 1 | ≤ 3 | PASS |
| newton_max_iterations | SIN-100Hz-0.1V | 2 | ≤ 12 | PASS |
| nonlinear_esr_db | SIN-1000Hz-0.1V | -80.59 dB | ≤ -20 | PASS |
| harmonics_2_5_db | SIN-1000Hz-0.1V | 0.0009675 dB | ≤ 0.5 | PASS |
| thd_pp | SIN-1000Hz-0.1V | 2.017e-07 pp | — | INFO |
| newton_failures | SIN-1000Hz-0.1V | 0 | ≤ 0 | PASS |
| newton_median_iterations | SIN-1000Hz-0.1V | 1 | ≤ 3 | PASS |
| newton_max_iterations | SIN-1000Hz-0.1V | 2 | ≤ 12 | PASS |
| nonlinear_esr_db | SIN-5000Hz-0.1V | -68.55 dB | ≤ -20 | PASS |
| harmonics_2_5_db | SIN-5000Hz-0.1V | 0.09361 dB | ≤ 0.5 | PASS |
| thd_pp | SIN-5000Hz-0.1V | 0.0001017 pp | — | INFO |
| newton_failures | SIN-5000Hz-0.1V | 0 | ≤ 0 | PASS |
| newton_median_iterations | SIN-5000Hz-0.1V | 2 | ≤ 3 | PASS |
| newton_max_iterations | SIN-5000Hz-0.1V | 2 | ≤ 12 | PASS |
| nonlinear_esr_db | SIN-100Hz-1V | -84.11 dB | ≤ -20 | PASS |
| harmonics_2_5_db | SIN-100Hz-1V | 0.000521 dB | ≤ 0.5 | PASS |
| thd_pp | SIN-100Hz-1V | 8.059e-06 pp | — | INFO |
| newton_failures | SIN-100Hz-1V | 0 | ≤ 0 | PASS |
| newton_median_iterations | SIN-100Hz-1V | 1 | ≤ 3 | PASS |
| newton_max_iterations | SIN-100Hz-1V | 2 | ≤ 12 | PASS |
| nonlinear_esr_db | SIN-1000Hz-1V | -80.57 dB | ≤ -20 | PASS |
| harmonics_2_5_db | SIN-1000Hz-1V | 0.001835 dB | ≤ 0.5 | PASS |
| thd_pp | SIN-1000Hz-1V | 5.855e-05 pp | — | INFO |
| newton_failures | SIN-1000Hz-1V | 0 | ≤ 0 | PASS |
| newton_median_iterations | SIN-1000Hz-1V | 2 | ≤ 3 | PASS |
| newton_max_iterations | SIN-1000Hz-1V | 2 | ≤ 12 | PASS |
| nonlinear_esr_db | SIN-5000Hz-1V | -68.52 dB | ≤ -20 | PASS |
| harmonics_2_5_db | SIN-5000Hz-1V | 0.02713 dB | ≤ 0.5 | PASS |
| thd_pp | SIN-5000Hz-1V | 0.001738 pp | — | INFO |
| newton_failures | SIN-5000Hz-1V | 0 | ≤ 0 | PASS |
| newton_median_iterations | SIN-5000Hz-1V | 2 | ≤ 3 | PASS |
| newton_max_iterations | SIN-5000Hz-1V | 2 | ≤ 12 | PASS |
| nonlinear_esr_db | SIN-100Hz-5V | -83.46 dB | ≤ -20 | PASS |
| harmonics_2_5_db | SIN-100Hz-5V | 0.001436 dB | ≤ 0.5 | PASS |
| thd_pp | SIN-100Hz-5V | 1.737e-05 pp | — | INFO |
| newton_failures | SIN-100Hz-5V | 0 | ≤ 0 | PASS |
| newton_median_iterations | SIN-100Hz-5V | 1 | ≤ 3 | PASS |
| newton_max_iterations | SIN-100Hz-5V | 2 | ≤ 12 | PASS |
| nonlinear_esr_db | SIN-1000Hz-5V | -79.92 dB | ≤ -20 | PASS |
| harmonics_2_5_db | SIN-1000Hz-5V | 0.03055 dB | ≤ 0.5 | PASS |
| thd_pp | SIN-1000Hz-5V | 0.0008429 pp | — | INFO |
| newton_failures | SIN-1000Hz-5V | 0 | ≤ 0 | PASS |
| newton_median_iterations | SIN-1000Hz-5V | 2 | ≤ 3 | PASS |
| newton_max_iterations | SIN-1000Hz-5V | 2 | ≤ 12 | PASS |
| nonlinear_esr_db | SIN-5000Hz-5V | -67.23 dB | ≤ -20 | PASS |
| harmonics_2_5_db | SIN-5000Hz-5V | 0.04569 dB | ≤ 0.5 | PASS |
| thd_pp | SIN-5000Hz-5V | 0.01865 pp | — | INFO |
| newton_failures | SIN-5000Hz-5V | 0 | ≤ 0 | PASS |
| newton_median_iterations | SIN-5000Hz-5V | 2 | ≤ 3 | PASS |
| newton_max_iterations | SIN-5000Hz-5V | 3 | ≤ 12 | PASS |
| nonlinear_esr_db_overdrive | SIN-100Hz-20V | -75.07 dB | ≤ -20 | PASS |
| harmonics_2_5_db | SIN-100Hz-20V | 0.003758 dB | ≤ 0.5 | PASS |
| thd_pp | SIN-100Hz-20V | 0.002551 pp | — | INFO |
| newton_failures | SIN-100Hz-20V | 0 | ≤ 0 | PASS |
| newton_median_iterations | SIN-100Hz-20V | 1 | ≤ 3 | PASS |
| newton_max_iterations | SIN-100Hz-20V | 2 | ≤ 12 | PASS |
| nonlinear_esr_db_overdrive | SIN-1000Hz-20V | -74.15 dB | ≤ -20 | PASS |
| harmonics_2_5_db | SIN-1000Hz-20V | 0.00795 dB | ≤ 0.5 | PASS |
| thd_pp | SIN-1000Hz-20V | 0.002113 pp | — | INFO |
| newton_failures | SIN-1000Hz-20V | 0 | ≤ 0 | PASS |
| newton_median_iterations | SIN-1000Hz-20V | 2 | ≤ 3 | PASS |
| newton_max_iterations | SIN-1000Hz-20V | 3 | ≤ 12 | PASS |
| nonlinear_esr_db_overdrive | SIN-5000Hz-20V | -65.08 dB | ≤ -20 | PASS |
| harmonics_2_5_db | SIN-5000Hz-20V | 0.0819 dB | ≤ 0.5 | PASS |
| thd_pp | SIN-5000Hz-20V | 0.0003368 pp | — | INFO |
| newton_failures | SIN-5000Hz-20V | 0 | ≤ 0 | PASS |
| newton_median_iterations | SIN-5000Hz-20V | 2 | ≤ 3 | PASS |
| newton_max_iterations | SIN-5000Hz-20V | 4 | ≤ 12 | PASS |
| nonlinear_esr_db | SWP-lo-0.1V | -65.03 dB | ≤ -20 | PASS |
| newton_failures | SWP-lo-0.1V | 0 | ≤ 0 | PASS |
| newton_median_iterations | SWP-lo-0.1V | 1 | ≤ 3 | PASS |
| newton_max_iterations | SWP-lo-0.1V | 3 | ≤ 12 | PASS |
| nonlinear_esr_db | SWP-hi-1V | -64.98 dB | ≤ -20 | PASS |
| newton_failures | SWP-hi-1V | 0 | ≤ 0 | PASS |
| newton_median_iterations | SWP-hi-1V | 1 | ≤ 3 | PASS |
| newton_max_iterations | SWP-hi-1V | 3 | ≤ 12 | PASS |
| nonlinear_esr_db | TT-0.5V | -83.68 dB | ≤ -20 | PASS |
| newton_failures | TT-0.5V | 0 | ≤ 0 | PASS |
| newton_median_iterations | TT-0.5V | 1 | ≤ 3 | PASS |
| newton_max_iterations | TT-0.5V | 2 | ≤ 12 | PASS |
| nonlinear_esr_db | BURST-5V | -80.14 dB | ≤ -20 | PASS |
| newton_failures | BURST-5V | 0 | ≤ 0 | PASS |
| newton_median_iterations | BURST-5V | 1 | ≤ 3 | PASS |
| newton_max_iterations | BURST-5V | 2 | ≤ 12 | PASS |
| nonlinear_esr_db | STEP-1V | -80.09 dB | ≤ -20 | PASS |
| newton_failures | STEP-1V | 0 | ≤ 0 | PASS |
| newton_median_iterations | STEP-1V | 1 | ≤ 3 | PASS |
| newton_max_iterations | STEP-1V | 2 | ≤ 12 | PASS |

## Signals

| Signal | Peak F [V] | ESR [dB] | Best lag | ESR at lag [dB] | THD F / E [%] | Power F / E [W] | ns/sample | Newton fail / median / max it |
|---|---|---|---|---|---|---|---|---|
| SIN-100Hz-0.1V | 1.556 | -84.13 | 0 | -84.13 | 0.0457 / 0.0457 | — | 287.2 | 0 / 1 / 2 |
| SIN-1000Hz-0.1V | 1.536 | -80.59 | 0 | -80.59 | 0.0488 / 0.0488 | — | 281.6 | 0 / 1 / 2 |
| SIN-5000Hz-0.1V | 1.547 | -68.55 | 0 | -68.55 | 0.0519 / 0.052 | — | 407.3 | 0 / 2 / 2 |
| SIN-100Hz-1V | 15.01 | -84.11 | 0 | -84.11 | 1.25 / 1.25 | — | 282.5 | 0 / 1 / 2 |
| SIN-1000Hz-1V | 15.43 | -80.57 | 0 | -80.57 | 1.5 / 1.5 | — | 387.8 | 0 / 2 / 2 |
| SIN-5000Hz-1V | 15.57 | -68.52 | 0 | -68.52 | 1.59 / 1.59 | — | 421.3 | 0 / 2 / 2 |
| SIN-100Hz-5V | 79.46 | -83.46 | 0 | -83.46 | 4.33 / 4.33 | — | 292.5 | 0 / 1 / 2 |
| SIN-1000Hz-5V | 92.77 | -79.92 | 0 | -79.92 | 5.04 / 5.04 | — | 462.4 | 0 / 2 / 2 |
| SIN-5000Hz-5V | 92.86 | -67.23 | 0 | -67.23 | 5.29 / 5.31 | — | 479.5 | 0 / 2 / 3 |
| SIN-100Hz-20V | 132.2 | -75.07 | 0 | -75.07 | 47.2 / 47.2 | — | 388.8 | 0 / 1 / 2 |
| SIN-1000Hz-20V | 140.7 | -74.15 | 0 | -74.15 | 50.4 / 50.4 | — | 453.6 | 0 / 2 / 3 |
| SIN-5000Hz-20V | 142.7 | -65.08 | 0 | -65.08 | 50.2 / 50.2 | — | 615.5 | 0 / 2 / 4 |
| SWP-lo-0.1V | 2.48 | -65.03 | 0 | -65.03 | — | — | 316.4 | 0 / 1 / 3 |
| SWP-hi-1V | 24.65 | -64.98 | 0 | -64.98 | — | — | 342.7 | 0 / 1 / 3 |
| TT-0.5V | 14.08 | -83.68 | 0 | -83.68 | — | — | 281.5 | 0 / 1 / 2 |
| BURST-5V | 97.88 | -80.14 | 0 | -80.14 | — | — | 371.1 | 0 / 1 / 2 |
| STEP-1V | 17.72 | -80.09 | 0 | -80.09 | — | — | 282.8 | 0 / 1 / 2 |

## DC operating point

| Node | SPICE [V] | Engine [V] | Rel. error |
|---|---|---|---|
| g3a | 43.8295 | 43.8284 | 2.529e-05 |
| g3b | 43.8295 | 43.8284 | 2.529e-05 |
| k3 | 45.2502 | 45.2491 | 2.526e-05 |
| nf | 43.8285 | 43.8274 | 2.526e-05 |
| ng | 13.5790 | 13.5787 | 2.526e-05 |
| npr | 0.0000 | — | — |
| p3a | 272.6113 | 272.6146 | 1.223e-05 |
| p3b | 263.9512 | 263.9548 | 1.353e-05 |
| y | 0.0000 | -0.0000 | 4.441e-12 |

Engine values are the probe values after the first sample (zero input) following `reset()`. Nodes the engine does not probe yet show —.

## Figures

### Waveform overlay and error (most demanding signal)

![Waveform overlay and error (most demanding signal)](20261015_b5open_DEFAULT/waveform.png)

### Harmonics

![Harmonics](20261015_b5open_DEFAULT/harmonics.png)
