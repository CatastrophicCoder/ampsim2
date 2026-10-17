# M5 wrapper null test: plugin vs ampwav at 96 kHz with the same resampling

- Date: 2026-10-15, commit `476d21f (dirty)`; host rate 48000 Hz, 2x, Power Amp Rate half, exact J4
- Plugin: `amp2203_render plugin` (the Amp2203Processor offline). Reference: the processor's oversampler up stage, `ampwav --block full --units fs --j4 exact` at 96000 Hz, the down stage. Default knobs and levels.
- Tolerance (SPEC §7 M5): ESR ≤ -60 dB.

PASS 26 · FAIL 0

| Filter | Signal | Peak out [FS] | ESR | Status | Render log |
|---|---|---|---|---|---|
| fir | SIN-100Hz--40dBFS | 0.094 | -138.5 dB | PASS | processBlock 0.031 s for 0.200 s of audio: 15.6 % of one core, real-time factor 6.42
latency 58 samples, Newton failures 0 (rescued 0), max iterations 3 |
| fir | SIN-100Hz--20dBFS | 0.667 | -138.5 dB | PASS | processBlock 0.031 s for 0.200 s of audio: 15.4 % of one core, real-time factor 6.48
latency 58 samples, Newton failures 0 (rescued 0), max iterations 4 |
| fir | SIN-100Hz--6dBFS | 0.773 | -138.5 dB | PASS | processBlock 0.036 s for 0.200 s of audio: 18.2 % of one core, real-time factor 5.51
latency 58 samples, Newton failures 0 (rescued 0), max iterations 9 |
| fir | SIN-1000Hz--40dBFS | 0.289 | -138.5 dB | PASS | processBlock 0.048 s for 0.200 s of audio: 24.0 % of one core, real-time factor 4.17
latency 58 samples, Newton failures 0 (rescued 0), max iterations 6 |
| fir | SIN-1000Hz--20dBFS | 0.602 | -138.5 dB | PASS | processBlock 0.051 s for 0.200 s of audio: 25.6 % of one core, real-time factor 3.90
latency 58 samples, Newton failures 0 (rescued 0), max iterations 9 |
| fir | SIN-1000Hz--6dBFS | 0.659 | -138.5 dB | PASS | processBlock 0.056 s for 0.200 s of audio: 28.1 % of one core, real-time factor 3.56
latency 58 samples, Newton failures 0 (rescued 0), max iterations 12 |
| fir | SIN-5000Hz--40dBFS | 0.627 | -138.5 dB | PASS | processBlock 0.067 s for 0.200 s of audio: 33.7 % of one core, real-time factor 2.97
latency 58 samples, Newton failures 0 (rescued 0), max iterations 19 |
| fir | SIN-5000Hz--20dBFS | 0.646 | -138.5 dB | PASS | processBlock 0.074 s for 0.200 s of audio: 36.9 % of one core, real-time factor 2.71
latency 58 samples, Newton failures 0 (rescued 0), max iterations 19 |
| fir | SIN-5000Hz--6dBFS | 0.666 | -138.5 dB | PASS | processBlock 0.191 s for 0.200 s of audio: 95.5 % of one core, real-time factor 1.05
latency 58 samples, Newton failures 879 (rescued 77), max iterations 32 |
| fir | DI-GTR L | 0.686 | -138.5 dB | PASS | processBlock 0.873 s for 5.000 s of audio: 17.5 % of one core, real-time factor 5.73
latency 58 samples, Newton failures 0 (rescued 0), max iterations 20 |
| fir | DI-GTR LL | 0.650 | -138.5 dB | PASS | processBlock 0.851 s for 5.000 s of audio: 17.0 % of one core, real-time factor 5.88
latency 58 samples, Newton failures 0 (rescued 0), max iterations 17 |
| fir | DI-GTR R | 0.748 | -138.5 dB | PASS | processBlock 0.855 s for 5.000 s of audio: 17.1 % of one core, real-time factor 5.84
latency 58 samples, Newton failures 0 (rescued 0), max iterations 20 |
| fir | DI-GTR RR | 0.708 | -138.5 dB | PASS | processBlock 0.872 s for 5.000 s of audio: 17.4 % of one core, real-time factor 5.73
latency 58 samples, Newton failures 0 (rescued 0), max iterations 18 |
| iir | SIN-100Hz--40dBFS | 0.094 | -138.5 dB | PASS | processBlock 0.028 s for 0.200 s of audio: 13.8 % of one core, real-time factor 7.25
latency 13 samples, Newton failures 0 (rescued 0), max iterations 2 |
| iir | SIN-100Hz--20dBFS | 0.667 | -138.5 dB | PASS | processBlock 0.031 s for 0.200 s of audio: 15.4 % of one core, real-time factor 6.51
latency 13 samples, Newton failures 0 (rescued 0), max iterations 6 |
| iir | SIN-100Hz--6dBFS | 0.773 | -138.5 dB | PASS | processBlock 0.032 s for 0.200 s of audio: 16.0 % of one core, real-time factor 6.25
latency 13 samples, Newton failures 0 (rescued 0), max iterations 6 |
| iir | SIN-1000Hz--40dBFS | 0.315 | -138.5 dB | PASS | processBlock 0.045 s for 0.200 s of audio: 22.3 % of one core, real-time factor 4.48
latency 13 samples, Newton failures 0 (rescued 0), max iterations 7 |
| iir | SIN-1000Hz--20dBFS | 0.672 | -138.5 dB | PASS | processBlock 0.048 s for 0.200 s of audio: 23.9 % of one core, real-time factor 4.18
latency 13 samples, Newton failures 0 (rescued 0), max iterations 9 |
| iir | SIN-1000Hz--6dBFS | 0.725 | -138.5 dB | PASS | processBlock 0.053 s for 0.200 s of audio: 26.7 % of one core, real-time factor 3.74
latency 13 samples, Newton failures 0 (rescued 0), max iterations 22 |
| iir | SIN-5000Hz--40dBFS | 0.666 | -138.5 dB | PASS | processBlock 0.067 s for 0.200 s of audio: 33.5 % of one core, real-time factor 2.99
latency 13 samples, Newton failures 0 (rescued 0), max iterations 16 |
| iir | SIN-5000Hz--20dBFS | 0.702 | -138.5 dB | PASS | processBlock 0.076 s for 0.200 s of audio: 38.2 % of one core, real-time factor 2.62
latency 13 samples, Newton failures 0 (rescued 0), max iterations 22 |
| iir | SIN-5000Hz--6dBFS | 0.710 | -138.5 dB | PASS | processBlock 0.106 s for 0.200 s of audio: 52.9 % of one core, real-time factor 1.89
latency 13 samples, Newton failures 253 (rescued 185), max iterations 32 |
| iir | DI-GTR L | 0.741 | -138.5 dB | PASS | processBlock 0.870 s for 5.000 s of audio: 17.4 % of one core, real-time factor 5.75
latency 13 samples, Newton failures 0 (rescued 0), max iterations 18 |
| iir | DI-GTR LL | 0.683 | -138.5 dB | PASS | processBlock 0.843 s for 5.000 s of audio: 16.9 % of one core, real-time factor 5.93
latency 13 samples, Newton failures 0 (rescued 0), max iterations 22 |
| iir | DI-GTR R | 0.775 | -138.5 dB | PASS | processBlock 0.844 s for 5.000 s of audio: 16.9 % of one core, real-time factor 5.93
latency 13 samples, Newton failures 0 (rescued 0), max iterations 18 |
| iir | DI-GTR RR | 0.740 | -138.5 dB | PASS | processBlock 0.861 s for 5.000 s of audio: 17.2 % of one core, real-time factor 5.81
latency 13 samples, Newton failures 0 (rescued 0), max iterations 21 |
