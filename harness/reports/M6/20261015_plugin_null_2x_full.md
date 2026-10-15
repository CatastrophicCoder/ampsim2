# M5 wrapper null test: plugin vs ampwav at 96 kHz with the same resampling

- Date: 2026-10-15, commit `9778d0d (dirty)`; host rate 48000 Hz, 2x, Power Amp Rate full, exact J4
- Plugin: `amp2203_render plugin` (the Amp2203Processor offline). Reference: the processor's oversampler up stage, `ampwav --block full --units fs --j4 exact` at 96000 Hz, the down stage. Default knobs and levels.
- Tolerance (SPEC §7 M5): ESR ≤ -60 dB.

PASS 26 · FAIL 0

| Filter | Signal | Peak out [FS] | ESR | Status | Render log |
|---|---|---|---|---|---|
| fir | SIN-100Hz--40dBFS | 0.111 | -138.5 dB | PASS | processBlock 0.041 s for 0.200 s of audio: 20.4 % of one core, real-time factor 4.91
latency 49 samples, Newton failures 0 (rescued 0), max iterations 3 |
| fir | SIN-100Hz--20dBFS | 0.719 | -138.5 dB | PASS | processBlock 0.044 s for 0.200 s of audio: 22.0 % of one core, real-time factor 4.55
latency 49 samples, Newton failures 0 (rescued 0), max iterations 4 |
| fir | SIN-100Hz--6dBFS | 0.825 | -138.5 dB | PASS | processBlock 0.045 s for 0.200 s of audio: 22.3 % of one core, real-time factor 4.48
latency 49 samples, Newton failures 0 (rescued 0), max iterations 9 |
| fir | SIN-1000Hz--40dBFS | 0.341 | -138.5 dB | PASS | processBlock 0.065 s for 0.200 s of audio: 32.4 % of one core, real-time factor 3.09
latency 49 samples, Newton failures 0 (rescued 0), max iterations 6 |
| fir | SIN-1000Hz--20dBFS | 0.708 | -138.5 dB | PASS | processBlock 0.069 s for 0.200 s of audio: 34.5 % of one core, real-time factor 2.90
latency 49 samples, Newton failures 0 (rescued 0), max iterations 9 |
| fir | SIN-1000Hz--6dBFS | 0.704 | -138.5 dB | PASS | processBlock 0.075 s for 0.200 s of audio: 37.4 % of one core, real-time factor 2.67
latency 49 samples, Newton failures 0 (rescued 0), max iterations 12 |
| fir | SIN-5000Hz--40dBFS | 0.677 | -138.5 dB | PASS | processBlock 0.091 s for 0.200 s of audio: 45.4 % of one core, real-time factor 2.20
latency 49 samples, Newton failures 0 (rescued 0), max iterations 18 |
| fir | SIN-5000Hz--20dBFS | 0.706 | -138.5 dB | PASS | processBlock 0.110 s for 0.200 s of audio: 54.8 % of one core, real-time factor 1.82
latency 49 samples, Newton failures 0 (rescued 0), max iterations 19 |
| fir | SIN-5000Hz--6dBFS | 0.738 | -138.5 dB | PASS | processBlock 0.176 s for 0.200 s of audio: 88.2 % of one core, real-time factor 1.13
latency 49 samples, Newton failures 441 (rescued 194), max iterations 32 |
| fir | DI-GTR L | 0.767 | -138.5 dB | PASS | processBlock 1.232 s for 5.000 s of audio: 24.6 % of one core, real-time factor 4.06
latency 49 samples, Newton failures 0 (rescued 0), max iterations 17 |
| fir | DI-GTR LL | 0.711 | -138.5 dB | PASS | processBlock 1.196 s for 5.000 s of audio: 23.9 % of one core, real-time factor 4.18
latency 49 samples, Newton failures 0 (rescued 0), max iterations 17 |
| fir | DI-GTR R | 0.782 | -138.5 dB | PASS | processBlock 1.209 s for 5.000 s of audio: 24.2 % of one core, real-time factor 4.14
latency 49 samples, Newton failures 0 (rescued 0), max iterations 20 |
| fir | DI-GTR RR | 0.798 | -138.5 dB | PASS | processBlock 1.241 s for 5.000 s of audio: 24.8 % of one core, real-time factor 4.03
latency 49 samples, Newton failures 0 (rescued 0), max iterations 18 |
| iir | SIN-100Hz--40dBFS | 0.111 | -138.5 dB | PASS | processBlock 0.041 s for 0.200 s of audio: 20.3 % of one core, real-time factor 4.92
latency 4 samples, Newton failures 0 (rescued 0), max iterations 2 |
| iir | SIN-100Hz--20dBFS | 0.719 | -138.5 dB | PASS | processBlock 0.044 s for 0.200 s of audio: 22.1 % of one core, real-time factor 4.54
latency 4 samples, Newton failures 0 (rescued 0), max iterations 6 |
| iir | SIN-100Hz--6dBFS | 0.826 | -138.5 dB | PASS | processBlock 0.044 s for 0.200 s of audio: 21.9 % of one core, real-time factor 4.56
latency 4 samples, Newton failures 0 (rescued 0), max iterations 6 |
| iir | SIN-1000Hz--40dBFS | 0.362 | -138.5 dB | PASS | processBlock 0.064 s for 0.200 s of audio: 32.2 % of one core, real-time factor 3.10
latency 4 samples, Newton failures 0 (rescued 0), max iterations 7 |
| iir | SIN-1000Hz--20dBFS | 0.774 | -138.5 dB | PASS | processBlock 0.068 s for 0.200 s of audio: 34.0 % of one core, real-time factor 2.94
latency 4 samples, Newton failures 0 (rescued 0), max iterations 9 |
| iir | SIN-1000Hz--6dBFS | 0.881 | -138.5 dB | PASS | processBlock 0.077 s for 0.200 s of audio: 38.5 % of one core, real-time factor 2.60
latency 4 samples, Newton failures 0 (rescued 0), max iterations 22 |
| iir | SIN-5000Hz--40dBFS | 0.816 | -138.5 dB | PASS | processBlock 0.092 s for 0.200 s of audio: 46.0 % of one core, real-time factor 2.17
latency 4 samples, Newton failures 0 (rescued 0), max iterations 16 |
| iir | SIN-5000Hz--20dBFS | 0.826 | -138.5 dB | PASS | processBlock 0.111 s for 0.200 s of audio: 55.6 % of one core, real-time factor 1.80
latency 4 samples, Newton failures 0 (rescued 0), max iterations 22 |
| iir | SIN-5000Hz--6dBFS | 0.841 | -138.5 dB | PASS | processBlock 0.140 s for 0.200 s of audio: 69.8 % of one core, real-time factor 1.43
latency 4 samples, Newton failures 157 (rescued 80), max iterations 32 |
| iir | DI-GTR L | 0.814 | -138.5 dB | PASS | processBlock 1.222 s for 5.000 s of audio: 24.4 % of one core, real-time factor 4.09
latency 4 samples, Newton failures 0 (rescued 0), max iterations 18 |
| iir | DI-GTR LL | 0.781 | -138.5 dB | PASS | processBlock 1.180 s for 5.000 s of audio: 23.6 % of one core, real-time factor 4.24
latency 4 samples, Newton failures 0 (rescued 0), max iterations 23 |
| iir | DI-GTR R | 0.891 | -138.5 dB | PASS | processBlock 1.188 s for 5.000 s of audio: 23.8 % of one core, real-time factor 4.21
latency 4 samples, Newton failures 0 (rescued 0), max iterations 17 |
| iir | DI-GTR RR | 0.866 | -138.5 dB | PASS | processBlock 1.227 s for 5.000 s of audio: 24.5 % of one core, real-time factor 4.08
latency 4 samples, Newton failures 0 (rescued 0), max iterations 20 |
