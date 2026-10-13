# M5 automation: every knob over its full range at audio rate

- Date: 2026-10-13, commit `794e143 (dirty)`, engine `ampwav --block full` at 768000 Hz
- Modulation: 10 Hz triangle 0 → 1 → 0 (`lfo`) or 0 ↔ 1 jumps every 50 ms (`jump`) on the knob targets (smoothed by the engine, 20 ms); bases: default knobs, every knob at 1.0.
- SPEC §7 M5: no NaN / Inf, no Newton failures. Newton failures count first solves that did not converge in 12 iterations; the fallback (D-042) rescued or did not rescue them.

Runs: 56 · non-finite outputs: 0 · Newton failures (first solve): 6367 · unrescued: 0

| Base | Knobs | Modulation | Signal | finite | peak [V] | solves | failures | rescued | unrescued | median / max it | ns/sample |
|---|---|---|---|---|---|---|---|---|---|---|---|
| default | preampVolume | lfo | SIN-220Hz-0.3V | yes | 33.3 | 1536001 | 0 | 0 | 0 | 2 / 4 | 6417 |
| default | preampVolume | lfo | DI-GTR L | yes | 47.3 | 7680212 | 0 | 0 | 0 | 2 / 7 | 7023 |
| default | preampVolume | jump | SIN-220Hz-0.3V | yes | 38.4 | 1536001 | 0 | 0 | 0 | 1 / 5 | 6243 |
| default | preampVolume | jump | DI-GTR L | yes | 48.2 | 7680574 | 0 | 0 | 0 | 2 / 7 | 6988 |
| default | treble | lfo | SIN-220Hz-0.3V | yes | 40.3 | 1536001 | 0 | 0 | 0 | 2 / 5 | 14554 |
| default | treble | lfo | DI-GTR L | yes | 57.8 | 7680328 | 0 | 0 | 0 | 2 / 7 | 15711 |
| default | treble | jump | SIN-220Hz-0.3V | yes | 34.0 | 1536001 | 0 | 0 | 0 | 2 / 4 | 14174 |
| default | treble | jump | DI-GTR L | yes | 66.7 | 7680747 | 0 | 0 | 0 | 2 / 7 | 15539 |
| default | middle | lfo | SIN-220Hz-0.3V | yes | 34.1 | 1536001 | 0 | 0 | 0 | 2 / 5 | 6638 |
| default | middle | lfo | DI-GTR L | yes | 48.4 | 7680310 | 0 | 0 | 0 | 2 / 7 | 7456 |
| default | middle | jump | SIN-220Hz-0.3V | yes | 38.7 | 1536001 | 0 | 0 | 0 | 2 / 5 | 6522 |
| default | middle | jump | DI-GTR L | yes | 52.6 | 7680451 | 0 | 0 | 0 | 2 / 7 | 7396 |
| default | bass | lfo | SIN-220Hz-0.3V | yes | 37.1 | 1536001 | 0 | 0 | 0 | 1 / 5 | 15067 |
| default | bass | lfo | DI-GTR L | yes | 48.0 | 7680368 | 0 | 0 | 0 | 2 / 7 | 15243 |
| default | bass | jump | SIN-220Hz-0.3V | yes | 42.2 | 1536001 | 0 | 0 | 0 | 1 / 5 | 14393 |
| default | bass | jump | DI-GTR L | yes | 51.1 | 7680353 | 0 | 0 | 0 | 2 / 7 | 15963 |
| default | master | lfo | SIN-220Hz-0.3V | yes | 69.3 | 1536001 | 0 | 0 | 0 | 2 / 6 | 14624 |
| default | master | lfo | DI-GTR L | yes | 72.3 | 7693132 | 0 | 0 | 0 | 2 / 7 | 15992 |
| default | master | jump | SIN-220Hz-0.3V | yes | 72.8 | 1536532 | 0 | 0 | 0 | 2 / 5 | 14253 |
| default | master | jump | DI-GTR L | yes | 73.8 | 7697566 | 3 | 3 | 0 | 2 / 12 | 15890 |
| default | presence | lfo | SIN-220Hz-0.3V | yes | 34.2 | 1536001 | 0 | 0 | 0 | 2 / 5 | 11303 |
| default | presence | lfo | DI-GTR L | yes | 50.0 | 7680359 | 0 | 0 | 0 | 2 / 7 | 13047 |
| default | presence | jump | SIN-220Hz-0.3V | yes | 51.8 | 1536011 | 0 | 0 | 0 | 2 / 5 | 10941 |
| default | presence | jump | DI-GTR L | yes | 66.8 | 7680698 | 0 | 0 | 0 | 2 / 7 | 12304 |
| default | all | lfo | SIN-220Hz-0.3V | yes | 71.0 | 1536401 | 0 | 0 | 0 | 2 / 6 | 16027 |
| default | all | lfo | DI-GTR L | yes | 73.2 | 7695635 | 0 | 0 | 0 | 2 / 9 | 19519 |
| default | all | jump | SIN-220Hz-0.3V | yes | 73.9 | 1538727 | 0 | 0 | 0 | 2 / 7 | 14988 |
| default | all | jump | DI-GTR L | yes | 75.3 | 7708463 | 120 | 120 | 0 | 2 / 12 | 19541 |
| max | preampVolume | lfo | SIN-220Hz-0.3V | yes | 69.6 | 1540467 | 0 | 0 | 0 | 2 / 6 | 6531 |
| max | preampVolume | lfo | DI-GTR L | yes | 74.1 | 7744525 | 207 | 207 | 0 | 2 / 12 | 7564 |
| max | preampVolume | jump | SIN-220Hz-0.3V | yes | 73.3 | 1540396 | 0 | 0 | 0 | 2 / 7 | 6307 |
| max | preampVolume | jump | DI-GTR L | yes | 74.6 | 7746565 | 740 | 740 | 0 | 2 / 12 | 7647 |
| max | treble | lfo | SIN-220Hz-0.3V | yes | 72.8 | 1543512 | 0 | 0 | 0 | 2 / 7 | 16479 |
| max | treble | lfo | DI-GTR L | yes | 72.4 | 7751914 | 393 | 393 | 0 | 2 / 12 | 17264 |
| max | treble | jump | SIN-220Hz-0.3V | yes | 72.8 | 1542599 | 0 | 0 | 0 | 2 / 7 | 15082 |
| max | treble | jump | DI-GTR L | yes | 72.4 | 7748749 | 262 | 262 | 0 | 2 / 12 | 17064 |
| max | middle | lfo | SIN-220Hz-0.3V | yes | 72.8 | 1548251 | 0 | 0 | 0 | 2 / 8 | 7214 |
| max | middle | lfo | DI-GTR L | yes | 72.5 | 7755935 | 516 | 516 | 0 | 2 / 12 | 8248 |
| max | middle | jump | SIN-220Hz-0.3V | yes | 72.8 | 1548553 | 0 | 0 | 0 | 2 / 8 | 6910 |
| max | middle | jump | DI-GTR L | yes | 72.5 | 7757381 | 693 | 693 | 0 | 2 / 12 | 8308 |
| max | bass | lfo | SIN-220Hz-0.3V | yes | 72.8 | 1542853 | 0 | 0 | 0 | 2 / 7 | 15572 |
| max | bass | lfo | DI-GTR L | yes | 72.5 | 7780312 | 955 | 955 | 0 | 2 / 12 | 16896 |
| max | bass | jump | SIN-220Hz-0.3V | yes | 72.8 | 1542972 | 0 | 0 | 0 | 2 / 7 | 14965 |
| max | bass | jump | DI-GTR L | yes | 72.4 | 7776475 | 1089 | 1089 | 0 | 2 / 12 | 17435 |
| max | master | lfo | SIN-220Hz-0.3V | yes | 71.7 | 1541292 | 0 | 0 | 0 | 2 / 6 | 17033 |
| max | master | lfo | DI-GTR L | yes | 72.7 | 7714692 | 36 | 36 | 0 | 2 / 12 | 16801 |
| max | master | jump | SIN-220Hz-0.3V | yes | 74.8 | 1540628 | 0 | 0 | 0 | 2 / 7 | 15225 |
| max | master | jump | DI-GTR L | yes | 74.8 | 7719871 | 369 | 369 | 0 | 2 / 12 | 16032 |
| max | presence | lfo | SIN-220Hz-0.3V | yes | 72.8 | 1543032 | 0 | 0 | 0 | 2 / 7 | 12490 |
| max | presence | lfo | DI-GTR L | yes | 72.5 | 7758603 | 406 | 406 | 0 | 2 / 12 | 13418 |
| max | presence | jump | SIN-220Hz-0.3V | yes | 72.8 | 1543018 | 0 | 0 | 0 | 2 / 7 | 12793 |
| max | presence | jump | DI-GTR L | yes | 72.5 | 7760097 | 458 | 458 | 0 | 2 / 12 | 12494 |
| max | all | lfo | SIN-220Hz-0.3V | yes | 71.0 | 1536401 | 0 | 0 | 0 | 2 / 6 | 17782 |
| max | all | lfo | DI-GTR L | yes | 73.2 | 7695635 | 0 | 0 | 0 | 2 / 9 | 17892 |
| max | all | jump | SIN-220Hz-0.3V | yes | 73.9 | 1538727 | 0 | 0 | 0 | 2 / 7 | 16726 |
| max | all | jump | DI-GTR L | yes | 75.3 | 7708463 | 120 | 120 | 0 | 2 / 12 | 17094 |
