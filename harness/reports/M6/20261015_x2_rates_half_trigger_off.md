# M6 A1: full engine (exact J4) at lower internal rates, plugin model check

- Date 2026-10-15; options: divisor 2, trigger 0.0; rates 48 kHz × [2, 4].
- Plugin model check (D-051): ESR ≤ -20 dB vs SPICE (cached 384000 Hz references, 48 kHz band, first 5 s of every DI track, first 0.1 s excluded). Unseen settings: the 16 X4 validation settings, 10 s, vs the engine at 16x (first 1 s excluded). Cost: ampwav --stats on 10 s of DI GTR L, one run at a time; oversampling filters not included.

## Summary

| Rate | vs SPICE worst (CRUNCH / DRIVE / HOT) | Check | Unseen settings vs 16x (worst / median) | Newton failures (not rescued) / runs, max iterations | Cost default / HOT (% of one core) |
|---|---|---|---|---|---|
| 2x (96 kHz) | -38.2 / -36.8 / -22.6 dB | PASS | -19.7 / -29.8 dB | 40 (31) / 28, 32 | 23.9 / 32.3 |
| 4x (192 kHz) | -48.1 / -52.8 / -24.3 dB | PASS | -18.8 / -33.2 dB | 4 (0) / 28, 32 | 45.1 / 60.1 |

## vs SPICE per clip

| Clip | 2x | 4x |
|---|---|---|
| CRUNCH GTR L | -38.8 dB | -48.5 dB |
| CRUNCH GTR LL | -38.2 dB | -48.1 dB |
| CRUNCH GTR R | -39.5 dB | -48.4 dB |
| CRUNCH GTR RR | -38.9 dB | -48.4 dB |
| DRIVE GTR L | -36.8 dB | -52.8 dB |
| DRIVE GTR LL | -38.0 dB | -53.6 dB |
| DRIVE GTR R | -38.0 dB | -53.6 dB |
| DRIVE GTR RR | -36.9 dB | -52.9 dB |
| HOT GTR L | -23.1 dB | -24.3 dB |
| HOT GTR LL | -23.7 dB | -25.0 dB |
| HOT GTR R | -23.9 dB | -25.3 dB |
| HOT GTR RR | -22.6 dB | -24.3 dB |

## Aliasing (power outside the harmonics of f0, relative to the total; 16x for reference)

Each cell: non-harmonic power, then Newton failures (not rescued) in that 2 s run. Non-converged samples add non-harmonic power, so a cell with many failures measures solver error as well as aliasing.

| Preset | f0 | amplitude (FS) | 16x | 2x | 4x |
|---|---|---|---|---|---|
| CRUNCH | 5000 Hz | 0.1 | -26.0 dB, 0 (0) | -14.1 dB, 0 (0) | -19.7 dB, 0 (0) |
| CRUNCH | 5000 Hz | 0.4 | -25.7 dB, 0 (0) | -11.8 dB, 2102 (1993) | -17.5 dB, 0 (0) |
| CRUNCH | 9000 Hz | 0.1 | -39.9 dB, 0 (0) | -9.4 dB, 0 (0) | -17.1 dB, 1 (0) |
| CRUNCH | 9000 Hz | 0.4 | -38.5 dB, 0 (0) | -4.2 dB, 28226 (26103) | -17.2 dB, 2239 (961) |
| HOT | 5000 Hz | 0.1 | -30.0 dB, 387 (0) | -14.4 dB, 0 (0) | -13.4 dB, 0 (0) |
| HOT | 5000 Hz | 0.4 | -29.8 dB, 408 (0) | -13.5 dB, 728 (697) | -13.3 dB, 0 (0) |
| HOT | 9000 Hz | 0.1 | -38.2 dB, 60 (0) | -4.9 dB, 0 (0) | -13.8 dB, 1 (0) |
| HOT | 9000 Hz | 0.4 | -36.0 dB, 25 (0) | -7.0 dB, 20490 (18206) | -17.7 dB, 5699 (1851) |

## Cost

| Rate | ns per engine sample (default / HOT) | % of one core at 48 kHz |
|---|---|---|
| 2x | 2489 / 3360 | 23.9 / 32.3 |
| 4x | 2350 / 3132 | 45.1 / 60.1 |
