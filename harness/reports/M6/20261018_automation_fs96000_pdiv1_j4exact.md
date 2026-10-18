# M5 automation: every knob over its full range at audio rate

- Date: 2026-10-18, commit `b193fc7 (dirty)`, engine `ampwav --block full` at 96000 Hz
- Modulation: 10 Hz triangle 0 → 1 → 0 (`lfo`) or 0 ↔ 1 jumps every 50 ms (`jump`) on the knob targets (smoothed by the engine, 20 ms); bases: default knobs, every knob at 1.0.
- SPEC §7 M5: no NaN / Inf, no Newton failures. Newton failures count first solves that did not converge in 12 iterations; the fallback (D-042) rescued or did not rescue them.

Runs: 56 · non-finite outputs: 0 · Newton failures (first solve): 0 · unrescued: 0

| Base | Knobs | Modulation | Signal | finite | peak [V] | solves | failures | rescued | unrescued | median / max it | ns/sample |
|---|---|---|---|---|---|---|---|---|---|---|---|
| default | preampVolume | lfo | SIN-220Hz-0.3V | yes | 34.7 | 288000 | 0 | 0 | 0 | 1 / 7 | 3479 |
| default | preampVolume | lfo | DI-GTR L | yes | 55.7 | 1440103 | 0 | 0 | 0 | 1 / 15 | 3694 |
| default | preampVolume | jump | SIN-220Hz-0.3V | yes | 40.3 | 288000 | 0 | 0 | 0 | 1 / 22 | 3370 |
| default | preampVolume | jump | DI-GTR L | yes | 59.8 | 1440215 | 0 | 0 | 0 | 1 / 21 | 4136 |
| default | treble | lfo | SIN-220Hz-0.3V | yes | 41.4 | 288000 | 0 | 0 | 0 | 1 / 22 | 3612 |
| default | treble | lfo | DI-GTR L | yes | 64.3 | 1440164 | 0 | 0 | 0 | 1 / 16 | 4480 |
| default | treble | jump | SIN-220Hz-0.3V | yes | 39.1 | 288000 | 0 | 0 | 0 | 1 / 22 | 3801 |
| default | treble | jump | DI-GTR L | yes | 76.6 | 1440435 | 0 | 0 | 0 | 1 / 16 | 4301 |
| default | middle | lfo | SIN-220Hz-0.3V | yes | 35.1 | 288000 | 0 | 0 | 0 | 1 / 22 | 3647 |
| default | middle | lfo | DI-GTR L | yes | 56.3 | 1440137 | 0 | 0 | 0 | 1 / 22 | 4490 |
| default | middle | jump | SIN-220Hz-0.3V | yes | 39.9 | 288000 | 0 | 0 | 0 | 1 / 22 | 3760 |
| default | middle | jump | DI-GTR L | yes | 65.0 | 1440220 | 0 | 0 | 0 | 1 / 17 | 4382 |
| default | bass | lfo | SIN-220Hz-0.3V | yes | 39.3 | 288000 | 0 | 0 | 0 | 1 / 22 | 3425 |
| default | bass | lfo | DI-GTR L | yes | 59.1 | 1440151 | 0 | 0 | 0 | 1 / 17 | 4235 |
| default | bass | jump | SIN-220Hz-0.3V | yes | 45.3 | 288000 | 0 | 0 | 0 | 1 / 22 | 3165 |
| default | bass | jump | DI-GTR L | yes | 57.5 | 1440155 | 0 | 0 | 0 | 1 / 19 | 3580 |
| default | master | lfo | SIN-220Hz-0.3V | yes | 75.8 | 288199 | 0 | 0 | 0 | 1 / 22 | 4045 |
| default | master | lfo | DI-GTR L | yes | 129.1 | 1448472 | 0 | 0 | 0 | 1 / 24 | 4804 |
| default | master | jump | SIN-220Hz-0.3V | yes | 82.8 | 288897 | 0 | 0 | 0 | 1 / 22 | 3908 |
| default | master | jump | DI-GTR L | yes | 152.9 | 1452539 | 0 | 0 | 0 | 1 / 24 | 4793 |
| default | presence | lfo | SIN-220Hz-0.3V | yes | 35.7 | 288000 | 0 | 0 | 0 | 1 / 22 | 3359 |
| default | presence | lfo | DI-GTR L | yes | 58.0 | 1440156 | 0 | 0 | 0 | 1 / 16 | 4280 |
| default | presence | jump | SIN-220Hz-0.3V | yes | 66.3 | 288002 | 0 | 0 | 0 | 1 / 22 | 3257 |
| default | presence | jump | DI-GTR L | yes | 75.2 | 1440443 | 0 | 0 | 0 | 1 / 16 | 4218 |
| default | all | lfo | SIN-220Hz-0.3V | yes | 77.1 | 288685 | 0 | 0 | 0 | 1 / 8 | 4671 |
| default | all | lfo | DI-GTR L | yes | 154.1 | 1451253 | 0 | 0 | 0 | 1 / 26 | 8436 |
| default | all | jump | SIN-220Hz-0.3V | yes | 138.3 | 290424 | 0 | 0 | 0 | 1 / 22 | 4939 |
| default | all | jump | DI-GTR L | yes | 171.9 | 1459694 | 0 | 0 | 0 | 1 / 29 | 8738 |
| max | preampVolume | lfo | SIN-220Hz-0.3V | yes | 79.5 | 290730 | 0 | 0 | 0 | 1 / 22 | 4446 |
| max | preampVolume | lfo | DI-GTR L | yes | 205.9 | 1501526 | 0 | 0 | 0 | 1 / 21 | 5586 |
| max | preampVolume | jump | SIN-220Hz-0.3V | yes | 138.3 | 295128 | 0 | 0 | 0 | 1 / 22 | 5108 |
| max | preampVolume | jump | DI-GTR L | yes | 215.9 | 1496665 | 0 | 0 | 0 | 1 / 21 | 6021 |
| max | treble | lfo | SIN-220Hz-0.3V | yes | 124.9 | 303592 | 0 | 0 | 0 | 2 / 25 | 6752 |
| max | treble | lfo | DI-GTR L | yes | 210.1 | 1512029 | 0 | 0 | 0 | 1 / 28 | 6960 |
| max | treble | jump | SIN-220Hz-0.3V | yes | 138.3 | 303523 | 0 | 0 | 0 | 2 / 22 | 5918 |
| max | treble | jump | DI-GTR L | yes | 215.0 | 1510091 | 0 | 0 | 0 | 1 / 28 | 6752 |
| max | middle | lfo | SIN-220Hz-0.3V | yes | 183.1 | 309517 | 0 | 0 | 0 | 2 / 22 | 6802 |
| max | middle | lfo | DI-GTR L | yes | 215.1 | 1509466 | 0 | 0 | 0 | 2 / 23 | 6536 |
| max | middle | jump | SIN-220Hz-0.3V | yes | 138.3 | 306251 | 0 | 0 | 0 | 2 / 22 | 6458 |
| max | middle | jump | DI-GTR L | yes | 218.1 | 1511465 | 0 | 0 | 0 | 2 / 21 | 6446 |
| max | bass | lfo | SIN-220Hz-0.3V | yes | 143.8 | 312438 | 0 | 0 | 0 | 2 / 22 | 7032 |
| max | bass | lfo | DI-GTR L | yes | 215.6 | 1529947 | 0 | 0 | 0 | 2 / 21 | 6601 |
| max | bass | jump | SIN-220Hz-0.3V | yes | 150.6 | 311582 | 0 | 0 | 0 | 2 / 22 | 6612 |
| max | bass | jump | DI-GTR L | yes | 214.0 | 1527546 | 0 | 0 | 0 | 2 / 22 | 6301 |
| max | master | lfo | SIN-220Hz-0.3V | yes | 75.0 | 292222 | 0 | 0 | 0 | 1 / 22 | 4792 |
| max | master | lfo | DI-GTR L | yes | 170.7 | 1471723 | 0 | 0 | 0 | 1 / 25 | 5441 |
| max | master | jump | SIN-220Hz-0.3V | yes | 138.3 | 294841 | 0 | 0 | 0 | 1 / 22 | 4625 |
| max | master | jump | DI-GTR L | yes | 181.6 | 1472693 | 0 | 0 | 0 | 1 / 26 | 5284 |
| max | presence | lfo | SIN-220Hz-0.3V | yes | 107.9 | 303464 | 0 | 0 | 0 | 2 / 22 | 6324 |
| max | presence | lfo | DI-GTR L | yes | 211.7 | 1501643 | 0 | 0 | 0 | 2 / 21 | 5598 |
| max | presence | jump | SIN-220Hz-0.3V | yes | 138.3 | 304322 | 0 | 0 | 0 | 2 / 22 | 6326 |
| max | presence | jump | DI-GTR L | yes | 212.4 | 1503289 | 0 | 0 | 0 | 2 / 19 | 5033 |
| max | all | lfo | SIN-220Hz-0.3V | yes | 77.1 | 288685 | 0 | 0 | 0 | 1 / 8 | 4234 |
| max | all | lfo | DI-GTR L | yes | 154.1 | 1451253 | 0 | 0 | 0 | 1 / 26 | 6967 |
| max | all | jump | SIN-220Hz-0.3V | yes | 138.3 | 290424 | 0 | 0 | 0 | 1 / 22 | 5216 |
| max | all | jump | DI-GTR L | yes | 171.9 | 1459694 | 0 | 0 | 0 | 1 / 29 | 6952 |
