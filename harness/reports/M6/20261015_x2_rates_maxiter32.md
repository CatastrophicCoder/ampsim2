# M6 A1: full engine (exact J4) at lower internal rates, plugin model check

- Date 2026-10-15; options: defaults (power amp at full rate, Newton tol 1e-9 V); rates 48 kHz × [2, 4, 8].
- Plugin model check (D-051): ESR ≤ -20 dB vs SPICE (cached 384000 Hz references, 48 kHz band, first 5 s of every DI track, first 0.1 s excluded). Unseen settings: the 16 X4 validation settings, 10 s, vs the engine at 16x (first 1 s excluded). Cost: ampwav --stats on 10 s of DI GTR L, one run at a time; oversampling filters not included.

## Summary

| Rate | vs SPICE worst (CRUNCH / DRIVE / HOT) | Check | Unseen settings vs 16x (worst / median) | Newton failures (not rescued) / runs, max iterations | Cost default / HOT (% of one core) |
|---|---|---|---|---|---|
| 2x (96 kHz) | -46.5 / -50.0 / -26.2 dB | PASS | -20.6 / -39.1 dB | 15 (7) / 28, 32 | 34.1 / 51.8 |
| 4x (192 kHz) | -48.5 / -55.6 / -33.2 dB | PASS | -26.9 / -54.2 dB | 4 (0) / 28, 32 | 67.3 / 91.6 |
| 8x (384 kHz) | -48.7 / -55.8 / -38.6 dB | PASS | -39.1 / -69.7 dB | 3 (0) / 28, 32 | 129.0 / 165.7 |

## vs SPICE per clip

| Clip | 2x | 4x | 8x |
|---|---|---|---|
| CRUNCH GTR L | -47.5 dB | -49.0 dB | -49.0 dB |
| CRUNCH GTR LL | -46.5 dB | -48.5 dB | -48.7 dB |
| CRUNCH GTR R | -47.6 dB | -48.9 dB | -48.9 dB |
| CRUNCH GTR RR | -47.5 dB | -49.0 dB | -49.0 dB |
| DRIVE GTR L | -50.0 dB | -55.6 dB | -55.8 dB |
| DRIVE GTR LL | -51.0 dB | -56.1 dB | -56.2 dB |
| DRIVE GTR R | -51.1 dB | -56.0 dB | -56.1 dB |
| DRIVE GTR RR | -50.5 dB | -55.9 dB | -56.0 dB |
| HOT GTR L | -26.2 dB | -33.2 dB | -38.7 dB |
| HOT GTR LL | -27.0 dB | -34.0 dB | -38.6 dB |
| HOT GTR R | -27.5 dB | -34.1 dB | -38.8 dB |
| HOT GTR RR | -26.2 dB | -33.7 dB | -38.9 dB |

## Aliasing (power outside the harmonics of f0, relative to the total; 16x for reference)

Each cell: non-harmonic power, then Newton failures (not rescued) in that 2 s run. Non-converged samples add non-harmonic power, so a cell with many failures measures solver error as well as aliasing.

| Preset | f0 | amplitude (FS) | 16x | 2x | 4x | 8x |
|---|---|---|---|---|---|---|
| CRUNCH | 5000 Hz | 0.1 | -26.0 dB, 0 (0) | -19.6 dB, 0 (0) | -25.8 dB, 0 (0) | -26.0 dB, 0 (0) |
| CRUNCH | 5000 Hz | 0.4 | -25.7 dB, 0 (0) | -18.5 dB, 2021 (1993) | -24.3 dB, 0 (0) | -25.4 dB, 0 (0) |
| CRUNCH | 9000 Hz | 0.1 | -39.9 dB, 0 (0) | -15.6 dB, 0 (0) | -26.1 dB, 1 (0) | -36.6 dB, 0 (0) |
| CRUNCH | 9000 Hz | 0.4 | -38.5 dB, 0 (0) | -6.4 dB, 31818 (29769) | -22.0 dB, 2345 (1001) | -33.3 dB, 0 (0) |
| HOT | 5000 Hz | 0.1 | -30.0 dB, 387 (0) | -16.9 dB, 0 (0) | -17.2 dB, 0 (0) | -26.1 dB, 11 (0) |
| HOT | 5000 Hz | 0.4 | -29.8 dB, 408 (0) | -16.3 dB, 686 (680) | -18.7 dB, 0 (0) | -26.2 dB, 59 (0) |
| HOT | 9000 Hz | 0.1 | -38.2 dB, 60 (0) | -14.3 dB, 0 (0) | -14.9 dB, 1 (0) | -24.8 dB, 29 (0) |
| HOT | 9000 Hz | 0.4 | -36.0 dB, 25 (0) | -7.4 dB, 25634 (22778) | -15.4 dB, 4663 (1119) | -27.9 dB, 13 (0) |

## Cost

| Rate | ns per engine sample (default / HOT) | % of one core at 48 kHz |
|---|---|---|
| 2x | 3553 / 5392 | 34.1 / 51.8 |
| 4x | 3503 / 4773 | 67.3 / 91.6 |
| 8x | 3360 / 4316 | 129.0 / 165.7 |
