# M4 study: J5 partition error (PI plates -> power stage), closed loop

- Date: 2026-10-13, ngspice 47, fs = 768000 Hz, SPICE reltol 1e-08; `python -m harness.partition_j5 --variants self,diff,2port`
- Reference: `b56` (B5 + B6 + B7 joined, instantaneous NFB). Partitioned: `b56j5`, C14 / C16 fed by J5
  Thevenin sources (VCVS copies of the unloaded p3a / p3b), NFB closed. Both driven by the J4 source at the B56
  levels (0.1 / 1 / 5 / 20 V), constant preamp loads. ESR at the speaker node out, each signal minus its DC.
- Variants: `self` R_thA / R_thB = 58068 / 56937 Ω (|Z_aa|, |Z_bb| at 1 kHz); `diff` R_th = Z_aa − Z_ab =
  25962 Ω, Z_bb − Z_ba = 31487 Ω; `2port` v = v_oc − Z·i with Z = [[58061, 32100], [25450, 56936]] Ω (real
  parts at 1 kHz, measured on b5open with harness/rth_measure.py).
- This report was written from the run's log: the run stopped at the third variant when ngspice could not
  render `2port` / Presence 0 / SIN-5000Hz-0.1V ("timestep too small", trouble with node y, at RELTOL 1e-8 and
  1e-7); the owner decided on the Presence 0 results (PROGRESS Q19, DECISIONS D-041: B5 and B6 merged). The
  script now records such a render as a note instead of stopping.

## Small-signal (AC, J4 source → out, Presence 0.5)

| Variant | 100 Hz | 1 kHz | 10 kHz |
|---|---|---|---|
| self | −0.431 dB / −0.41° | −0.474 dB / −0.71° | −0.407 dB / −6.52° |
| diff | −0.003 dB / −0.01° | −0.001 dB / −0.04° | +0.122 dB / −0.56° |
| 2port | −0.001 dB / −0.01° | +0.002 dB / −0.04° | +0.125 dB / −0.56° |

## Transient, Presence 0 (ESR at out, dB)

| Signal | self | diff | 2port |
|---|---|---|---|
| SIN-100Hz-0.1V | -26.2 | -68.1 | -75.4 |
| SIN-1000Hz-0.1V | -25.8 | -62.5 | -62.7 |
| SIN-5000Hz-0.1V | -23.1 | -46.2 | — |
| SIN-100Hz-1V | -26.1 | -58.8 | — |
| SIN-1000Hz-1V | -25.8 | -57.5 | — |
| SIN-5000Hz-1V | -23.1 | -46.1 | — |
| SIN-100Hz-5V | -30.2 | -37.3 | — |
| SIN-1000Hz-5V | -31.8 | -34.7 | — |
| SIN-5000Hz-5V | -25.1 | -34.2 | — |
| SIN-100Hz-20V | -18.0 | -13.7 | — |
| SIN-1000Hz-20V | -20.6 | -18.6 | — |
| SIN-5000Hz-20V | -17.8 | -17.1 | — |
| SWP-lo-0.1V | -22.4 | -38.6 | — |
| TT-0.5V | -26.0 | -58.6 | — |
| BURST-5V | -31.9 | -34.5 | — |
| STEP-1V | -26.0 | -53.4 | — |

Context: the M4 total targets are ESR ≤ −20 dB on overdrive signals; this table is the J5 junction alone.
