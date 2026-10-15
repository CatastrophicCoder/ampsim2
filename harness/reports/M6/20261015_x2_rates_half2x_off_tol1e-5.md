# M6 A1: full engine (exact J4) at lower internal rates, plugin model check

- Date 2026-10-15; options: divisor 2, tol 1e-05, trigger 0.0; rates 48 kHz × [2].
- Plugin model check (D-051): ESR ≤ -20 dB vs SPICE (cached 384000 Hz references, 48 kHz band, first 5 s of every DI track, first 0.1 s excluded). Unseen settings: the 16 X4 validation settings, 10 s, vs the engine at 16x (first 1 s excluded). Cost: ampwav --stats on 10 s of DI GTR L, one run at a time; oversampling filters not included.

## Summary

| Rate | vs SPICE worst (CRUNCH / DRIVE / HOT) | Check | Unseen settings vs 16x (worst / median) | Newton failures (not rescued) / runs, max iterations | Cost default / HOT (% of one core) |
|---|---|---|---|---|---|
| 2x (96 kHz) | -38.2 / -36.8 / -22.6 dB | PASS | -19.7 / -29.8 dB | 12 (2) / 28, 32 | 21.5 / 28.3 |

## vs SPICE per clip

| Clip | 2x |
|---|---|
| CRUNCH GTR L | -38.8 dB |
| CRUNCH GTR LL | -38.2 dB |
| CRUNCH GTR R | -39.5 dB |
| CRUNCH GTR RR | -38.9 dB |
| DRIVE GTR L | -36.8 dB |
| DRIVE GTR LL | -38.0 dB |
| DRIVE GTR R | -38.0 dB |
| DRIVE GTR RR | -36.9 dB |
| HOT GTR L | -23.1 dB |
| HOT GTR LL | -23.7 dB |
| HOT GTR R | -23.9 dB |
| HOT GTR RR | -22.6 dB |

## Aliasing (power outside the harmonics of f0, relative to the total; 16x for reference)

Each cell: non-harmonic power, then Newton failures (not rescued) in that 2 s run. Non-converged samples add non-harmonic power, so a cell with many failures measures solver error as well as aliasing.

| Preset | f0 | amplitude (FS) | 16x | 2x |
|---|---|---|---|---|
| CRUNCH | 5000 Hz | 0.1 | -26.0 dB, 0 (0) | -14.1 dB, 0 (0) |
| CRUNCH | 5000 Hz | 0.4 | -25.7 dB, 0 (0) | -11.8 dB, 2065 (1992) |
| CRUNCH | 9000 Hz | 0.1 | -39.9 dB, 0 (0) | -9.4 dB, 0 (0) |
| CRUNCH | 9000 Hz | 0.4 | -38.5 dB, 0 (0) | -4.3 dB, 26687 (24457) |
| HOT | 5000 Hz | 0.1 | -30.0 dB, 387 (0) | -14.4 dB, 0 (0) |
| HOT | 5000 Hz | 0.4 | -29.8 dB, 408 (0) | -13.5 dB, 700 (697) |
| HOT | 9000 Hz | 0.1 | -38.2 dB, 60 (0) | -4.9 dB, 0 (0) |
| HOT | 9000 Hz | 0.4 | -36.0 dB, 25 (0) | -7.1 dB, 19041 (16294) |

## Cost

| Rate | ns per engine sample (default / HOT) | % of one core at 48 kHz |
|---|---|---|
| 2x | 2237 / 2948 | 21.5 / 28.3 |
