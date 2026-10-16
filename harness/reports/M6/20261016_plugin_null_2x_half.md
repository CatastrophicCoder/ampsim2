# M5 wrapper null test: plugin vs ampwav at 96 kHz with the same resampling

- Date: 2026-10-16, commit `3cb4124 (dirty)`; host rate 48000 Hz, 2x, Power Amp Rate half, exact J4
- Plugin: `amp2203_render plugin` (the Amp2203Processor offline). Reference: the processor's oversampler up stage, `ampwav --block full --units fs --j4 exact` at 96000 Hz, the down stage. Default knobs and levels.
- Tolerance (SPEC §7 M5): ESR ≤ -60 dB.

PASS 26 · FAIL 0

| Filter | Signal | Peak out [FS] | ESR | Status | Render log |
|---|---|---|---|---|---|
| fir | SIN-100Hz--40dBFS | 0.145 | -138.5 dB | PASS | processBlock 0.029 s for 0.200 s of audio: 14.6 % of one core, real-time factor 6.85
latency 58 samples, Newton failures 0 (rescued 0), max iterations 3 |
| fir | SIN-100Hz--20dBFS | 0.869 | -138.5 dB | PASS | processBlock 0.035 s for 0.200 s of audio: 17.5 % of one core, real-time factor 5.72
latency 58 samples, Newton failures 0 (rescued 0), max iterations 4 |
| fir | SIN-100Hz--6dBFS | 0.915 | -138.5 dB | PASS | processBlock 0.033 s for 0.200 s of audio: 16.3 % of one core, real-time factor 6.15
latency 58 samples, Newton failures 0 (rescued 0), max iterations 9 |
| fir | SIN-1000Hz--40dBFS | 0.395 | -138.5 dB | PASS | processBlock 0.051 s for 0.200 s of audio: 25.3 % of one core, real-time factor 3.95
latency 58 samples, Newton failures 0 (rescued 0), max iterations 6 |
| fir | SIN-1000Hz--20dBFS | 0.795 | -138.5 dB | PASS | processBlock 0.053 s for 0.200 s of audio: 26.3 % of one core, real-time factor 3.80
latency 58 samples, Newton failures 0 (rescued 0), max iterations 9 |
| fir | SIN-1000Hz--6dBFS | 0.994 | -138.5 dB | PASS | processBlock 0.057 s for 0.200 s of audio: 28.6 % of one core, real-time factor 3.50
latency 58 samples, Newton failures 0 (rescued 0), max iterations 12 |
| fir | SIN-5000Hz--40dBFS | 0.879 | -138.5 dB | PASS | processBlock 0.068 s for 0.200 s of audio: 34.2 % of one core, real-time factor 2.92
latency 58 samples, Newton failures 0 (rescued 0), max iterations 19 |
| fir | SIN-5000Hz--20dBFS | 0.913 | -138.5 dB | PASS | processBlock 0.077 s for 0.200 s of audio: 38.4 % of one core, real-time factor 2.60
latency 58 samples, Newton failures 0 (rescued 0), max iterations 19 |
| fir | SIN-5000Hz--6dBFS | 0.967 | -138.5 dB | PASS | processBlock 0.191 s for 0.200 s of audio: 95.6 % of one core, real-time factor 1.05
latency 58 samples, Newton failures 869 (rescued 74), max iterations 32 |
| fir | DI-GTR L | 0.844 | -138.5 dB | PASS | processBlock 0.881 s for 5.000 s of audio: 17.6 % of one core, real-time factor 5.68
latency 58 samples, Newton failures 0 (rescued 0), max iterations 19 |
| fir | DI-GTR LL | 0.826 | -138.5 dB | PASS | processBlock 0.852 s for 5.000 s of audio: 17.0 % of one core, real-time factor 5.87
latency 58 samples, Newton failures 0 (rescued 0), max iterations 17 |
| fir | DI-GTR R | 0.916 | -138.5 dB | PASS | processBlock 0.863 s for 5.000 s of audio: 17.3 % of one core, real-time factor 5.80
latency 58 samples, Newton failures 0 (rescued 0), max iterations 20 |
| fir | DI-GTR RR | 0.918 | -138.5 dB | PASS | processBlock 0.879 s for 5.000 s of audio: 17.6 % of one core, real-time factor 5.69
latency 58 samples, Newton failures 0 (rescued 0), max iterations 17 |
| iir | SIN-100Hz--40dBFS | 0.145 | -138.5 dB | PASS | processBlock 0.028 s for 0.200 s of audio: 14.1 % of one core, real-time factor 7.11
latency 13 samples, Newton failures 0 (rescued 0), max iterations 2 |
| iir | SIN-100Hz--20dBFS | 0.870 | -138.5 dB | PASS | processBlock 0.031 s for 0.200 s of audio: 15.5 % of one core, real-time factor 6.44
latency 13 samples, Newton failures 0 (rescued 0), max iterations 6 |
| iir | SIN-100Hz--6dBFS | 0.915 | -138.5 dB | PASS | processBlock 0.032 s for 0.200 s of audio: 15.8 % of one core, real-time factor 6.33
latency 13 samples, Newton failures 0 (rescued 0), max iterations 6 |
| iir | SIN-1000Hz--40dBFS | 0.384 | -138.5 dB | PASS | processBlock 0.045 s for 0.200 s of audio: 22.6 % of one core, real-time factor 4.42
latency 13 samples, Newton failures 0 (rescued 0), max iterations 7 |
| iir | SIN-1000Hz--20dBFS | 0.836 | -138.5 dB | PASS | processBlock 0.048 s for 0.200 s of audio: 24.1 % of one core, real-time factor 4.15
latency 13 samples, Newton failures 0 (rescued 0), max iterations 9 |
| iir | SIN-1000Hz--6dBFS | 1.011 | -138.5 dB | PASS | processBlock 0.051 s for 0.200 s of audio: 25.3 % of one core, real-time factor 3.96
latency 13 samples, Newton failures 0 (rescued 0), max iterations 22 |
| iir | SIN-5000Hz--40dBFS | 0.915 | -138.5 dB | PASS | processBlock 0.067 s for 0.200 s of audio: 33.7 % of one core, real-time factor 2.97
latency 13 samples, Newton failures 0 (rescued 0), max iterations 16 |
| iir | SIN-5000Hz--20dBFS | 1.003 | -138.5 dB | PASS | processBlock 0.076 s for 0.200 s of audio: 37.9 % of one core, real-time factor 2.64
latency 13 samples, Newton failures 0 (rescued 0), max iterations 22 |
| iir | SIN-5000Hz--6dBFS | 0.986 | -138.5 dB | PASS | processBlock 0.109 s for 0.200 s of audio: 54.3 % of one core, real-time factor 1.84
latency 13 samples, Newton failures 240 (rescued 184), max iterations 32 |
| iir | DI-GTR L | 0.991 | -138.5 dB | PASS | processBlock 0.870 s for 5.000 s of audio: 17.4 % of one core, real-time factor 5.75
latency 13 samples, Newton failures 0 (rescued 0), max iterations 18 |
| iir | DI-GTR LL | 0.905 | -138.5 dB | PASS | processBlock 0.847 s for 5.000 s of audio: 16.9 % of one core, real-time factor 5.90
latency 13 samples, Newton failures 0 (rescued 0), max iterations 22 |
| iir | DI-GTR R | 0.982 | -138.5 dB | PASS | processBlock 0.848 s for 5.000 s of audio: 17.0 % of one core, real-time factor 5.90
latency 13 samples, Newton failures 0 (rescued 0), max iterations 18 |
| iir | DI-GTR RR | 0.969 | -138.5 dB | PASS | processBlock 0.876 s for 5.000 s of audio: 17.5 % of one core, real-time factor 5.71
latency 13 samples, Newton failures 0 (rescued 0), max iterations 21 |
