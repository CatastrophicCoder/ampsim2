# M5 wrapper null test: plugin vs ampwav at 96 kHz with the same resampling

- Date: 2026-10-15, commit `e4521b3 (dirty)`; host rate 48000 Hz, 2x, Power Amp Rate full, exact J4
- Plugin: `amp2203_render plugin` (the Amp2203Processor offline). Reference: the processor's oversampler up stage, `ampwav --block full --units fs --j4 exact` at 96000 Hz, the down stage. Default knobs and levels.
- Tolerance (SPEC §7 M5): ESR ≤ -60 dB.

PASS 26 · FAIL 0

| Filter | Signal | Peak out [FS] | ESR | Status | Render log |
|---|---|---|---|---|---|
| fir | SIN-100Hz--40dBFS | 0.111 | -138.5 dB | PASS | processBlock 0.061 s for 0.200 s of audio: 30.4 % of one core, real-time factor 3.29
latency 49 samples, Newton failures 0 (rescued 0), max iterations 3 |
| fir | SIN-100Hz--20dBFS | 0.719 | -138.5 dB | PASS | processBlock 0.066 s for 0.200 s of audio: 32.9 % of one core, real-time factor 3.04
latency 49 samples, Newton failures 0 (rescued 0), max iterations 5 |
| fir | SIN-100Hz--6dBFS | 0.825 | -138.5 dB | PASS | processBlock 0.065 s for 0.200 s of audio: 32.4 % of one core, real-time factor 3.08
latency 49 samples, Newton failures 0 (rescued 0), max iterations 10 |
| fir | SIN-1000Hz--40dBFS | 0.341 | -138.5 dB | PASS | processBlock 0.075 s for 0.200 s of audio: 37.7 % of one core, real-time factor 2.65
latency 49 samples, Newton failures 0 (rescued 0), max iterations 7 |
| fir | SIN-1000Hz--20dBFS | 0.708 | -138.5 dB | PASS | processBlock 0.084 s for 0.200 s of audio: 42.2 % of one core, real-time factor 2.37
latency 49 samples, Newton failures 0 (rescued 0), max iterations 10 |
| fir | SIN-1000Hz--6dBFS | 0.704 | -138.5 dB | PASS | processBlock 0.092 s for 0.200 s of audio: 46.2 % of one core, real-time factor 2.17
latency 49 samples, Newton failures 0 (rescued 0), max iterations 13 |
| fir | SIN-5000Hz--40dBFS | 0.677 | -138.5 dB | PASS | processBlock 0.110 s for 0.200 s of audio: 55.1 % of one core, real-time factor 1.82
latency 49 samples, Newton failures 0 (rescued 0), max iterations 19 |
| fir | SIN-5000Hz--20dBFS | 0.706 | -138.5 dB | PASS | processBlock 0.127 s for 0.200 s of audio: 63.6 % of one core, real-time factor 1.57
latency 49 samples, Newton failures 0 (rescued 0), max iterations 20 |
| fir | SIN-5000Hz--6dBFS | 0.738 | -138.5 dB | PASS | processBlock 0.196 s for 0.200 s of audio: 98.0 % of one core, real-time factor 1.02
latency 49 samples, Newton failures 443 (rescued 195), max iterations 32 |
| fir | DI-GTR L | 0.767 | -138.5 dB | PASS | processBlock 1.586 s for 5.000 s of audio: 31.7 % of one core, real-time factor 3.15
latency 49 samples, Newton failures 0 (rescued 0), max iterations 18 |
| fir | DI-GTR LL | 0.711 | -138.5 dB | PASS | processBlock 1.568 s for 5.000 s of audio: 31.4 % of one core, real-time factor 3.19
latency 49 samples, Newton failures 0 (rescued 0), max iterations 18 |
| fir | DI-GTR R | 0.782 | -138.5 dB | PASS | processBlock 1.569 s for 5.000 s of audio: 31.4 % of one core, real-time factor 3.19
latency 49 samples, Newton failures 0 (rescued 0), max iterations 21 |
| fir | DI-GTR RR | 0.798 | -138.5 dB | PASS | processBlock 1.581 s for 5.000 s of audio: 31.6 % of one core, real-time factor 3.16
latency 49 samples, Newton failures 0 (rescued 0), max iterations 19 |
| iir | SIN-100Hz--40dBFS | 0.111 | -138.5 dB | PASS | processBlock 0.061 s for 0.200 s of audio: 30.4 % of one core, real-time factor 3.29
latency 4 samples, Newton failures 0 (rescued 0), max iterations 3 |
| iir | SIN-100Hz--20dBFS | 0.719 | -138.5 dB | PASS | processBlock 0.066 s for 0.200 s of audio: 32.8 % of one core, real-time factor 3.05
latency 4 samples, Newton failures 0 (rescued 0), max iterations 6 |
| iir | SIN-100Hz--6dBFS | 0.826 | -138.5 dB | PASS | processBlock 0.064 s for 0.200 s of audio: 32.1 % of one core, real-time factor 3.11
latency 4 samples, Newton failures 0 (rescued 0), max iterations 7 |
| iir | SIN-1000Hz--40dBFS | 0.362 | -138.5 dB | PASS | processBlock 0.075 s for 0.200 s of audio: 37.4 % of one core, real-time factor 2.68
latency 4 samples, Newton failures 0 (rescued 0), max iterations 8 |
| iir | SIN-1000Hz--20dBFS | 0.774 | -138.5 dB | PASS | processBlock 0.082 s for 0.200 s of audio: 40.9 % of one core, real-time factor 2.45
latency 4 samples, Newton failures 0 (rescued 0), max iterations 11 |
| iir | SIN-1000Hz--6dBFS | 0.881 | -138.5 dB | PASS | processBlock 0.094 s for 0.200 s of audio: 46.8 % of one core, real-time factor 2.14
latency 4 samples, Newton failures 0 (rescued 0), max iterations 22 |
| iir | SIN-5000Hz--40dBFS | 0.816 | -138.5 dB | PASS | processBlock 0.110 s for 0.200 s of audio: 55.1 % of one core, real-time factor 1.81
latency 4 samples, Newton failures 0 (rescued 0), max iterations 17 |
| iir | SIN-5000Hz--20dBFS | 0.826 | -138.5 dB | PASS | processBlock 0.128 s for 0.200 s of audio: 63.8 % of one core, real-time factor 1.57
latency 4 samples, Newton failures 0 (rescued 0), max iterations 22 |
| iir | SIN-5000Hz--6dBFS | 0.841 | -138.5 dB | PASS | processBlock 0.174 s for 0.200 s of audio: 86.9 % of one core, real-time factor 1.15
latency 4 samples, Newton failures 305 (rescued 96), max iterations 32 |
| iir | DI-GTR L | 0.814 | -138.5 dB | PASS | processBlock 1.576 s for 5.000 s of audio: 31.5 % of one core, real-time factor 3.17
latency 4 samples, Newton failures 0 (rescued 0), max iterations 19 |
| iir | DI-GTR LL | 0.781 | -138.5 dB | PASS | processBlock 1.563 s for 5.000 s of audio: 31.3 % of one core, real-time factor 3.20
latency 4 samples, Newton failures 0 (rescued 0), max iterations 24 |
| iir | DI-GTR R | 0.891 | -138.5 dB | PASS | processBlock 1.553 s for 5.000 s of audio: 31.1 % of one core, real-time factor 3.22
latency 4 samples, Newton failures 0 (rescued 0), max iterations 18 |
| iir | DI-GTR RR | 0.866 | -138.5 dB | PASS | processBlock 1.566 s for 5.000 s of audio: 31.3 % of one core, real-time factor 3.19
latency 4 samples, Newton failures 0 (rescued 0), max iterations 20 |
