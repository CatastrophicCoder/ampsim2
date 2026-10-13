# M2 diagnostics: b12

- Date: 2026-10-13, engine commit `2f6e4c4 (dirty)`, ngspice 47, fs = 384000 Hz, SPICE reltol 1e-06 unless stated
- Output node p1b; ESR on each signal minus its own DC operating point (D-018). Not scored.

## 1. Grid-diode CJO/TT (ignored by the engine, SPEC §5)

SPICE′ = SPICE with CJO = TT = 0 in a copy of tubes.lib (harness/cache/; reference unchanged).

| Preamp Volume | Signal | SPICE′ vs SPICE | engine vs SPICE | engine vs SPICE′ |
|---|---|---|---|---|
| 0.2 | SIN-100Hz-0.01V | -95.6 dB | -76.2 dB | -76.1 dB |
| 0.2 | SIN-1000Hz-0.01V | -72.4 dB | -69.0 dB | -72.1 dB |
| 0.2 | SIN-5000Hz-0.01V | -59.1 dB | -56.3 dB | -59.0 dB |
| 0.2 | SIN-5000Hz-2V | -57.0 dB | -55.9 dB | -59.2 dB |
| 0.6 | SIN-100Hz-0.01V | -92.7 dB | -76.1 dB | -76.1 dB |
| 0.6 | SIN-1000Hz-0.01V | -70.2 dB | -67.5 dB | -72.1 dB |
| 0.6 | SIN-5000Hz-0.01V | -57.4 dB | -56.1 dB | -61.0 dB |
| 0.6 | SIN-5000Hz-2V | -54.5 dB | -52.4 dB | -53.5 dB |
| 1 | SIN-100Hz-0.01V | -89.0 dB | -75.6 dB | -76.1 dB |
| 1 | SIN-1000Hz-0.01V | -68.8 dB | -66.2 dB | -72.4 dB |
| 1 | SIN-5000Hz-0.01V | -57.6 dB | -56.1 dB | -62.9 dB |
| 1 | SIN-5000Hz-2V | -53.5 dB | -49.4 dB | -50.2 dB |

## 2. Engine discretisation

ESR vs SPICE with the engine at 1x / 2x / 4x of 384 kHz (input: the piecewise-linear signal through the 384 kHz samples, as SPICE sees it; output at the 384 kHz instants), and at 1x after an 8th-order low-pass (40 kHz, 20 kHz, 5 kHz, zero phase) on both signals.

| Preamp Volume | Signal | 384 kHz | 768 kHz | 1536 kHz | LP 40 kHz | LP 20 kHz | LP 5 kHz |
|---|---|---|---|---|---|---|---|
| 0.2 | STEP-0.1V | -34.5 dB | -44.4 dB | -52.6 dB | -44.1 dB | -50.7 dB | -63.7 dB |
| 0.2 | SIN-5000Hz-0.1V | -56.3 dB | -58.5 dB | -58.6 dB | -56.3 dB | -56.3 dB | -56.3 dB |
| 0.2 | SIN-5000Hz-2V | -55.9 dB | -56.9 dB | -56.8 dB | -56.0 dB | -56.0 dB | -56.9 dB |
| 0.6 | STEP-0.1V | -52.1 dB | -58.3 dB | -59.6 dB | -55.5 dB | -58.0 dB | -65.3 dB |
| 0.6 | SIN-5000Hz-0.1V | -56.1 dB | -56.9 dB | -56.9 dB | -56.1 dB | -56.1 dB | -56.1 dB |
| 0.6 | SIN-5000Hz-2V | -52.4 dB | -54.9 dB | -54.7 dB | -57.5 dB | -58.6 dB | -63.1 dB |
| 1 | STEP-0.1V | -63.2 dB | -67.1 dB | -67.6 dB | -65.6 dB | -66.8 dB | -70.0 dB |
| 1 | SIN-5000Hz-0.1V | -56.1 dB | -56.8 dB | -56.9 dB | -56.1 dB | -56.1 dB | -56.1 dB |
| 1 | SIN-5000Hz-2V | -49.4 dB | -53.8 dB | -54.0 dB | -56.0 dB | -57.5 dB | -61.9 dB |

## 3. SPICE reference convergence (harmonics vs RELTOL)

Preamp Volume 0.2, SIN-5000Hz-0.1V:

| Source | H2 | H3 | H4 | ESR engine vs this |
|---|---|---|---|---|
| engine | -41.60 dBc | -71.23 dBc | -99.20 dBc | — |
| SPICE reltol 1e-05 | -41.61 dBc | -71.28 dBc | -98.15 dBc | -55.7 dB |
| SPICE reltol 1e-06 | -41.61 dBc | -70.68 dBc | -98.38 dBc | -56.3 dB |
| SPICE reltol 1e-07 | -41.61 dBc | -71.03 dBc | -98.83 dBc | -56.2 dB |
| SPICE reltol 1e-08 | -41.62 dBc | -71.13 dBc | -99.16 dBc | -56.1 dB |
