# M5 wrapper null test: plugin vs ampwav at 96 kHz with the same resampling

- Date: 2026-10-19, commit `ecf9042 (dirty)`; host rate 48000 Hz, 2x, Power Amp Rate full, exact J4
- Plugin: `amp2203_render plugin` (the Amp2203Processor offline). Reference: the processor's oversampler up stage, `ampwav --block full --units fs --j4 exact` at 96000 Hz, the down stage. Default knobs and levels.
- Tolerance (SPEC §7 M5): ESR ≤ -60 dB.

PASS 26 · FAIL 0

| Filter | Signal | Peak out [FS] | ESR | Status | Render log |
|---|---|---|---|---|---|
| fir | SIN-100Hz--40dBFS | 0.145 | -138.5 dB | PASS | processBlock 0.031 s for 0.200 s of audio: 15.3 % of one core, real-time factor 6.54
latency 49 samples, Newton failures 0 (rescued 0), max iterations 2 |
| fir | SIN-100Hz--20dBFS | 0.870 | -138.5 dB | PASS | processBlock 0.043 s for 0.200 s of audio: 21.5 % of one core, real-time factor 4.64
latency 49 samples, Newton failures 0 (rescued 0), max iterations 5 |
| fir | SIN-100Hz--6dBFS | 0.915 | -138.5 dB | PASS | processBlock 0.050 s for 0.200 s of audio: 24.9 % of one core, real-time factor 4.02
latency 49 samples, Newton failures 0 (rescued 0), max iterations 6 |
| fir | SIN-1000Hz--40dBFS | 0.383 | -138.5 dB | PASS | processBlock 0.049 s for 0.200 s of audio: 24.6 % of one core, real-time factor 4.07
latency 49 samples, Newton failures 0 (rescued 0), max iterations 7 |
| fir | SIN-1000Hz--20dBFS | 0.850 | -138.5 dB | PASS | processBlock 0.059 s for 0.200 s of audio: 29.7 % of one core, real-time factor 3.37
latency 49 samples, Newton failures 0 (rescued 0), max iterations 7 |
| fir | SIN-1000Hz--6dBFS | 0.918 | -138.5 dB | PASS | processBlock 0.055 s for 0.200 s of audio: 27.6 % of one core, real-time factor 3.62
latency 49 samples, Newton failures 0 (rescued 0), max iterations 13 |
| fir | SIN-5000Hz--40dBFS | 0.857 | -138.5 dB | PASS | processBlock 0.074 s for 0.200 s of audio: 36.9 % of one core, real-time factor 2.71
latency 49 samples, Newton failures 0 (rescued 0), max iterations 15 |
| fir | SIN-5000Hz--20dBFS | 0.891 | -138.5 dB | PASS | processBlock 0.084 s for 0.200 s of audio: 41.9 % of one core, real-time factor 2.39
latency 49 samples, Newton failures 0 (rescued 0), max iterations 17 |
| fir | SIN-5000Hz--6dBFS | 0.871 | -138.5 dB | PASS | processBlock 0.096 s for 0.200 s of audio: 47.9 % of one core, real-time factor 2.09
latency 49 samples, Newton failures 0 (rescued 0), max iterations 35 |
| fir | DI-GTR L | 0.984 | -138.5 dB | PASS | processBlock 0.925 s for 5.000 s of audio: 18.5 % of one core, real-time factor 5.40
latency 49 samples, Newton failures 0 (rescued 0), max iterations 13 |
| fir | DI-GTR LL | 0.861 | -138.5 dB | PASS | processBlock 0.928 s for 5.000 s of audio: 18.6 % of one core, real-time factor 5.39
latency 49 samples, Newton failures 0 (rescued 0), max iterations 15 |
| fir | DI-GTR R | 0.944 | -138.5 dB | PASS | processBlock 0.927 s for 5.000 s of audio: 18.5 % of one core, real-time factor 5.40
latency 49 samples, Newton failures 0 (rescued 0), max iterations 16 |
| fir | DI-GTR RR | 0.990 | -138.5 dB | PASS | processBlock 0.928 s for 5.000 s of audio: 18.6 % of one core, real-time factor 5.39
latency 49 samples, Newton failures 0 (rescued 0), max iterations 14 |
| iir | SIN-100Hz--40dBFS | 0.145 | -138.5 dB | PASS | processBlock 0.030 s for 0.200 s of audio: 15.2 % of one core, real-time factor 6.59
latency 4 samples, Newton failures 0 (rescued 0), max iterations 1 |
| iir | SIN-100Hz--20dBFS | 0.870 | -138.5 dB | PASS | processBlock 0.041 s for 0.200 s of audio: 20.5 % of one core, real-time factor 4.88
latency 4 samples, Newton failures 0 (rescued 0), max iterations 4 |
| iir | SIN-100Hz--6dBFS | 0.915 | -138.5 dB | PASS | processBlock 0.038 s for 0.200 s of audio: 18.9 % of one core, real-time factor 5.30
latency 4 samples, Newton failures 0 (rescued 0), max iterations 4 |
| iir | SIN-1000Hz--40dBFS | 0.402 | -138.5 dB | PASS | processBlock 0.053 s for 0.200 s of audio: 26.5 % of one core, real-time factor 3.77
latency 4 samples, Newton failures 0 (rescued 0), max iterations 7 |
| iir | SIN-1000Hz--20dBFS | 0.932 | -138.5 dB | PASS | processBlock 0.047 s for 0.200 s of audio: 23.5 % of one core, real-time factor 4.25
latency 4 samples, Newton failures 0 (rescued 0), max iterations 7 |
| iir | SIN-1000Hz--6dBFS | 1.087 | -138.5 dB | PASS | processBlock 0.067 s for 0.200 s of audio: 33.4 % of one core, real-time factor 3.00
latency 4 samples, Newton failures 0 (rescued 0), max iterations 19 |
| iir | SIN-5000Hz--40dBFS | 0.987 | -138.5 dB | PASS | processBlock 0.074 s for 0.200 s of audio: 36.8 % of one core, real-time factor 2.72
latency 4 samples, Newton failures 0 (rescued 0), max iterations 8 |
| iir | SIN-5000Hz--20dBFS | 1.048 | -138.5 dB | PASS | processBlock 0.086 s for 0.200 s of audio: 43.0 % of one core, real-time factor 2.33
latency 4 samples, Newton failures 0 (rescued 0), max iterations 17 |
| iir | SIN-5000Hz--6dBFS | 1.040 | -138.5 dB | PASS | processBlock 0.111 s for 0.200 s of audio: 55.5 % of one core, real-time factor 1.80
latency 4 samples, Newton failures 0 (rescued 0), max iterations 34 |
| iir | DI-GTR L | 0.950 | -138.5 dB | PASS | processBlock 0.908 s for 5.000 s of audio: 18.2 % of one core, real-time factor 5.51
latency 4 samples, Newton failures 0 (rescued 0), max iterations 14 |
| iir | DI-GTR LL | 0.955 | -138.5 dB | PASS | processBlock 0.882 s for 5.000 s of audio: 17.6 % of one core, real-time factor 5.67
latency 4 samples, Newton failures 0 (rescued 0), max iterations 15 |
| iir | DI-GTR R | 1.046 | -138.5 dB | PASS | processBlock 0.892 s for 5.000 s of audio: 17.8 % of one core, real-time factor 5.61
latency 4 samples, Newton failures 0 (rescued 0), max iterations 16 |
| iir | DI-GTR RR | 1.049 | -138.5 dB | PASS | processBlock 0.889 s for 5.000 s of audio: 17.8 % of one core, real-time factor 5.62
latency 4 samples, Newton failures 0 (rescued 0), max iterations 15 |
