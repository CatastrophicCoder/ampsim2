# M5 wrapper null test: plugin vs ampwav at 96 kHz with the same resampling

- Date: 2026-10-18, commit `d37baf0 (dirty)`; host rate 48000 Hz, 2x, Power Amp Rate full, exact J4
- Plugin: `amp2203_render plugin` (the Amp2203Processor offline). Reference: the processor's oversampler up stage, `ampwav --block full --units fs --j4 exact` at 96000 Hz, the down stage. Default knobs and levels.
- Tolerance (SPEC §7 M5): ESR ≤ -60 dB.

PASS 26 · FAIL 0

| Filter | Signal | Peak out [FS] | ESR | Status | Render log |
|---|---|---|---|---|---|
| fir | SIN-100Hz--40dBFS | 0.145 | -138.5 dB | PASS | processBlock 0.046 s for 0.200 s of audio: 23.1 % of one core, real-time factor 4.32
latency 49 samples, Newton failures 0 (rescued 0), max iterations 2 |
| fir | SIN-100Hz--20dBFS | 0.870 | -138.5 dB | PASS | processBlock 0.048 s for 0.200 s of audio: 24.0 % of one core, real-time factor 4.18
latency 49 samples, Newton failures 0 (rescued 0), max iterations 5 |
| fir | SIN-100Hz--6dBFS | 0.915 | -138.5 dB | PASS | processBlock 0.050 s for 0.200 s of audio: 24.8 % of one core, real-time factor 4.03
latency 49 samples, Newton failures 0 (rescued 0), max iterations 10 |
| fir | SIN-1000Hz--40dBFS | 0.383 | -138.5 dB | PASS | processBlock 0.073 s for 0.200 s of audio: 36.6 % of one core, real-time factor 2.73
latency 49 samples, Newton failures 0 (rescued 0), max iterations 7 |
| fir | SIN-1000Hz--20dBFS | 0.850 | -138.5 dB | PASS | processBlock 0.072 s for 0.200 s of audio: 35.9 % of one core, real-time factor 2.78
latency 49 samples, Newton failures 0 (rescued 0), max iterations 12 |
| fir | SIN-1000Hz--6dBFS | 0.918 | -138.5 dB | PASS | processBlock 0.079 s for 0.200 s of audio: 39.3 % of one core, real-time factor 2.55
latency 49 samples, Newton failures 0 (rescued 0), max iterations 13 |
| fir | SIN-5000Hz--40dBFS | 0.857 | -138.5 dB | PASS | processBlock 0.089 s for 0.200 s of audio: 44.7 % of one core, real-time factor 2.24
latency 49 samples, Newton failures 0 (rescued 0), max iterations 17 |
| fir | SIN-5000Hz--20dBFS | 0.891 | -138.5 dB | PASS | processBlock 0.102 s for 0.200 s of audio: 50.8 % of one core, real-time factor 1.97
latency 49 samples, Newton failures 0 (rescued 0), max iterations 19 |
| fir | SIN-5000Hz--6dBFS | 0.863 | -138.5 dB | PASS | processBlock 0.515 s for 0.200 s of audio: 257.3 % of one core, real-time factor 0.39
latency 49 samples, Newton failures 3518 (rescued 89), max iterations 32 |
| fir | DI-GTR L | 0.984 | -138.5 dB | PASS | processBlock 1.236 s for 5.000 s of audio: 24.7 % of one core, real-time factor 4.04
latency 49 samples, Newton failures 0 (rescued 0), max iterations 18 |
| fir | DI-GTR LL | 0.861 | -138.5 dB | PASS | processBlock 1.189 s for 5.000 s of audio: 23.8 % of one core, real-time factor 4.21
latency 49 samples, Newton failures 0 (rescued 0), max iterations 17 |
| fir | DI-GTR R | 0.944 | -138.5 dB | PASS | processBlock 1.206 s for 5.000 s of audio: 24.1 % of one core, real-time factor 4.15
latency 49 samples, Newton failures 0 (rescued 0), max iterations 16 |
| fir | DI-GTR RR | 0.990 | -138.5 dB | PASS | processBlock 1.245 s for 5.000 s of audio: 24.9 % of one core, real-time factor 4.02
latency 49 samples, Newton failures 0 (rescued 0), max iterations 19 |
| iir | SIN-100Hz--40dBFS | 0.145 | -138.5 dB | PASS | processBlock 0.042 s for 0.200 s of audio: 20.8 % of one core, real-time factor 4.81
latency 4 samples, Newton failures 0 (rescued 0), max iterations 2 |
| iir | SIN-100Hz--20dBFS | 0.870 | -138.5 dB | PASS | processBlock 0.044 s for 0.200 s of audio: 22.1 % of one core, real-time factor 4.52
latency 4 samples, Newton failures 0 (rescued 0), max iterations 4 |
| iir | SIN-100Hz--6dBFS | 0.915 | -138.5 dB | PASS | processBlock 0.045 s for 0.200 s of audio: 22.4 % of one core, real-time factor 4.47
latency 4 samples, Newton failures 0 (rescued 0), max iterations 6 |
| iir | SIN-1000Hz--40dBFS | 0.402 | -138.5 dB | PASS | processBlock 0.066 s for 0.200 s of audio: 32.8 % of one core, real-time factor 3.05
latency 4 samples, Newton failures 0 (rescued 0), max iterations 7 |
| iir | SIN-1000Hz--20dBFS | 0.932 | -138.5 dB | PASS | processBlock 0.070 s for 0.200 s of audio: 34.9 % of one core, real-time factor 2.87
latency 4 samples, Newton failures 0 (rescued 0), max iterations 10 |
| iir | SIN-1000Hz--6dBFS | 1.087 | -138.5 dB | PASS | processBlock 0.076 s for 0.200 s of audio: 37.8 % of one core, real-time factor 2.65
latency 4 samples, Newton failures 0 (rescued 0), max iterations 19 |
| iir | SIN-5000Hz--40dBFS | 0.987 | -138.5 dB | PASS | processBlock 0.090 s for 0.200 s of audio: 45.0 % of one core, real-time factor 2.22
latency 4 samples, Newton failures 0 (rescued 0), max iterations 16 |
| iir | SIN-5000Hz--20dBFS | 1.048 | -138.5 dB | PASS | processBlock 0.100 s for 0.200 s of audio: 50.1 % of one core, real-time factor 1.99
latency 4 samples, Newton failures 0 (rescued 0), max iterations 18 |
| iir | SIN-5000Hz--6dBFS | 1.032 | -138.5 dB | PASS | processBlock 0.574 s for 0.200 s of audio: 286.8 % of one core, real-time factor 0.35
latency 4 samples, Newton failures 3955 (rescued 58), max iterations 32 |
| iir | DI-GTR L | 0.950 | -138.5 dB | PASS | processBlock 1.230 s for 5.000 s of audio: 24.6 % of one core, real-time factor 4.07
latency 4 samples, Newton failures 0 (rescued 0), max iterations 18 |
| iir | DI-GTR LL | 0.955 | -138.5 dB | PASS | processBlock 1.175 s for 5.000 s of audio: 23.5 % of one core, real-time factor 4.25
latency 4 samples, Newton failures 0 (rescued 0), max iterations 19 |
| iir | DI-GTR R | 1.046 | -138.5 dB | PASS | processBlock 1.192 s for 5.000 s of audio: 23.8 % of one core, real-time factor 4.19
latency 4 samples, Newton failures 0 (rescued 0), max iterations 18 |
| iir | DI-GTR RR | 1.049 | -138.5 dB | PASS | processBlock 1.230 s for 5.000 s of audio: 24.6 % of one core, real-time factor 4.07
latency 4 samples, Newton failures 0 (rescued 0), max iterations 23 |
