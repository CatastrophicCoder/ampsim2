# M6 A1: full engine (exact J4) at lower internal rates, plugin model check

- Date 2026-10-19; options: divisor 2, tol 0.0001; rates 48 kHz × [2].
- Plugin model check (D-051): ESR ≤ -20 dB vs SPICE (cached 384000 Hz references, 48 kHz band, first 5 s of every DI track, first 0.1 s excluded). Unseen settings: the 16 X4 validation settings, 10 s, vs the engine at 16x (first 1 s excluded). Cost: ampwav --stats on 10 s of DI GTR L, one run at a time; oversampling filters not included.

## Summary

| Rate | vs SPICE worst (CRUNCH / DRIVE / HOT) | Check | Unseen settings vs 16x (worst / median) | Newton failures (not rescued) / runs, max iterations | Cost default / HOT (% of one core) |
|---|---|---|---|---|---|
| 2x (96 kHz) | -37.7 / -37.2 / -18.9 dB | FAIL | -16.6 / -28.0 dB | 0 (0) / 28, 38 | 13.5 / 21.2 |

## vs SPICE per clip

| Clip | 2x |
|---|---|
| CRUNCH GTR L | -38.0 dB |
| CRUNCH GTR LL | -37.7 dB |
| CRUNCH GTR R | -38.9 dB |
| CRUNCH GTR RR | -38.2 dB |
| DRIVE GTR L | -37.2 dB |
| DRIVE GTR LL | -38.3 dB |
| DRIVE GTR R | -38.6 dB |
| DRIVE GTR RR | -37.5 dB |
| HOT GTR L | -18.9 dB |
| HOT GTR LL | -19.6 dB |
| HOT GTR R | -20.0 dB |
| HOT GTR RR | -19.0 dB |

## Aliasing (power outside the harmonics of f0, relative to the total; 16x for reference)

Each cell: non-harmonic power, then Newton failures (not rescued) in that 2 s run. Non-converged samples add non-harmonic power, so a cell with many failures measures solver error as well as aliasing.

| Preset | f0 | amplitude (FS) | 16x | 2x |
|---|---|---|---|---|
| CRUNCH | 5000 Hz | 0.1 | -25.3 dB, 0 (0) | -12.6 dB, 0 (0) |
| CRUNCH | 5000 Hz | 0.4 | -24.4 dB, 0 (0) | -12.1 dB, 0 (0) |
| CRUNCH | 9000 Hz | 0.1 | -40.9 dB, 0 (0) | -9.2 dB, 0 (0) |
| CRUNCH | 9000 Hz | 0.4 | -40.0 dB, 0 (0) | -8.6 dB, 0 (0) |
| HOT | 5000 Hz | 0.1 | -25.1 dB, 0 (0) | -9.0 dB, 0 (0) |
| HOT | 5000 Hz | 0.4 | -25.0 dB, 0 (0) | -10.2 dB, 0 (0) |
| HOT | 9000 Hz | 0.1 | -38.8 dB, 0 (0) | -7.0 dB, 0 (0) |
| HOT | 9000 Hz | 0.4 | -33.3 dB, 0 (0) | -7.1 dB, 0 (0) |

## Cost

| Rate | ns per engine sample (default / HOT) | % of one core at 48 kHz |
|---|---|---|
| 2x | 1409 / 2203 | 13.5 / 21.2 |
