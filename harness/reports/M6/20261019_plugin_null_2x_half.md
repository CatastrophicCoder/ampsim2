# M5 wrapper null test: plugin vs ampwav at 96 kHz with the same resampling

- Date: 2026-10-19, commit `8986f9f`; host rate 48000 Hz, 2x, Power Amp Rate half, exact J4
- Plugin: `amp2203_render plugin` (the Amp2203Processor offline). Reference: the processor's oversampler up stage, `ampwav --block full --units fs --j4 exact` at 96000 Hz, the down stage. Default knobs and levels.
- Tolerance (SPEC §7 M5): ESR ≤ -60 dB.

PASS 26 · FAIL 0

| Filter | Signal | Peak out [FS] | ESR | Status | Render log |
|---|---|---|---|---|---|
| fir | SIN-100Hz--40dBFS | 0.145 | -138.5 dB | PASS | processBlock 0.034 s for 0.200 s of audio: 17.0 % of one core, real-time factor 5.87
latency 58 samples, Newton failures 0 (rescued 0), max iterations 2 |
| fir | SIN-100Hz--20dBFS | 0.870 | -138.5 dB | PASS | processBlock 0.047 s for 0.200 s of audio: 23.6 % of one core, real-time factor 4.24
latency 58 samples, Newton failures 0 (rescued 0), max iterations 5 |
| fir | SIN-100Hz--6dBFS | 0.915 | -138.5 dB | PASS | processBlock 0.036 s for 0.200 s of audio: 17.9 % of one core, real-time factor 5.58
latency 58 samples, Newton failures 0 (rescued 0), max iterations 6 |
| fir | SIN-1000Hz--40dBFS | 0.376 | -138.5 dB | PASS | processBlock 0.048 s for 0.200 s of audio: 24.0 % of one core, real-time factor 4.16
latency 58 samples, Newton failures 0 (rescued 0), max iterations 7 |
| fir | SIN-1000Hz--20dBFS | 0.791 | -138.5 dB | PASS | processBlock 0.039 s for 0.200 s of audio: 19.3 % of one core, real-time factor 5.18
latency 58 samples, Newton failures 0 (rescued 0), max iterations 7 |
| fir | SIN-1000Hz--6dBFS | 0.994 | -138.5 dB | PASS | processBlock 0.044 s for 0.200 s of audio: 21.8 % of one core, real-time factor 4.58
latency 58 samples, Newton failures 0 (rescued 0), max iterations 13 |
| fir | SIN-5000Hz--40dBFS | 0.868 | -138.5 dB | PASS | processBlock 0.055 s for 0.200 s of audio: 27.7 % of one core, real-time factor 3.60
latency 58 samples, Newton failures 0 (rescued 0), max iterations 15 |
| fir | SIN-5000Hz--20dBFS | 0.923 | -138.5 dB | PASS | processBlock 0.077 s for 0.200 s of audio: 38.7 % of one core, real-time factor 2.58
latency 58 samples, Newton failures 0 (rescued 0), max iterations 17 |
| fir | SIN-5000Hz--6dBFS | 0.977 | -138.5 dB | PASS | processBlock 0.077 s for 0.200 s of audio: 38.5 % of one core, real-time factor 2.60
latency 58 samples, Newton failures 0 (rescued 0), max iterations 35 |
| fir | DI-GTR L | 0.874 | -138.5 dB | PASS | processBlock 0.671 s for 5.000 s of audio: 13.4 % of one core, real-time factor 7.46
latency 58 samples, Newton failures 0 (rescued 0), max iterations 13 |
| fir | DI-GTR LL | 0.834 | -138.5 dB | PASS | processBlock 0.674 s for 5.000 s of audio: 13.5 % of one core, real-time factor 7.41
latency 58 samples, Newton failures 0 (rescued 0), max iterations 15 |
| fir | DI-GTR R | 0.908 | -138.5 dB | PASS | processBlock 0.675 s for 5.000 s of audio: 13.5 % of one core, real-time factor 7.40
latency 58 samples, Newton failures 0 (rescued 0), max iterations 16 |
| fir | DI-GTR RR | 0.912 | -138.5 dB | PASS | processBlock 0.669 s for 5.000 s of audio: 13.4 % of one core, real-time factor 7.48
latency 58 samples, Newton failures 0 (rescued 0), max iterations 14 |
| iir | SIN-100Hz--40dBFS | 0.145 | -138.5 dB | PASS | processBlock 0.032 s for 0.200 s of audio: 16.1 % of one core, real-time factor 6.21
latency 13 samples, Newton failures 0 (rescued 0), max iterations 2 |
| iir | SIN-100Hz--20dBFS | 0.870 | -138.5 dB | PASS | processBlock 0.029 s for 0.200 s of audio: 14.5 % of one core, real-time factor 6.89
latency 13 samples, Newton failures 0 (rescued 0), max iterations 4 |
| iir | SIN-100Hz--6dBFS | 0.915 | -138.5 dB | PASS | processBlock 0.030 s for 0.200 s of audio: 14.8 % of one core, real-time factor 6.76
latency 13 samples, Newton failures 0 (rescued 0), max iterations 4 |
| iir | SIN-1000Hz--40dBFS | 0.394 | -138.5 dB | PASS | processBlock 0.035 s for 0.200 s of audio: 17.4 % of one core, real-time factor 5.74
latency 13 samples, Newton failures 0 (rescued 0), max iterations 7 |
| iir | SIN-1000Hz--20dBFS | 0.863 | -138.5 dB | PASS | processBlock 0.039 s for 0.200 s of audio: 19.7 % of one core, real-time factor 5.09
latency 13 samples, Newton failures 0 (rescued 0), max iterations 7 |
| iir | SIN-1000Hz--6dBFS | 0.997 | -138.5 dB | PASS | processBlock 0.041 s for 0.200 s of audio: 20.4 % of one core, real-time factor 4.91
latency 13 samples, Newton failures 0 (rescued 0), max iterations 19 |
| iir | SIN-5000Hz--40dBFS | 0.880 | -138.5 dB | PASS | processBlock 0.055 s for 0.200 s of audio: 27.3 % of one core, real-time factor 3.66
latency 13 samples, Newton failures 0 (rescued 0), max iterations 8 |
| iir | SIN-5000Hz--20dBFS | 1.020 | -138.5 dB | PASS | processBlock 0.072 s for 0.200 s of audio: 36.1 % of one core, real-time factor 2.77
latency 13 samples, Newton failures 0 (rescued 0), max iterations 17 |
| iir | SIN-5000Hz--6dBFS | 0.991 | -138.5 dB | PASS | processBlock 0.081 s for 0.200 s of audio: 40.3 % of one core, real-time factor 2.48
latency 13 samples, Newton failures 0 (rescued 0), max iterations 34 |
| iir | DI-GTR L | 0.998 | -138.5 dB | PASS | processBlock 0.654 s for 5.000 s of audio: 13.1 % of one core, real-time factor 7.64
latency 13 samples, Newton failures 0 (rescued 0), max iterations 14 |
| iir | DI-GTR LL | 0.889 | -138.5 dB | PASS | processBlock 0.647 s for 5.000 s of audio: 12.9 % of one core, real-time factor 7.72
latency 13 samples, Newton failures 0 (rescued 0), max iterations 15 |
| iir | DI-GTR R | 0.983 | -138.5 dB | PASS | processBlock 0.644 s for 5.000 s of audio: 12.9 % of one core, real-time factor 7.76
latency 13 samples, Newton failures 0 (rescued 0), max iterations 16 |
| iir | DI-GTR RR | 0.989 | -138.5 dB | PASS | processBlock 0.686 s for 5.000 s of audio: 13.7 % of one core, real-time factor 7.29
latency 13 samples, Newton failures 0 (rescued 0), max iterations 15 |
