# M6 plugin CPU under knob automation

- Date 2026-10-17, commit `eeea956 (dirty)`; `amp2203_render plugin` at the plugin defaults (2x, Power Amp Rate Full, exact J4, Newton tol 1e-4 V, half-steps on; D-054 – D-056, D-060), FIR oversampling, 20 s of DI GTR L at 48000 Hz; processBlock time only.
- Automation between host blocks: lfo = 10 Hz triangle 0 → 10 → 0, jump = 0 ↔ 10 every 50 ms; "all" moves the six knobs together. One render at a time.

| Base | Knob(s) | Modulation | Host block | CPU (% of one core) | Newton failures (rescued) |
|---|---|---|---|---|---|
| default | — | static | 512 | 26.4 | 0 (0) |
| every knob 10 | — | static | 512 | 50.2 | 0 (0) |
| default | preampVolume | lfo | 512 | 26.2 | 0 (0) |
| default | preampVolume | jump | 512 | 25.8 | 0 (0) |
| default | treble | lfo | 512 | 27.3 | 0 (0) |
| default | treble | jump | 512 | 27.3 | 0 (0) |
| default | middle | lfo | 512 | 27.3 | 0 (0) |
| default | middle | jump | 512 | 27.3 | 0 (0) |
| default | bass | lfo | 512 | 27.3 | 0 (0) |
| default | bass | jump | 512 | 27.3 | 0 (0) |
| default | master | lfo | 512 | 31.0 | 0 (0) |
| default | master | jump | 512 | 31.4 | 0 (0) |
| default | presence | lfo | 512 | 28.8 | 0 (0) |
| default | presence | jump | 512 | 28.9 | 0 (0) |
| default | all | lfo | 512 | 32.3 | 0 (0) |
| default | all | jump | 512 | 34.2 | 0 (0) |
| every knob 10 | preampVolume | lfo | 512 | 43.3 | 0 (0) |
| every knob 10 | preampVolume | jump | 512 | 42.9 | 0 (0) |
| every knob 10 | treble | lfo | 512 | 48.1 | 0 (0) |
| every knob 10 | treble | jump | 512 | 47.8 | 0 (0) |
| every knob 10 | middle | lfo | 512 | 48.4 | 0 (0) |
| every knob 10 | middle | jump | 512 | 48.2 | 0 (0) |
| every knob 10 | bass | lfo | 512 | 52.5 | 0 (0) |
| every knob 10 | bass | jump | 512 | 52.3 | 0 (0) |
| every knob 10 | master | lfo | 512 | 38.1 | 0 (0) |
| every knob 10 | master | jump | 512 | 38.6 | 0 (0) |
| every knob 10 | presence | lfo | 512 | 47.6 | 27 (27) |
| every knob 10 | presence | jump | 512 | 47.9 | 16 (16) |
| every knob 10 | all | lfo | 512 | 33.0 | 0 (0) |
| every knob 10 | all | jump | 512 | 35.0 | 0 (0) |
| default | all | jump | 64 | 34.2 | 0 (0) |
| every knob 10 | all | jump | 64 | 35.0 | 0 (0) |
