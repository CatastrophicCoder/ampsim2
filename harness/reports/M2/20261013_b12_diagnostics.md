# M2 diagnostics: b12

- Date: 2026-10-13, engine commit `9c92e73 (dirty)`, ngspice 47, fs = 768000 Hz, SPICE reltol 1e-08 unless stated
- Output node p1b; ESR on each signal minus its own DC operating point (D-018). Not scored.

## 1. Grid-diode CJO/TT (ignored by the engine, SPEC §5)

SPICE′ = SPICE with CJO = TT = 0 in a copy of tubes.lib (harness/cache/; reference unchanged).

| Preamp Volume | Signal | SPICE′ vs SPICE | engine vs SPICE | engine vs SPICE′ |
|---|---|---|---|---|
| 0.2 | SIN-100Hz-0.01V | -95.6 dB | -76.2 dB | -76.1 dB |
| 0.2 | SIN-1000Hz-0.01V | -72.4 dB | -69.9 dB | -73.2 dB |
| 0.2 | SIN-5000Hz-0.01V | -59.1 dB | -58.4 dB | -67.1 dB |
| 0.2 | SIN-5000Hz-2V | -57.0 dB | -56.9 dB | -65.5 dB |
| 0.6 | SIN-100Hz-0.01V | -92.7 dB | -76.1 dB | -76.1 dB |
| 0.6 | SIN-1000Hz-0.01V | -70.2 dB | -68.2 dB | -73.3 dB |
| 0.6 | SIN-5000Hz-0.01V | -57.4 dB | -57.0 dB | -68.3 dB |
| 0.6 | SIN-5000Hz-2V | -54.5 dB | -54.9 dB | -64.8 dB |
| 1 | SIN-100Hz-0.01V | -89.0 dB | -75.6 dB | -76.1 dB |
| 1 | SIN-1000Hz-0.01V | -68.8 dB | -66.6 dB | -73.4 dB |
| 1 | SIN-5000Hz-0.01V | -57.6 dB | -57.0 dB | -69.5 dB |
| 1 | SIN-5000Hz-2V | -53.5 dB | -53.8 dB | -61.1 dB |

## 2. Engine discretisation

ESR vs SPICE with the engine at 1x / 2x / 4x of 768 kHz (input: the piecewise-linear signal through the verification-rate samples, as SPICE sees it; output at those instants), and at 1x after an 8th-order low-pass (40 kHz, 20 kHz, 5 kHz, zero phase) on both signals.

| Preamp Volume | Signal | 768 kHz | 1536 kHz | 3072 kHz | LP 40 kHz | LP 20 kHz | LP 5 kHz |
|---|---|---|---|---|---|---|---|
| 0.2 | STEP-0.1V | -55.6 dB | -56.7 dB | -56.8 dB | -55.8 dB | -57.3 dB | -65.2 dB |
| 0.2 | SIN-5000Hz-0.1V | -58.4 dB | -58.6 dB | -58.7 dB | -58.4 dB | -58.4 dB | -58.4 dB |
| 0.2 | SIN-5000Hz-2V | -56.9 dB | -56.9 dB | -56.8 dB | -56.9 dB | -57.0 dB | -59.6 dB |
| 0.6 | STEP-0.1V | -60.3 dB | -60.5 dB | -60.5 dB | -60.4 dB | -60.7 dB | -65.4 dB |
| 0.6 | SIN-5000Hz-0.1V | -56.9 dB | -56.9 dB | -56.9 dB | -56.9 dB | -56.9 dB | -56.9 dB |
| 0.6 | SIN-5000Hz-2V | -54.9 dB | -54.7 dB | -54.6 dB | -55.6 dB | -57.0 dB | -62.2 dB |
| 1 | STEP-0.1V | -67.9 dB | -68.0 dB | -68.0 dB | -67.9 dB | -68.0 dB | -69.9 dB |
| 1 | SIN-5000Hz-0.1V | -56.8 dB | -56.9 dB | -56.9 dB | -56.8 dB | -56.8 dB | -56.8 dB |
| 1 | SIN-5000Hz-2V | -53.8 dB | -54.0 dB | -53.9 dB | -55.0 dB | -56.8 dB | -61.5 dB |

## 3. SPICE reference convergence (harmonics vs RELTOL)

Preamp Volume 0.2, SIN-5000Hz-0.1V:

| Source | H2 | H3 | H4 | ESR engine vs this |
|---|---|---|---|---|
| engine | -41.60 dBc | -71.24 dBc | -99.21 dBc | — |
| SPICE reltol 1e-05 | -41.61 dBc | -71.23 dBc | -99.19 dBc | -58.0 dB |
| SPICE reltol 1e-06 | -41.61 dBc | -71.18 dBc | -98.73 dBc | -58.2 dB |
| SPICE reltol 1e-07 | -41.61 dBc | -71.02 dBc | -99.11 dBc | -58.4 dB |
| SPICE reltol 1e-08 | -41.61 dBc | -71.13 dBc | -99.18 dBc | -58.4 dB |
