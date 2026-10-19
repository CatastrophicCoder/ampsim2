# M5 wrapper null test: plugin vs ampwav at 96 kHz with the same resampling

- Date: 2026-10-19, commit `ecf9042 (dirty)`; host rate 48000 Hz, 2x, Power Amp Rate half, exact J4
- Plugin: `amp2203_render plugin` (the Amp2203Processor offline). Reference: the processor's oversampler up stage, `ampwav --block full --units fs --j4 exact` at 96000 Hz, the down stage. Default knobs and levels.
- Tolerance (SPEC §7 M5): ESR ≤ -60 dB.

PASS 26 · FAIL 0

| Filter | Signal | Peak out [FS] | ESR | Status | Render log |
|---|---|---|---|---|---|
| fir | SIN-100Hz--40dBFS | 0.145 | -138.5 dB | PASS | processBlock 0.022 s for 0.200 s of audio: 11.2 % of one core, real-time factor 8.94
latency 58 samples, Newton failures 0 (rescued 0), max iterations 2 |
| fir | SIN-100Hz--20dBFS | 0.870 | -138.5 dB | PASS | processBlock 0.034 s for 0.200 s of audio: 17.2 % of one core, real-time factor 5.81
latency 58 samples, Newton failures 0 (rescued 0), max iterations 5 |
| fir | SIN-100Hz--6dBFS | 0.915 | -138.5 dB | PASS | processBlock 0.027 s for 0.200 s of audio: 13.7 % of one core, real-time factor 7.29
latency 58 samples, Newton failures 0 (rescued 0), max iterations 6 |
| fir | SIN-1000Hz--40dBFS | 0.376 | -138.5 dB | PASS | processBlock 0.040 s for 0.200 s of audio: 20.0 % of one core, real-time factor 4.99
latency 58 samples, Newton failures 0 (rescued 0), max iterations 7 |
| fir | SIN-1000Hz--20dBFS | 0.791 | -138.5 dB | PASS | processBlock 0.051 s for 0.200 s of audio: 25.7 % of one core, real-time factor 3.90
latency 58 samples, Newton failures 0 (rescued 0), max iterations 7 |
| fir | SIN-1000Hz--6dBFS | 0.994 | -138.5 dB | PASS | processBlock 0.036 s for 0.200 s of audio: 18.1 % of one core, real-time factor 5.53
latency 58 samples, Newton failures 0 (rescued 0), max iterations 13 |
| fir | SIN-5000Hz--40dBFS | 0.868 | -138.5 dB | PASS | processBlock 0.061 s for 0.200 s of audio: 30.7 % of one core, real-time factor 3.25
latency 58 samples, Newton failures 0 (rescued 0), max iterations 15 |
| fir | SIN-5000Hz--20dBFS | 0.923 | -138.5 dB | PASS | processBlock 0.072 s for 0.200 s of audio: 35.8 % of one core, real-time factor 2.79
latency 58 samples, Newton failures 0 (rescued 0), max iterations 17 |
| fir | SIN-5000Hz--6dBFS | 0.977 | -138.5 dB | PASS | processBlock 0.082 s for 0.200 s of audio: 40.8 % of one core, real-time factor 2.45
latency 58 samples, Newton failures 0 (rescued 0), max iterations 35 |
| fir | DI-GTR L | 0.874 | -138.5 dB | PASS | processBlock 0.715 s for 5.000 s of audio: 14.3 % of one core, real-time factor 6.99
latency 58 samples, Newton failures 0 (rescued 0), max iterations 13 |
| fir | DI-GTR LL | 0.834 | -138.5 dB | PASS | processBlock 0.686 s for 5.000 s of audio: 13.7 % of one core, real-time factor 7.29
latency 58 samples, Newton failures 0 (rescued 0), max iterations 15 |
| fir | DI-GTR R | 0.908 | -138.5 dB | PASS | processBlock 0.694 s for 5.000 s of audio: 13.9 % of one core, real-time factor 7.20
latency 58 samples, Newton failures 0 (rescued 0), max iterations 16 |
| fir | DI-GTR RR | 0.912 | -138.5 dB | PASS | processBlock 0.709 s for 5.000 s of audio: 14.2 % of one core, real-time factor 7.05
latency 58 samples, Newton failures 0 (rescued 0), max iterations 14 |
| iir | SIN-100Hz--40dBFS | 0.145 | -138.5 dB | PASS | processBlock 0.022 s for 0.200 s of audio: 11.1 % of one core, real-time factor 9.04
latency 13 samples, Newton failures 0 (rescued 0), max iterations 2 |
| iir | SIN-100Hz--20dBFS | 0.870 | -138.5 dB | PASS | processBlock 0.031 s for 0.200 s of audio: 15.3 % of one core, real-time factor 6.55
latency 13 samples, Newton failures 0 (rescued 0), max iterations 4 |
| iir | SIN-100Hz--6dBFS | 0.915 | -138.5 dB | PASS | processBlock 0.030 s for 0.200 s of audio: 15.2 % of one core, real-time factor 6.58
latency 13 samples, Newton failures 0 (rescued 0), max iterations 4 |
| iir | SIN-1000Hz--40dBFS | 0.394 | -138.5 dB | PASS | processBlock 0.039 s for 0.200 s of audio: 19.6 % of one core, real-time factor 5.11
latency 13 samples, Newton failures 0 (rescued 0), max iterations 7 |
| iir | SIN-1000Hz--20dBFS | 0.863 | -138.5 dB | PASS | processBlock 0.038 s for 0.200 s of audio: 19.0 % of one core, real-time factor 5.26
latency 13 samples, Newton failures 0 (rescued 0), max iterations 7 |
| iir | SIN-1000Hz--6dBFS | 0.997 | -138.5 dB | PASS | processBlock 0.039 s for 0.200 s of audio: 19.6 % of one core, real-time factor 5.10
latency 13 samples, Newton failures 0 (rescued 0), max iterations 19 |
| iir | SIN-5000Hz--40dBFS | 0.880 | -138.5 dB | PASS | processBlock 0.052 s for 0.200 s of audio: 26.1 % of one core, real-time factor 3.84
latency 13 samples, Newton failures 0 (rescued 0), max iterations 8 |
| iir | SIN-5000Hz--20dBFS | 1.020 | -138.5 dB | PASS | processBlock 0.079 s for 0.200 s of audio: 39.3 % of one core, real-time factor 2.55
latency 13 samples, Newton failures 0 (rescued 0), max iterations 17 |
| iir | SIN-5000Hz--6dBFS | 0.991 | -138.5 dB | PASS | processBlock 0.077 s for 0.200 s of audio: 38.3 % of one core, real-time factor 2.61
latency 13 samples, Newton failures 0 (rescued 0), max iterations 34 |
| iir | DI-GTR L | 0.998 | -138.5 dB | PASS | processBlock 0.669 s for 5.000 s of audio: 13.4 % of one core, real-time factor 7.47
latency 13 samples, Newton failures 0 (rescued 0), max iterations 14 |
| iir | DI-GTR LL | 0.889 | -138.5 dB | PASS | processBlock 0.650 s for 5.000 s of audio: 13.0 % of one core, real-time factor 7.70
latency 13 samples, Newton failures 0 (rescued 0), max iterations 15 |
| iir | DI-GTR R | 0.983 | -138.5 dB | PASS | processBlock 0.657 s for 5.000 s of audio: 13.1 % of one core, real-time factor 7.61
latency 13 samples, Newton failures 0 (rescued 0), max iterations 16 |
| iir | DI-GTR RR | 0.989 | -138.5 dB | PASS | processBlock 0.701 s for 5.000 s of audio: 14.0 % of one core, real-time factor 7.14
latency 13 samples, Newton failures 0 (rescued 0), max iterations 15 |
