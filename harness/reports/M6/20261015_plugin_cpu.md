# M6 plugin CPU under knob automation

- Date 2026-10-15, commit `476d21f (dirty)`; `amp2203_render plugin` at the plugin defaults (2x, Power Amp Rate Half, exact J4, Newton tol 1e-4 V, half-steps on; D-054 – D-056), FIR oversampling, 20 s of DI GTR L at 48000 Hz; processBlock time only.
- Automation between host blocks: lfo = 10 Hz triangle 0 → 10 → 0, jump = 0 ↔ 10 every 50 ms; "all" moves the six knobs together. One render at a time.

| Base | Knob(s) | Modulation | Host block | CPU (% of one core) | Newton failures (rescued) |
|---|---|---|---|---|---|
| default | — | static | 512 | 19.1 | 0 (0) |
| every knob 10 | — | static | 512 | 31.6 | 0 (0) |
| default | preampVolume | lfo | 512 | 19.0 | 0 (0) |
| default | preampVolume | jump | 512 | 18.6 | 0 (0) |
| default | treble | lfo | 512 | 20.0 | 0 (0) |
| default | treble | jump | 512 | 19.7 | 0 (0) |
| default | middle | lfo | 512 | 19.5 | 0 (0) |
| default | middle | jump | 512 | 19.6 | 0 (0) |
| default | bass | lfo | 512 | 19.5 | 0 (0) |
| default | bass | jump | 512 | 19.5 | 0 (0) |
| default | master | lfo | 512 | 21.5 | 0 (0) |
| default | master | jump | 512 | 22.7 | 0 (0) |
| default | presence | lfo | 512 | 19.9 | 0 (0) |
| default | presence | jump | 512 | 19.8 | 0 (0) |
| default | all | lfo | 512 | 22.2 | 0 (0) |
| default | all | jump | 512 | 23.9 | 0 (0) |
| every knob 10 | preampVolume | lfo | 512 | 31.4 | 0 (0) |
| every knob 10 | preampVolume | jump | 512 | 31.7 | 0 (0) |
| every knob 10 | treble | lfo | 512 | 31.4 | 0 (0) |
| every knob 10 | treble | jump | 512 | 30.9 | 0 (0) |
| every knob 10 | middle | lfo | 512 | 31.4 | 0 (0) |
| every knob 10 | middle | jump | 512 | 31.5 | 0 (0) |
| every knob 10 | bass | lfo | 512 | 33.4 | 0 (0) |
| every knob 10 | bass | jump | 512 | 32.6 | 0 (0) |
| every knob 10 | master | lfo | 512 | 26.3 | 0 (0) |
| every knob 10 | master | jump | 512 | 26.4 | 0 (0) |
| every knob 10 | presence | lfo | 512 | 32.2 | 0 (0) |
| every knob 10 | presence | jump | 512 | 32.5 | 0 (0) |
| every knob 10 | all | lfo | 512 | 22.6 | 0 (0) |
| every knob 10 | all | jump | 512 | 24.4 | 0 (0) |
| default | all | jump | 64 | 24.1 | 0 (0) |
| every knob 10 | all | jump | 64 | 24.3 | 0 (0) |
