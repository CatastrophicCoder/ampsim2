# Reference model: Marshall JCM800 2203 (SPICE)

Transcribed from the Marshall drawings "2203 STD Preamp" (issue 4) and "2203 STD Output Stage & PSU"
(issue 6); the drawings are not included in the repository (docs/DECISIONS.md D-021). This model is the ground truth for the WDF engine.

## Files
- `spice/jcm800_2203.cir` — full amp: preamp, tone stack, master, long-tail-pair PI with NFB and
  presence, 4x EL34, output transformer, HT supply chain, bias network.
- `spice/tubes.lib` — Koren 12AX7 and EL34 models. The EL34 screen current is a modification (see below).
- `spice/spice_tools.py` — helpers: `op`, `ac`, `tran`, `thd_power`. Run it directly to print the DC
  operating point.
- `plots/tonestack_sweeps.png` — tone stack response (V2b cathode → Treble wiper) for each control.
- `plots/overdrive_stages.png` — stage waveforms at Preamp 8, Master 3, 300 mV peak @ 220 Hz.

Knobs are `.param` rotations 0..1: `vol treble middle bass master presence` (log pots use an A-taper
approximation, 10 % at mid rotation). `rv1` is the bias pot, `vnl`/`rsrc` the HT source.

```python
import sys; sys.path.insert(0, 'reference/spice')
from spice_tools import op, ac, tran, thd_power
print(op(['v(ct)', 'v(p1a)'], vol=0.8))
f, h = ac('db(v(out)/v(in))', presence=1.0)
t, (vin, vout) = tran(0.025, 1000, vol=0.2, master=1.0)
print(thd_power(t, vout, 1000))      # ≈ (101 W, 2.8 %)
```

## Transcription items to double-check
See `docs/CIRCUIT.md` §6 (O1–O6): C18 47 pF between the PI outputs; Middle pot wiring as drawn;
C5 1 nF across Preamp Volume top-to-wiper; "50+50 µF" cans modelled as 50 µF per node; NFB from the
16 Ω tap; Low input not modelled.

## Assumed values (not on the schematic)
HT source 505 V behind 100 Ω; bias −60 V at the C20 node with RV1 = 4 kΩ; generic Koren tube parameters; resistive
16 Ω load; ideal guitar source.

## Output transformer (Hammond 1750U datasheet)
OT 1.7 kΩ : 16 Ω (nominal ratio), DCR 15.36 / 16.56 Ω per half-primary (Blue–White on the pA side) and
0.320 Ω secondary, coupling k = 0.9997925 from the 7.97 mH leakage inductance, primary inductance 25.61 H
fitted to the datasheet's 27 dBu response and phase curves (`python -m harness.ot_fit`). The 1750U is
Hammond's drop-in replacement for the original part; see `docs/CIRCUIT.md` §3 and `docs/DECISIONS.md`
D-023.

## Choke (Hammond 194F datasheet)
5 H @ 120 mA DC, DCR 115 Ω (±15 % @ 20 °C), Hammond's replacement "for JCP and JCM800" (D-024). The amp
runs it at ≈ 27 mA; the inductance at that current is not specified and stays 5 H.

## Sanity results
- DC: B+ 489 V, screens 486 V, PI supply 402 V, preamp supply 389 V; EL34 34.1 mA plate + 4.57 mA screen
  per tube (≈ 16.6 W plate dissipation, ≈ 66 % of 25 W).
- Global NFB ≈ 6 dB: 6.6 / 6.2 / 6.0 dB at 100 Hz / 1 kHz / 5 kHz (verified negative by removing R22).
- Clean power (Preamp 2, Master 10, 1 kHz; input 25 / 30 / 40 mV peak): ≈ 101 W at 3.2 % THD,
  ≈ 133 W at 3.5 %, ≈ 151 W at 9.6 % (assumed OT and choke before D-023/D-024: 101 W / 130 W / 145 W at
  2.8 / 3.8 / 9.6 %).
- Overdrive (Preamp 8, Master 3, 300 mV @ 220 Hz): V2a is the main clipping stage; B+ sags ≈ 20 V.
- Speed: ≈ 27x slower than real time for the full amp.

## Known limitations
No rectifier/ripple or mains-transformer dynamics, no OT core saturation or hysteresis, no reactive
speaker load, generic tube models. The Koren pentode screen equation gives ~0 screen current at this
bias, so `tubes.lib` computes screen current from the same E1 term as the plate current, plus diversion
of plate current to the screen when the plate swings low.
