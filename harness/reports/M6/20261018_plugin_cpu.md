# M6 plugin CPU under knob automation

- Date 2026-10-18, commit `9a422dd (dirty)`; `amp2203_render plugin` at the plugin defaults (2x, Power Amp Rate Full, exact J4, Newton tol 1e-4 V, half-steps on; D-054 – D-056, D-060), FIR oversampling, 20 s of DI GTR L at 48000 Hz; processBlock time only.
- Automation between host blocks: lfo = 10 Hz triangle 0 → 10 → 0, jump = 0 ↔ 10 every 50 ms; "all" moves the six knobs together. One render at a time.

| Base | Knob(s) | Modulation | Host block | CPU (% of one core) | Newton failures (rescued) |
|---|---|---|---|---|---|
| default | — | static | 512 | 27.4 | 0 (0) |
| every knob 10 | — | static | 512 | 50.8 | 32 (32) |
| default | preampVolume | lfo | 512 | 26.3 | 0 (0) |
| default | preampVolume | jump | 512 | 25.9 | 0 (0) |
| default | treble | lfo | 512 | 27.6 | 0 (0) |
| default | treble | jump | 512 | 27.7 | 0 (0) |
| default | middle | lfo | 512 | 27.4 | 0 (0) |
| default | middle | jump | 512 | 27.4 | 0 (0) |
| default | bass | lfo | 512 | 27.3 | 0 (0) |
| default | bass | jump | 512 | 27.3 | 0 (0) |
| default | master | lfo | 512 | 31.6 | 2 (2) |
| default | master | jump | 512 | 31.9 | 2 (2) |
| default | presence | lfo | 512 | 28.8 | 0 (0) |
| default | presence | jump | 512 | 28.7 | 0 (0) |
| default | all | lfo | 512 | 32.5 | 4 (4) |
| default | all | jump | 512 | 34.7 | 4 (4) |
| every knob 10 | preampVolume | lfo | 512 | 43.7 | 18 (18) |
| every knob 10 | preampVolume | jump | 512 | 43.6 | 19 (19) |
| every knob 10 | treble | lfo | 512 | 48.6 | 27 (27) |
| every knob 10 | treble | jump | 512 | 47.8 | 32 (32) |
| every knob 10 | middle | lfo | 512 | 48.6 | 31 (31) |
| every knob 10 | middle | jump | 512 | 48.9 | 23 (23) |
| every knob 10 | bass | lfo | 512 | 52.8 | 32 (32) |
| every knob 10 | bass | jump | 512 | 52.2 | 31 (31) |
| every knob 10 | master | lfo | 512 | 38.2 | 6 (6) |
| every knob 10 | master | jump | 512 | 38.5 | 5 (5) |
| every knob 10 | presence | lfo | 512 | 47.3 | 34 (34) |
| every knob 10 | presence | jump | 512 | 47.8 | 26 (26) |
| every knob 10 | all | lfo | 512 | 33.2 | 3 (3) |
| every knob 10 | all | jump | 512 | 34.7 | 1 (1) |
| default | all | jump | 64 | 34.3 | 5 (5) |
| every knob 10 | all | jump | 64 | 34.7 | 4 (4) |
