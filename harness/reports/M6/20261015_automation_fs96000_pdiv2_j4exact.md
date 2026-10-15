# M5 automation: every knob over its full range at audio rate

- Date: 2026-10-15, commit `e4521b3 (dirty)`, engine `ampwav --block full` at 96000 Hz
- Modulation: 10 Hz triangle 0 → 1 → 0 (`lfo`) or 0 ↔ 1 jumps every 50 ms (`jump`) on the knob targets (smoothed by the engine, 20 ms); bases: default knobs, every knob at 1.0.
- SPEC §7 M5: no NaN / Inf, no Newton failures. Newton failures count first solves that did not converge in 12 iterations; the fallback (D-042) rescued or did not rescue them.

Runs: 56 · non-finite outputs: 0 · Newton failures (first solve): 0 · unrescued: 0

| Base | Knobs | Modulation | Signal | finite | peak [V] | solves | failures | rescued | unrescued | median / max it | ns/sample |
|---|---|---|---|---|---|---|---|---|---|---|---|
| default | preampVolume | lfo | SIN-220Hz-0.3V | yes | 33.2 | 192033 | 0 | 0 | 0 | 2 / 20 | 5668 |
| default | preampVolume | lfo | DI-GTR L | yes | 47.2 | 960095 | 0 | 0 | 0 | 2 / 19 | 5820 |
| default | preampVolume | jump | SIN-220Hz-0.3V | yes | 38.4 | 192033 | 0 | 0 | 0 | 2 / 26 | 5536 |
| default | preampVolume | jump | DI-GTR L | yes | 45.9 | 960165 | 0 | 0 | 0 | 2 / 20 | 5817 |
| default | treble | lfo | SIN-220Hz-0.3V | yes | 40.3 | 192033 | 0 | 0 | 0 | 2 / 23 | 5736 |
| default | treble | lfo | DI-GTR L | yes | 50.7 | 960114 | 0 | 0 | 0 | 2 / 20 | 6169 |
| default | treble | jump | SIN-220Hz-0.3V | yes | 32.5 | 192034 | 0 | 0 | 0 | 2 / 23 | 5597 |
| default | treble | jump | DI-GTR L | yes | 65.6 | 960208 | 0 | 0 | 0 | 2 / 20 | 6097 |
| default | middle | lfo | SIN-220Hz-0.3V | yes | 34.1 | 192033 | 0 | 0 | 0 | 2 / 26 | 5770 |
| default | middle | lfo | DI-GTR L | yes | 45.9 | 960112 | 0 | 0 | 0 | 2 / 19 | 6149 |
| default | middle | jump | SIN-220Hz-0.3V | yes | 38.7 | 192033 | 0 | 0 | 0 | 2 / 23 | 5579 |
| default | middle | jump | DI-GTR L | yes | 52.5 | 960143 | 0 | 0 | 0 | 2 / 20 | 6100 |
| default | bass | lfo | SIN-220Hz-0.3V | yes | 37.1 | 192033 | 0 | 0 | 0 | 2 / 23 | 5655 |
| default | bass | lfo | DI-GTR L | yes | 45.5 | 960126 | 0 | 0 | 0 | 2 / 20 | 5763 |
| default | bass | jump | SIN-220Hz-0.3V | yes | 42.2 | 192033 | 0 | 0 | 0 | 2 / 23 | 5498 |
| default | bass | jump | DI-GTR L | yes | 48.3 | 960123 | 0 | 0 | 0 | 2 / 18 | 5731 |
| default | master | lfo | SIN-220Hz-0.3V | yes | 69.3 | 192040 | 0 | 0 | 0 | 2 / 23 | 6067 |
| default | master | lfo | DI-GTR L | yes | 75.1 | 966556 | 0 | 0 | 0 | 2 / 26 | 6430 |
| default | master | jump | SIN-220Hz-0.3V | yes | 75.9 | 192466 | 0 | 0 | 0 | 2 / 23 | 6023 |
| default | master | jump | DI-GTR L | yes | 81.2 | 969786 | 0 | 0 | 0 | 2 / 23 | 6508 |
| default | presence | lfo | SIN-220Hz-0.3V | yes | 34.2 | 192033 | 0 | 0 | 0 | 2 / 23 | 6457 |
| default | presence | lfo | DI-GTR L | yes | 45.6 | 960120 | 0 | 0 | 0 | 2 / 19 | 6608 |
| default | presence | jump | SIN-220Hz-0.3V | yes | 49.3 | 192034 | 0 | 0 | 0 | 2 / 23 | 6255 |
| default | presence | jump | DI-GTR L | yes | 65.6 | 960191 | 0 | 0 | 0 | 2 / 19 | 6590 |
| default | all | lfo | SIN-220Hz-0.3V | yes | 71.1 | 192398 | 0 | 0 | 0 | 2 / 12 | 10594 |
| default | all | lfo | DI-GTR L | yes | 78.7 | 970090 | 0 | 0 | 0 | 2 / 23 | 13166 |
| default | all | jump | SIN-220Hz-0.3V | yes | 77.5 | 195640 | 0 | 0 | 0 | 2 / 26 | 10531 |
| default | all | jump | DI-GTR L | yes | 82.2 | 976641 | 0 | 0 | 0 | 2 / 25 | 13172 |
| max | preampVolume | lfo | SIN-220Hz-0.3V | yes | 75.4 | 199440 | 0 | 0 | 0 | 3 / 14 | 7118 |
| max | preampVolume | lfo | DI-GTR L | yes | 94.4 | 1030617 | 0 | 0 | 0 | 3 / 19 | 7639 |
| max | preampVolume | jump | SIN-220Hz-0.3V | yes | 84.3 | 204266 | 0 | 0 | 0 | 3 / 26 | 7115 |
| max | preampVolume | jump | DI-GTR L | yes | 89.5 | 1030611 | 0 | 0 | 0 | 3 / 21 | 7537 |
| max | treble | lfo | SIN-220Hz-0.3V | yes | 79.4 | 210094 | 0 | 0 | 0 | 2 / 26 | 7584 |
| max | treble | lfo | DI-GTR L | yes | 88.0 | 1023639 | 0 | 0 | 0 | 2 / 28 | 7505 |
| max | treble | jump | SIN-220Hz-0.3V | yes | 84.6 | 209196 | 0 | 0 | 0 | 2 / 26 | 7371 |
| max | treble | jump | DI-GTR L | yes | 90.0 | 1021417 | 0 | 0 | 0 | 2 / 26 | 7431 |
| max | middle | lfo | SIN-220Hz-0.3V | yes | 85.5 | 211603 | 0 | 0 | 0 | 2 / 26 | 7621 |
| max | middle | lfo | DI-GTR L | yes | 91.7 | 1029512 | 0 | 0 | 0 | 3 / 22 | 7867 |
| max | middle | jump | SIN-220Hz-0.3V | yes | 87.2 | 212094 | 0 | 0 | 0 | 2 / 26 | 7507 |
| max | middle | jump | DI-GTR L | yes | 90.2 | 1030194 | 0 | 0 | 0 | 3 / 20 | 7830 |
| max | bass | lfo | SIN-220Hz-0.3V | yes | 79.9 | 216154 | 0 | 0 | 0 | 2 / 26 | 7975 |
| max | bass | lfo | DI-GTR L | yes | 92.0 | 1040450 | 0 | 0 | 0 | 3 / 22 | 7684 |
| max | bass | jump | SIN-220Hz-0.3V | yes | 80.1 | 214554 | 0 | 0 | 0 | 2 / 26 | 7663 |
| max | bass | jump | DI-GTR L | yes | 91.0 | 1036902 | 0 | 0 | 0 | 3 / 22 | 7622 |
| max | master | lfo | SIN-220Hz-0.3V | yes | 73.3 | 199385 | 0 | 0 | 0 | 2 / 26 | 6884 |
| max | master | lfo | DI-GTR L | yes | 81.6 | 984999 | 0 | 0 | 0 | 2 / 28 | 7004 |
| max | master | jump | SIN-220Hz-0.3V | yes | 77.5 | 201669 | 0 | 0 | 0 | 2 / 26 | 6776 |
| max | master | jump | DI-GTR L | yes | 86.4 | 985722 | 0 | 0 | 0 | 2 / 26 | 6909 |
| max | presence | lfo | SIN-220Hz-0.3V | yes | 82.6 | 211727 | 0 | 0 | 0 | 2 / 26 | 8444 |
| max | presence | lfo | DI-GTR L | yes | 92.3 | 1022025 | 0 | 0 | 0 | 3 / 22 | 8293 |
| max | presence | jump | SIN-220Hz-0.3V | yes | 79.9 | 212150 | 0 | 0 | 0 | 2 / 26 | 8175 |
| max | presence | jump | DI-GTR L | yes | 92.3 | 1024014 | 0 | 0 | 0 | 3 / 25 | 8307 |
| max | all | lfo | SIN-220Hz-0.3V | yes | 71.1 | 192398 | 0 | 0 | 0 | 2 / 12 | 10612 |
| max | all | lfo | DI-GTR L | yes | 78.7 | 970090 | 0 | 0 | 0 | 2 / 23 | 13037 |
| max | all | jump | SIN-220Hz-0.3V | yes | 77.5 | 195640 | 0 | 0 | 0 | 2 / 26 | 10572 |
| max | all | jump | DI-GTR L | yes | 82.2 | 976641 | 0 | 0 | 0 | 2 / 25 | 13051 |
