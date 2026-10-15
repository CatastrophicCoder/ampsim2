# M6 A1: full engine (exact J4) at lower internal rates, plugin model check

- Date 2026-10-15; options: divisor 2; rates 48 kHz × [2, 4, 8].
- Plugin model check (D-051): ESR ≤ -20 dB vs SPICE (cached 384000 Hz references, 48 kHz band, first 5 s of every DI track, first 0.1 s excluded). Unseen settings: the 16 X4 validation settings, 10 s, vs the engine at 16x (first 1 s excluded). Cost: ampwav --stats on 10 s of DI GTR L, one run at a time; oversampling filters not included.

## Summary

| Rate | vs SPICE worst (CRUNCH / DRIVE / HOT) | Check | Unseen settings vs 16x (worst / median) | Newton failures (not rescued) / runs, max iterations | Cost default / HOT (% of one core) |
|---|---|---|---|---|---|
| 2x (96 kHz) | -38.8 / -38.0 / -22.5 dB | PASS | -18.9 / -30.1 dB | 40 (31) / 28, 32 | 24.1 / 35.8 |
| 4x (192 kHz) | -47.6 / -51.0 / -26.3 dB | PASS | -20.6 / -40.1 dB | 4 (0) / 28, 32 | 45.5 / 63.4 |
| 8x (384 kHz) | -48.7 / -55.7 / -33.2 dB | PASS | -26.8 / -54.5 dB | 3 (0) / 28, 32 | 88.7 / 114.8 |

## vs SPICE per clip

| Clip | 2x | 4x | 8x |
|---|---|---|---|
| CRUNCH GTR L | -39.3 dB | -48.2 dB | -49.0 dB |
| CRUNCH GTR LL | -38.8 dB | -47.6 dB | -48.7 dB |
| CRUNCH GTR R | -40.2 dB | -48.2 dB | -48.9 dB |
| CRUNCH GTR RR | -39.6 dB | -48.1 dB | -49.0 dB |
| DRIVE GTR L | -38.0 dB | -51.0 dB | -55.7 dB |
| DRIVE GTR LL | -39.0 dB | -51.8 dB | -56.2 dB |
| DRIVE GTR R | -39.2 dB | -51.9 dB | -56.0 dB |
| DRIVE GTR RR | -38.1 dB | -51.2 dB | -55.9 dB |
| HOT GTR L | -22.7 dB | -26.3 dB | -33.2 dB |
| HOT GTR LL | -23.7 dB | -27.0 dB | -33.8 dB |
| HOT GTR R | -23.5 dB | -27.3 dB | -34.1 dB |
| HOT GTR RR | -22.5 dB | -26.4 dB | -33.5 dB |

## Aliasing (power outside the harmonics of f0, relative to the total; 16x for reference)

Each cell: non-harmonic power, then Newton failures (not rescued) in that 2 s run. Non-converged samples add non-harmonic power, so a cell with many failures measures solver error as well as aliasing.

| Preset | f0 | amplitude (FS) | 16x | 2x | 4x | 8x |
|---|---|---|---|---|---|---|
| CRUNCH | 5000 Hz | 0.1 | -26.0 dB, 0 (0) | -14.4 dB, 0 (0) | -19.8 dB, 0 (0) | -25.5 dB, 0 (0) |
| CRUNCH | 5000 Hz | 0.4 | -25.7 dB, 0 (0) | -12.1 dB, 2037 (1993) | -17.4 dB, 0 (0) | -23.5 dB, 0 (0) |
| CRUNCH | 9000 Hz | 0.1 | -39.9 dB, 0 (0) | -8.9 dB, 0 (0) | -16.8 dB, 1 (0) | -29.1 dB, 0 (0) |
| CRUNCH | 9000 Hz | 0.4 | -38.5 dB, 0 (0) | -4.6 dB, 26930 (24584) | -17.0 dB, 2368 (977) | -21.8 dB, 0 (0) |
| HOT | 5000 Hz | 0.1 | -30.0 dB, 387 (0) | -11.2 dB, 0 (0) | -16.1 dB, 0 (0) | -17.3 dB, 0 (0) |
| HOT | 5000 Hz | 0.4 | -29.8 dB, 408 (0) | -10.2 dB, 662 (653) | -16.0 dB, 0 (0) | -16.6 dB, 0 (0) |
| HOT | 9000 Hz | 0.1 | -38.2 dB, 60 (0) | -10.3 dB, 0 (0) | -13.7 dB, 1 (0) | -17.8 dB, 0 (0) |
| HOT | 9000 Hz | 0.4 | -36.0 dB, 25 (0) | -7.3 dB, 19100 (15938) | -15.5 dB, 6191 (2259) | -14.9 dB, 0 (0) |

## Cost

| Rate | ns per engine sample (default / HOT) | % of one core at 48 kHz |
|---|---|---|
| 2x | 2505 / 3729 | 24.1 / 35.8 |
| 4x | 2367 / 3305 | 45.5 / 63.4 |
| 8x | 2309 / 2989 | 88.7 / 114.8 |
