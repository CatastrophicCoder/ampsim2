# M5 wrapper null test: plugin vs ampwav at 768 kHz with the same resampling

- Date: 2026-10-14, commit `6754b4f (dirty)`; host rate 48000 Hz, 16x
- Plugin: `amp2203_render plugin` (the Amp2203Processor offline). Reference: the processor's oversampler up stage, `ampwav --block full --units fs` at 768 kHz, the down stage. Default knobs and levels.
- Tolerance (SPEC §7 M5): ESR ≤ -60 dB.

PASS 26 · FAIL 0

| Filter | Signal | Peak out [FS] | ESR | Status | Render log |
|---|---|---|---|---|---|
| fir | SIN-100Hz--40dBFS | 0.111 | -96.6 dB | PASS | latency 67 samples, Newton failures 0 (rescued 0), max iterations 2 |
| fir | SIN-100Hz--20dBFS | 0.716 | -107.8 dB | PASS | latency 67 samples, Newton failures 0 (rescued 0), max iterations 4 |
| fir | SIN-100Hz--6dBFS | 0.822 | -110.9 dB | PASS | latency 67 samples, Newton failures 0 (rescued 0), max iterations 4 |
| fir | SIN-1000Hz--40dBFS | 0.343 | -102.5 dB | PASS | latency 67 samples, Newton failures 0 (rescued 0), max iterations 5 |
| fir | SIN-1000Hz--20dBFS | 0.717 | -97.9 dB | PASS | latency 67 samples, Newton failures 0 (rescued 0), max iterations 7 |
| fir | SIN-1000Hz--6dBFS | 0.747 | -93.0 dB | PASS | latency 67 samples, Newton failures 0 (rescued 0), max iterations 8 |
| fir | SIN-5000Hz--40dBFS | 0.702 | -105.5 dB | PASS | latency 67 samples, Newton failures 0 (rescued 0), max iterations 7 |
| fir | SIN-5000Hz--20dBFS | 0.732 | -109.7 dB | PASS | latency 67 samples, Newton failures 0 (rescued 0), max iterations 9 |
| fir | SIN-5000Hz--6dBFS | 0.740 | -108.1 dB | PASS | latency 67 samples, Newton failures 0 (rescued 0), max iterations 10 |
| fir | DI-GTR L | 0.799 | -101.4 dB | PASS | latency 67 samples, Newton failures 0 (rescued 0), max iterations 7 |
| fir | DI-GTR LL | 0.715 | -100.8 dB | PASS | latency 67 samples, Newton failures 0 (rescued 0), max iterations 7 |
| fir | DI-GTR R | 0.799 | -101.7 dB | PASS | latency 67 samples, Newton failures 0 (rescued 0), max iterations 7 |
| fir | DI-GTR RR | 0.830 | -101.3 dB | PASS | latency 67 samples, Newton failures 0 (rescued 0), max iterations 7 |
| iir | SIN-100Hz--40dBFS | 0.111 | -96.6 dB | PASS | latency 6 samples, Newton failures 0 (rescued 0), max iterations 2 |
| iir | SIN-100Hz--20dBFS | 0.719 | -107.8 dB | PASS | latency 6 samples, Newton failures 0 (rescued 0), max iterations 4 |
| iir | SIN-100Hz--6dBFS | 0.826 | -110.9 dB | PASS | latency 6 samples, Newton failures 0 (rescued 0), max iterations 4 |
| iir | SIN-1000Hz--40dBFS | 0.360 | -102.5 dB | PASS | latency 6 samples, Newton failures 0 (rescued 0), max iterations 5 |
| iir | SIN-1000Hz--20dBFS | 0.781 | -97.8 dB | PASS | latency 6 samples, Newton failures 0 (rescued 0), max iterations 7 |
| iir | SIN-1000Hz--6dBFS | 0.915 | -92.6 dB | PASS | latency 6 samples, Newton failures 0 (rescued 0), max iterations 8 |
| iir | SIN-5000Hz--40dBFS | 0.827 | -105.5 dB | PASS | latency 6 samples, Newton failures 0 (rescued 0), max iterations 7 |
| iir | SIN-5000Hz--20dBFS | 0.850 | -109.7 dB | PASS | latency 6 samples, Newton failures 0 (rescued 0), max iterations 9 |
| iir | SIN-5000Hz--6dBFS | 0.859 | -108.2 dB | PASS | latency 6 samples, Newton failures 0 (rescued 0), max iterations 10 |
| iir | DI-GTR L | 0.834 | -101.4 dB | PASS | latency 6 samples, Newton failures 0 (rescued 0), max iterations 7 |
| iir | DI-GTR LL | 0.784 | -100.8 dB | PASS | latency 6 samples, Newton failures 0 (rescued 0), max iterations 7 |
| iir | DI-GTR R | 0.913 | -101.8 dB | PASS | latency 6 samples, Newton failures 0 (rescued 0), max iterations 7 |
| iir | DI-GTR RR | 0.879 | -101.3 dB | PASS | latency 6 samples, Newton failures 0 (rescued 0), max iterations 7 |
