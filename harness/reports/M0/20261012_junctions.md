# M0 junction Thevenin resistances

- Date: 2026-10-12, ngspice 47, default parameters unless stated
- Method (SPEC §7 M0.5): 1 A AC current into the open-circuited output node of the upstream block testbench, block input AC-grounded, linearised at the DC operating point. DC = 1 mHz point of the sweep (capacitors effectively open), cross-checked by a ±1 µA finite difference of two DC operating points.
- Upstream junctions inside a testbench (Rth_j2 in b3) use the junctions.py value, or the DC value measured here while it is unset.

| Junction | Node | Testbench | R_th DC | DC (op finite diff.) | Z @ 1 kHz | Re Z @ 1 kHz |
|---|---|---|---|---|---|---|
| J2 | p1b | b12 | 90.11 kΩ | 90.11 kΩ | 89.73 kΩ ∠-1.0° | 89.72 kΩ |
| J3 | k2b | b3 | 359.9 Ω | 359.9 Ω | 359.9 Ω ∠+0.1° | 359.9 Ω |
| J5a (R_thA) | p3a | b5open | 41.4 kΩ | 41.4 kΩ | 58.06 kΩ ∠-0.9° | 58.06 kΩ |
| J5b (R_thB) | p3b | b5open | 45.51 kΩ | 45.51 kΩ | 56.93 kΩ ∠+0.4° | 56.93 kΩ |

## Spread over presets

| Junction | Knob sets | R_th DC min–max | \|Z\| @ 1 kHz min–max |
|---|---|---|---|
| J2 | 29 | 90.11 kΩ – 90.11 kΩ | 88.39 kΩ – 90.1 kΩ |
| J3 | 29 | 359.9 Ω – 359.9 Ω | 359.9 Ω – 359.9 Ω |
| J5a (R_thA) | 29 | 41.4 kΩ – 41.4 kΩ | 56.16 kΩ – 58.07 kΩ |
| J5b (R_thB) | 29 | 45.51 kΩ – 45.51 kΩ | 56.93 kΩ – 59.11 kΩ |

## LTP coupling (J5)

Transimpedance to the other PI plate (v(other) per A injected); the J-Thevenin model has no such term.

| Injected at | Observed | DC | @ 1 kHz | Ratio to self @ 1 kHz |
|---|---|---|---|---|
| p3a | p3b | 14.45 kΩ | 25.47 kΩ ∠+1.8° | 0.44 |
| p3b | p3a | 14.52 kΩ | 32.11 kΩ ∠-0.8° | 0.56 |

## Sensitivity to the upstream R_th

Max relative change when every upstream R_th is switched from its DC value to |Z(1 kHz)|: j2 0.0e+00, j3 8.7e-08, j5a 0.0e+00, j5b 0.0e+00

## |Z(f)|

![impedance](20261012_junctions.png)

## |Z| at selected frequencies

| Junction | 0.001 Hz | 10 Hz | 100 Hz | 1000 Hz | 10000 Hz |
|---|---|---|---|---|---|
| J2 | 90.11 kΩ ∠-0.0° | 90.11 kΩ ∠-0.0° | 90.1 kΩ ∠-0.1° | 89.73 kΩ ∠-1.0° | 85.63 kΩ ∠-3.3° |
| J3 | 359.9 Ω ∠+0.0° | 359.9 Ω ∠+0.0° | 359.9 Ω ∠+0.0° | 359.9 Ω ∠+0.1° | 361.1 Ω ∠+0.4° |
| J5a (R_thA) | 41.4 kΩ ∠+0.0° | 57.21 kΩ ∠+6.0° | 58.67 kΩ ∠+0.0° | 58.06 kΩ ∠-0.9° | 57.36 kΩ ∠-7.9° |
| J5b (R_thB) | 45.51 kΩ ∠+0.1° | 58.7 kΩ ∠-6.1° | 56.25 kΩ ∠-0.0° | 56.93 kΩ ∠+0.4° | 57.68 kΩ ∠+3.1° |
