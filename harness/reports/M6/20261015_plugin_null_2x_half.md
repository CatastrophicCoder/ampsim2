# M5 wrapper null test: plugin vs ampwav at 96 kHz with the same resampling

- Date: 2026-10-15, commit `831e393 (dirty)`; host rate 48000 Hz, 2x, Power Amp Rate half, exact J4
- Plugin: `amp2203_render plugin` (the Amp2203Processor offline). Reference: the processor's oversampler up stage, `ampwav --block full --units fs --j4 exact` at 96000 Hz, the down stage. Default knobs and levels.
- Tolerance (SPEC §7 M5): ESR ≤ -60 dB.

PASS 26 · FAIL 0

| Filter | Signal | Peak out [FS] | ESR | Status | Render log |
|---|---|---|---|---|---|
| fir | SIN-100Hz--40dBFS | 0.111 | -138.5 dB | PASS | processBlock 0.028 s for 0.200 s of audio: 14.0 % of one core, real-time factor 7.16
latency 58 samples, Newton failures 0 (rescued 0), max iterations 3 |
| fir | SIN-100Hz--20dBFS | 0.719 | -138.5 dB | PASS | processBlock 0.031 s for 0.200 s of audio: 15.3 % of one core, real-time factor 6.53
latency 58 samples, Newton failures 0 (rescued 0), max iterations 4 |
| fir | SIN-100Hz--6dBFS | 0.825 | -138.5 dB | PASS | processBlock 0.032 s for 0.200 s of audio: 15.8 % of one core, real-time factor 6.33
latency 58 samples, Newton failures 0 (rescued 0), max iterations 9 |
| fir | SIN-1000Hz--40dBFS | 0.335 | -138.5 dB | PASS | processBlock 0.045 s for 0.200 s of audio: 22.6 % of one core, real-time factor 4.43
latency 58 samples, Newton failures 0 (rescued 0), max iterations 6 |
| fir | SIN-1000Hz--20dBFS | 0.655 | -138.5 dB | PASS | processBlock 0.048 s for 0.200 s of audio: 24.2 % of one core, real-time factor 4.14
latency 58 samples, Newton failures 0 (rescued 0), max iterations 9 |
| fir | SIN-1000Hz--6dBFS | 0.708 | -138.5 dB | PASS | processBlock 0.055 s for 0.200 s of audio: 27.3 % of one core, real-time factor 3.67
latency 58 samples, Newton failures 0 (rescued 0), max iterations 12 |
| fir | SIN-5000Hz--40dBFS | 0.703 | -138.5 dB | PASS | processBlock 0.067 s for 0.200 s of audio: 33.3 % of one core, real-time factor 3.00
latency 58 samples, Newton failures 0 (rescued 0), max iterations 18 |
| fir | SIN-5000Hz--20dBFS | 0.706 | -138.5 dB | PASS | processBlock 0.077 s for 0.200 s of audio: 38.7 % of one core, real-time factor 2.59
latency 58 samples, Newton failures 0 (rescued 0), max iterations 19 |
| fir | SIN-5000Hz--6dBFS | 0.744 | -138.5 dB | PASS | processBlock 0.143 s for 0.200 s of audio: 71.7 % of one core, real-time factor 1.39
latency 58 samples, Newton failures 449 (rescued 191), max iterations 32 |
| fir | DI-GTR L | 0.741 | -138.5 dB | PASS | processBlock 0.877 s for 5.000 s of audio: 17.5 % of one core, real-time factor 5.70
latency 58 samples, Newton failures 0 (rescued 0), max iterations 17 |
| fir | DI-GTR LL | 0.756 | -138.5 dB | PASS | processBlock 0.858 s for 5.000 s of audio: 17.2 % of one core, real-time factor 5.83
latency 58 samples, Newton failures 0 (rescued 0), max iterations 17 |
| fir | DI-GTR R | 0.783 | -138.5 dB | PASS | processBlock 0.860 s for 5.000 s of audio: 17.2 % of one core, real-time factor 5.81
latency 58 samples, Newton failures 0 (rescued 0), max iterations 20 |
| fir | DI-GTR RR | 0.758 | -138.5 dB | PASS | processBlock 0.882 s for 5.000 s of audio: 17.6 % of one core, real-time factor 5.67
latency 58 samples, Newton failures 0 (rescued 0), max iterations 18 |
| iir | SIN-100Hz--40dBFS | 0.111 | -138.5 dB | PASS | processBlock 0.028 s for 0.200 s of audio: 13.9 % of one core, real-time factor 7.21
latency 13 samples, Newton failures 0 (rescued 0), max iterations 2 |
| iir | SIN-100Hz--20dBFS | 0.719 | -138.5 dB | PASS | processBlock 0.031 s for 0.200 s of audio: 15.3 % of one core, real-time factor 6.54
latency 13 samples, Newton failures 0 (rescued 0), max iterations 6 |
| iir | SIN-100Hz--6dBFS | 0.826 | -138.5 dB | PASS | processBlock 0.031 s for 0.200 s of audio: 15.5 % of one core, real-time factor 6.46
latency 13 samples, Newton failures 0 (rescued 0), max iterations 6 |
| iir | SIN-1000Hz--40dBFS | 0.358 | -138.5 dB | PASS | processBlock 0.045 s for 0.200 s of audio: 22.3 % of one core, real-time factor 4.48
latency 13 samples, Newton failures 0 (rescued 0), max iterations 7 |
| iir | SIN-1000Hz--20dBFS | 0.729 | -138.5 dB | PASS | processBlock 0.047 s for 0.200 s of audio: 23.6 % of one core, real-time factor 4.23
latency 13 samples, Newton failures 0 (rescued 0), max iterations 9 |
| iir | SIN-1000Hz--6dBFS | 0.766 | -138.5 dB | PASS | processBlock 0.054 s for 0.200 s of audio: 26.9 % of one core, real-time factor 3.72
latency 13 samples, Newton failures 0 (rescued 0), max iterations 22 |
| iir | SIN-5000Hz--40dBFS | 0.682 | -138.5 dB | PASS | processBlock 0.067 s for 0.200 s of audio: 33.4 % of one core, real-time factor 3.00
latency 13 samples, Newton failures 0 (rescued 0), max iterations 16 |
| iir | SIN-5000Hz--20dBFS | 0.788 | -138.5 dB | PASS | processBlock 0.080 s for 0.200 s of audio: 40.2 % of one core, real-time factor 2.49
latency 13 samples, Newton failures 0 (rescued 0), max iterations 22 |
| iir | SIN-5000Hz--6dBFS | 0.803 | -138.5 dB | PASS | processBlock 0.108 s for 0.200 s of audio: 54.0 % of one core, real-time factor 1.85
latency 13 samples, Newton failures 163 (rescued 86), max iterations 32 |
| iir | DI-GTR L | 0.796 | -138.5 dB | PASS | processBlock 0.869 s for 5.000 s of audio: 17.4 % of one core, real-time factor 5.76
latency 13 samples, Newton failures 0 (rescued 0), max iterations 18 |
| iir | DI-GTR LL | 0.750 | -138.5 dB | PASS | processBlock 0.843 s for 5.000 s of audio: 16.9 % of one core, real-time factor 5.93
latency 13 samples, Newton failures 0 (rescued 0), max iterations 23 |
| iir | DI-GTR R | 0.837 | -138.5 dB | PASS | processBlock 0.846 s for 5.000 s of audio: 16.9 % of one core, real-time factor 5.91
latency 13 samples, Newton failures 0 (rescued 0), max iterations 17 |
| iir | DI-GTR RR | 0.843 | -138.5 dB | PASS | processBlock 0.864 s for 5.000 s of audio: 17.3 % of one core, real-time factor 5.78
latency 13 samples, Newton failures 0 (rescued 0), max iterations 20 |
