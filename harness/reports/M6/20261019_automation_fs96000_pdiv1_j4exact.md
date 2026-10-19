# M5 automation: every knob over its full range at audio rate

- Date: 2026-10-19, commit `ecf9042 (dirty)`, engine `ampwav --block full` at 96000 Hz
- Modulation: 10 Hz triangle 0 → 1 → 0 (`lfo`) or 0 ↔ 1 jumps every 50 ms (`jump`) on the knob targets (smoothed by the engine, 20 ms); bases: default knobs, every knob at 1.0.
- SPEC §7 M5: no NaN / Inf, no Newton failures. Newton failures count first solves that did not converge in 12 iterations; the fallback (D-042) rescued or did not rescue them.

Runs: 56 · non-finite outputs: 0 · Newton failures (first solve): 0 · unrescued: 0

| Base | Knobs | Modulation | Signal | finite | peak [V] | solves | failures | rescued | unrescued | median / max it | ns/sample |
|---|---|---|---|---|---|---|---|---|---|---|---|
| default | preampVolume | lfo | SIN-220Hz-0.3V | yes | 34.7 | 192000 | 0 | 0 | 0 | 1 / 6 | 2667 |
| default | preampVolume | lfo | DI-GTR L | yes | 55.7 | 963977 | 0 | 0 | 0 | 1 / 14 | 3069 |
| default | preampVolume | jump | SIN-220Hz-0.3V | yes | 40.3 | 192059 | 0 | 0 | 0 | 1 / 22 | 2590 |
| default | preampVolume | jump | DI-GTR L | yes | 59.8 | 966452 | 0 | 0 | 0 | 1 / 15 | 2781 |
| default | treble | lfo | SIN-220Hz-0.3V | yes | 41.4 | 192001 | 0 | 0 | 0 | 1 / 22 | 2848 |
| default | treble | lfo | DI-GTR L | yes | 64.3 | 965841 | 0 | 0 | 0 | 1 / 15 | 3250 |
| default | treble | jump | SIN-220Hz-0.3V | yes | 39.1 | 192007 | 0 | 0 | 0 | 1 / 22 | 2727 |
| default | treble | jump | DI-GTR L | yes | 76.6 | 967068 | 0 | 0 | 0 | 1 / 15 | 3368 |
| default | middle | lfo | SIN-220Hz-0.3V | yes | 35.1 | 192003 | 0 | 0 | 0 | 1 / 22 | 2937 |
| default | middle | lfo | DI-GTR L | yes | 56.3 | 965788 | 0 | 0 | 0 | 1 / 12 | 3379 |
| default | middle | jump | SIN-220Hz-0.3V | yes | 39.9 | 192006 | 0 | 0 | 0 | 1 / 22 | 2695 |
| default | middle | jump | DI-GTR L | yes | 65.0 | 966106 | 0 | 0 | 0 | 1 / 18 | 3379 |
| default | bass | lfo | SIN-220Hz-0.3V | yes | 39.3 | 192005 | 0 | 0 | 0 | 1 / 22 | 2532 |
| default | bass | lfo | DI-GTR L | yes | 59.1 | 965770 | 0 | 0 | 0 | 1 / 12 | 2714 |
| default | bass | jump | SIN-220Hz-0.3V | yes | 45.3 | 192005 | 0 | 0 | 0 | 1 / 22 | 2681 |
| default | bass | jump | DI-GTR L | yes | 57.5 | 965827 | 0 | 0 | 0 | 1 / 16 | 2954 |
| default | master | lfo | SIN-220Hz-0.3V | yes | 75.8 | 217572 | 0 | 0 | 0 | 1 / 22 | 2935 |
| default | master | lfo | DI-GTR L | yes | 129.1 | 1066454 | 0 | 0 | 0 | 1 / 22 | 3500 |
| default | master | jump | SIN-220Hz-0.3V | yes | 82.8 | 229875 | 0 | 0 | 0 | 1 / 22 | 2852 |
| default | master | jump | DI-GTR L | yes | 152.9 | 1096948 | 0 | 0 | 0 | 1 / 25 | 4113 |
| default | presence | lfo | SIN-220Hz-0.3V | yes | 35.7 | 192005 | 0 | 0 | 0 | 1 / 22 | 2847 |
| default | presence | lfo | DI-GTR L | yes | 58.0 | 965946 | 0 | 0 | 0 | 1 / 15 | 3094 |
| default | presence | jump | SIN-220Hz-0.3V | yes | 66.3 | 192021 | 0 | 0 | 0 | 1 / 22 | 2660 |
| default | presence | jump | DI-GTR L | yes | 75.2 | 966991 | 0 | 0 | 0 | 1 / 15 | 3162 |
| default | all | lfo | SIN-220Hz-0.3V | yes | 77.1 | 220360 | 0 | 0 | 0 | 1 / 7 | 3533 |
| default | all | lfo | DI-GTR L | yes | 154.1 | 1082674 | 0 | 0 | 0 | 1 / 25 | 6988 |
| default | all | jump | SIN-220Hz-0.3V | yes | 138.3 | 235862 | 0 | 0 | 0 | 1 / 22 | 3499 |
| default | all | jump | DI-GTR L | yes | 171.9 | 1123594 | 0 | 0 | 0 | 1 / 26 | 6936 |
| max | preampVolume | lfo | SIN-220Hz-0.3V | yes | 79.5 | 249747 | 0 | 0 | 0 | 1 / 11 | 2937 |
| max | preampVolume | lfo | DI-GTR L | yes | 213.7 | 1254609 | 0 | 0 | 0 | 1 / 19 | 3887 |
| max | preampVolume | jump | SIN-220Hz-0.3V | yes | 138.3 | 270652 | 0 | 0 | 0 | 1 / 22 | 3393 |
| max | preampVolume | jump | DI-GTR L | yes | 216.8 | 1281858 | 0 | 0 | 0 | 1 / 21 | 4413 |
| max | treble | lfo | SIN-220Hz-0.3V | yes | 124.9 | 302151 | 0 | 0 | 0 | 1 / 22 | 4042 |
| max | treble | lfo | DI-GTR L | yes | 210.1 | 1393862 | 0 | 0 | 0 | 1 / 24 | 4939 |
| max | treble | jump | SIN-220Hz-0.3V | yes | 138.3 | 301692 | 0 | 0 | 0 | 1 / 22 | 3984 |
| max | treble | jump | DI-GTR L | yes | 215.0 | 1385739 | 0 | 0 | 0 | 1 / 23 | 4895 |
| max | middle | lfo | SIN-220Hz-0.3V | yes | 183.1 | 307742 | 0 | 0 | 0 | 1 / 22 | 4658 |
| max | middle | lfo | DI-GTR L | yes | 214.6 | 1361724 | 0 | 0 | 0 | 1 / 24 | 4763 |
| max | middle | jump | SIN-220Hz-0.3V | yes | 138.3 | 304051 | 0 | 0 | 0 | 1 / 22 | 4243 |
| max | middle | jump | DI-GTR L | yes | 214.7 | 1360147 | 0 | 0 | 0 | 1 / 29 | 4726 |
| max | bass | lfo | SIN-220Hz-0.3V | yes | 143.8 | 311162 | 0 | 0 | 0 | 1 / 22 | 4913 |
| max | bass | lfo | DI-GTR L | yes | 217.2 | 1399126 | 0 | 0 | 0 | 1 / 25 | 4694 |
| max | bass | jump | SIN-220Hz-0.3V | yes | 149.5 | 309821 | 0 | 0 | 0 | 1 / 22 | 4568 |
| max | bass | jump | DI-GTR L | yes | 215.7 | 1396010 | 0 | 0 | 0 | 1 / 20 | 4469 |
| max | master | lfo | SIN-220Hz-0.3V | yes | 75.0 | 247384 | 0 | 0 | 0 | 1 / 22 | 3091 |
| max | master | lfo | DI-GTR L | yes | 170.7 | 1187928 | 0 | 0 | 0 | 1 / 26 | 3948 |
| max | master | jump | SIN-220Hz-0.3V | yes | 138.3 | 249404 | 0 | 0 | 0 | 1 / 22 | 3302 |
| max | master | jump | DI-GTR L | yes | 181.6 | 1186712 | 0 | 0 | 0 | 1 / 31 | 3928 |
| max | presence | lfo | SIN-220Hz-0.3V | yes | 107.9 | 301617 | 0 | 0 | 0 | 1 / 22 | 4084 |
| max | presence | lfo | DI-GTR L | yes | 211.7 | 1341314 | 0 | 0 | 0 | 1 / 26 | 4010 |
| max | presence | jump | SIN-220Hz-0.3V | yes | 138.3 | 302590 | 0 | 0 | 0 | 1 / 22 | 4307 |
| max | presence | jump | DI-GTR L | yes | 212.4 | 1342728 | 0 | 0 | 0 | 1 / 22 | 3873 |
| max | all | lfo | SIN-220Hz-0.3V | yes | 77.1 | 220360 | 0 | 0 | 0 | 1 / 7 | 3777 |
| max | all | lfo | DI-GTR L | yes | 154.1 | 1082674 | 0 | 0 | 0 | 1 / 25 | 5991 |
| max | all | jump | SIN-220Hz-0.3V | yes | 138.3 | 235862 | 0 | 0 | 0 | 1 / 22 | 3400 |
| max | all | jump | DI-GTR L | yes | 171.9 | 1123594 | 0 | 0 | 0 | 1 / 26 | 5910 |
