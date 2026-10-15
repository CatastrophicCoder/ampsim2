# M5 wrapper null test: plugin vs ampwav at 96 kHz with the same resampling

- Date: 2026-10-15, commit `e4521b3 (dirty)`; host rate 48000 Hz, 2x, Power Amp Rate half, exact J4
- Plugin: `amp2203_render plugin` (the Amp2203Processor offline). Reference: the processor's oversampler up stage, `ampwav --block full --units fs --j4 exact` at 96000 Hz, the down stage. Default knobs and levels.
- Tolerance (SPEC §7 M5): ESR ≤ -60 dB.

PASS 25 · FAIL 1

| Filter | Signal | Peak out [FS] | ESR | Status | Render log |
|---|---|---|---|---|---|
| fir | SIN-100Hz--40dBFS | 0.111 | -138.3 dB | PASS | processBlock 0.041 s for 0.200 s of audio: 20.5 % of one core, real-time factor 4.88
latency 58 samples, Newton failures 0 (rescued 0), max iterations 3 |
| fir | SIN-100Hz--20dBFS | 0.719 | -138.1 dB | PASS | processBlock 0.045 s for 0.200 s of audio: 22.4 % of one core, real-time factor 4.46
latency 58 samples, Newton failures 0 (rescued 0), max iterations 5 |
| fir | SIN-100Hz--6dBFS | 0.825 | -133.8 dB | PASS | processBlock 0.045 s for 0.200 s of audio: 22.5 % of one core, real-time factor 4.45
latency 58 samples, Newton failures 0 (rescued 0), max iterations 10 |
| fir | SIN-1000Hz--40dBFS | 0.335 | -137.1 dB | PASS | processBlock 0.055 s for 0.200 s of audio: 27.7 % of one core, real-time factor 3.60
latency 58 samples, Newton failures 0 (rescued 0), max iterations 7 |
| fir | SIN-1000Hz--20dBFS | 0.655 | -124.6 dB | PASS | processBlock 0.059 s for 0.200 s of audio: 29.5 % of one core, real-time factor 3.39
latency 58 samples, Newton failures 0 (rescued 0), max iterations 10 |
| fir | SIN-1000Hz--6dBFS | 0.708 | -111.3 dB | PASS | processBlock 0.066 s for 0.200 s of audio: 33.1 % of one core, real-time factor 3.02
latency 58 samples, Newton failures 0 (rescued 0), max iterations 13 |
| fir | SIN-5000Hz--40dBFS | 0.703 | -131.6 dB | PASS | processBlock 0.080 s for 0.200 s of audio: 40.2 % of one core, real-time factor 2.49
latency 58 samples, Newton failures 0 (rescued 0), max iterations 19 |
| fir | SIN-5000Hz--20dBFS | 0.706 | -113.0 dB | PASS | processBlock 0.089 s for 0.200 s of audio: 44.3 % of one core, real-time factor 2.26
latency 58 samples, Newton failures 0 (rescued 0), max iterations 20 |
| fir | SIN-5000Hz--6dBFS | 0.744 | -99.0 dB | PASS | processBlock 0.157 s for 0.200 s of audio: 78.3 % of one core, real-time factor 1.28
latency 58 samples, Newton failures 450 (rescued 190), max iterations 32 |
| fir | DI-GTR L | 0.741 | -138.5 dB | PASS | processBlock 1.096 s for 5.000 s of audio: 21.9 % of one core, real-time factor 4.56
latency 58 samples, Newton failures 0 (rescued 0), max iterations 18 |
| fir | DI-GTR LL | 0.756 | -138.5 dB | PASS | processBlock 1.078 s for 5.000 s of audio: 21.6 % of one core, real-time factor 4.64
latency 58 samples, Newton failures 0 (rescued 0), max iterations 18 |
| fir | DI-GTR R | 0.783 | -138.5 dB | PASS | processBlock 1.083 s for 5.000 s of audio: 21.7 % of one core, real-time factor 4.62
latency 58 samples, Newton failures 0 (rescued 0), max iterations 21 |
| fir | DI-GTR RR | 0.758 | -138.5 dB | PASS | processBlock 1.097 s for 5.000 s of audio: 21.9 % of one core, real-time factor 4.56
latency 58 samples, Newton failures 0 (rescued 0), max iterations 19 |
| iir | SIN-100Hz--40dBFS | 0.111 | -103.8 dB | PASS | processBlock 0.041 s for 0.200 s of audio: 20.5 % of one core, real-time factor 4.88
latency 13 samples, Newton failures 0 (rescued 0), max iterations 3 |
| iir | SIN-100Hz--20dBFS | 0.719 | -87.2 dB | PASS | processBlock 0.045 s for 0.200 s of audio: 22.4 % of one core, real-time factor 4.47
latency 13 samples, Newton failures 0 (rescued 0), max iterations 6 |
| iir | SIN-100Hz--6dBFS | 0.826 | -68.0 dB | PASS | processBlock 0.044 s for 0.200 s of audio: 22.2 % of one core, real-time factor 4.51
latency 13 samples, Newton failures 0 (rescued 0), max iterations 7 |
| iir | SIN-1000Hz--40dBFS | 0.358 | -81.4 dB | PASS | processBlock 0.055 s for 0.200 s of audio: 27.5 % of one core, real-time factor 3.64
latency 13 samples, Newton failures 0 (rescued 0), max iterations 8 |
| iir | SIN-1000Hz--20dBFS | 0.729 | -62.5 dB | PASS | processBlock 0.057 s for 0.200 s of audio: 28.6 % of one core, real-time factor 3.50
latency 13 samples, Newton failures 0 (rescued 0), max iterations 11 |
| iir | SIN-1000Hz--6dBFS | 0.766 | -60.3 dB | PASS | processBlock 0.066 s for 0.200 s of audio: 32.9 % of one core, real-time factor 3.04
latency 13 samples, Newton failures 0 (rescued 0), max iterations 22 |
| iir | SIN-5000Hz--40dBFS | 0.682 | -68.4 dB | PASS | processBlock 0.080 s for 0.200 s of audio: 40.1 % of one core, real-time factor 2.49
latency 13 samples, Newton failures 0 (rescued 0), max iterations 17 |
| iir | SIN-5000Hz--20dBFS | 0.788 | -62.6 dB | PASS | processBlock 0.088 s for 0.200 s of audio: 43.8 % of one core, real-time factor 2.28
latency 13 samples, Newton failures 0 (rescued 0), max iterations 22 |
| iir | SIN-5000Hz--6dBFS | 0.803 | -53.5 dB | FAIL | processBlock 0.122 s for 0.200 s of audio: 60.8 % of one core, real-time factor 1.64
latency 13 samples, Newton failures 176 (rescued 98), max iterations 32 |
| iir | DI-GTR L | 0.796 | -138.5 dB | PASS | processBlock 1.091 s for 5.000 s of audio: 21.8 % of one core, real-time factor 4.58
latency 13 samples, Newton failures 0 (rescued 0), max iterations 19 |
| iir | DI-GTR LL | 0.750 | -138.5 dB | PASS | processBlock 1.072 s for 5.000 s of audio: 21.4 % of one core, real-time factor 4.67
latency 13 samples, Newton failures 0 (rescued 0), max iterations 24 |
| iir | DI-GTR R | 0.837 | -138.5 dB | PASS | processBlock 1.072 s for 5.000 s of audio: 21.4 % of one core, real-time factor 4.67
latency 13 samples, Newton failures 0 (rescued 0), max iterations 18 |
| iir | DI-GTR RR | 0.843 | -138.5 dB | PASS | processBlock 1.084 s for 5.000 s of audio: 21.7 % of one core, real-time factor 4.61
latency 13 samples, Newton failures 0 (rescued 0), max iterations 20 |
