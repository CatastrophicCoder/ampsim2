# M6 plugin CPU under knob automation

- Date 2026-10-15, commit `e274e3d (dirty)`; `amp2203_render plugin` at the plugin defaults (2x, Power Amp Rate Half, exact J4, Newton tol 1e-4 V, half-steps on; D-054 – D-056), FIR oversampling, 20 s of DI GTR L at 48000 Hz; processBlock time only.
- Automation between host blocks: lfo = 10 Hz triangle 0 → 10 → 0, jump = 0 ↔ 10 every 50 ms; "all" moves the six knobs together. One render at a time.

| Base | Knob(s) | Modulation | Host block | CPU (% of one core) | Newton failures (rescued) |
|---|---|---|---|---|---|
| default | — | static | 512 | 18.9 | 0 (0) |
| every knob 10 | — | static | 512 | 32.1 | 0 (0) |
| default | preampVolume | lfo | 512 | 45.9 | 0 (0) |
| default | preampVolume | jump | 512 | 45.8 | 0 (0) |
| default | treble | lfo | 512 | 46.6 | 0 (0) |
| default | treble | jump | 512 | 46.4 | 0 (0) |
| default | middle | lfo | 512 | 47.8 | 0 (0) |
| default | middle | jump | 512 | 46.7 | 0 (0) |
| default | bass | lfo | 512 | 46.5 | 0 (0) |
| default | bass | jump | 512 | 47.6 | 0 (0) |
| default | master | lfo | 512 | 48.7 | 0 (0) |
| default | master | jump | 512 | 49.6 | 0 (0) |
| default | presence | lfo | 512 | 55.0 | 0 (0) |
| default | presence | jump | 512 | 54.1 | 0 (0) |
| default | all | lfo | 512 | 84.6 | 0 (0) |
| default | all | jump | 512 | 85.4 | 0 (0) |
| every knob 10 | preampVolume | lfo | 512 | 58.9 | 0 (0) |
| every knob 10 | preampVolume | jump | 512 | 58.6 | 0 (0) |
| every knob 10 | treble | lfo | 512 | 59.3 | 0 (0) |
| every knob 10 | treble | jump | 512 | 59.6 | 0 (0) |
| every knob 10 | middle | lfo | 512 | 59.2 | 0 (0) |
| every knob 10 | middle | jump | 512 | 60.2 | 0 (0) |
| every knob 10 | bass | lfo | 512 | 61.5 | 0 (0) |
| every knob 10 | bass | jump | 512 | 60.8 | 0 (0) |
| every knob 10 | master | lfo | 512 | 55.1 | 0 (0) |
| every knob 10 | master | jump | 512 | 54.4 | 0 (0) |
| every knob 10 | presence | lfo | 512 | 67.7 | 0 (0) |
| every knob 10 | presence | jump | 512 | 66.7 | 0 (0) |
| every knob 10 | all | lfo | 512 | 86.4 | 0 (0) |
| every knob 10 | all | jump | 512 | 86.6 | 0 (0) |
| default | all | jump | 64 | 87.1 | 1 (1) |
| every knob 10 | all | jump | 64 | 86.1 | 1 (1) |
