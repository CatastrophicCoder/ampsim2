# M6 plugin CPU under knob automation

- Date 2026-10-18, commit `daf089b (dirty)`; `amp2203_render plugin` at the plugin defaults (2x, Power Amp Rate Full, exact J4, Newton tol 1e-4 V, half-steps on; D-054 – D-056, D-060), FIR oversampling, 20 s of DI GTR L at 48000 Hz; processBlock time only.
- Automation between host blocks: lfo = 10 Hz triangle 0 → 10 → 0, jump = 0 ↔ 10 every 50 ms; "all" moves the six knobs together. One render at a time.

| Base | Knob(s) | Modulation | Host block | CPU (% of one core) | Newton failures (rescued) |
|---|---|---|---|---|---|
| default | — | static | 512 | 27.0 | 0 (0) |
| every knob 10 | — | static | 512 | 49.7 | 0 (0) |
| default | preampVolume | lfo | 512 | 25.6 | 0 (0) |
| default | preampVolume | jump | 512 | 25.3 | 0 (0) |
| default | treble | lfo | 512 | 26.9 | 0 (0) |
| default | treble | jump | 512 | 27.4 | 0 (0) |
| default | middle | lfo | 512 | 26.8 | 0 (0) |
| default | middle | jump | 512 | 26.6 | 0 (0) |
| default | bass | lfo | 512 | 26.6 | 0 (0) |
| default | bass | jump | 512 | 26.6 | 0 (0) |
| default | master | lfo | 512 | 31.0 | 0 (0) |
| default | master | jump | 512 | 30.8 | 0 (0) |
| default | presence | lfo | 512 | 27.9 | 0 (0) |
| default | presence | jump | 512 | 28.0 | 0 (0) |
| default | all | lfo | 512 | 31.5 | 0 (0) |
| default | all | jump | 512 | 34.3 | 0 (0) |
| every knob 10 | preampVolume | lfo | 512 | 42.9 | 0 (0) |
| every knob 10 | preampVolume | jump | 512 | 42.4 | 0 (0) |
| every knob 10 | treble | lfo | 512 | 47.8 | 0 (0) |
| every knob 10 | treble | jump | 512 | 49.9 | 0 (0) |
| every knob 10 | middle | lfo | 512 | 47.9 | 0 (0) |
| every knob 10 | middle | jump | 512 | 48.4 | 0 (0) |
| every knob 10 | bass | lfo | 512 | 52.4 | 0 (0) |
| every knob 10 | bass | jump | 512 | 51.6 | 0 (0) |
| every knob 10 | master | lfo | 512 | 37.2 | 0 (0) |
| every knob 10 | master | jump | 512 | 38.5 | 0 (0) |
| every knob 10 | presence | lfo | 512 | 46.8 | 0 (0) |
| every knob 10 | presence | jump | 512 | 47.4 | 0 (0) |
| every knob 10 | all | lfo | 512 | 32.6 | 0 (0) |
| every knob 10 | all | jump | 512 | 34.5 | 0 (0) |
| default | all | jump | 64 | 33.5 | 0 (0) |
| every knob 10 | all | jump | 64 | 33.9 | 0 (0) |
