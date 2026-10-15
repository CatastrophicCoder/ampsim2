# M5 automation: every knob over its full range at audio rate

- Date: 2026-10-15, commit `9778d0d (dirty)`, engine `ampwav --block full` at 96000 Hz
- Modulation: 10 Hz triangle 0 → 1 → 0 (`lfo`) or 0 ↔ 1 jumps every 50 ms (`jump`) on the knob targets (smoothed by the engine, 20 ms); bases: default knobs, every knob at 1.0.
- SPEC §7 M5: no NaN / Inf, no Newton failures. Newton failures count first solves that did not converge in 12 iterations; the fallback (D-042) rescued or did not rescue them.

Runs: 56 · non-finite outputs: 0 · Newton failures (first solve): 0 · unrescued: 0

| Base | Knobs | Modulation | Signal | finite | peak [V] | solves | failures | rescued | unrescued | median / max it | ns/sample |
|---|---|---|---|---|---|---|---|---|---|---|---|
| default | preampVolume | lfo | SIN-220Hz-0.3V | yes | 33.2 | 192033 | 0 | 0 | 0 | 1 / 19 | 4949 |
| default | preampVolume | lfo | DI-GTR L | yes | 47.2 | 960095 | 0 | 0 | 0 | 1 / 17 | 5208 |
| default | preampVolume | jump | SIN-220Hz-0.3V | yes | 38.4 | 192033 | 0 | 0 | 0 | 1 / 25 | 4931 |
| default | preampVolume | jump | DI-GTR L | yes | 45.9 | 960165 | 0 | 0 | 0 | 1 / 19 | 5209 |
| default | treble | lfo | SIN-220Hz-0.3V | yes | 40.3 | 192033 | 0 | 0 | 0 | 1 / 22 | 5034 |
| default | treble | lfo | DI-GTR L | yes | 50.7 | 960114 | 0 | 0 | 0 | 1 / 19 | 5662 |
| default | treble | jump | SIN-220Hz-0.3V | yes | 32.5 | 192034 | 0 | 0 | 0 | 1 / 22 | 4916 |
| default | treble | jump | DI-GTR L | yes | 65.6 | 960208 | 0 | 0 | 0 | 1 / 19 | 5538 |
| default | middle | lfo | SIN-220Hz-0.3V | yes | 34.1 | 192033 | 0 | 0 | 0 | 1 / 25 | 5234 |
| default | middle | lfo | DI-GTR L | yes | 45.9 | 960112 | 0 | 0 | 0 | 1 / 18 | 5641 |
| default | middle | jump | SIN-220Hz-0.3V | yes | 38.7 | 192033 | 0 | 0 | 0 | 1 / 22 | 4961 |
| default | middle | jump | DI-GTR L | yes | 52.5 | 960143 | 0 | 0 | 0 | 1 / 19 | 5613 |
| default | bass | lfo | SIN-220Hz-0.3V | yes | 37.1 | 192033 | 0 | 0 | 0 | 1 / 22 | 5019 |
| default | bass | lfo | DI-GTR L | yes | 45.5 | 960126 | 0 | 0 | 0 | 1 / 19 | 5335 |
| default | bass | jump | SIN-220Hz-0.3V | yes | 42.2 | 192033 | 0 | 0 | 0 | 1 / 22 | 4866 |
| default | bass | jump | DI-GTR L | yes | 48.3 | 960123 | 0 | 0 | 0 | 1 / 17 | 5219 |
| default | master | lfo | SIN-220Hz-0.3V | yes | 69.3 | 192040 | 0 | 0 | 0 | 1 / 22 | 5343 |
| default | master | lfo | DI-GTR L | yes | 75.1 | 966556 | 0 | 0 | 0 | 1 / 25 | 5877 |
| default | master | jump | SIN-220Hz-0.3V | yes | 75.9 | 192466 | 0 | 0 | 0 | 1 / 22 | 5298 |
| default | master | jump | DI-GTR L | yes | 81.2 | 969786 | 0 | 0 | 0 | 1 / 22 | 5834 |
| default | presence | lfo | SIN-220Hz-0.3V | yes | 34.2 | 192033 | 0 | 0 | 0 | 1 / 22 | 5685 |
| default | presence | lfo | DI-GTR L | yes | 45.6 | 960120 | 0 | 0 | 0 | 1 / 18 | 5973 |
| default | presence | jump | SIN-220Hz-0.3V | yes | 49.3 | 192034 | 0 | 0 | 0 | 1 / 22 | 5520 |
| default | presence | jump | DI-GTR L | yes | 65.6 | 960191 | 0 | 0 | 0 | 1 / 18 | 5924 |
| default | all | lfo | SIN-220Hz-0.3V | yes | 71.1 | 192398 | 0 | 0 | 0 | 1 / 11 | 9666 |
| default | all | lfo | DI-GTR L | yes | 78.7 | 970077 | 0 | 0 | 0 | 1 / 23 | 12244 |
| default | all | jump | SIN-220Hz-0.3V | yes | 77.5 | 195640 | 0 | 0 | 0 | 1 / 25 | 9566 |
| default | all | jump | DI-GTR L | yes | 82.2 | 976641 | 0 | 0 | 0 | 1 / 24 | 13015 |
| max | preampVolume | lfo | SIN-220Hz-0.3V | yes | 74.4 | 200376 | 0 | 0 | 0 | 2 / 13 | 6401 |
| max | preampVolume | lfo | DI-GTR L | yes | 92.5 | 1031436 | 0 | 0 | 0 | 2 / 22 | 6701 |
| max | preampVolume | jump | SIN-220Hz-0.3V | yes | 84.1 | 204462 | 0 | 0 | 0 | 2 / 25 | 6299 |
| max | preampVolume | jump | DI-GTR L | yes | 96.3 | 1030859 | 0 | 0 | 0 | 2 / 19 | 6615 |
| max | treble | lfo | SIN-220Hz-0.3V | yes | 76.6 | 211133 | 0 | 0 | 0 | 1 / 25 | 6722 |
| max | treble | lfo | DI-GTR L | yes | 88.7 | 1024109 | 0 | 0 | 0 | 2 / 26 | 6767 |
| max | treble | jump | SIN-220Hz-0.3V | yes | 81.2 | 209657 | 0 | 0 | 0 | 1 / 25 | 6481 |
| max | treble | jump | DI-GTR L | yes | 90.8 | 1020679 | 0 | 0 | 0 | 2 / 28 | 6618 |
| max | middle | lfo | SIN-220Hz-0.3V | yes | 85.5 | 211603 | 0 | 0 | 0 | 2 / 25 | 6806 |
| max | middle | lfo | DI-GTR L | yes | 95.5 | 1020606 | 0 | 0 | 0 | 2 / 20 | 6662 |
| max | middle | jump | SIN-220Hz-0.3V | yes | 87.2 | 212094 | 0 | 0 | 0 | 2 / 25 | 6548 |
| max | middle | jump | DI-GTR L | yes | 92.6 | 1020912 | 0 | 0 | 0 | 2 / 21 | 6572 |
| max | bass | lfo | SIN-220Hz-0.3V | yes | 79.7 | 215682 | 0 | 0 | 0 | 2 / 25 | 6933 |
| max | bass | lfo | DI-GTR L | yes | 90.1 | 1031382 | 0 | 0 | 0 | 2 / 20 | 6540 |
| max | bass | jump | SIN-220Hz-0.3V | yes | 81.5 | 214476 | 0 | 0 | 0 | 2 / 25 | 7001 |
| max | bass | jump | DI-GTR L | yes | 92.9 | 1028087 | 0 | 0 | 0 | 2 / 20 | 6419 |
| max | master | lfo | SIN-220Hz-0.3V | yes | 73.3 | 199385 | 0 | 0 | 0 | 1 / 25 | 6042 |
| max | master | lfo | DI-GTR L | yes | 81.6 | 984915 | 0 | 0 | 0 | 1 / 27 | 6291 |
| max | master | jump | SIN-220Hz-0.3V | yes | 77.5 | 201669 | 0 | 0 | 0 | 1 / 25 | 5960 |
| max | master | jump | DI-GTR L | yes | 86.4 | 985722 | 0 | 0 | 0 | 1 / 25 | 6136 |
| max | presence | lfo | SIN-220Hz-0.3V | yes | 82.6 | 211567 | 0 | 0 | 0 | 2 / 25 | 7378 |
| max | presence | lfo | DI-GTR L | yes | 89.3 | 1016866 | 0 | 0 | 0 | 2 / 19 | 6999 |
| max | presence | jump | SIN-220Hz-0.3V | yes | 79.4 | 212158 | 0 | 0 | 0 | 2 / 25 | 7131 |
| max | presence | jump | DI-GTR L | yes | 90.6 | 1017189 | 0 | 0 | 0 | 2 / 21 | 6987 |
| max | all | lfo | SIN-220Hz-0.3V | yes | 71.1 | 192398 | 0 | 0 | 0 | 1 / 11 | 9643 |
| max | all | lfo | DI-GTR L | yes | 78.7 | 970077 | 0 | 0 | 0 | 1 / 23 | 12208 |
| max | all | jump | SIN-220Hz-0.3V | yes | 77.5 | 195640 | 0 | 0 | 0 | 1 / 25 | 9560 |
| max | all | jump | DI-GTR L | yes | 82.2 | 976641 | 0 | 0 | 0 | 1 / 24 | 12879 |
