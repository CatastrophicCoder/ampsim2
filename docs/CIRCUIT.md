# Circuit Description: Marshall JCM800 2203

Source drawings: Marshall "2203 STD Preamp" (issue 4, 19-5-88) and "2203 STD Output Stage & PSU"
(issue 6), not included in the repository (DECISIONS D-021). The authoritative machine-readable form is
`reference/spice/jcm800_2203.cir`. If this document and the netlist disagree, the netlist wins and this
document must be corrected.

Node names below are the netlist's. "gnd" is node 0.

---

## 1. Blocks and components

### B12 — Input stage V1a and Preamp Volume network V1b (one block, joint root; DECISIONS D-017)

V1a part:
| Ref | Value | Between | Notes |
|---|---|---|---|
| Vin | — | in – gnd | Guitar source (ideal in v1; optional pickup model later) |
| R2 | 1 MΩ | in – gnd | Input grid leak |
| R3 | 68 kΩ | in – g1a | Grid stopper (with Cgp: Miller low-pass) |
| V1a | 12AX7 | p1a, g1a, k1a | |
| R1 | 2.7 kΩ | k1a – gnd | |
| C1 | 0.68 µF | k1a – gnd | Full cathode bypass |
| R4 | 100 kΩ | bpA – p1a | Plate load; bpA from supply B7 |

There is no junction between V1a and V1b: p1a is an internal node of B12 (the J1 junction was
measured and removed in M0; see §4 and DECISIONS D-017).

V1b part:
| Ref | Value | Between | Notes |
|---|---|---|---|
| C3 | 0.022 µF | p1a – nA | Coupling |
| R5 | 470 kΩ | nA – vtop | |
| C4 | 470 pF | nA – vtop | Bright cap across R5 |
| C5 | 1 nF | vtop – w1 | Across VR1 top-to-wiper (as drawn; open item O3) |
| VR1 | 1 MΩ log | vtop / w1 / gnd | Preamp Volume; wiper w1 = V1b grid (no stopper) |
| V1b | 12AX7 | p1b, w1, k1b | |
| R6 | 10 kΩ | k1b – gnd | Unbypassed cathode |
| R7 | 100 kΩ | bpA – p1b | Plate load |

Note: in the real amp the Low input jack connects at node nA, bypassing V1a (out of scope, O6).

### B3 — V2a gain stage and V2b cathode follower (joint root)
| Ref | Value | Between | Notes |
|---|---|---|---|
| (J2) | v_oc(p1b), R_th2 | → C7 | |
| C7 | 0.022 µF | p1b – nB | Coupling |
| R10 | 470 kΩ | nB – g2a | |
| C8 | 470 pF | nB – g2a | Across R10 |
| R11 | 470 kΩ | g2a – gnd | |
| V2a | 12AX7 | p2a, g2a, k2a | Main clipping stage |
| R9 | 820 Ω | k2a – gnd | Unbypassed |
| R12 | 100 kΩ | bpX – p2a | Plate load; p2a is also V2b's grid (direct coupling) |
| V2b | 12AX7 | bpX (plate), p2a (grid), k2b | Cathode follower |
| R13 | 100 kΩ | k2b – gnd | |

Output junction J3: `v_oc(k2b)`, `R_th3` (expected several hundred ohms).

### B4 — Tone stack and Master (linear)
| Ref | Value | Between | Notes |
|---|---|---|---|
| (J3) | v_oc(k2b), R_th3 | → k2b | |
| C10 | 470 pF | k2b – tt | Treble cap |
| R15 | 33 kΩ | k2b – nD | Slope resistor |
| C11 | 0.022 µF | nD – nE | Bass cap |
| C12 | 0.022 µF | nD – mw | Mid cap, to Middle **wiper** as drawn (O2) |
| VR3 | 220 kΩ lin | tt / tw / nE | Treble; wiper tw feeds Master via link "R14" |
| VR5 | 1 MΩ log | nE – nbb | Bass, rheostat (wiper tied to top) |
| VR4 | 22 kΩ lin | nbb / mw / gnd | Middle |
| VR2 | 1 MΩ log | tw / y / gnd | Master; wiper y feeds the PI |

Output junction J4: open-circuit `v_oc(y)` and `R_th4` (master- and stack-dependent; formula chosen in
M1/M4 and documented in `DECISIONS.md`).

### B5 — Phase inverter (long-tail pair), NFB, Presence
| Ref | Value | Between | Notes |
|---|---|---|---|
| (J4) | v_oc(y), R_th4 | → C9 | |
| C9 | 0.022 µF | y – g3a | Coupling |
| R17 | 1 MΩ | g3a – nF | Grid leak returned to tail tap (bootstrapped) |
| R16 | 470 Ω | k3 – nF | Shared cathode resistor |
| R19 | 1 MΩ | nF – g3b | |
| C15 | 0.1 µF | g3b – nG | V3b grid AC-coupled to the feedback node |
| R20 | 10 kΩ | nF – nG | Tail |
| R23 | 4.7 kΩ | nG – gnd | Feedback shunt |
| C17 | 0.1 µF | nG – npr | Presence |
| VR6 | 22 kΩ lin | npr – gnd | Presence rheostat |
| R22 | 100 kΩ | out – nG | Global NFB from 16 Ω output (O5); out = previous sample in v1 |
| V3a / V3b | 12AX7 | p3a,g3a,k3 / p3b,g3b,k3 | Shared cathode k3 |
| R18 | 82 kΩ | bpX – p3a | |
| R21 | 100 kΩ | bpX – p3b | |

Output junction J5: `v_oc(p3a)`, `v_oc(p3b)`, `R_thA`, `R_thB`.

### B6 — Output stage and output transformer
| Ref | Value | Between | Notes |
|---|---|---|---|
| (J5) | Thevenin sources | → C14, C16 | |
| C14 | 0.022 µF | p3a – nGn | Drives V4/V5 |
| C16 | 0.022 µF | p3b – nOr | Drives V6/V7 |
| C18 | 47 pF | nGn – nOr | Between the two PI outputs (O1) |
| R24 / R25 | 220 kΩ each | nGn – nbias / nbias – nOr | Grid leaks to bias |
| C19 | 10 µF | nbias – gnd | Bias filter |
| Bias Thevenin | −46.364 V, 11.59 kΩ | → nbias | Equivalent of Vc20 = −60 V, R27 15 kΩ, R26 47 kΩ + RV1 4 kΩ |
| R31,R33 / R32,R34 | 5.6 kΩ each | nGn – g4,g5 / nOr – g6,g7 | Grid stoppers (pair equivalent 2.8 kΩ) |
| R35,R37 / R36,R38 | 1 kΩ 5 W each | screens – nScr | (pair equivalent 500 Ω) |
| V4–V7 | EL34 | cathodes to gnd | Fixed bias; V4‖V5 plate pA, V6‖V7 plate pB |
| OT | see §3 | pA – ct – pB, secondary out – gnd | |
| Rload | 16 Ω | out – gnd | |

Bias Thevenin check: Vth = −60 V · 51 kΩ / 66 kΩ = −46.364 V; Rth = 15 kΩ ‖ 51 kΩ = 11.59 kΩ.

### B7 — HT supply
| Ref | Value | Between | Notes |
|---|---|---|---|
| Vht | 505 V | raw – gnd | Assumed no-load DC (rectifier + mains TX approximated) |
| Rsrc | 100 Ω | raw – ct | Assumed source resistance (sag) |
| C23 | 50 µF | ct – gnd | Reservoir; OT centre tap here (O4) |
| T3 choke | 5 H, 60 Ω | ct – nScr | Assumed values |
| C22 | 50 µF | nScr – gnd | Screen supply (O4) |
| R29 | 10 kΩ 1 W | nScr – bpX | |
| C21b | 50 µF | bpX – gnd | Feeds V2a, V2b, PI (O4) |
| R8 | 10 kΩ 1 W | bpX – bpA | |
| C21a | 50 µF | bpA – gnd | Feeds V1a, V1b (O4) |

---

## 2. Device parameters (from `reference/spice/tubes.lib`)

| | mu | ex | kg1 | kg2 | kp | kvb | rgi | Ccg | Cgp | Ccp |
|---|---|---|---|---|---|---|---|---|---|---|
| 12AX7 | 100 | 1.4 | 1060 | — | 600 | 300 | 2000 Ω | 2.3 pF | 2.4 pF | 0.9 pF |
| EL34 | 11 | 1.35 | 650 | 4200 | 60 | 24 | 1000 Ω | 15 pF | 1 pF | 8 pF |

Grid diode: IS = 1 nA, RS = 1 Ω (in series with rgi), n = 1. CJO = 1 pF and TT = 1 ns are ignored in
the engine. EL34 screen current uses the repository modification (see `WDF_DESIGN.md` §3.2).

---

## 3. Output transformer (assumed values; C2668 data not available)

| Parameter | Value |
|---|---|
| Plate-to-plate impedance at 16 Ω | 1.7 kΩ → turns ratio n = sqrt(1700/16) = 10.308 (full primary : secondary) |
| Total primary inductance | 25 H (each half-primary 6.25 H) |
| Secondary inductance | 25 / n² = 0.2353 H |
| Coupling coefficient (all pairs) | 0.999 |
| Winding resistance | 40 Ω per half-primary, 0.3 Ω secondary |
| Load | 16 Ω resistive |

The engine's linear equivalent circuit must reproduce the SPICE coupled-inductor model: verify with a
small-signal comparison of the transformer network alone (plates driven by ideal current sources,
SPICE vs engine) in M4.

---

## 4. Junction Thevenin resistances (measured in M0)

Measured with `python -m harness.rth_measure` (DECISIONS D-016; report
`harness/reports/M0/20261012_junctions.md`), default parameters, upstream block testbench with the output
open and the input AC-grounded. The value used by the testbenches and the engine is |Z(1 kHz)|
(`harness/junctions.py`; DECISIONS D-018); J5 is provisional until its partition error is measured in M4.

| Junction | Node | R_th (DC small-signal) | Z @ 1 kHz | Spread over presets (\|Z\| @ 1 kHz) | Method / notes |
|---|---|---|---|---|---|
| (J1) | p1a | 74.79 kΩ | 39.25 kΩ ∠−15.1° | none | Removed: B1 and B2 merged into B12 (D-017). C1 bypasses the cathode above ~100 Hz; \|Z\| falls to 31 kΩ at 10 kHz. Measured at commit a64bf9b |
| J2 | p1b | 90.11 kΩ | 89.73 kΩ ∠−1.0° | 88.4–90.1 kΩ (Preamp Volume, Miller) | Measured on B12. R6 unbypassed; flat to ~1 kHz, 85.6 kΩ at 10 kHz |
| J3 | k2b | 359.9 Ω | 359.9 Ω ∠+0.1° | none | Cathode follower; flat |
| J4 | y | R_VR2b ‖ (R_VR2a + R_VR3b + R_VR5 + R_VR4) (knob-dependent) | — | — | DC, J3 shorted, caps open; reactive part ignored (DECISIONS D-015) |
| J5a | p3a | 41.40 kΩ | 58.06 kΩ ∠−0.9° | 56.2–58.1 kΩ (Master) | LTP: DC value only below ~1 Hz; 57–59 kΩ flat 10 Hz–5 kHz. Transimpedance to p3b 25.5 kΩ @ 1 kHz (0.44 of self) |
| J5b | p3b | 45.51 kΩ | 56.93 kΩ ∠+0.4° | 56.9–59.1 kΩ (Master) | As J5a; transimpedance to p3a 32.1 kΩ @ 1 kHz (0.56 of self) |

J1 partition error (SPICE, harness/partition.py at commit a64bf9b, report
`harness/reports/M0/20261012_partition_j1.md`): the b1 → b2 chain vs B1+B2 joined, at p1b. Worst
sine ESR with R_th1 = DC / \|Z(1 kHz)\| / Re Z(1 kHz): CLEAN −8.7 / −11.2 / −11.2 dB, DEFAULT −14.8 / −17.6 /
−17.6 dB, DRIVE −22.6 / −19.3 / −19.1 dB. Cause: B2's input impedance at p1a falls to 54 kΩ (CLEAN) /
177 kΩ (DEFAULT) at 10 kHz (C5 bright cap, C4, V1b Miller), so J1's frequency-dependent output impedance
matters; no constant R_th1 represents it. Owner decision: merge B1 and B2 into B12 (DECISIONS
D-017).

J2 partition error (`python -m harness.partition --junction j2`, report
`harness/reports/M0/20261012_partition_j2.md`): the b12 → b3 chain vs B12+B3 joined (b123), at k2b. The
three candidates differ by < 0.4 % and give the same result within 0.1 dB. Worst ESR (always a 2 V or
0.5 V 5 kHz sine): CLEAN −30.6 dB, DEFAULT −27.0 dB, DRIVE −26.2 dB; small-signal magnitude error
≤ 0.18 dB (20 Hz–20 kHz); harmonics 2–5 within 1.4–3.2 dB. At the loaded junction node p1b the worst is
−19.4 dB (DRIVE, 2 V, 5 kHz), where V2a grid current loads V1b at large swing.

---

## 5. Reference DC operating point (default parameters, from SPICE)

Defaults: vol 0.6, treble/middle/bass/presence 0.5, master 0.3, rv1 4 kΩ, vnl 505 V, rsrc 100 Ω.

| Node | Voltage | Node | Voltage |
|---|---|---|---|
| ct (B+, OT CT) | 488.445 V | p2a | 231.658 V |
| nScr | 486.828 V | k2a | 1.408 V |
| bpX | 403.307 V | k2b | 232.216 V |
| bpA | 390.288 V | p3a | 271.512 V |
| nbias | −46.364 V | p3b | 262.884 V |
| p1a | 297.104 V | k3 | 45.049 V |
| k1a | 2.516 V | nF | 43.634 V |
| p1b | 353.278 V | nG | 13.519 V |
| k1b | 3.701 V | | |

| Current | Value |
|---|---|
| V1a plate (R4) | 0.932 mA |
| V1b plate (R7) | 0.370 mA |
| V2a plate (R12) | 1.716 mA |
| V2b cathode (R13) | 2.322 mA |
| PI plates (R18 / R21) | 1.607 / 1.404 mA |
| R29 (preamp + PI total from nScr) | 8.352 mA |
| EL34 plate, per tube | 34.65 mA |
| EL34 screen, per tube | 4.65 mA |
| Total HT current (Vht) | 165.6 mA |

Regenerate with `python3 reference/spice/spice_tools.py` after any netlist change.

---

## 6. Open items (transcription and assumptions)

| ID | Item | Current model | Impact if wrong |
|---|---|---|---|
| O1 | C18 47 pF connection | Between the two PI outputs after C14/C16 | HF balance of PI drive |
| O2 | Middle pot wiring | As drawn: C12 to wiper, Bass bottom to top lug | ≤ 1.4 dB, only at low Middle settings |
| O3 | C5 1 nF | VR1 top to wiper | Brightness vs Preamp Volume setting |
| O4 | "50+50 µF" filter cans | 50 µF per node | Sag and ripple time constants |
| O5 | NFB tap | 16 Ω output with 16 Ω load | Feedback amount if taken from another tap |
| O6 | Low input | Not modelled | Feature only |
| A1 | HT source 505 V / 100 Ω | Assumed | B+ level and sag |
| A2 | OT values | Assumed (see §3) | Power, LF/HF response, feedback |
| A3 | Choke 5 H / 60 Ω | Assumed | Screen node dynamics |
| A4 | Bias −60 V at C20, RV1 4 kΩ | Assumed | Idle current, crossover |
| A5 | Koren generic tube parameters | Generic | Overall voltages and distortion |

Any change to these must be made in the netlist first, re-verified, and logged in `DECISIONS.md`.
