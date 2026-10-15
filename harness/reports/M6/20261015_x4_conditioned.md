# M6 experiment X4: LSTM-32 with Preamp Volume, Master, Presence as inputs, vs SPICE

- Date 2026-10-15; teacher `ampwav --block full --j4 exact` at 768000 Hz; tone stack 0.5; 96 training settings (20 s each, random DI track, offset and input gain in (-6.0, 0.0, 6.0, 12.0) dB), 16 validation settings on the held-out last 20 s; student LSTM 32 on (audio, 3 knobs) at 48000 Hz.
- SPICE: full netlist at 384000 Hz on the held-out first 5 s of every DI track. Training: 2400 epochs, 8786 s.

## Accuracy vs SPICE (48 kHz band, first 0.1 s excluded)

| Preset | Track | Teacher vs SPICE | Student vs SPICE | Student vs teacher | X3 vs SPICE |
|---|---|---|---|---|---|
| CRUNCH | GTR L | -49.0 dB | -3.3 dB | -3.3 dB | -16.5 dB |
| CRUNCH | GTR LL | -48.7 dB | -3.1 dB | -3.1 dB | -16.8 dB |
| CRUNCH | GTR R | -48.9 dB | -3.8 dB | -3.8 dB | -17.1 dB |
| CRUNCH | GTR RR | -49.0 dB | -3.0 dB | -3.0 dB | -16.7 dB |
| DRIVE | GTR L | -55.7 dB | -5.1 dB | -5.1 dB | — |
| DRIVE | GTR LL | -56.1 dB | -5.0 dB | -5.0 dB | — |
| DRIVE | GTR R | -56.0 dB | -5.4 dB | -5.4 dB | — |
| DRIVE | GTR RR | -55.9 dB | -5.0 dB | -5.0 dB | — |
| HOT | GTR L | -40.0 dB | -7.0 dB | -7.0 dB | — |
| HOT | GTR LL | -39.6 dB | -6.8 dB | -6.8 dB | — |
| HOT | GTR R | -39.6 dB | -6.8 dB | -6.8 dB | — |
| HOT | GTR RR | -40.1 dB | -7.1 dB | -7.1 dB | — |

## Unseen settings: student vs teacher (16 validation settings, 20 s, first 1 s excluded)

Overall -8.4 dB.

| Preamp Volume | Master | Presence | ESR |
|---|---|---|---|
| 0.992 | 0.789 | 0.654 | -10.9 dB |
| 1.000 | 0.824 | 0.943 | -10.6 dB |
| 0.755 | 0.405 | 0.820 | -10.4 dB |
| 0.758 | 0.971 | 0.715 | -9.8 dB |
| 0.408 | 0.792 | 0.000 | -9.7 dB |
| 0.224 | 0.853 | 0.080 | -8.9 dB |
| 0.278 | 0.861 | 0.374 | -8.8 dB |
| 0.124 | 0.985 | 0.351 | -8.7 dB |
| 1.000 | 0.413 | 0.993 | -7.5 dB |
| 0.566 | 0.084 | 0.519 | -5.2 dB |
| 0.507 | 0.265 | 0.000 | -4.3 dB |
| 0.123 | 0.576 | 1.000 | -1.9 dB |
| 0.000 | 0.526 | 0.269 | 23.7 dB |
| 0.000 | 0.330 | 0.849 | 26.1 dB |
| 0.591 | 0.000 | 0.919 | 34.6 dB |
| 0.000 | 0.013 | 0.398 | 62.2 dB |

## Aliasing (sines; power outside the harmonics of f0, relative to the total)

| Preset | f0 | amplitude (FS) | teacher (768 kHz → 48 kHz) | student (48 kHz) |
|---|---|---|---|---|
| CRUNCH | 5000 Hz | 0.1 | -26.0 dB | -19.8 dB |
| CRUNCH | 5000 Hz | 0.4 | -25.7 dB | -23.3 dB |
| CRUNCH | 9000 Hz | 0.1 | -39.9 dB | -18.7 dB |
| CRUNCH | 9000 Hz | 0.4 | -38.5 dB | -18.9 dB |
| HOT | 5000 Hz | 0.1 | -30.0 dB | -19.3 dB |
| HOT | 5000 Hz | 0.4 | -29.8 dB | -22.3 dB |
| HOT | 9000 Hz | 0.1 | -38.2 dB | -20.0 dB |
| HOT | 9000 Hz | 0.4 | -36.0 dB | -19.1 dB |

## Cost

Static knobs folded into the LSTM bias, same C++ kernel as X3: 442.4 ns per 48 kHz sample = 2.12 % of one core at 48 kHz. A knob change costs one 4·32 × 3 product.

## Training log (epoch, train loss sum, validation ESR)

- 20: 11.3881, -0.5 dB (71 s)
- 180: 8.6594, -3.0 dB (664 s)
- 340: 6.5097, -4.8 dB (1247 s)
- 500: 5.6850, -5.6 dB (1832 s)
- 660: 4.8720, -6.7 dB (2411 s)
- 820: 4.6264, -7.1 dB (3001 s)
- 980: 4.3583, -7.6 dB (3587 s)
- 1140: 3.7460, -7.8 dB (4176 s)
- 1300: 3.8545, -8.0 dB (4765 s)
- 1460: 3.8782, -8.0 dB (5351 s)
- 1620: 3.7328, -8.1 dB (5933 s)
- 1780: 3.4030, -8.1 dB (6518 s)
- 1940: 3.7793, -8.1 dB (7108 s)
- 2100: 4.2810, -8.1 dB (7696 s)
- 2260: 3.1078, -8.1 dB (8279 s)
- 2400: 3.9575, -8.1 dB (8786 s)
