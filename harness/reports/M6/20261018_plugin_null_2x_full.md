# M5 wrapper null test: plugin vs ampwav at 96 kHz with the same resampling

- Date: 2026-10-18, commit `b193fc7 (dirty)`; host rate 48000 Hz, 2x, Power Amp Rate full, exact J4
- Plugin: `amp2203_render plugin` (the Amp2203Processor offline). Reference: the processor's oversampler up stage, `ampwav --block full --units fs --j4 exact` at 96000 Hz, the down stage. Default knobs and levels.
- Tolerance (SPEC §7 M5): ESR ≤ -60 dB.

PASS 26 · FAIL 0

| Filter | Signal | Peak out [FS] | ESR | Status | Render log |
|---|---|---|---|---|---|
| fir | SIN-100Hz--40dBFS | 0.145 | -138.5 dB | PASS | processBlock 0.059 s for 0.200 s of audio: 29.7 % of one core, real-time factor 3.37
latency 49 samples, Newton failures 0 (rescued 0), max iterations 2 |
| fir | SIN-100Hz--20dBFS | 0.870 | -138.5 dB | PASS | processBlock 0.062 s for 0.200 s of audio: 31.2 % of one core, real-time factor 3.20
latency 49 samples, Newton failures 0 (rescued 0), max iterations 5 |
| fir | SIN-100Hz--6dBFS | 0.915 | -138.5 dB | PASS | processBlock 0.050 s for 0.200 s of audio: 24.8 % of one core, real-time factor 4.03
latency 49 samples, Newton failures 0 (rescued 0), max iterations 10 |
| fir | SIN-1000Hz--40dBFS | 0.383 | -138.5 dB | PASS | processBlock 0.073 s for 0.200 s of audio: 36.4 % of one core, real-time factor 2.75
latency 49 samples, Newton failures 0 (rescued 0), max iterations 7 |
| fir | SIN-1000Hz--20dBFS | 0.850 | -138.5 dB | PASS | processBlock 0.087 s for 0.200 s of audio: 43.6 % of one core, real-time factor 2.29
latency 49 samples, Newton failures 0 (rescued 0), max iterations 12 |
| fir | SIN-1000Hz--6dBFS | 0.918 | -138.5 dB | PASS | processBlock 0.099 s for 0.200 s of audio: 49.3 % of one core, real-time factor 2.03
latency 49 samples, Newton failures 0 (rescued 0), max iterations 13 |
| fir | SIN-5000Hz--40dBFS | 0.857 | -138.5 dB | PASS | processBlock 0.099 s for 0.200 s of audio: 49.7 % of one core, real-time factor 2.01
latency 49 samples, Newton failures 0 (rescued 0), max iterations 17 |
| fir | SIN-5000Hz--20dBFS | 0.891 | -138.5 dB | PASS | processBlock 0.115 s for 0.200 s of audio: 57.5 % of one core, real-time factor 1.74
latency 49 samples, Newton failures 0 (rescued 0), max iterations 19 |
| fir | SIN-5000Hz--6dBFS | 0.871 | -138.5 dB | PASS | processBlock 0.152 s for 0.200 s of audio: 76.2 % of one core, real-time factor 1.31
latency 49 samples, Newton failures 0 (rescued 0), max iterations 53 |
| fir | DI-GTR L | 0.984 | -138.5 dB | PASS | processBlock 1.302 s for 5.000 s of audio: 26.0 % of one core, real-time factor 3.84
latency 49 samples, Newton failures 0 (rescued 0), max iterations 18 |
| fir | DI-GTR LL | 0.861 | -138.5 dB | PASS | processBlock 1.263 s for 5.000 s of audio: 25.3 % of one core, real-time factor 3.96
latency 49 samples, Newton failures 0 (rescued 0), max iterations 17 |
| fir | DI-GTR R | 0.944 | -138.5 dB | PASS | processBlock 1.263 s for 5.000 s of audio: 25.3 % of one core, real-time factor 3.96
latency 49 samples, Newton failures 0 (rescued 0), max iterations 16 |
| fir | DI-GTR RR | 0.990 | -138.5 dB | PASS | processBlock 1.281 s for 5.000 s of audio: 25.6 % of one core, real-time factor 3.90
latency 49 samples, Newton failures 0 (rescued 0), max iterations 19 |
| iir | SIN-100Hz--40dBFS | 0.145 | -138.5 dB | PASS | processBlock 0.053 s for 0.200 s of audio: 26.4 % of one core, real-time factor 3.78
latency 4 samples, Newton failures 0 (rescued 0), max iterations 2 |
| iir | SIN-100Hz--20dBFS | 0.870 | -138.5 dB | PASS | processBlock 0.047 s for 0.200 s of audio: 23.5 % of one core, real-time factor 4.25
latency 4 samples, Newton failures 0 (rescued 0), max iterations 4 |
| iir | SIN-100Hz--6dBFS | 0.915 | -138.5 dB | PASS | processBlock 0.054 s for 0.200 s of audio: 26.9 % of one core, real-time factor 3.72
latency 4 samples, Newton failures 0 (rescued 0), max iterations 6 |
| iir | SIN-1000Hz--40dBFS | 0.402 | -138.5 dB | PASS | processBlock 0.070 s for 0.200 s of audio: 35.2 % of one core, real-time factor 2.84
latency 4 samples, Newton failures 0 (rescued 0), max iterations 7 |
| iir | SIN-1000Hz--20dBFS | 0.932 | -138.5 dB | PASS | processBlock 0.078 s for 0.200 s of audio: 39.0 % of one core, real-time factor 2.56
latency 4 samples, Newton failures 0 (rescued 0), max iterations 10 |
| iir | SIN-1000Hz--6dBFS | 1.087 | -138.5 dB | PASS | processBlock 0.079 s for 0.200 s of audio: 39.7 % of one core, real-time factor 2.52
latency 4 samples, Newton failures 0 (rescued 0), max iterations 19 |
| iir | SIN-5000Hz--40dBFS | 0.987 | -138.5 dB | PASS | processBlock 0.110 s for 0.200 s of audio: 55.2 % of one core, real-time factor 1.81
latency 4 samples, Newton failures 0 (rescued 0), max iterations 16 |
| iir | SIN-5000Hz--20dBFS | 1.048 | -138.5 dB | PASS | processBlock 0.101 s for 0.200 s of audio: 50.7 % of one core, real-time factor 1.97
latency 4 samples, Newton failures 0 (rescued 0), max iterations 18 |
| iir | SIN-5000Hz--6dBFS | 1.040 | -138.5 dB | PASS | processBlock 0.131 s for 0.200 s of audio: 65.6 % of one core, real-time factor 1.52
latency 4 samples, Newton failures 0 (rescued 0), max iterations 50 |
| iir | DI-GTR L | 0.950 | -138.5 dB | PASS | processBlock 1.256 s for 5.000 s of audio: 25.1 % of one core, real-time factor 3.98
latency 4 samples, Newton failures 0 (rescued 0), max iterations 18 |
| iir | DI-GTR LL | 0.955 | -138.5 dB | PASS | processBlock 1.234 s for 5.000 s of audio: 24.7 % of one core, real-time factor 4.05
latency 4 samples, Newton failures 0 (rescued 0), max iterations 19 |
| iir | DI-GTR R | 1.046 | -138.5 dB | PASS | processBlock 1.212 s for 5.000 s of audio: 24.2 % of one core, real-time factor 4.12
latency 4 samples, Newton failures 0 (rescued 0), max iterations 18 |
| iir | DI-GTR RR | 1.049 | -138.5 dB | PASS | processBlock 1.248 s for 5.000 s of audio: 25.0 % of one core, real-time factor 4.01
latency 4 samples, Newton failures 0 (rescued 0), max iterations 23 |
