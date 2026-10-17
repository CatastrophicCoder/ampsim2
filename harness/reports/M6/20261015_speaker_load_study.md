# M6 speaker-load study: 4x12-style impedance vs the 16 Ω resistor (SPICE copies, reference unchanged)

- Full netlist with the D-058 EL34; `Rload out 0 16` replaced by Re 13 Ω + (Le ‖ R_le) + a parallel RLC resonance at 100 Hz
  (Q 2). No published Thiele–Small data for the Celestion G12T-75 was found, so three representative variants bracket a
  nominal 16 Ω 4x12 (|Z| at 50 / 100 / 400 / 1k / 5k / 10k Hz):
  low 17 / 35 / 14 / 14 / 23 / 32 Ω (Le 0.5 mH ‖ 30 Ω, Res 22 Ω); typical 20 / 50 / 14 / 15 / 35 / 48 Ω (1.0 mH ‖ 45 Ω,
  37 Ω); high 26 / 70 / 15 / 16 / 46 / 62 Ω (1.5 mH ‖ 60 Ω, 57 Ω).
- "Your settings" (owner, Logic): Preamp 10, Treble 7, Middle 1, Bass 0, Presence 7, Master 3. Speaker voltage is the
  plugin's output (the cab IR follows it).

## Small-signal response at the speaker, difference to 16 Ω

| Variant | 50 Hz | 100 Hz | 200 Hz | 400 Hz | 1 kHz | 2 kHz | 3 kHz | 5 kHz | 8 kHz |
|---|---|---|---|---|---|---|---|---|---|
| low (default knobs) | +0.6 | +3.0 | +0.3 | −0.8 | −0.9 | −0.1 | +0.6 | +1.8 | +2.7 dB |
| typical (default knobs) | +1.9 | +4.0 | +1.3 | −0.6 | −0.4 | +1.2 | +2.4 | +3.5 | +4.0 dB |
| high (default knobs) | +3.2 | +4.7 | +2.4 | −0.3 | +0.1 | +2.3 | +3.4 | +4.3 | +4.7 dB |
| typical (your settings) | +1.8 | +4.0 | +1.4 | −0.6 | −0.5 | +1.3 | +2.5 | +3.7 | +4.4 dB |

## Driven tones (0.3 V, your settings): spectral centroid and band energy vs 16 Ω

| Tone | 16 Ω centroid | speaker loads centroid | 5–10 kHz energy | 2–5 kHz energy | < 300 Hz energy |
|---|---|---|---|---|---|
| 110 Hz | 1427 Hz | 1590 … 1624 Hz | +1.7 … +3.5 dB | 0.0 … +2.3 dB | +2.7 … +4.3 dB |
| 220 Hz | 3308 Hz | 3878 … 3971 Hz | +1.7 … +3.4 dB | +0.7 … +2.9 dB | +0.1 … +2.0 dB |
| 330 Hz | 4864 Hz | 5817 … 5888 Hz | +1.7 … +3.4 dB | +0.8 … +3.0 dB | ≈ 0 dB |

Output level rises 0 … +3.5 dB with the speaker loads; the harmonic ratios H2–H6 change by up to ±4 dB (110 Hz).
