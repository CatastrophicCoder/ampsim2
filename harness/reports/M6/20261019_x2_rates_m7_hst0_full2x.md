# M6 A1: full engine (exact J4) at lower internal rates, plugin model check

- Date 2026-10-19; options: tol 0.0001, trigger 0.0; rates 48 kHz × [2].
- Plugin model check (D-051): ESR ≤ -20 dB vs SPICE (cached 384000 Hz references, 48 kHz band, first 5 s of every DI track, first 0.1 s excluded). Unseen settings: the 16 X4 validation settings, 10 s, vs the engine at 16x (first 1 s excluded). Cost: ampwav --stats on 10 s of DI GTR L, one run at a time; oversampling filters not included.

## Summary

| Rate | vs SPICE worst (CRUNCH / DRIVE / HOT) | Check | Unseen settings vs 16x (worst / median) | Newton failures (not rescued) / runs, max iterations | Cost default / HOT (% of one core) |
|---|---|---|---|---|---|
| 2x (96 kHz) | -45.7 / -51.1 / -22.7 dB | PASS | -19.8 / -37.3 dB | 0 (0) / 28, 38 | 22.3 / 29.6 |

## vs SPICE per clip

| Clip | 2x |
|---|---|
| CRUNCH GTR L | -46.2 dB |
| CRUNCH GTR LL | -45.7 dB |
| CRUNCH GTR R | -46.5 dB |
| CRUNCH GTR RR | -46.1 dB |
| DRIVE GTR L | -51.1 dB |
| DRIVE GTR LL | -52.1 dB |
| DRIVE GTR R | -51.9 dB |
| DRIVE GTR RR | -51.7 dB |
| HOT GTR L | -22.7 dB |
| HOT GTR LL | -23.5 dB |
| HOT GTR R | -23.7 dB |
| HOT GTR RR | -22.9 dB |

## Aliasing (power outside the harmonics of f0, relative to the total; 16x for reference)

Each cell: non-harmonic power, then Newton failures (not rescued) in that 2 s run. Non-converged samples add non-harmonic power, so a cell with many failures measures solver error as well as aliasing.

| Preset | f0 | amplitude (FS) | 16x | 2x |
|---|---|---|---|---|
| CRUNCH | 5000 Hz | 0.1 | -25.3 dB, 0 (0) | -19.8 dB, 0 (0) |
| CRUNCH | 5000 Hz | 0.4 | -24.4 dB, 0 (0) | -16.7 dB, 0 (0) |
| CRUNCH | 9000 Hz | 0.1 | -40.9 dB, 0 (0) | -18.2 dB, 0 (0) |
| CRUNCH | 9000 Hz | 0.4 | -40.0 dB, 0 (0) | -14.7 dB, 0 (0) |
| HOT | 5000 Hz | 0.1 | -25.1 dB, 0 (0) | -13.1 dB, 0 (0) |
| HOT | 5000 Hz | 0.4 | -25.0 dB, 0 (0) | -13.2 dB, 0 (0) |
| HOT | 9000 Hz | 0.1 | -38.8 dB, 0 (0) | -14.6 dB, 0 (0) |
| HOT | 9000 Hz | 0.4 | -33.3 dB, 0 (0) | -15.9 dB, 0 (0) |

## Cost

| Rate | ns per engine sample (default / HOT) | % of one core at 48 kHz |
|---|---|---|
| 2x | 2318 / 3083 | 22.3 / 29.6 |
