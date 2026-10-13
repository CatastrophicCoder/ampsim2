# M5 automation: every knob over its full range at audio rate

- Date: 2026-10-14, commit `18c866c (dirty)`, engine `ampwav --block full` at 768000 Hz
- Modulation: 10 Hz triangle 0 → 1 → 0 (`lfo`) or 0 ↔ 1 jumps every 50 ms (`jump`) on the knob targets (smoothed by the engine, 20 ms); bases: default knobs, every knob at 1.0.
- SPEC §7 M5: no NaN / Inf, no Newton failures. Newton failures count first solves that did not converge in 12 iterations; the fallback (D-042) rescued or did not rescue them.

Runs: 56 · non-finite outputs: 0 · Newton failures (first solve): 90 · unrescued: 0

| Base | Knobs | Modulation | Signal | finite | peak [V] | solves | failures | rescued | unrescued | median / max it | ns/sample |
|---|---|---|---|---|---|---|---|---|---|---|---|
| default | preampVolume | lfo | SIN-220Hz-0.3V | yes | 33.3 | 1536001 | 0 | 0 | 0 | 2 / 4 | 5898 |
| default | preampVolume | lfo | DI-GTR L | yes | 47.3 | 7680212 | 0 | 0 | 0 | 2 / 7 | 6429 |
| default | preampVolume | jump | SIN-220Hz-0.3V | yes | 38.4 | 1536001 | 0 | 0 | 0 | 1 / 5 | 5739 |
| default | preampVolume | jump | DI-GTR L | yes | 48.2 | 7680574 | 0 | 0 | 0 | 2 / 7 | 6368 |
| default | treble | lfo | SIN-220Hz-0.3V | yes | 40.3 | 1536001 | 0 | 0 | 0 | 2 / 5 | 14494 |
| default | treble | lfo | DI-GTR L | yes | 57.8 | 7680328 | 0 | 0 | 0 | 2 / 7 | 15216 |
| default | treble | jump | SIN-220Hz-0.3V | yes | 34.0 | 1536001 | 0 | 0 | 0 | 2 / 4 | 13939 |
| default | treble | jump | DI-GTR L | yes | 66.7 | 7680747 | 0 | 0 | 0 | 2 / 7 | 14939 |
| default | middle | lfo | SIN-220Hz-0.3V | yes | 34.1 | 1536001 | 0 | 0 | 0 | 2 / 5 | 6192 |
| default | middle | lfo | DI-GTR L | yes | 48.4 | 7680310 | 0 | 0 | 0 | 2 / 7 | 6881 |
| default | middle | jump | SIN-220Hz-0.3V | yes | 38.7 | 1536001 | 0 | 0 | 0 | 2 / 5 | 6045 |
| default | middle | jump | DI-GTR L | yes | 52.6 | 7680451 | 0 | 0 | 0 | 2 / 7 | 6795 |
| default | bass | lfo | SIN-220Hz-0.3V | yes | 37.1 | 1536001 | 0 | 0 | 0 | 1 / 5 | 14522 |
| default | bass | lfo | DI-GTR L | yes | 48.0 | 7680368 | 0 | 0 | 0 | 2 / 7 | 14737 |
| default | bass | jump | SIN-220Hz-0.3V | yes | 42.2 | 1536001 | 0 | 0 | 0 | 1 / 5 | 13830 |
| default | bass | jump | DI-GTR L | yes | 51.1 | 7680353 | 0 | 0 | 0 | 2 / 7 | 14608 |
| default | master | lfo | SIN-220Hz-0.3V | yes | 69.3 | 1536001 | 0 | 0 | 0 | 2 / 6 | 14220 |
| default | master | lfo | DI-GTR L | yes | 72.3 | 7693132 | 0 | 0 | 0 | 2 / 7 | 15232 |
| default | master | jump | SIN-220Hz-0.3V | yes | 72.8 | 1536532 | 0 | 0 | 0 | 2 / 5 | 13722 |
| default | master | jump | DI-GTR L | yes | 73.8 | 7697566 | 0 | 0 | 0 | 2 / 16 | 15161 |
| default | presence | lfo | SIN-220Hz-0.3V | yes | 34.2 | 1536001 | 0 | 0 | 0 | 2 / 5 | 10932 |
| default | presence | lfo | DI-GTR L | yes | 50.0 | 7680359 | 0 | 0 | 0 | 2 / 7 | 11458 |
| default | presence | jump | SIN-220Hz-0.3V | yes | 51.8 | 1536011 | 0 | 0 | 0 | 2 / 5 | 10412 |
| default | presence | jump | DI-GTR L | yes | 66.8 | 7680698 | 0 | 0 | 0 | 2 / 7 | 11399 |
| default | all | lfo | SIN-220Hz-0.3V | yes | 71.0 | 1536401 | 0 | 0 | 0 | 2 / 6 | 14762 |
| default | all | lfo | DI-GTR L | yes | 73.2 | 7695635 | 0 | 0 | 0 | 2 / 9 | 18172 |
| default | all | jump | SIN-220Hz-0.3V | yes | 73.9 | 1538727 | 0 | 0 | 0 | 2 / 7 | 14830 |
| default | all | jump | DI-GTR L | yes | 75.3 | 7708463 | 1 | 1 | 0 | 2 / 16 | 17969 |
| max | preampVolume | lfo | SIN-220Hz-0.3V | yes | 69.6 | 1540467 | 0 | 0 | 0 | 2 / 6 | 6722 |
| max | preampVolume | lfo | DI-GTR L | yes | 74.1 | 7744525 | 3 | 3 | 0 | 2 / 16 | 6662 |
| max | preampVolume | jump | SIN-220Hz-0.3V | yes | 73.3 | 1540396 | 0 | 0 | 0 | 2 / 7 | 5777 |
| max | preampVolume | jump | DI-GTR L | yes | 74.6 | 7746565 | 6 | 6 | 0 | 2 / 16 | 6418 |
| max | treble | lfo | SIN-220Hz-0.3V | yes | 72.8 | 1543512 | 0 | 0 | 0 | 2 / 7 | 14268 |
| max | treble | lfo | DI-GTR L | yes | 72.4 | 7751914 | 2 | 2 | 0 | 2 / 16 | 15177 |
| max | treble | jump | SIN-220Hz-0.3V | yes | 72.8 | 1542599 | 0 | 0 | 0 | 2 / 7 | 13988 |
| max | treble | jump | DI-GTR L | yes | 72.4 | 7748749 | 4 | 4 | 0 | 2 / 16 | 14982 |
| max | middle | lfo | SIN-220Hz-0.3V | yes | 72.8 | 1548251 | 0 | 0 | 0 | 2 / 8 | 6340 |
| max | middle | lfo | DI-GTR L | yes | 72.5 | 7755935 | 6 | 6 | 0 | 2 / 16 | 6967 |
| max | middle | jump | SIN-220Hz-0.3V | yes | 72.8 | 1548553 | 0 | 0 | 0 | 2 / 8 | 6213 |
| max | middle | jump | DI-GTR L | yes | 72.5 | 7757381 | 8 | 8 | 0 | 2 / 16 | 6979 |
| max | bass | lfo | SIN-220Hz-0.3V | yes | 72.8 | 1542853 | 0 | 0 | 0 | 2 / 7 | 14395 |
| max | bass | lfo | DI-GTR L | yes | 72.5 | 7780312 | 16 | 16 | 0 | 2 / 16 | 14741 |
| max | bass | jump | SIN-220Hz-0.3V | yes | 72.8 | 1542972 | 0 | 0 | 0 | 2 / 7 | 13841 |
| max | bass | jump | DI-GTR L | yes | 72.4 | 7776475 | 14 | 14 | 0 | 2 / 16 | 14482 |
| max | master | lfo | SIN-220Hz-0.3V | yes | 71.7 | 1541292 | 0 | 0 | 0 | 2 / 6 | 14073 |
| max | master | lfo | DI-GTR L | yes | 72.7 | 7714692 | 1 | 1 | 0 | 2 / 16 | 14544 |
| max | master | jump | SIN-220Hz-0.3V | yes | 74.8 | 1540628 | 0 | 0 | 0 | 2 / 7 | 13598 |
| max | master | jump | DI-GTR L | yes | 74.8 | 7719871 | 3 | 3 | 0 | 2 / 16 | 14316 |
| max | presence | lfo | SIN-220Hz-0.3V | yes | 72.8 | 1543032 | 0 | 0 | 0 | 2 / 7 | 11023 |
| max | presence | lfo | DI-GTR L | yes | 72.5 | 7758603 | 12 | 12 | 0 | 2 / 16 | 11109 |
| max | presence | jump | SIN-220Hz-0.3V | yes | 72.8 | 1543018 | 0 | 0 | 0 | 2 / 7 | 10601 |
| max | presence | jump | DI-GTR L | yes | 72.5 | 7760097 | 13 | 13 | 0 | 2 / 16 | 10895 |
| max | all | lfo | SIN-220Hz-0.3V | yes | 71.0 | 1536401 | 0 | 0 | 0 | 2 / 6 | 14686 |
| max | all | lfo | DI-GTR L | yes | 73.2 | 7695635 | 0 | 0 | 0 | 2 / 9 | 16633 |
| max | all | jump | SIN-220Hz-0.3V | yes | 73.9 | 1538727 | 0 | 0 | 0 | 2 / 7 | 14075 |
| max | all | jump | DI-GTR L | yes | 75.3 | 7708463 | 1 | 1 | 0 | 2 / 16 | 16410 |
