# M5 wrapper null test: plugin vs ampwav at 96 kHz with the same resampling

- Date: 2026-10-15, commit `9778d0d (dirty)`; host rate 48000 Hz, 2x, Power Amp Rate half, exact J4
- Plugin: `amp2203_render plugin` (the Amp2203Processor offline). Reference: the processor's oversampler up stage, `ampwav --block full --units fs --j4 exact` at 96000 Hz, the down stage. Default knobs and levels.
- Tolerance (SPEC §7 M5): ESR ≤ -60 dB.

PASS 26 · FAIL 0

| Filter | Signal | Peak out [FS] | ESR | Status | Render log |
|---|---|---|---|---|---|
| fir | SIN-100Hz--40dBFS | 0.111 | -138.5 dB | PASS | processBlock 0.029 s for 0.200 s of audio: 14.4 % of one core, real-time factor 6.95
latency 58 samples, Newton failures 0 (rescued 0), max iterations 3 |
| fir | SIN-100Hz--20dBFS | 0.719 | -138.5 dB | PASS | processBlock 0.031 s for 0.200 s of audio: 15.7 % of one core, real-time factor 6.38
latency 58 samples, Newton failures 0 (rescued 0), max iterations 4 |
| fir | SIN-100Hz--6dBFS | 0.825 | -138.5 dB | PASS | processBlock 0.033 s for 0.200 s of audio: 16.3 % of one core, real-time factor 6.14
latency 58 samples, Newton failures 0 (rescued 0), max iterations 9 |
| fir | SIN-1000Hz--40dBFS | 0.335 | -138.5 dB | PASS | processBlock 0.046 s for 0.200 s of audio: 23.2 % of one core, real-time factor 4.31
latency 58 samples, Newton failures 0 (rescued 0), max iterations 6 |
| fir | SIN-1000Hz--20dBFS | 0.655 | -138.5 dB | PASS | processBlock 0.050 s for 0.200 s of audio: 25.1 % of one core, real-time factor 3.98
latency 58 samples, Newton failures 0 (rescued 0), max iterations 9 |
| fir | SIN-1000Hz--6dBFS | 0.708 | -138.5 dB | PASS | processBlock 0.055 s for 0.200 s of audio: 27.4 % of one core, real-time factor 3.64
latency 58 samples, Newton failures 0 (rescued 0), max iterations 12 |
| fir | SIN-5000Hz--40dBFS | 0.703 | -138.5 dB | PASS | processBlock 0.067 s for 0.200 s of audio: 33.5 % of one core, real-time factor 2.98
latency 58 samples, Newton failures 0 (rescued 0), max iterations 18 |
| fir | SIN-5000Hz--20dBFS | 0.706 | -138.5 dB | PASS | processBlock 0.078 s for 0.200 s of audio: 38.9 % of one core, real-time factor 2.57
latency 58 samples, Newton failures 0 (rescued 0), max iterations 19 |
| fir | SIN-5000Hz--6dBFS | 0.744 | -138.5 dB | PASS | processBlock 0.145 s for 0.200 s of audio: 72.5 % of one core, real-time factor 1.38
latency 58 samples, Newton failures 449 (rescued 191), max iterations 32 |
| fir | DI-GTR L | 0.741 | -138.5 dB | PASS | processBlock 0.889 s for 5.000 s of audio: 17.8 % of one core, real-time factor 5.63
latency 58 samples, Newton failures 0 (rescued 0), max iterations 17 |
| fir | DI-GTR LL | 0.756 | -138.5 dB | PASS | processBlock 0.870 s for 5.000 s of audio: 17.4 % of one core, real-time factor 5.74
latency 58 samples, Newton failures 0 (rescued 0), max iterations 17 |
| fir | DI-GTR R | 0.783 | -138.5 dB | PASS | processBlock 0.873 s for 5.000 s of audio: 17.5 % of one core, real-time factor 5.73
latency 58 samples, Newton failures 0 (rescued 0), max iterations 20 |
| fir | DI-GTR RR | 0.758 | -138.5 dB | PASS | processBlock 0.890 s for 5.000 s of audio: 17.8 % of one core, real-time factor 5.62
latency 58 samples, Newton failures 0 (rescued 0), max iterations 18 |
| iir | SIN-100Hz--40dBFS | 0.111 | -138.5 dB | PASS | processBlock 0.028 s for 0.200 s of audio: 14.0 % of one core, real-time factor 7.12
latency 13 samples, Newton failures 0 (rescued 0), max iterations 2 |
| iir | SIN-100Hz--20dBFS | 0.719 | -138.5 dB | PASS | processBlock 0.031 s for 0.200 s of audio: 15.5 % of one core, real-time factor 6.45
latency 13 samples, Newton failures 0 (rescued 0), max iterations 6 |
| iir | SIN-100Hz--6dBFS | 0.826 | -138.5 dB | PASS | processBlock 0.033 s for 0.200 s of audio: 16.3 % of one core, real-time factor 6.14
latency 13 samples, Newton failures 0 (rescued 0), max iterations 6 |
| iir | SIN-1000Hz--40dBFS | 0.358 | -138.5 dB | PASS | processBlock 0.046 s for 0.200 s of audio: 22.9 % of one core, real-time factor 4.36
latency 13 samples, Newton failures 0 (rescued 0), max iterations 7 |
| iir | SIN-1000Hz--20dBFS | 0.729 | -138.5 dB | PASS | processBlock 0.049 s for 0.200 s of audio: 24.6 % of one core, real-time factor 4.06
latency 13 samples, Newton failures 0 (rescued 0), max iterations 9 |
| iir | SIN-1000Hz--6dBFS | 0.766 | -138.5 dB | PASS | processBlock 0.054 s for 0.200 s of audio: 27.1 % of one core, real-time factor 3.69
latency 13 samples, Newton failures 0 (rescued 0), max iterations 22 |
| iir | SIN-5000Hz--40dBFS | 0.682 | -138.5 dB | PASS | processBlock 0.067 s for 0.200 s of audio: 33.5 % of one core, real-time factor 2.99
latency 13 samples, Newton failures 0 (rescued 0), max iterations 16 |
| iir | SIN-5000Hz--20dBFS | 0.788 | -138.5 dB | PASS | processBlock 0.076 s for 0.200 s of audio: 38.1 % of one core, real-time factor 2.63
latency 13 samples, Newton failures 0 (rescued 0), max iterations 22 |
| iir | SIN-5000Hz--6dBFS | 0.803 | -138.5 dB | PASS | processBlock 0.107 s for 0.200 s of audio: 53.7 % of one core, real-time factor 1.86
latency 13 samples, Newton failures 163 (rescued 86), max iterations 32 |
| iir | DI-GTR L | 0.796 | -138.5 dB | PASS | processBlock 0.871 s for 5.000 s of audio: 17.4 % of one core, real-time factor 5.74
latency 13 samples, Newton failures 0 (rescued 0), max iterations 18 |
| iir | DI-GTR LL | 0.750 | -138.5 dB | PASS | processBlock 0.844 s for 5.000 s of audio: 16.9 % of one core, real-time factor 5.93
latency 13 samples, Newton failures 0 (rescued 0), max iterations 23 |
| iir | DI-GTR R | 0.837 | -138.5 dB | PASS | processBlock 0.849 s for 5.000 s of audio: 17.0 % of one core, real-time factor 5.89
latency 13 samples, Newton failures 0 (rescued 0), max iterations 17 |
| iir | DI-GTR RR | 0.843 | -138.5 dB | PASS | processBlock 0.872 s for 5.000 s of audio: 17.4 % of one core, real-time factor 5.73
latency 13 samples, Newton failures 0 (rescued 0), max iterations 20 |
