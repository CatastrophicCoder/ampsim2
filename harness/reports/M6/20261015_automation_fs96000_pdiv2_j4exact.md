# M5 automation: every knob over its full range at audio rate

- Date: 2026-10-15, commit `9f01825 (dirty)`, engine `ampwav --block full` at 96000 Hz
- Modulation: 10 Hz triangle 0 → 1 → 0 (`lfo`) or 0 ↔ 1 jumps every 50 ms (`jump`) on the knob targets (smoothed by the engine, 20 ms); bases: default knobs, every knob at 1.0.
- SPEC §7 M5: no NaN / Inf, no Newton failures. Newton failures count first solves that did not converge in 12 iterations; the fallback (D-042) rescued or did not rescue them.

Runs: 56 · non-finite outputs: 0 · Newton failures (first solve): 7652 · unrescued: 0

| Base | Knobs | Modulation | Signal | finite | peak [V] | solves | failures | rescued | unrescued | median / max it | ns/sample |
|---|---|---|---|---|---|---|---|---|---|---|---|
| default | preampVolume | lfo | SIN-220Hz-0.3V | yes | 33.2 | 192033 | 0 | 0 | 0 | 1 / 19 | 5048 |
| default | preampVolume | lfo | DI-GTR L | yes | 50.7 | 960033 | 0 | 0 | 0 | 1 / 17 | 5333 |
| default | preampVolume | jump | SIN-220Hz-0.3V | yes | 38.4 | 192033 | 0 | 0 | 0 | 1 / 25 | 4922 |
| default | preampVolume | jump | DI-GTR L | yes | 51.6 | 960033 | 0 | 0 | 0 | 1 / 19 | 5287 |
| default | treble | lfo | SIN-220Hz-0.3V | yes | 40.3 | 192033 | 0 | 0 | 0 | 1 / 22 | 5139 |
| default | treble | lfo | DI-GTR L | yes | 61.2 | 960033 | 0 | 0 | 0 | 1 / 19 | 5736 |
| default | treble | jump | SIN-220Hz-0.3V | yes | 34.3 | 192033 | 0 | 0 | 0 | 1 / 22 | 5021 |
| default | treble | jump | DI-GTR L | yes | 73.0 | 960033 | 0 | 0 | 0 | 1 / 19 | 5675 |
| default | middle | lfo | SIN-220Hz-0.3V | yes | 34.1 | 192033 | 0 | 0 | 0 | 1 / 25 | 5182 |
| default | middle | lfo | DI-GTR L | yes | 52.2 | 960033 | 0 | 0 | 0 | 1 / 18 | 5751 |
| default | middle | jump | SIN-220Hz-0.3V | yes | 38.7 | 192033 | 0 | 0 | 0 | 1 / 22 | 5041 |
| default | middle | jump | DI-GTR L | yes | 60.2 | 960033 | 0 | 0 | 0 | 1 / 19 | 5681 |
| default | bass | lfo | SIN-220Hz-0.3V | yes | 37.1 | 192033 | 0 | 0 | 0 | 1 / 22 | 5099 |
| default | bass | lfo | DI-GTR L | yes | 54.3 | 960033 | 0 | 0 | 0 | 1 / 19 | 5318 |
| default | bass | jump | SIN-220Hz-0.3V | yes | 42.2 | 192033 | 0 | 0 | 0 | 1 / 22 | 4886 |
| default | bass | jump | DI-GTR L | yes | 55.0 | 960033 | 0 | 0 | 0 | 1 / 17 | 5312 |
| default | master | lfo | SIN-220Hz-0.3V | yes | 69.3 | 192033 | 0 | 0 | 0 | 1 / 22 | 5434 |
| default | master | lfo | DI-GTR L | yes | 82.3 | 960033 | 0 | 0 | 0 | 1 / 24 | 5916 |
| default | master | jump | SIN-220Hz-0.3V | yes | 76.6 | 192033 | 0 | 0 | 0 | 1 / 22 | 5387 |
| default | master | jump | DI-GTR L | yes | 84.8 | 960033 | 0 | 0 | 0 | 1 / 25 | 5870 |
| default | presence | lfo | SIN-220Hz-0.3V | yes | 34.2 | 192033 | 0 | 0 | 0 | 1 / 22 | 5836 |
| default | presence | lfo | DI-GTR L | yes | 54.0 | 960033 | 0 | 0 | 0 | 1 / 18 | 6145 |
| default | presence | jump | SIN-220Hz-0.3V | yes | 53.2 | 192033 | 0 | 0 | 0 | 1 / 22 | 5656 |
| default | presence | jump | DI-GTR L | yes | 73.2 | 960033 | 0 | 0 | 0 | 1 / 18 | 6105 |
| default | all | lfo | SIN-220Hz-0.3V | yes | 71.0 | 192033 | 0 | 0 | 0 | 1 / 11 | 9975 |
| default | all | lfo | DI-GTR L | yes | 79.3 | 960033 | 0 | 0 | 0 | 1 / 25 | 12490 |
| default | all | jump | SIN-220Hz-0.3V | yes | 80.3 | 192033 | 0 | 0 | 0 | 1 / 25 | 9774 |
| default | all | jump | DI-GTR L | yes | 87.0 | 960033 | 0 | 0 | 0 | 1 / 23 | 12559 |
| max | preampVolume | lfo | SIN-220Hz-0.3V | yes | 87.7 | 192033 | 1191 | 1191 | 0 | 5 / 32 | 10447 |
| max | preampVolume | lfo | DI-GTR L | yes | 96.3 | 960033 | 1670 | 1670 | 0 | 5 / 32 | 10396 |
| max | preampVolume | jump | SIN-220Hz-0.3V | yes | 91.5 | 192033 | 768 | 768 | 0 | 5 / 32 | 9979 |
| max | preampVolume | jump | DI-GTR L | yes | 92.6 | 960033 | 1732 | 1732 | 0 | 5 / 32 | 10305 |
| max | treble | lfo | SIN-220Hz-0.3V | yes | 80.3 | 192033 | 0 | 0 | 0 | 1 / 25 | 6212 |
| max | treble | lfo | DI-GTR L | yes | 90.3 | 960033 | 0 | 0 | 0 | 2 / 27 | 6449 |
| max | treble | jump | SIN-220Hz-0.3V | yes | 80.3 | 192033 | 0 | 0 | 0 | 2 / 25 | 6114 |
| max | treble | jump | DI-GTR L | yes | 91.6 | 960033 | 0 | 0 | 0 | 2 / 26 | 6354 |
| max | middle | lfo | SIN-220Hz-0.3V | yes | 80.9 | 192033 | 0 | 0 | 0 | 2 / 25 | 6105 |
| max | middle | lfo | DI-GTR L | yes | 94.9 | 960033 | 513 | 513 | 0 | 2 / 32 | 7530 |
| max | middle | jump | SIN-220Hz-0.3V | yes | 80.3 | 192033 | 0 | 0 | 0 | 2 / 25 | 6001 |
| max | middle | jump | DI-GTR L | yes | 97.3 | 960033 | 714 | 714 | 0 | 2 / 32 | 7775 |
| max | bass | lfo | SIN-220Hz-0.3V | yes | 80.3 | 192033 | 0 | 0 | 0 | 2 / 25 | 6193 |
| max | bass | lfo | DI-GTR L | yes | 98.3 | 960033 | 329 | 329 | 0 | 2 / 32 | 7525 |
| max | bass | jump | SIN-220Hz-0.3V | yes | 80.3 | 192033 | 0 | 0 | 0 | 2 / 25 | 6060 |
| max | bass | jump | DI-GTR L | yes | 99.0 | 960033 | 701 | 701 | 0 | 3 / 32 | 8289 |
| max | master | lfo | SIN-220Hz-0.3V | yes | 74.4 | 192033 | 0 | 0 | 0 | 1 / 25 | 5935 |
| max | master | lfo | DI-GTR L | yes | 79.1 | 960033 | 0 | 0 | 0 | 1 / 25 | 6205 |
| max | master | jump | SIN-220Hz-0.3V | yes | 80.3 | 192033 | 0 | 0 | 0 | 1 / 25 | 5744 |
| max | master | jump | DI-GTR L | yes | 83.9 | 960033 | 0 | 0 | 0 | 1 / 26 | 6134 |
| max | presence | lfo | SIN-220Hz-0.3V | yes | 80.3 | 192033 | 0 | 0 | 0 | 2 / 25 | 6861 |
| max | presence | lfo | DI-GTR L | yes | 89.7 | 960033 | 14 | 14 | 0 | 2 / 32 | 7447 |
| max | presence | jump | SIN-220Hz-0.3V | yes | 80.3 | 192033 | 0 | 0 | 0 | 2 / 25 | 6691 |
| max | presence | jump | DI-GTR L | yes | 93.9 | 960033 | 20 | 20 | 0 | 2 / 32 | 7425 |
| max | all | lfo | SIN-220Hz-0.3V | yes | 71.0 | 192033 | 0 | 0 | 0 | 1 / 11 | 9993 |
| max | all | lfo | DI-GTR L | yes | 79.3 | 960033 | 0 | 0 | 0 | 1 / 25 | 12467 |
| max | all | jump | SIN-220Hz-0.3V | yes | 80.3 | 192033 | 0 | 0 | 0 | 1 / 25 | 9738 |
| max | all | jump | DI-GTR L | yes | 87.0 | 960033 | 0 | 0 | 0 | 1 / 23 | 12507 |
