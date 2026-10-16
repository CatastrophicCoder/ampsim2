# M6 A1: full engine (exact J4) at lower internal rates, plugin model check

- Date 2026-10-16; options: divisor 2, tol 0.0001, trigger 0.0; rates 48 kHz × [2].
- Plugin model check (D-051): ESR ≤ -20 dB vs SPICE (cached 384000 Hz references, 48 kHz band, first 5 s of every DI track, first 0.1 s excluded). Unseen settings: the 16 X4 validation settings, 10 s, vs the engine at 16x (first 1 s excluded). Cost: ampwav --stats on 10 s of DI GTR L, one run at a time; oversampling filters not included.

## Summary

| Rate | vs SPICE worst (CRUNCH / DRIVE / HOT) | Check | Unseen settings vs 16x (worst / median) | Newton failures (not rescued) / runs, max iterations | Cost default / HOT (% of one core) |
|---|---|---|---|---|---|
| 2x (96 kHz) | -36.8 / -36.0 / -19.2 dB | FAIL | -17.1 / -26.9 dB | 44 (1) / 28, 32 | 19.1 / 25.6 |

## vs SPICE per clip

| Clip | 2x |
|---|---|
| CRUNCH GTR L | -37.3 dB |
| CRUNCH GTR LL | -36.8 dB |
| CRUNCH GTR R | -38.0 dB |
| CRUNCH GTR RR | -37.1 dB |
| DRIVE GTR L | -36.0 dB |
| DRIVE GTR LL | -37.2 dB |
| DRIVE GTR R | -37.0 dB |
| DRIVE GTR RR | -36.0 dB |
| HOT GTR L | -19.7 dB |
| HOT GTR LL | -20.4 dB |
| HOT GTR R | -20.4 dB |
| HOT GTR RR | -19.2 dB |

## Aliasing (power outside the harmonics of f0, relative to the total; 16x for reference)

Each cell: non-harmonic power, then Newton failures (not rescued) in that 2 s run. Non-converged samples add non-harmonic power, so a cell with many failures measures solver error as well as aliasing.

| Preset | f0 | amplitude (FS) | 16x | 2x |
|---|---|---|---|---|
| CRUNCH | 5000 Hz | 0.1 | -25.0 dB, 0 (0) | -13.9 dB, 0 (0) |
| CRUNCH | 5000 Hz | 0.4 | -24.7 dB, 0 (0) | -11.5 dB, 1930 (1929) |
| CRUNCH | 9000 Hz | 0.1 | -39.4 dB, 0 (0) | -9.3 dB, 0 (0) |
| CRUNCH | 9000 Hz | 0.4 | -38.5 dB, 0 (0) | -5.1 dB, 27298 (25061) |
| HOT | 5000 Hz | 0.1 | -25.0 dB, 1 (0) | -12.6 dB, 0 (0) |
| HOT | 5000 Hz | 0.4 | -25.4 dB, 1 (0) | -12.2 dB, 802 (800) |
| HOT | 9000 Hz | 0.1 | -37.4 dB, 0 (0) | -5.5 dB, 0 (0) |
| HOT | 9000 Hz | 0.4 | -35.0 dB, 0 (0) | -8.1 dB, 20164 (17684) |

## Cost

| Rate | ns per engine sample (default / HOT) | % of one core at 48 kHz |
|---|---|---|
| 2x | 1985 / 2662 | 19.1 / 25.6 |
