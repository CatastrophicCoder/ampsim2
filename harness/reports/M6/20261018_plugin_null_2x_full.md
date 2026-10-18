# M5 wrapper null test: plugin vs ampwav at 96 kHz with the same resampling

- Date: 2026-10-18, commit `9a422dd (dirty)`; host rate 48000 Hz, 2x, Power Amp Rate full, exact J4
- Plugin: `amp2203_render plugin` (the Amp2203Processor offline). Reference: the processor's oversampler up stage, `ampwav --block full --units fs --j4 exact` at 96000 Hz, the down stage. Default knobs and levels.
- Tolerance (SPEC §7 M5): ESR ≤ -60 dB.

PASS 26 · FAIL 0

| Filter | Signal | Peak out [FS] | ESR | Status | Render log |
|---|---|---|---|---|---|
| fir | SIN-100Hz--40dBFS | 0.145 | -138.5 dB | PASS | processBlock 0.043 s for 0.200 s of audio: 21.5 % of one core, real-time factor 4.64
latency 49 samples, Newton failures 0 (rescued 0), max iterations 3 |
| fir | SIN-100Hz--20dBFS | 0.869 | -138.5 dB | PASS | processBlock 0.049 s for 0.200 s of audio: 24.6 % of one core, real-time factor 4.06
latency 49 samples, Newton failures 0 (rescued 0), max iterations 4 |
| fir | SIN-100Hz--6dBFS | 0.915 | -138.5 dB | PASS | processBlock 0.049 s for 0.200 s of audio: 24.5 % of one core, real-time factor 4.08
latency 49 samples, Newton failures 0 (rescued 0), max iterations 9 |
| fir | SIN-1000Hz--40dBFS | 0.374 | -138.5 dB | PASS | processBlock 0.072 s for 0.200 s of audio: 35.9 % of one core, real-time factor 2.78
latency 49 samples, Newton failures 0 (rescued 0), max iterations 6 |
| fir | SIN-1000Hz--20dBFS | 0.828 | -138.5 dB | PASS | processBlock 0.071 s for 0.200 s of audio: 35.4 % of one core, real-time factor 2.83
latency 49 samples, Newton failures 0 (rescued 0), max iterations 9 |
| fir | SIN-1000Hz--6dBFS | 0.972 | -138.5 dB | PASS | processBlock 0.076 s for 0.200 s of audio: 38.2 % of one core, real-time factor 2.62
latency 49 samples, Newton failures 0 (rescued 0), max iterations 12 |
| fir | SIN-5000Hz--40dBFS | 0.864 | -138.5 dB | PASS | processBlock 0.090 s for 0.200 s of audio: 44.9 % of one core, real-time factor 2.22
latency 49 samples, Newton failures 0 (rescued 0), max iterations 19 |
| fir | SIN-5000Hz--20dBFS | 0.889 | -138.5 dB | PASS | processBlock 0.102 s for 0.200 s of audio: 50.9 % of one core, real-time factor 1.96
latency 49 samples, Newton failures 0 (rescued 0), max iterations 19 |
| fir | SIN-5000Hz--6dBFS | 0.930 | -138.5 dB | PASS | processBlock 0.235 s for 0.200 s of audio: 117.3 % of one core, real-time factor 0.85
latency 49 samples, Newton failures 1009 (rescued 68), max iterations 32 |
| fir | DI-GTR L | 0.977 | -138.5 dB | PASS | processBlock 1.242 s for 5.000 s of audio: 24.8 % of one core, real-time factor 4.03
latency 49 samples, Newton failures 0 (rescued 0), max iterations 19 |
| fir | DI-GTR LL | 0.857 | -138.5 dB | PASS | processBlock 1.202 s for 5.000 s of audio: 24.0 % of one core, real-time factor 4.16
latency 49 samples, Newton failures 0 (rescued 0), max iterations 17 |
| fir | DI-GTR R | 0.945 | -138.5 dB | PASS | processBlock 1.215 s for 5.000 s of audio: 24.3 % of one core, real-time factor 4.12
latency 49 samples, Newton failures 0 (rescued 0), max iterations 20 |
| fir | DI-GTR RR | 0.979 | -138.5 dB | PASS | processBlock 1.252 s for 5.000 s of audio: 25.0 % of one core, real-time factor 3.99
latency 49 samples, Newton failures 0 (rescued 0), max iterations 17 |
| iir | SIN-100Hz--40dBFS | 0.145 | -138.5 dB | PASS | processBlock 0.041 s for 0.200 s of audio: 20.7 % of one core, real-time factor 4.82
latency 4 samples, Newton failures 0 (rescued 0), max iterations 2 |
| iir | SIN-100Hz--20dBFS | 0.870 | -138.5 dB | PASS | processBlock 0.044 s for 0.200 s of audio: 22.2 % of one core, real-time factor 4.51
latency 4 samples, Newton failures 0 (rescued 0), max iterations 6 |
| iir | SIN-100Hz--6dBFS | 0.915 | -138.5 dB | PASS | processBlock 0.046 s for 0.200 s of audio: 22.8 % of one core, real-time factor 4.39
latency 4 samples, Newton failures 0 (rescued 0), max iterations 6 |
| iir | SIN-1000Hz--40dBFS | 0.414 | -138.5 dB | PASS | processBlock 0.067 s for 0.200 s of audio: 33.4 % of one core, real-time factor 3.00
latency 4 samples, Newton failures 0 (rescued 0), max iterations 7 |
| iir | SIN-1000Hz--20dBFS | 0.928 | -138.5 dB | PASS | processBlock 0.071 s for 0.200 s of audio: 35.3 % of one core, real-time factor 2.84
latency 4 samples, Newton failures 0 (rescued 0), max iterations 9 |
| iir | SIN-1000Hz--6dBFS | 1.044 | -138.5 dB | PASS | processBlock 0.075 s for 0.200 s of audio: 37.4 % of one core, real-time factor 2.67
latency 4 samples, Newton failures 0 (rescued 0), max iterations 22 |
| iir | SIN-5000Hz--40dBFS | 0.982 | -138.5 dB | PASS | processBlock 0.090 s for 0.200 s of audio: 44.9 % of one core, real-time factor 2.23
latency 4 samples, Newton failures 0 (rescued 0), max iterations 16 |
| iir | SIN-5000Hz--20dBFS | 1.015 | -138.5 dB | PASS | processBlock 0.101 s for 0.200 s of audio: 50.6 % of one core, real-time factor 1.98
latency 4 samples, Newton failures 0 (rescued 0), max iterations 22 |
| iir | SIN-5000Hz--6dBFS | 1.020 | -138.5 dB | PASS | processBlock 0.139 s for 0.200 s of audio: 69.3 % of one core, real-time factor 1.44
latency 4 samples, Newton failures 268 (rescued 184), max iterations 32 |
| iir | DI-GTR L | 0.985 | -138.5 dB | PASS | processBlock 1.250 s for 5.000 s of audio: 25.0 % of one core, real-time factor 4.00
latency 4 samples, Newton failures 0 (rescued 0), max iterations 18 |
| iir | DI-GTR LL | 0.985 | -138.5 dB | PASS | processBlock 1.217 s for 5.000 s of audio: 24.3 % of one core, real-time factor 4.11
latency 4 samples, Newton failures 0 (rescued 0), max iterations 22 |
| iir | DI-GTR R | 1.027 | -138.5 dB | PASS | processBlock 1.216 s for 5.000 s of audio: 24.3 % of one core, real-time factor 4.11
latency 4 samples, Newton failures 0 (rescued 0), max iterations 18 |
| iir | DI-GTR RR | 1.036 | -138.5 dB | PASS | processBlock 1.250 s for 5.000 s of audio: 25.0 % of one core, real-time factor 4.00
latency 4 samples, Newton failures 0 (rescued 0), max iterations 21 |
