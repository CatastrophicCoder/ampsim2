# M5 wrapper null test: plugin vs ampwav at 96 kHz with the same resampling

- Date: 2026-10-15, commit `9f01825 (dirty)`; host rate 48000 Hz, 2x, Power Amp Rate half, exact J4
- Plugin: `amp2203_render plugin` (the Amp2203Processor offline). Reference: the processor's oversampler up stage, `ampwav --block full --units fs --j4 exact` at 96000 Hz, the down stage. Default knobs and levels.
- Tolerance (SPEC §7 M5): ESR ≤ -60 dB.

PASS 26 · FAIL 0

| Filter | Signal | Peak out [FS] | ESR | Status | Render log |
|---|---|---|---|---|---|
| fir | SIN-100Hz--40dBFS | 0.111 | -138.5 dB | PASS | processBlock 0.029 s for 0.200 s of audio: 14.7 % of one core, real-time factor 6.79
latency 58 samples, Newton failures 0 (rescued 0), max iterations 3 |
| fir | SIN-100Hz--20dBFS | 0.719 | -138.5 dB | PASS | processBlock 0.032 s for 0.200 s of audio: 16.1 % of one core, real-time factor 6.22
latency 58 samples, Newton failures 0 (rescued 0), max iterations 4 |
| fir | SIN-100Hz--6dBFS | 0.825 | -138.5 dB | PASS | processBlock 0.033 s for 0.200 s of audio: 16.7 % of one core, real-time factor 5.98
latency 58 samples, Newton failures 0 (rescued 0), max iterations 9 |
| fir | SIN-1000Hz--40dBFS | 0.335 | -138.5 dB | PASS | processBlock 0.048 s for 0.200 s of audio: 23.8 % of one core, real-time factor 4.21
latency 58 samples, Newton failures 0 (rescued 0), max iterations 6 |
| fir | SIN-1000Hz--20dBFS | 0.696 | -138.5 dB | PASS | processBlock 0.052 s for 0.200 s of audio: 26.2 % of one core, real-time factor 3.82
latency 58 samples, Newton failures 0 (rescued 0), max iterations 9 |
| fir | SIN-1000Hz--6dBFS | 0.886 | -138.5 dB | PASS | processBlock 0.054 s for 0.200 s of audio: 26.8 % of one core, real-time factor 3.73
latency 58 samples, Newton failures 0 (rescued 0), max iterations 12 |
| fir | SIN-5000Hz--40dBFS | 0.775 | -138.5 dB | PASS | processBlock 0.066 s for 0.200 s of audio: 33.1 % of one core, real-time factor 3.02
latency 58 samples, Newton failures 0 (rescued 0), max iterations 18 |
| fir | SIN-5000Hz--20dBFS | 0.866 | -138.5 dB | PASS | processBlock 0.074 s for 0.200 s of audio: 37.0 % of one core, real-time factor 2.70
latency 58 samples, Newton failures 0 (rescued 0), max iterations 19 |
| fir | SIN-5000Hz--6dBFS | 0.914 | -138.5 dB | PASS | processBlock 0.144 s for 0.200 s of audio: 72.2 % of one core, real-time factor 1.39
latency 58 samples, Newton failures 449 (rescued 193), max iterations 32 |
| fir | DI-GTR L | 0.786 | -138.5 dB | PASS | processBlock 0.892 s for 5.000 s of audio: 17.8 % of one core, real-time factor 5.60
latency 58 samples, Newton failures 0 (rescued 0), max iterations 17 |
| fir | DI-GTR LL | 0.775 | -138.5 dB | PASS | processBlock 0.866 s for 5.000 s of audio: 17.3 % of one core, real-time factor 5.77
latency 58 samples, Newton failures 0 (rescued 0), max iterations 17 |
| fir | DI-GTR R | 0.869 | -138.5 dB | PASS | processBlock 0.875 s for 5.000 s of audio: 17.5 % of one core, real-time factor 5.71
latency 58 samples, Newton failures 0 (rescued 0), max iterations 20 |
| fir | DI-GTR RR | 0.815 | -138.5 dB | PASS | processBlock 0.887 s for 5.000 s of audio: 17.7 % of one core, real-time factor 5.64
latency 58 samples, Newton failures 0 (rescued 0), max iterations 18 |
| iir | SIN-100Hz--40dBFS | 0.111 | -138.5 dB | PASS | processBlock 0.028 s for 0.200 s of audio: 14.1 % of one core, real-time factor 7.11
latency 13 samples, Newton failures 0 (rescued 0), max iterations 2 |
| iir | SIN-100Hz--20dBFS | 0.719 | -138.5 dB | PASS | processBlock 0.031 s for 0.200 s of audio: 15.6 % of one core, real-time factor 6.42
latency 13 samples, Newton failures 0 (rescued 0), max iterations 6 |
| iir | SIN-100Hz--6dBFS | 0.826 | -138.5 dB | PASS | processBlock 0.032 s for 0.200 s of audio: 15.8 % of one core, real-time factor 6.31
latency 13 samples, Newton failures 0 (rescued 0), max iterations 6 |
| iir | SIN-1000Hz--40dBFS | 0.358 | -138.5 dB | PASS | processBlock 0.046 s for 0.200 s of audio: 23.1 % of one core, real-time factor 4.33
latency 13 samples, Newton failures 0 (rescued 0), max iterations 7 |
| iir | SIN-1000Hz--20dBFS | 0.774 | -138.5 dB | PASS | processBlock 0.049 s for 0.200 s of audio: 24.3 % of one core, real-time factor 4.11
latency 13 samples, Newton failures 0 (rescued 0), max iterations 9 |
| iir | SIN-1000Hz--6dBFS | 0.854 | -138.5 dB | PASS | processBlock 0.051 s for 0.200 s of audio: 25.4 % of one core, real-time factor 3.93
latency 13 samples, Newton failures 0 (rescued 0), max iterations 22 |
| iir | SIN-5000Hz--40dBFS | 0.790 | -138.5 dB | PASS | processBlock 0.064 s for 0.200 s of audio: 31.8 % of one core, real-time factor 3.14
latency 13 samples, Newton failures 0 (rescued 0), max iterations 16 |
| iir | SIN-5000Hz--20dBFS | 0.835 | -138.5 dB | PASS | processBlock 0.071 s for 0.200 s of audio: 35.4 % of one core, real-time factor 2.82
latency 13 samples, Newton failures 0 (rescued 0), max iterations 22 |
| iir | SIN-5000Hz--6dBFS | 0.843 | -138.5 dB | PASS | processBlock 0.101 s for 0.200 s of audio: 50.4 % of one core, real-time factor 1.99
latency 13 samples, Newton failures 161 (rescued 84), max iterations 32 |
| iir | DI-GTR L | 0.836 | -138.5 dB | PASS | processBlock 0.899 s for 5.000 s of audio: 18.0 % of one core, real-time factor 5.56
latency 13 samples, Newton failures 0 (rescued 0), max iterations 18 |
| iir | DI-GTR LL | 0.806 | -138.5 dB | PASS | processBlock 0.870 s for 5.000 s of audio: 17.4 % of one core, real-time factor 5.75
latency 13 samples, Newton failures 0 (rescued 0), max iterations 23 |
| iir | DI-GTR R | 0.887 | -138.5 dB | PASS | processBlock 0.860 s for 5.000 s of audio: 17.2 % of one core, real-time factor 5.81
latency 13 samples, Newton failures 0 (rescued 0), max iterations 17 |
| iir | DI-GTR RR | 0.848 | -138.5 dB | PASS | processBlock 0.881 s for 5.000 s of audio: 17.6 % of one core, real-time factor 5.68
latency 13 samples, Newton failures 0 (rescued 0), max iterations 20 |
