# M6 A1: full engine (exact J4) at lower internal rates, plugin model check

> **Not a valid A1 result for exact J4:** the engine runs the exact J4 coupling only with the power amp at full rate (`Amp2203::exactJ4()`, divisor 1). These runs used the Thevenin J4 (D-015), which explains HOT at −13 dB (the pre-X1 J4 error). The stats field `exact_j4` reported the requested value until ampwav was fixed to report the value in effect.

- Date 2026-10-15; options: divisor 2, tol 1e-05; rates 48 kHz × [2, 4, 8].
- Plugin model check (D-051): ESR ≤ -20 dB vs SPICE (cached 384000 Hz references, 48 kHz band, first 5 s of every DI track, first 0.1 s excluded). Unseen settings: the 16 X4 validation settings, 10 s, vs the engine at 16x (first 1 s excluded). Cost: ampwav --stats on 10 s of DI GTR L, one run at a time; oversampling filters not included.

## Summary

| Rate | vs SPICE worst (CRUNCH / DRIVE / HOT) | Check | Unseen settings vs 16x (worst / median) | Newton failures (not rescued) / runs, max iterations | Cost default / HOT (% of one core) |
|---|---|---|---|---|---|
| 2x (96 kHz) | -40.1 / -39.2 / -12.9 dB | FAIL | -17.1 / -29.0 dB | 11346 (1145) / 28, 16 | 17.1 / 27.0 |
| 4x (192 kHz) | -47.8 / -51.8 / -13.0 dB | FAIL | -18.2 / -36.5 dB | 5926 (165) / 28, 16 | 31.1 / 43.9 |
| 8x (384 kHz) | -48.7 / -55.8 / -13.0 dB | FAIL | -20.2 / -49.1 dB | 1600 (2) / 28, 16 | 54.8 / 75.0 |

## vs SPICE per clip

| Clip | 2x | 4x | 8x |
|---|---|---|---|
| CRUNCH GTR L | -40.4 dB | -48.4 dB | -49.1 dB |
| CRUNCH GTR LL | -40.1 dB | -47.8 dB | -48.7 dB |
| CRUNCH GTR R | -41.3 dB | -48.4 dB | -48.9 dB |
| CRUNCH GTR RR | -40.8 dB | -48.5 dB | -49.0 dB |
| DRIVE GTR L | -39.2 dB | -51.8 dB | -55.8 dB |
| DRIVE GTR LL | -40.3 dB | -52.6 dB | -56.2 dB |
| DRIVE GTR R | -40.5 dB | -52.5 dB | -56.2 dB |
| DRIVE GTR RR | -39.8 dB | -52.0 dB | -56.1 dB |
| HOT GTR L | -13.0 dB | -13.2 dB | -13.2 dB |
| HOT GTR LL | -13.3 dB | -13.4 dB | -13.4 dB |
| HOT GTR R | -13.1 dB | -13.2 dB | -13.2 dB |
| HOT GTR RR | -12.9 dB | -13.0 dB | -13.0 dB |

## Aliasing (power outside the harmonics of f0, relative to the total; 16x for reference)

Each cell: non-harmonic power, then Newton failures (not rescued) in that 2 s run. Non-converged samples add non-harmonic power, so a cell with many failures measures solver error as well as aliasing.

| Preset | f0 | amplitude (FS) | 16x | 2x | 4x | 8x |
|---|---|---|---|---|---|---|
| CRUNCH | 5000 Hz | 0.1 | -26.0 dB, 0 (0) | -18.7 dB, 3956 (3935) | -26.8 dB, 7 (7) | -26.2 dB, 0 (0) |
| CRUNCH | 5000 Hz | 0.4 | -25.7 dB, 0 (0) | -5.5 dB, 43015 (41134) | -24.5 dB, 9705 (7126) | -25.8 dB, 5 (4) |
| CRUNCH | 9000 Hz | 0.1 | -39.9 dB, 0 (0) | -7.8 dB, 38212 (17066) | -25.4 dB, 24 (23) | -37.3 dB, 0 (0) |
| CRUNCH | 9000 Hz | 0.4 | -38.5 dB, 0 (0) | -3.0 dB, 160580 (157904) | -3.8 dB, 77792 (75144) | -31.5 dB, 11637 (4584) |
| HOT | 5000 Hz | 0.1 | -30.0 dB, 1990 (0) | -13.9 dB, 7700 (3494) | -15.3 dB, 1180 (7) | -14.5 dB, 4454 (0) |
| HOT | 5000 Hz | 0.4 | -29.8 dB, 2395 (0) | -2.6 dB, 97620 (93071) | -9.4 dB, 26172 (23590) | -17.3 dB, 2797 (4) |
| HOT | 9000 Hz | 0.1 | -38.2 dB, 1424 (0) | -8.8 dB, 39596 (18160) | -10.4 dB, 8522 (2837) | -13.9 dB, 3770 (0) |
| HOT | 9000 Hz | 0.4 | -36.0 dB, 833 (20) | -1.1 dB, 94772 (79905) | -1.7 dB, 82497 (76306) | -15.0 dB, 17683 (7089) |

## Cost

| Rate | ns per engine sample (default / HOT) | % of one core at 48 kHz |
|---|---|---|
| 2x | 1786 / 2809 | 17.1 / 27.0 |
| 4x | 1621 / 2286 | 31.1 / 43.9 |
| 8x | 1427 / 1954 | 54.8 / 75.0 |
