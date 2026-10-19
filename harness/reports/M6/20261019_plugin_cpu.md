# M6 plugin CPU under knob automation

- Date 2026-10-19, commit `ecf9042 (dirty)`; `amp2203_render plugin` at the plugin defaults (2x, Power Amp Rate Half, exact J4, Newton tol 1e-4 V, half-steps on; D-054 – D-056, D-069), FIR oversampling, 20 s of DI GTR L at 48000 Hz; processBlock time only.
- Automation between host blocks: lfo = 10 Hz triangle 0 → 10 → 0, jump = 0 ↔ 10 every 50 ms; "all" moves the six knobs together. One render at a time.

| Base | Knob(s) | Modulation | Host block | CPU (% of one core) | Newton failures (rescued) |
|---|---|---|---|---|---|
| default | — | static | 512 | 13.6 | 0 (0) |
| every knob 10 | — | static | 512 | 25.7 | 0 (0) |
| default | preampVolume | lfo | 512 | 14.2 | 0 (0) |
| default | preampVolume | jump | 512 | 13.7 | 0 (0) |
| default | treble | lfo | 512 | 14.2 | 0 (0) |
| default | treble | jump | 512 | 14.4 | 0 (0) |
| default | middle | lfo | 512 | 14.2 | 0 (0) |
| default | middle | jump | 512 | 14.3 | 0 (0) |
| default | bass | lfo | 512 | 14.2 | 0 (0) |
| default | bass | jump | 512 | 14.1 | 0 (0) |
| default | master | lfo | 512 | 15.9 | 0 (0) |
| default | master | jump | 512 | 16.2 | 0 (0) |
| default | presence | lfo | 512 | 14.6 | 0 (0) |
| default | presence | jump | 512 | 14.6 | 0 (0) |
| default | all | lfo | 512 | 16.8 | 0 (0) |
| default | all | jump | 512 | 17.6 | 0 (0) |
| every knob 10 | preampVolume | lfo | 512 | 24.0 | 0 (0) |
| every knob 10 | preampVolume | jump | 512 | 23.7 | 0 (0) |
| every knob 10 | treble | lfo | 512 | 23.7 | 0 (0) |
| every knob 10 | treble | jump | 512 | 23.1 | 0 (0) |
| every knob 10 | middle | lfo | 512 | 23.7 | 2 (1) |
| every knob 10 | middle | jump | 512 | 23.7 | 0 (0) |
| every knob 10 | bass | lfo | 512 | 25.2 | 0 (0) |
| every knob 10 | bass | jump | 512 | 25.1 | 0 (0) |
| every knob 10 | master | lfo | 512 | 19.9 | 0 (0) |
| every knob 10 | master | jump | 512 | 19.7 | 0 (0) |
| every knob 10 | presence | lfo | 512 | 23.4 | 0 (0) |
| every knob 10 | presence | jump | 512 | 23.6 | 2 (2) |
| every knob 10 | all | lfo | 512 | 16.6 | 0 (0) |
| every knob 10 | all | jump | 512 | 17.7 | 0 (0) |
| default | all | jump | 64 | 17.6 | 0 (0) |
| every knob 10 | all | jump | 64 | 17.7 | 0 (0) |
