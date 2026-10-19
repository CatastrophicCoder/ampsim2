# M5 wrapper null test: plugin vs ampwav at 96 kHz with the same resampling

- Date: 2026-10-19, commit `8986f9f (dirty)`; host rate 48000 Hz, 2x, Power Amp Rate full, exact J4
- Plugin: `amp2203_render plugin` (the Amp2203Processor offline). Reference: the processor's oversampler up stage, `ampwav --block full --units fs --j4 exact` at 96000 Hz, the down stage. Default knobs and levels.
- Tolerance (SPEC §7 M5): ESR ≤ -60 dB.

PASS 26 · FAIL 0

| Filter | Signal | Peak out [FS] | ESR | Status | Render log |
|---|---|---|---|---|---|
| fir | SIN-100Hz--40dBFS | 0.145 | -138.5 dB | PASS | processBlock 0.036 s for 0.200 s of audio: 17.8 % of one core, real-time factor 5.62
latency 49 samples, Newton failures 0 (rescued 0), max iterations 2 |
| fir | SIN-100Hz--20dBFS | 0.870 | -138.5 dB | PASS | processBlock 0.039 s for 0.200 s of audio: 19.7 % of one core, real-time factor 5.08
latency 49 samples, Newton failures 0 (rescued 0), max iterations 5 |
| fir | SIN-100Hz--6dBFS | 0.915 | -138.5 dB | PASS | processBlock 0.038 s for 0.200 s of audio: 18.8 % of one core, real-time factor 5.32
latency 49 samples, Newton failures 0 (rescued 0), max iterations 6 |
| fir | SIN-1000Hz--40dBFS | 0.383 | -138.5 dB | PASS | processBlock 0.052 s for 0.200 s of audio: 25.8 % of one core, real-time factor 3.88
latency 49 samples, Newton failures 0 (rescued 0), max iterations 7 |
| fir | SIN-1000Hz--20dBFS | 0.850 | -138.5 dB | PASS | processBlock 0.060 s for 0.200 s of audio: 30.1 % of one core, real-time factor 3.32
latency 49 samples, Newton failures 0 (rescued 0), max iterations 7 |
| fir | SIN-1000Hz--6dBFS | 0.918 | -138.5 dB | PASS | processBlock 0.056 s for 0.200 s of audio: 28.0 % of one core, real-time factor 3.57
latency 49 samples, Newton failures 0 (rescued 0), max iterations 13 |
| fir | SIN-5000Hz--40dBFS | 0.857 | -138.5 dB | PASS | processBlock 0.079 s for 0.200 s of audio: 39.3 % of one core, real-time factor 2.54
latency 49 samples, Newton failures 0 (rescued 0), max iterations 15 |
| fir | SIN-5000Hz--20dBFS | 0.891 | -138.5 dB | PASS | processBlock 0.078 s for 0.200 s of audio: 38.8 % of one core, real-time factor 2.57
latency 49 samples, Newton failures 0 (rescued 0), max iterations 17 |
| fir | SIN-5000Hz--6dBFS | 0.871 | -138.5 dB | PASS | processBlock 0.090 s for 0.200 s of audio: 45.2 % of one core, real-time factor 2.21
latency 49 samples, Newton failures 0 (rescued 0), max iterations 35 |
| fir | DI-GTR L | 0.984 | -138.5 dB | PASS | processBlock 0.926 s for 5.000 s of audio: 18.5 % of one core, real-time factor 5.40
latency 49 samples, Newton failures 0 (rescued 0), max iterations 13 |
| fir | DI-GTR LL | 0.861 | -138.5 dB | PASS | processBlock 0.924 s for 5.000 s of audio: 18.5 % of one core, real-time factor 5.41
latency 49 samples, Newton failures 0 (rescued 0), max iterations 15 |
| fir | DI-GTR R | 0.944 | -138.5 dB | PASS | processBlock 0.932 s for 5.000 s of audio: 18.6 % of one core, real-time factor 5.37
latency 49 samples, Newton failures 0 (rescued 0), max iterations 16 |
| fir | DI-GTR RR | 0.990 | -138.5 dB | PASS | processBlock 0.917 s for 5.000 s of audio: 18.3 % of one core, real-time factor 5.46
latency 49 samples, Newton failures 0 (rescued 0), max iterations 14 |
| iir | SIN-100Hz--40dBFS | 0.145 | -138.5 dB | PASS | processBlock 0.030 s for 0.200 s of audio: 15.0 % of one core, real-time factor 6.68
latency 4 samples, Newton failures 0 (rescued 0), max iterations 1 |
| iir | SIN-100Hz--20dBFS | 0.870 | -138.5 dB | PASS | processBlock 0.037 s for 0.200 s of audio: 18.3 % of one core, real-time factor 5.48
latency 4 samples, Newton failures 0 (rescued 0), max iterations 4 |
| iir | SIN-100Hz--6dBFS | 0.915 | -138.5 dB | PASS | processBlock 0.046 s for 0.200 s of audio: 22.8 % of one core, real-time factor 4.38
latency 4 samples, Newton failures 0 (rescued 0), max iterations 4 |
| iir | SIN-1000Hz--40dBFS | 0.402 | -138.5 dB | PASS | processBlock 0.054 s for 0.200 s of audio: 26.9 % of one core, real-time factor 3.71
latency 4 samples, Newton failures 0 (rescued 0), max iterations 7 |
| iir | SIN-1000Hz--20dBFS | 0.932 | -138.5 dB | PASS | processBlock 0.047 s for 0.200 s of audio: 23.7 % of one core, real-time factor 4.22
latency 4 samples, Newton failures 0 (rescued 0), max iterations 7 |
| iir | SIN-1000Hz--6dBFS | 1.087 | -138.5 dB | PASS | processBlock 0.051 s for 0.200 s of audio: 25.5 % of one core, real-time factor 3.92
latency 4 samples, Newton failures 0 (rescued 0), max iterations 19 |
| iir | SIN-5000Hz--40dBFS | 0.987 | -138.5 dB | PASS | processBlock 0.066 s for 0.200 s of audio: 32.8 % of one core, real-time factor 3.05
latency 4 samples, Newton failures 0 (rescued 0), max iterations 8 |
| iir | SIN-5000Hz--20dBFS | 1.048 | -138.5 dB | PASS | processBlock 0.091 s for 0.200 s of audio: 45.6 % of one core, real-time factor 2.19
latency 4 samples, Newton failures 0 (rescued 0), max iterations 17 |
| iir | SIN-5000Hz--6dBFS | 1.040 | -138.5 dB | PASS | processBlock 0.091 s for 0.200 s of audio: 45.5 % of one core, real-time factor 2.20
latency 4 samples, Newton failures 0 (rescued 0), max iterations 34 |
| iir | DI-GTR L | 0.950 | -138.5 dB | PASS | processBlock 0.894 s for 5.000 s of audio: 17.9 % of one core, real-time factor 5.59
latency 4 samples, Newton failures 0 (rescued 0), max iterations 14 |
| iir | DI-GTR LL | 0.955 | -138.5 dB | PASS | processBlock 0.880 s for 5.000 s of audio: 17.6 % of one core, real-time factor 5.68
latency 4 samples, Newton failures 0 (rescued 0), max iterations 15 |
| iir | DI-GTR R | 1.046 | -138.5 dB | PASS | processBlock 0.878 s for 5.000 s of audio: 17.6 % of one core, real-time factor 5.69
latency 4 samples, Newton failures 0 (rescued 0), max iterations 16 |
| iir | DI-GTR RR | 1.049 | -138.5 dB | PASS | processBlock 0.884 s for 5.000 s of audio: 17.7 % of one core, real-time factor 5.66
latency 4 samples, Newton failures 0 (rescued 0), max iterations 15 |
