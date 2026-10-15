# M5 automation: every knob over its full range at audio rate

- Date: 2026-10-15, commit `831e393 (dirty)`, engine `ampwav --block full` at 96000 Hz
- Modulation: 10 Hz triangle 0 → 1 → 0 (`lfo`) or 0 ↔ 1 jumps every 50 ms (`jump`) on the knob targets (smoothed by the engine, 20 ms); bases: default knobs, every knob at 1.0.
- SPEC §7 M5: no NaN / Inf, no Newton failures. Newton failures count first solves that did not converge in 12 iterations; the fallback (D-042) rescued or did not rescue them.

Runs: 56 · non-finite outputs: 0 · Newton failures (first solve): 0 · unrescued: 0

| Base | Knobs | Modulation | Signal | finite | peak [V] | solves | failures | rescued | unrescued | median / max it | ns/sample |
|---|---|---|---|---|---|---|---|---|---|---|---|
| default | preampVolume | lfo | SIN-220Hz-0.3V | yes | 33.0 | 192033 | 0 | 0 | 0 | 1 / 12 | 1989 |
| default | preampVolume | lfo | DI-GTR L | yes | 46.9 | 960098 | 0 | 0 | 0 | 1 / 17 | 2290 |
| default | preampVolume | jump | SIN-220Hz-0.3V | yes | 38.4 | 192033 | 0 | 0 | 0 | 1 / 25 | 1972 |
| default | preampVolume | jump | DI-GTR L | yes | 47.1 | 960162 | 0 | 0 | 0 | 1 / 20 | 2245 |
| default | treble | lfo | SIN-220Hz-0.3V | yes | 40.4 | 192033 | 0 | 0 | 0 | 1 / 22 | 2104 |
| default | treble | lfo | DI-GTR L | yes | 50.8 | 960112 | 0 | 0 | 0 | 1 / 19 | 2773 |
| default | treble | jump | SIN-220Hz-0.3V | yes | 32.5 | 192034 | 0 | 0 | 0 | 1 / 22 | 2115 |
| default | treble | jump | DI-GTR L | yes | 65.2 | 960214 | 0 | 0 | 0 | 1 / 19 | 2678 |
| default | middle | lfo | SIN-220Hz-0.3V | yes | 34.1 | 192033 | 0 | 0 | 0 | 1 / 25 | 2122 |
| default | middle | lfo | DI-GTR L | yes | 45.7 | 960112 | 0 | 0 | 0 | 1 / 18 | 2765 |
| default | middle | jump | SIN-220Hz-0.3V | yes | 38.7 | 192033 | 0 | 0 | 0 | 1 / 22 | 2135 |
| default | middle | jump | DI-GTR L | yes | 52.5 | 960141 | 0 | 0 | 0 | 1 / 19 | 2700 |
| default | bass | lfo | SIN-220Hz-0.3V | yes | 37.1 | 192033 | 0 | 0 | 0 | 1 / 22 | 1971 |
| default | bass | lfo | DI-GTR L | yes | 45.6 | 960126 | 0 | 0 | 0 | 1 / 19 | 2289 |
| default | bass | jump | SIN-220Hz-0.3V | yes | 42.2 | 192033 | 0 | 0 | 0 | 1 / 22 | 1971 |
| default | bass | jump | DI-GTR L | yes | 48.3 | 960122 | 0 | 0 | 0 | 1 / 17 | 2333 |
| default | master | lfo | SIN-220Hz-0.3V | yes | 69.3 | 192043 | 0 | 0 | 0 | 1 / 22 | 2387 |
| default | master | lfo | DI-GTR L | yes | 74.8 | 966574 | 0 | 0 | 0 | 1 / 23 | 2980 |
| default | master | jump | SIN-220Hz-0.3V | yes | 75.9 | 192428 | 0 | 0 | 0 | 1 / 22 | 2501 |
| default | master | jump | DI-GTR L | yes | 79.6 | 969939 | 0 | 0 | 0 | 1 / 24 | 2964 |
| default | presence | lfo | SIN-220Hz-0.3V | yes | 34.2 | 192033 | 0 | 0 | 0 | 1 / 22 | 1996 |
| default | presence | lfo | DI-GTR L | yes | 45.6 | 960120 | 0 | 0 | 0 | 1 / 18 | 2311 |
| default | presence | jump | SIN-220Hz-0.3V | yes | 49.3 | 192034 | 0 | 0 | 0 | 1 / 22 | 2026 |
| default | presence | jump | DI-GTR L | yes | 65.6 | 960187 | 0 | 0 | 0 | 1 / 18 | 2318 |
| default | all | lfo | SIN-220Hz-0.3V | yes | 70.9 | 192486 | 0 | 0 | 0 | 1 / 10 | 3135 |
| default | all | lfo | DI-GTR L | yes | 75.4 | 969884 | 0 | 0 | 0 | 1 / 25 | 5848 |
| default | all | jump | SIN-220Hz-0.3V | yes | 77.5 | 196473 | 0 | 0 | 0 | 1 / 25 | 3366 |
| default | all | jump | DI-GTR L | yes | 79.7 | 976884 | 0 | 0 | 0 | 1 / 23 | 6042 |
| max | preampVolume | lfo | SIN-220Hz-0.3V | yes | 75.8 | 199130 | 0 | 0 | 0 | 2 / 12 | 3276 |
| max | preampVolume | lfo | DI-GTR L | yes | 91.6 | 1028822 | 0 | 0 | 0 | 2 / 20 | 3792 |
| max | preampVolume | jump | SIN-220Hz-0.3V | yes | 82.9 | 204823 | 0 | 0 | 0 | 2 / 25 | 3528 |
| max | preampVolume | jump | DI-GTR L | yes | 91.6 | 1029192 | 0 | 0 | 0 | 2 / 20 | 3749 |
| max | treble | lfo | SIN-220Hz-0.3V | yes | 77.2 | 209907 | 0 | 0 | 0 | 1 / 25 | 3741 |
| max | treble | lfo | DI-GTR L | yes | 90.4 | 1023943 | 0 | 0 | 0 | 2 / 26 | 3922 |
| max | treble | jump | SIN-220Hz-0.3V | yes | 79.7 | 210856 | 0 | 0 | 0 | 1 / 25 | 3795 |
| max | treble | jump | DI-GTR L | yes | 89.9 | 1023805 | 0 | 0 | 0 | 2 / 25 | 3778 |
| max | middle | lfo | SIN-220Hz-0.3V | yes | 84.9 | 211625 | 0 | 0 | 0 | 2 / 25 | 3785 |
| max | middle | lfo | DI-GTR L | yes | 93.1 | 1021602 | 0 | 0 | 0 | 2 / 19 | 3806 |
| max | middle | jump | SIN-220Hz-0.3V | yes | 83.8 | 210931 | 0 | 0 | 0 | 2 / 25 | 3756 |
| max | middle | jump | DI-GTR L | yes | 92.6 | 1021405 | 0 | 0 | 0 | 2 / 19 | 3723 |
| max | bass | lfo | SIN-220Hz-0.3V | yes | 82.6 | 216639 | 0 | 0 | 0 | 2 / 25 | 4078 |
| max | bass | lfo | DI-GTR L | yes | 92.2 | 1031162 | 0 | 0 | 0 | 2 / 25 | 3538 |
| max | bass | jump | SIN-220Hz-0.3V | yes | 83.0 | 214264 | 0 | 0 | 0 | 2 / 25 | 3877 |
| max | bass | jump | DI-GTR L | yes | 92.6 | 1030219 | 0 | 0 | 0 | 2 / 20 | 3549 |
| max | master | lfo | SIN-220Hz-0.3V | yes | 73.5 | 199476 | 0 | 0 | 0 | 1 / 25 | 3164 |
| max | master | lfo | DI-GTR L | yes | 83.6 | 984506 | 0 | 0 | 0 | 1 / 26 | 3423 |
| max | master | jump | SIN-220Hz-0.3V | yes | 77.5 | 202077 | 0 | 0 | 0 | 1 / 25 | 3233 |
| max | master | jump | DI-GTR L | yes | 86.5 | 986627 | 0 | 0 | 0 | 1 / 25 | 3330 |
| max | presence | lfo | SIN-220Hz-0.3V | yes | 78.6 | 212027 | 0 | 0 | 0 | 2 / 25 | 3796 |
| max | presence | lfo | DI-GTR L | yes | 88.7 | 1015948 | 0 | 0 | 0 | 2 / 19 | 3380 |
| max | presence | jump | SIN-220Hz-0.3V | yes | 81.2 | 212607 | 0 | 0 | 0 | 2 / 25 | 3710 |
| max | presence | jump | DI-GTR L | yes | 91.9 | 1018271 | 0 | 0 | 0 | 2 / 19 | 3400 |
| max | all | lfo | SIN-220Hz-0.3V | yes | 70.9 | 192486 | 0 | 0 | 0 | 1 / 10 | 3163 |
| max | all | lfo | DI-GTR L | yes | 75.4 | 969884 | 0 | 0 | 0 | 1 / 25 | 5741 |
| max | all | jump | SIN-220Hz-0.3V | yes | 77.5 | 196473 | 0 | 0 | 0 | 1 / 25 | 3377 |
| max | all | jump | DI-GTR L | yes | 79.7 | 976884 | 0 | 0 | 0 | 1 / 23 | 5931 |
