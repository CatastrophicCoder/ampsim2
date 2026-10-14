# Architecture study: a playable, knob-accurate JCM800 2203 plugin

Date: 2026-10-14 (M6, owner request: "comprehensive research how to architecture the model so that it will be
playable through a plugin, adjusting the knobs won't make it useless and can be used together with other
plugins"). Sources: four literature / product surveys (academic methods, commercial products, neural and
hybrid models, fast solver techniques; source lists in §7) and this repository's measurements.

Criteria used throughout (the owner's priorities, PROGRESS.md / M5):
- **C1 latency**: no noticeable added latency.
- **C2 authenticity**: sounds like the amp at every knob setting.
- **C3 CPU**: light enough to run with other plugins. Unit below: % of one Apple M4 Pro core at 48 kHz (budget
  20.8 µs per host sample). The study uses ≤ 20 % as a working reference point; the owner may set another.
- **C4 knobs**: knob moves stay smooth and accurate (no retraining per setting, no instability).
- **C5 verifiability**: can be checked against the existing ngspice reference with the existing harness.
- **C6 effort / risk**: work to implement, and how much is unknown before trying.

---

## 1. Why the current architecture stalls (measured)

| Fact | Value | Source |
|---|---|---|
| Engine cost per engine sample | 2.24 µs default knobs, 3.02 µs at Master + Presence 10 | M6 Step 0 |
| Plugin CPU at 8x (384 kHz) | ≈ 86 % / 116 % of one core | 8 × 2.24 / 20.8 µs |
| Plugin CPU at 16x (768 kHz, the verification rate) | ≈ 172 % / 200 % | |
| Gap to a 20 % budget at 8x | ≈ 4.3× – 5.8× | |
| Where the time goes | dense LU 28 %, Jacobian build 22 %, tube transcendental functions 23 %, network products 13 % | M6 profile |
| Exact-output optimisations found in M5 + M6 | −22 % (M5) and −2 to −4 % (M6) | D-046, D-049 |
| Why the rate is 384–768 kHz | trapezoidal fixed-step error at tube cutoff transitions against a −40 … −60 dB ESR target vs SPICE (5 kHz high-level sines) | D-034, D-035, D-047 |
| Accuracy hole found in M6 | J4 partition (preamp → power amp at the master wiper): out ESR −71 dB at Master 0.3, −48 dB at 0.7, −31 dB at 0.85, −13 to −15 dB at 1.0 | M6 HOT study |
| Exact J4 fixes, cost | joint solve est. +5–15 %; merged 18-port root: LU 2.1× (measured), est. ≈ 0.7× real time at 8x | M6 |

Two structural causes follow from these numbers:
1. **The rate is set by the verification target.** The tolerances (VERIFICATION.md) ask for −40 to −60 dB ESR
   against SPICE on stiff high-level signals. With a fixed-step trapezoidal rule this needs 384–768 kHz. No
   paper or product found reports a real-time model held to that target; published real-time models are
   checked by plots, < 0.2 % peak error, or listening tests (§2).
2. **The circuit's couplings force either large Newton roots or partitions.** Large roots cost O(K³) per
   iteration at a very high rate; partitions (J4, earlier J1, J2, J5) lose accuracy in parts of the knob range
   that were not on the verified presets.

Neither cause is removed by further optimisation of the current solver: the remaining exact-output speedups
are a few percent each (M6 notes), against a 4–6× gap at 8x.

---

## 2. What the literature and products show

### 2.1 Circuit simulation in real time (academic)
| Work | Scope | Method | Rate / cost | Accuracy evidence |
|---|---|---|---|---|
| Dunkel et al., DAFx-16 | Bassman 5F6-A preamp (4 triodes), no tone stack | one 25-port WDF root, Newton | 176 kHz; C++ "slightly slower than real time" | < 0.2 % vs LTspice |
| Zhang & Smith, DAFx-18 | Mesa Mark II-B preamp (5 triodes) | WDF, modified blockwise (pairs of triodes, 4-D Newton), 1-D tables for the tube model, explicit 4×4 inverse | 176 kHz, 15 % of a 2.3 GHz i5 | up to 4.5 V max error vs LTspice; partition error not quantified |
| Mačák, DAFx-13 / thesis 2012 | JCM800 preamp (3 triodes, tone stack as linear load) | precomputed per-triode tables + Newton only on 2 connection currents | 48 kHz, 10–15 % of a 2.66 GHz core (undecomposed: not real time) | < 0.1 % of peak vs the undecomposed model (not vs SPICE) |
| Mačák thesis §5.5 | push-pull power amp, OT, presence, global NFB | decomposition with the NFB current as 1 Newton unknown (3 with Miller capacitances) | real time (2012 hardware) | a one-sample NFB delay is audible; Miller effect judged small |
| Mačák & Schimmel DAFx-11 | output transformer | Frohlich / Jiles–Atherton vs linear | Frohlich 0.12 %, J-A 0.27 % CPU | hysteresis audible only below ≈ 150 Hz |
| Cohen & Hélie DAFx-10 | single-ended power stage | state-space, Newton | "MHz" rates | no SPICE figures |
| Holters & Zölzer (ACME.jl) | general circuits | DK state-space, k-d-tree solution cache, automatic decomposition | cache: 2.59 → 2.31 mean iterations; decomposition 0–19 % speedup | — |
| Yeh 2009 / DK tables | 1–2 triode stages | precomputed solution of the nonlinear equation | practical up to 2–3 (≤ 4) dimensions | — |

Observations: (a) the fastest circuit-level results precompute per-device or per-block solutions and keep only a
few coupling unknowns in Newton (Mačák); (b) no paper simulates a full preamp + power amp + NFB + sag chain
against SPICE; the preamp → power-amp coupling (our J4) is not treated anywhere found; (c) oversampling used is
1×–8× at 44.1/48 kHz, with 8× described as sufficient for aliasing in listening tests (Yeh; Mačák: mean 3–4×,
one listener 14×).

### 2.2 Speed techniques (literature, applicable to circuit models)
- Tables for the nonlinear solution: −80 to −90 % CPU on 2-triode blocks (Mačák 76 % → 10 %); limited to small
  dimensions; table error × stage gain appears at the output.
- Tabulated tube equations (2-D splines) and SIMD across tubes: attack the ≈ 25 % transcendental share.
- Non-iterative (linearly implicit / Rosenbrock) schemes: one linear solve per sample, no Newton; shown on small
  circuits at 3–7× oversampling with ≈ 6 dB spectral tolerance; Holters DAFx-24 reports error bursts at
  saturation transitions in a 45 dB gain stage.
- Fixed-point / extended fixed-point WD solvers (Bernardini et al.): no Jacobian inverse; no tube results found.
- ADAA: reduces aliasing only, not discretisation error; not demonstrated for multi-port roots.
- Adaptive sub-stepping around transitions: better SPICE agreement on small circuits (Zea & Rivera); in WDF it
  needs step-dependent port resistances (this repository's D-043 half-steps are one instance).

### 2.3 Commercial products (public information only)
- No vendor publishes its solver, internal rate or per-instance CPU on named hardware. Statements: Fractal
  (component-level "virtual tubes", amp block "high oversampling", measured correction MIMIC); Line 6, Softube,
  UA, Positive Grid, Nembrini ("every component"; Softube: circuit split into blocks, discretised, hand-optimised
  C); Mercuriall (circuit simulation for stompboxes, "Neural Hybrid Engine" for amps; 8–20 % CPU at 4–8× in
  2015 user reports).
- Machine learning on circuit or internal-state data: Native Instruments ICM (state-space with embedded neural
  network trained on measured internal circuit states), Neural DSP (robot-recorded knob positions, knobs as
  network inputs; research LSTM compared against an LTspice model).
- User-reported CPU: NAM ≈ 2 % of a Ryzen 5 core; Neural DSP 4–75 % per instance (incl. cabinets); AmpliTube
  ≈ 1 % for a rig. Not controlled measurements.

### 2.4 Neural and grey-box models (literature)
| Work | Knobs | Data | Accuracy | Cost |
|---|---|---|---|---|
| Wright et al. 2019/2020 | 1 (or none) | 3 min hardware audio per setting | ESR 0.2–4 % (−17 … −27 dB); MUSHRA 80–98 | RNN-32: 8.3× real time on an iMac i5 (≈ 12 %); WaveNet3 1.1× |
| Juvela et al. (Neural DSP) 2023 | 5 (Matchless DC-30) | ≈ 4.5 h, robot-set knobs | DMOS: no significant difference vs an LTspice model of the amp | LSTM-32 (not reported; same class as above) |
| Miklánek et al. DAFx-23 | 3 tone knobs | 4–12 min | grey-box (DK tone stack + RNNs) ESR 0.02–0.05 on unseen settings, "Excellent" at an extreme setting where black-box was "Fair" | real-time factor ≈ 20 |
| DDSP guitar amp 2024 | knobs → DSP params | JVM data | MAE ≈ 2× a GRU baseline | 1352 vs 19872 ops/sample |
| Chowdhury 2020 (Klon) | 5 settings, cross-faded | SPICE output | ESR 0.5–1.7 % | GRU-8 |
| Darabundit et al. DAFx-22 | — | LTspice | MLP replaces one triode's Newton in a WDF | NN alone 26–40× real time at 96 kHz |
| NAM A1/A2 | none (snapshots) | ≈ 3 min | vendor median ESR 0.3–0.6 % | ≈ 2 % of a desktop core; 0 latency at the model rate |

Reported limitations: aliasing at 44.1/48 kHz without oversampling (Wright 2020: aliases ≈ 40 dB below the
first harmonic, some above 15 kHz louder than nearby harmonics); extrapolation to unseen knob combinations
needs many settings for black-box models; high gain gives the worst ESR; the global NFB / presence loop is not
modelled in any grey-box paper found; no paper evaluates power-supply sag in a neural model.

---

## 3. Candidate architectures for this project

Estimates marked "est." are not measured here. CPU figures assume 48 kHz host rate and one M4 Pro core.

### A. Current engine, exact J4 coupling (joint solve), further tuning
- C1: unchanged (oversampler + 1 engine sample). C2: SPICE-level wherever verified, J4 fixed. C3: est. 90–130 %
  at 8x; ≈ 200 % at 16x. C4: proven (M5 automation). C5: full. C6: moderate effort, low unknowns.
- Does not meet a 20 % budget by a factor ≈ 5 on current evidence.

### B. Decomposed circuit model with precomputed tube blocks (Mačák-style connection currents)
- Each tube (or tube pair) as a precomputed table / spline of its port solution; Newton only on a few
  connection unknowns per section (preamp couplings, PI ↔ power stage, NFB current). Linear parts (tone stack,
  coupling networks, OT, supply) as exact WDF / state-space. Rate 4–8x.
- C1: oversampler only. C2: circuit-based at all knob settings; accuracy vs SPICE unknown — table error is
  amplified by stage gain, and the rate (4–8x) with a fixed-step rule misses the current −60 dB targets at
  stiff transitions. C3: literature 10–15 % for a JCM800 preamp at 1× on 2013 hardware; with power amp and 4–8x
  est. 15–60 %. C4: pots enter linear parts or table dimensions (Mačák: pots enlarge tables). C5: full harness
  usable; tolerances would likely need owner review. C6: large rewrite of solver and blocks; unknowns: table
  dimensions for the pentode pair (3 ports each) and PI, accuracy at 4–8x.

### C. Knob-conditioned neural model trained on simulation data ("teacher → student")
- A small recurrent / convolutional network with the six knobs (and Interface Level) as inputs, trained on
  output of an accurate offline simulator. Teacher options: ngspice (≈ 1.3 core-hours per 5 s DI clip at
  384 kHz, HOT preset — hours of training audio would take core-years: impractical) or this engine run offline
  at 768 kHz with an exact J4 coupling (≈ 0.5× real time per core; 10 h of audio ≈ 1–1.5 days on 14 cores),
  itself verified against SPICE on the presets.
- C1: zero added latency at the model rate (sample-by-sample RNN), plus oversampler if used. C2: literature
  ESR ≈ −17 … −30 dB vs the teacher; Neural DSP's 5-knob LSTM was not distinguishable from an LTspice model in a
  listening test; high gain and long-memory effects (sag) are the reported weak points. C3: RNN-32 class ≈ 2–12 %
  of a core (literature, older CPUs). C4: knobs as inputs, smooth by construction; coverage of 6 knobs ×
  input levels needs thousands of settings (Neural DSP: "a few thousand"). C5: ESR vs SPICE / teacher
  measurable with the harness; the −40 … −60 dB tolerances are not reachable by any reported model. C6: new
  tooling (training stack — PyTorch would be a new dependency; inference can be hand-written C++), data
  generation, training; unknowns: achievable error at 6 knobs, aliasing at low oversampling, sag.

### D. Grey-box hybrid: exact linear blocks + learned or tabulated nonlinear stages
- Tone stack, coupling networks, OT and cabinet as exact circuit-derived linear filters whose coefficients
  follow the knobs; triode stages and the power stage as small learned or tabulated nonlinear blocks.
- C1: as C. C2: Miklánek: better generalisation to unseen tone settings than black-box with little data; the
  NFB / presence loop and inter-stage loading (our J4 problem) are exactly where decoupled blocks lose accuracy,
  and no paper covers them. C3: est. 5–20 %. C4: tone knobs exact by construction; gain / master / presence
  depend on how the coupled sections are treated. C5: per-block comparison with the harness possible. C6: large;
  research-level unknowns around the NFB loop and loading.

### E. Two engines: real-time model plus the current engine as an offline / "render" mode
- Any of B–D for playing; the existing verified engine (with exact J4) for offline rendering where CPU does not
  matter (e.g. bounce / freeze, like "high quality render" modes in some plugins).
- C1–C4 as the real-time engine chosen; C5: the offline mode remains SPICE-verified; C6: the plugin carries both;
  the two may sound different, which is visible to users.

---

## 4. Comparison (summary of §3)

| | C1 latency | C2 authenticity evidence | C3 CPU at 48 kHz | C4 knobs | C5 SPICE verification | C6 effort / unknowns |
|---|---|---|---|---|---|---|
| A circuit, exact J4 | oversampler | SPICE −40 … −60 dB on presets | 90–200 % (measured basis) | proven | full, current tolerances | moderate / low |
| B decomposed + tables | oversampler | circuit-based; vs SPICE unknown | est. 15–60 % | in linear parts / tables | full; tolerances likely revisited | large / medium–high |
| C neural (simulation-trained) | 0 + oversampler | listening parity reported for 5 knobs; ESR −17 … −30 dB | est. 2–12 % | inputs; needs data coverage | ESR measurable; current tolerances not reachable | large / medium |
| D grey-box hybrid | 0 + oversampler | tone stack exact; loops unproven | est. 5–20 % | tone exact | per block | large / high |
| E two engines | per real-time part | offline mode verified | per real-time part | per part | offline full | A + chosen |

---

## 5. Decisions this study leaves to the owner

1. **Acceptance target for the real-time plugin.** Every option that fits a 20 % budget gives up the −40 … −60 dB
   ESR-vs-SPICE tolerances in favour of looser metrics (ESR around −20 … −30 dB, harmonic and spectral checks,
   listening tests). Changing tolerances is the owner's decision (CLAUDE.md).
2. **CPU budget per instance** (the 20 % used here is a working value).
3. **Which architecture(s) to prototype** and whether the current engine stays as an offline mode (E).
4. **New dependencies** if C or D are chosen (training stack).

## 6. De-risking experiments (each small, each answers one unknown)

| Experiment | Answers | Size |
|---|---|---|
| X1: exact J4 in the current engine (joint solve), verified vs the HOT SPICE references | whether the engine is a valid teacher / reference at all knobs (needed by C, D, E) | days |
| X2: current engine at 2–4x with a relaxed metric (ESR at 48 kHz band, harmonics) vs SPICE | how far accuracy falls with rate under looser metrics (informs B, A) | hours |
| X3: one knob setting, train an LSTM-32 / GRU on the engine's output (DI + sines), measure ESR vs SPICE, CPU, aliasing | baseline quality and cost of C | days; needs a training dependency |
| X4: same with 2–3 knobs as inputs | knob generalisation (C) | days |
| X5: one preamp section as precomputed table + connection current, vs the current engine | table accuracy and cost (B) | days |

## 7. Sources (selection; full lists in the survey notes of 2026-10-14)

- Pakarinen & Yeh, CMJ 2009, doi 10.1162/comj.2009.33.2.85
- Yeh thesis 2009 https://ccrma.stanford.edu/~dtyeh/papers/DavidYehThesisdoublesided.pdf ; Yeh, Abel, Smith TASLP 2010 https://ccrma.stanford.edu/~dtyeh/papers/yeh10_taslp.pdf
- Mačák thesis 2012 https://ccrma.stanford.edu/~jingjiez/portfolio/gtr-amp-sim/pdfs/Real-time%20Digital%20Simulation%20of%20Guitar%20Amplifiers%20as%20Audio%20Effects.pdf ; Mačák DAFx-13 https://www.dafx.de/paper-archive/2013/papers/27.dafx2013_submission_10.pdf ; Mačák & Schimmel DAFx-11 https://www.dafx.de/paper-archive/2011/Papers/05_e.pdf
- Dunkel et al. DAFx-16 https://www.dafx.de/paper-archive/2016/dafxpapers/37-DAFx-16_paper_53-PN.pdf ; Zhang & Smith DAFx-18 https://www.dafx.de/paper-archive/2018/papers/DAFx2018_paper_25.pdf
- Holters & Zölzer: k-d cache EUSIPCO 2016 https://www.hsu-hh.de/ant/wp-content/uploads/sites/699/2017/10/Holters_treecachesolver_EUSIPCO16.pdf ; decomposition DAFx-17 https://dafx.de/paper-archive/details/zpy7FDHEZoOs0r1nNKEjgA ; ACME.jl https://github.com/HSU-ANT/ACME.jl ; ADAA stateful https://www.mdpi.com/2076-3417/10/1/20
- Cohen & Hélie DAFx-10 https://www.dafx.de/paper-archive/2010/DAFx10/CohenHelie_DAFx10_P45.pdf
- Andò, Bozzo, Fontana DAFx-25 https://dafx25.dii.univpm.it/wp-content/uploads/2025/09/DAFx25_paper_6.pdf ; Zea & Rivera DAFx-25 https://dafx25.dii.univpm.it/wp-content/uploads/2025/09/DAFx25_paper_45.pdf
- Bernardini et al. WD Newton-Raphson TASLP 2021 https://re.public.polimi.it/handle/11311/1203185 ; Proverbio et al. EUSIPCO 2020 https://re.public.polimi.it/handle/11311/1156970
- Wright et al. DAFx-19 https://acris.aalto.fi/ws/portalfiles/portal/36768418/ELEC_Wright_Real_time_black_box_DAFx2019.pdf ; Wright et al. 2020 https://doi.org/10.3390/app10030766
- Juvela et al. 2023 https://arxiv.org/abs/2403.08559 ; Miklánek et al. DAFx-23 https://dafx.de/paper-archive/details/UYVDlRJ95vRRQbsqJVUvqw ; DDSP guitar amp https://arxiv.org/abs/2408.11405
- Chowdhury 2020 https://arxiv.org/abs/2009.02833 ; Darabundit et al. DAFx-22 https://dafx.de/paper-archive/details/K1rlWgdQUsiUo-JpA2pA7A ; Parker et al. DAFx-19 https://dafx.de/paper-archive/2019/DAFx2019_paper_42.pdf
- Carson et al. (aliasing) https://arxiv.org/abs/2505.11375 ; NAM https://github.com/sdatkinson/neural-amp-modeler ; RTNeural https://arxiv.org/abs/2106.03037
- Products: Neural DSP https://neuraldsp.com/news/neural-dsp-amplifier-modeling-technology ; NI ICM https://blog.native-instruments.com/the-making-of-icm/ ; Softube (Sound On Sound) https://www.soundonsound.com/music-business/softube-modelling-hardware-software ; Fractal https://www.fractalaudio.com/iii/ ; Mercuriall http://mercuriall.com/cms/ ; Line 6 https://blog.line6.com/?p=7000

Caveats: several full texts were not accessible (Politecnico di Milano papers, Yeh Part II); figures marked
in the surveys as abstract-only are not used for estimates here. Commercial CPU figures are user reports.
