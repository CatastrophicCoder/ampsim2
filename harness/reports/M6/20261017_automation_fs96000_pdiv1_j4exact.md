# M5 automation: every knob over its full range at audio rate

- Date: 2026-10-17, commit `eeea956 (dirty)`, engine `ampwav --block full` at 96000 Hz
- Modulation: 10 Hz triangle 0 → 1 → 0 (`lfo`) or 0 ↔ 1 jumps every 50 ms (`jump`) on the knob targets (smoothed by the engine, 20 ms); bases: default knobs, every knob at 1.0.
- SPEC §7 M5: no NaN / Inf, no Newton failures. Newton failures count first solves that did not converge in 12 iterations; the fallback (D-042) rescued or did not rescue them.

Runs: 56 · non-finite outputs: 0 · Newton failures (first solve): 20 · unrescued: 0

| Base | Knobs | Modulation | Signal | finite | peak [V] | solves | failures | rescued | unrescued | median / max it | ns/sample |
|---|---|---|---|---|---|---|---|---|---|---|---|
| default | preampVolume | lfo | SIN-220Hz-0.3V | yes | 34.7 | 288000 | 0 | 0 | 0 | 1 / 7 | 2703 |
| default | preampVolume | lfo | DI-GTR L | yes | 55.9 | 1440105 | 0 | 0 | 0 | 1 / 18 | 3082 |
| default | preampVolume | jump | SIN-220Hz-0.3V | yes | 40.3 | 288000 | 0 | 0 | 0 | 1 / 25 | 2658 |
| default | preampVolume | jump | DI-GTR L | yes | 60.0 | 1440221 | 0 | 0 | 0 | 1 / 20 | 3013 |
| default | treble | lfo | SIN-220Hz-0.3V | yes | 41.4 | 288000 | 0 | 0 | 0 | 1 / 22 | 2811 |
| default | treble | lfo | DI-GTR L | yes | 64.5 | 1440166 | 0 | 0 | 0 | 1 / 17 | 3422 |
| default | treble | jump | SIN-220Hz-0.3V | yes | 39.8 | 288000 | 0 | 0 | 0 | 1 / 22 | 2811 |
| default | treble | jump | DI-GTR L | yes | 75.6 | 1440426 | 0 | 0 | 0 | 1 / 20 | 3575 |
| default | middle | lfo | SIN-220Hz-0.3V | yes | 35.1 | 288000 | 0 | 0 | 0 | 1 / 25 | 2836 |
| default | middle | lfo | DI-GTR L | yes | 54.9 | 1440145 | 0 | 0 | 0 | 1 / 18 | 3420 |
| default | middle | jump | SIN-220Hz-0.3V | yes | 39.9 | 288000 | 0 | 0 | 0 | 1 / 22 | 2831 |
| default | middle | jump | DI-GTR L | yes | 63.8 | 1440215 | 0 | 0 | 0 | 1 / 18 | 3571 |
| default | bass | lfo | SIN-220Hz-0.3V | yes | 39.3 | 288000 | 0 | 0 | 0 | 1 / 22 | 2693 |
| default | bass | lfo | DI-GTR L | yes | 57.7 | 1440152 | 0 | 0 | 0 | 1 / 20 | 3032 |
| default | bass | jump | SIN-220Hz-0.3V | yes | 45.2 | 288000 | 0 | 0 | 0 | 1 / 22 | 2684 |
| default | bass | jump | DI-GTR L | yes | 57.1 | 1440155 | 0 | 0 | 0 | 1 / 18 | 3047 |
| default | master | lfo | SIN-220Hz-0.3V | yes | 75.8 | 288215 | 0 | 0 | 0 | 1 / 22 | 3271 |
| default | master | lfo | DI-GTR L | yes | 137.4 | 1448413 | 0 | 0 | 0 | 1 / 19 | 3772 |
| default | master | jump | SIN-220Hz-0.3V | yes | 81.9 | 288929 | 0 | 0 | 0 | 1 / 22 | 3307 |
| default | master | jump | DI-GTR L | yes | 142.9 | 1452621 | 0 | 0 | 0 | 1 / 18 | 3968 |
| default | presence | lfo | SIN-220Hz-0.3V | yes | 35.7 | 288000 | 0 | 0 | 0 | 1 / 22 | 2842 |
| default | presence | lfo | DI-GTR L | yes | 58.1 | 1440162 | 0 | 0 | 0 | 1 / 19 | 3184 |
| default | presence | jump | SIN-220Hz-0.3V | yes | 67.9 | 288002 | 0 | 0 | 0 | 1 / 22 | 2852 |
| default | presence | jump | DI-GTR L | yes | 75.0 | 1440441 | 0 | 0 | 0 | 1 / 17 | 3288 |
| default | all | lfo | SIN-220Hz-0.3V | yes | 77.1 | 288679 | 0 | 0 | 0 | 1 / 8 | 4045 |
| default | all | lfo | DI-GTR L | yes | 161.9 | 1451205 | 0 | 0 | 0 | 1 / 19 | 6706 |
| default | all | jump | SIN-220Hz-0.3V | yes | 134.0 | 290865 | 0 | 0 | 0 | 1 / 25 | 4316 |
| default | all | jump | DI-GTR L | yes | 167.6 | 1459869 | 0 | 0 | 0 | 1 / 20 | 6882 |
| max | preampVolume | lfo | SIN-220Hz-0.3V | yes | 79.3 | 290727 | 0 | 0 | 0 | 1 / 21 | 3709 |
| max | preampVolume | lfo | DI-GTR L | yes | 206.3 | 1501570 | 0 | 0 | 0 | 2 / 20 | 4676 |
| max | preampVolume | jump | SIN-220Hz-0.3V | yes | 134.0 | 294933 | 0 | 0 | 0 | 1 / 25 | 4181 |
| max | preampVolume | jump | DI-GTR L | yes | 212.3 | 1496506 | 0 | 0 | 0 | 1 / 18 | 4497 |
| max | treble | lfo | SIN-220Hz-0.3V | yes | 129.5 | 303397 | 0 | 0 | 0 | 2 / 25 | 5223 |
| max | treble | lfo | DI-GTR L | yes | 208.9 | 1512086 | 0 | 0 | 0 | 1 / 22 | 5088 |
| max | treble | jump | SIN-220Hz-0.3V | yes | 134.0 | 303935 | 0 | 0 | 0 | 2 / 25 | 5230 |
| max | treble | jump | DI-GTR L | yes | 206.1 | 1509676 | 0 | 0 | 0 | 1 / 23 | 5193 |
| max | middle | lfo | SIN-220Hz-0.3V | yes | 181.9 | 309969 | 0 | 0 | 0 | 2 / 25 | 5891 |
| max | middle | lfo | DI-GTR L | yes | 212.9 | 1510320 | 0 | 0 | 0 | 2 / 19 | 5022 |
| max | middle | jump | SIN-220Hz-0.3V | yes | 143.6 | 307088 | 0 | 0 | 0 | 2 / 25 | 5553 |
| max | middle | jump | DI-GTR L | yes | 215.6 | 1509217 | 0 | 0 | 0 | 2 / 17 | 5174 |
| max | bass | lfo | SIN-220Hz-0.3V | yes | 149.2 | 312718 | 0 | 0 | 0 | 2 / 25 | 5914 |
| max | bass | lfo | DI-GTR L | yes | 213.1 | 1529438 | 0 | 0 | 0 | 2 / 18 | 5055 |
| max | bass | jump | SIN-220Hz-0.3V | yes | 148.4 | 312031 | 0 | 0 | 0 | 2 / 25 | 5799 |
| max | bass | jump | DI-GTR L | yes | 214.9 | 1527473 | 0 | 0 | 0 | 2 / 22 | 5081 |
| max | master | lfo | SIN-220Hz-0.3V | yes | 75.0 | 292542 | 0 | 0 | 0 | 1 / 25 | 3936 |
| max | master | lfo | DI-GTR L | yes | 196.6 | 1471596 | 0 | 0 | 0 | 1 / 21 | 4322 |
| max | master | jump | SIN-220Hz-0.3V | yes | 134.0 | 295718 | 0 | 0 | 0 | 1 / 25 | 4173 |
| max | master | jump | DI-GTR L | yes | 181.4 | 1472788 | 0 | 0 | 0 | 1 / 23 | 4480 |
| max | presence | lfo | SIN-220Hz-0.3V | yes | 104.1 | 303844 | 2 | 2 | 0 | 2 / 32 | 5353 |
| max | presence | lfo | DI-GTR L | yes | 201.8 | 1500881 | 11 | 11 | 0 | 2 / 32 | 4688 |
| max | presence | jump | SIN-220Hz-0.3V | yes | 134.0 | 304954 | 2 | 2 | 0 | 2 / 32 | 5398 |
| max | presence | jump | DI-GTR L | yes | 202.5 | 1501820 | 5 | 5 | 0 | 2 / 32 | 4715 |
| max | all | lfo | SIN-220Hz-0.3V | yes | 77.1 | 288679 | 0 | 0 | 0 | 1 / 8 | 4096 |
| max | all | lfo | DI-GTR L | yes | 161.9 | 1451205 | 0 | 0 | 0 | 1 / 19 | 6661 |
| max | all | jump | SIN-220Hz-0.3V | yes | 134.0 | 290865 | 0 | 0 | 0 | 1 / 25 | 4319 |
| max | all | jump | DI-GTR L | yes | 167.6 | 1459869 | 0 | 0 | 0 | 1 / 20 | 6818 |
