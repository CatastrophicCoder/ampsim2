# M5 automation: every knob over its full range at audio rate

- Date: 2026-10-19, commit `ecf9042 (dirty)`, engine `ampwav --block full` at 96000 Hz
- Modulation: 10 Hz triangle 0 → 1 → 0 (`lfo`) or 0 ↔ 1 jumps every 50 ms (`jump`) on the knob targets (smoothed by the engine, 20 ms); bases: default knobs, every knob at 1.0.
- SPEC §7 M5: no NaN / Inf, no Newton failures. Newton failures count first solves that did not converge in 12 iterations; the fallback (D-042) rescued or did not rescue them.

Runs: 56 · non-finite outputs: 0 · Newton failures (first solve): 0 · unrescued: 0

| Base | Knobs | Modulation | Signal | finite | peak [V] | solves | failures | rescued | unrescued | median / max it | ns/sample |
|---|---|---|---|---|---|---|---|---|---|---|---|
| default | preampVolume | lfo | SIN-220Hz-0.3V | yes | 34.7 | 144026 | 0 | 0 | 0 | 1 / 6 | 1776 |
| default | preampVolume | lfo | DI-GTR L | yes | 54.1 | 731830 | 0 | 0 | 0 | 1 / 14 | 2274 |
| default | preampVolume | jump | SIN-220Hz-0.3V | yes | 40.3 | 144312 | 0 | 0 | 0 | 1 / 22 | 1812 |
| default | preampVolume | jump | DI-GTR L | yes | 57.5 | 732766 | 0 | 0 | 0 | 1 / 15 | 2267 |
| default | treble | lfo | SIN-220Hz-0.3V | yes | 41.4 | 144029 | 0 | 0 | 0 | 1 / 22 | 2172 |
| default | treble | lfo | DI-GTR L | yes | 59.1 | 735894 | 0 | 0 | 0 | 1 / 15 | 2735 |
| default | treble | jump | SIN-220Hz-0.3V | yes | 35.6 | 144096 | 0 | 0 | 0 | 1 / 22 | 2117 |
| default | treble | jump | DI-GTR L | yes | 79.0 | 735783 | 0 | 0 | 0 | 1 / 15 | 2894 |
| default | middle | lfo | SIN-220Hz-0.3V | yes | 35.1 | 144062 | 0 | 0 | 0 | 1 / 22 | 1993 |
| default | middle | lfo | DI-GTR L | yes | 54.0 | 735125 | 0 | 0 | 0 | 1 / 12 | 2749 |
| default | middle | jump | SIN-220Hz-0.3V | yes | 39.9 | 144077 | 0 | 0 | 0 | 1 / 22 | 2174 |
| default | middle | jump | DI-GTR L | yes | 64.0 | 735151 | 0 | 0 | 0 | 1 / 18 | 2736 |
| default | bass | lfo | SIN-220Hz-0.3V | yes | 39.2 | 144078 | 0 | 0 | 0 | 1 / 22 | 1922 |
| default | bass | lfo | DI-GTR L | yes | 56.5 | 735321 | 0 | 0 | 0 | 1 / 12 | 2084 |
| default | bass | jump | SIN-220Hz-0.3V | yes | 45.3 | 144082 | 0 | 0 | 0 | 1 / 22 | 2005 |
| default | bass | jump | DI-GTR L | yes | 54.5 | 735899 | 0 | 0 | 0 | 1 / 16 | 2205 |
| default | master | lfo | SIN-220Hz-0.3V | yes | 75.8 | 163649 | 0 | 0 | 0 | 1 / 22 | 2239 |
| default | master | lfo | DI-GTR L | yes | 133.2 | 805672 | 0 | 0 | 0 | 1 / 22 | 2934 |
| default | master | jump | SIN-220Hz-0.3V | yes | 82.9 | 169587 | 0 | 0 | 0 | 1 / 22 | 2321 |
| default | master | jump | DI-GTR L | yes | 153.1 | 818998 | 0 | 0 | 0 | 1 / 27 | 3034 |
| default | presence | lfo | SIN-220Hz-0.3V | yes | 35.7 | 144076 | 0 | 0 | 0 | 1 / 22 | 2030 |
| default | presence | lfo | DI-GTR L | yes | 53.7 | 735597 | 0 | 0 | 0 | 1 / 15 | 2459 |
| default | presence | jump | SIN-220Hz-0.3V | yes | 61.8 | 144107 | 0 | 0 | 0 | 1 / 22 | 2077 |
| default | presence | jump | DI-GTR L | yes | 76.1 | 736615 | 0 | 0 | 0 | 1 / 15 | 2515 |
| default | all | lfo | SIN-220Hz-0.3V | yes | 77.1 | 166503 | 0 | 0 | 0 | 1 / 9 | 3153 |
| default | all | lfo | DI-GTR L | yes | 158.4 | 810363 | 0 | 0 | 0 | 1 / 21 | 6344 |
| default | all | jump | SIN-220Hz-0.3V | yes | 131.5 | 172871 | 0 | 0 | 0 | 1 / 22 | 2919 |
| default | all | jump | DI-GTR L | yes | 163.7 | 846000 | 0 | 0 | 0 | 1 / 28 | 6126 |
| max | preampVolume | lfo | SIN-220Hz-0.3V | yes | 139.0 | 195035 | 0 | 0 | 0 | 1 / 14 | 3055 |
| max | preampVolume | lfo | DI-GTR L | yes | 195.8 | 1014766 | 0 | 0 | 0 | 1 / 25 | 3555 |
| max | preampVolume | jump | SIN-220Hz-0.3V | yes | 139.9 | 199660 | 0 | 0 | 0 | 1 / 22 | 3509 |
| max | preampVolume | jump | DI-GTR L | yes | 209.3 | 1013260 | 0 | 0 | 0 | 1 / 32 | 3717 |
| max | treble | lfo | SIN-220Hz-0.3V | yes | 119.0 | 200298 | 0 | 0 | 0 | 1 / 22 | 3057 |
| max | treble | lfo | DI-GTR L | yes | 203.1 | 963138 | 0 | 0 | 0 | 1 / 28 | 3845 |
| max | treble | jump | SIN-220Hz-0.3V | yes | 131.5 | 201219 | 0 | 0 | 0 | 1 / 22 | 2811 |
| max | treble | jump | DI-GTR L | yes | 208.3 | 963204 | 0 | 0 | 0 | 1 / 24 | 3820 |
| max | middle | lfo | SIN-220Hz-0.3V | yes | 183.1 | 206717 | 0 | 0 | 0 | 1 / 22 | 3250 |
| max | middle | lfo | DI-GTR L | yes | 202.6 | 1013016 | 0 | 0 | 0 | 1 / 34 | 3967 |
| max | middle | jump | SIN-220Hz-0.3V | yes | 131.5 | 204279 | 0 | 0 | 0 | 1 / 22 | 3264 |
| max | middle | jump | DI-GTR L | yes | 204.8 | 1012085 | 0 | 0 | 0 | 1 / 29 | 3873 |
| max | bass | lfo | SIN-220Hz-0.3V | yes | 144.9 | 204554 | 0 | 0 | 0 | 1 / 22 | 3457 |
| max | bass | lfo | DI-GTR L | yes | 197.2 | 1020201 | 0 | 0 | 0 | 1 / 30 | 3719 |
| max | bass | jump | SIN-220Hz-0.3V | yes | 145.0 | 203571 | 0 | 0 | 0 | 1 / 22 | 3305 |
| max | bass | jump | DI-GTR L | yes | 198.2 | 1019531 | 0 | 0 | 0 | 1 / 32 | 3481 |
| max | master | lfo | SIN-220Hz-0.3V | yes | 86.0 | 184783 | 0 | 0 | 0 | 1 / 22 | 2796 |
| max | master | lfo | DI-GTR L | yes | 166.3 | 882620 | 0 | 0 | 0 | 1 / 28 | 3204 |
| max | master | jump | SIN-220Hz-0.3V | yes | 131.5 | 180618 | 0 | 0 | 0 | 1 / 22 | 2524 |
| max | master | jump | DI-GTR L | yes | 183.5 | 878362 | 0 | 0 | 0 | 1 / 30 | 3217 |
| max | presence | lfo | SIN-220Hz-0.3V | yes | 104.9 | 203081 | 0 | 0 | 0 | 1 / 22 | 2845 |
| max | presence | lfo | DI-GTR L | yes | 193.3 | 1004297 | 0 | 0 | 0 | 1 / 33 | 3264 |
| max | presence | jump | SIN-220Hz-0.3V | yes | 131.5 | 202853 | 0 | 0 | 0 | 1 / 22 | 3066 |
| max | presence | jump | DI-GTR L | yes | 203.0 | 1005211 | 0 | 0 | 0 | 1 / 33 | 3111 |
| max | all | lfo | SIN-220Hz-0.3V | yes | 77.1 | 166503 | 0 | 0 | 0 | 1 / 9 | 2800 |
| max | all | lfo | DI-GTR L | yes | 158.4 | 810363 | 0 | 0 | 0 | 1 / 21 | 5199 |
| max | all | jump | SIN-220Hz-0.3V | yes | 131.5 | 172871 | 0 | 0 | 0 | 1 / 22 | 3175 |
| max | all | jump | DI-GTR L | yes | 163.7 | 846000 | 0 | 0 | 0 | 1 / 28 | 5324 |
