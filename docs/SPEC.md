# Implementation Specification: JCM800 2203 Wave Digital Filter Engine and Plugin

Status: v1.0 (initial spec). Owner: Kimmo. Implementer: Claude Code.
Companion documents: `CIRCUIT.md` (component-level circuit description), `WDF_DESIGN.md` (math and
algorithms), `VERIFICATION.md` (harness, signals, metrics, tolerances), `DECISIONS.md` (decision log),
`PROGRESS.md` (milestone tracking).

---

## 1. Goal and scope

### 1.1 Goal
Build a real-time, circuit-based model of the Marshall JCM800 2203 (100 W, single channel, master
volume) as a C++ audio engine, wrapped as a JUCE plugin. The engine simulates the actual circuit using
Wave Digital Filters (WDF) implemented with the `chowdsp_wdf` library, so that every front-panel
control behaves like the real one.

Correctness is defined against the SPICE reference model in `reference/spice/` (ngspice). Every block
is verified against SPICE before it is considered done.

### 1.2 In scope (v1)
- High input, V1a, Preamp Volume network (incl. bright caps), V1b, V2a, V2b cathode follower.
- Tone stack (Treble, Middle, Bass) and Master Volume.
- Long-tail-pair phase inverter with global negative feedback and Presence.
- 4x EL34 push-pull output stage, output transformer (linear model), 16 Ω resistive load.
- HT supply sag (reservoir, choke, RC dropper chain) and the bias network.
- JUCE plugin (AU, VST3, Standalone) with parameters, smoothing, oversampling and latency reporting.
- Offline CLI tool and verification harness against SPICE.

### 1.3 Out of scope (v1), candidates for later milestones
- Low input jack (bypasses V1a), effects loop (the 2203 STD drawing has none), mains/rectifier ripple.
- Output transformer core saturation and hysteresis.
- Reactive speaker load (v1 uses 16 Ω resistive, matching the SPICE reference).
- Cabinet simulation inside the engine. The plugin may host an IR convolution stage, or the engine is
  integrated into the owner's existing amp-sim plugin project, which already has an IR loader.
- Fitting tube model parameters to measured tubes.

### 1.4 Relationship to the owner's existing project
The owner has a separate JUCE amp-sim plugin project that uses NAM as its engine. This repository
produces a JUCE-independent engine library so that it can later be added to that project as a second
engine. Keep the engine free of JUCE dependencies (see 3.1).

---

## 2. Tech stack, platform and dependencies

| Item | Choice | Notes |
|---|---|---|
| Language | C++20 | No exceptions or RTTI required in the audio path |
| Build | CMake ≥ 3.22, Ninja recommended | Owner uses CLion on macOS |
| Platform | macOS (Apple Silicon and Intel) first | Code must stay portable (no platform APIs in engine) |
| WDF library | `chowdsp_wdf` (header-only, BSD-3-Clause) | Pin to a release tag or commit via FetchContent. Use the compile-time `chowdsp::wdft` API in the audio path |
| Linear algebra (setup-time only) | Eigen 3.4 (header-only, MPL2) | Only for computing R-type scattering matrices and other non-per-sample work. Optional; a small hand-written LU is acceptable |
| Plugin framework | JUCE 8.x | Pin a tag via FetchContent or git submodule. Only used in `plugin/` |
| Unit tests | Catch2 v3 | Pin a tag |
| Audio file I/O (CLI) | `dr_wav` (single header, public domain/MIT-0) or `libsndfile` | Prefer `dr_wav` to avoid system dependencies |
| Python tooling | Python ≥ 3.11 with numpy, scipy, soundfile, matplotlib | See `requirements.txt` |
| SPICE | ngspice ≥ 40 | macOS: `brew install ngspice` |
| Plugin validation | `pluginval` (Tracktion), `auval` (macOS) | Used in M5 |

Rules:
- All third-party dependencies are pinned (tag or commit hash). No unpinned `main`/`master` fetches.
- Record every dependency and its pin in `DECISIONS.md`.
- Before using any `chowdsp_wdf` class, read its headers/examples at the pinned version and confirm the
  exact class names and template signatures (for example the R-type adaptor variants and the root
  R-type adaptor). This spec names concepts; the library's API is authoritative.

---

## 3. Architecture

### 3.1 Layers and repository layout (target)

```
wdf-amp-2203/
├── CMakeLists.txt              # top-level: options, FetchContent, subdirectories
├── cmake/                      # dependency helpers
├── engine/                     # amp2203 engine: pure C++20, depends on chowdsp_wdf (+Eigen at setup)
│   ├── include/amp2203/        # public headers
│   └── src/
├── tests/                      # Catch2 unit tests (engine only)
├── tools/
│   ├── ampwav/                 # CLI: WAV in -> engine/block -> WAV out
│   └── bench/                  # CPU benchmark executable
├── plugin/                     # JUCE plugin wrapper (only place that includes JUCE)
├── harness/                    # Python verification harness (see VERIFICATION.md)
│   ├── signals.py              # test signal generator
│   ├── render_spice.py         # renders SPICE references (blocks and full amp)
│   ├── run_engine.py           # runs ampwav on the same signals
│   ├── compare.py              # metrics + markdown/HTML report with plots
│   └── testbenches/            # per-block ngspice testbenches (generated or hand-written)
├── reference/                  # SPICE reference model, plots (already present; drawings not included)
└── docs/
```

Dependency direction: `plugin -> engine`, `tools -> engine`, `tests -> engine`. The engine never
includes JUCE.

### 3.2 Engine public API

```cpp
namespace amp2203 {

struct Params {               // knob rotations, 0.0 .. 1.0 (0 = fully counter-clockwise)
    double preampVolume = 0.6;
    double treble       = 0.5;
    double middle       = 0.5;
    double bass         = 0.5;
    double master       = 0.3;
    double presence     = 0.5;
};

struct EngineConfig {
    double sampleRate;        // the rate process() is called at (already oversampled by the host layer)
    int    maxBlockSize;
    double inputVoltsPerFullScale  = 4.361; // 0 dBFS input = +12 dBu (3.084 V rms, 4.361 V peak) at the jack
    double outputVoltsPerFullScale = 60.0;  // 60 V peak across 16 Ω = 0 dBFS output
    double paramSmoothingMs        = 20.0;
};

class Amp2203 {
public:
    void prepare(const EngineConfig& cfg);  // may allocate
    void reset();                           // returns all states to the DC operating point (see 3.6)
    void setParams(const Params& p);        // audio thread; values are smoothed internally
    void process(float* mono, int numSamples) noexcept;   // in-place, real-time safe
    // diagnostics (non-real-time)
    struct Stats { uint64_t newtonFailures; double maxIterationsUsed; };
    Stats getStats() const;
};

}  // namespace amp2203
```

Each signal block (section 4) is also a class with the same `prepare / reset / process` shape and its
own parameter setters, so that the CLI and tests can run a single block in isolation:
`StageV1aV1b`, `StageV2aV2bToneStack` (built from the verified `StageV2aV2b` and `ToneStackMaster`;
DECISIONS D-033), `PhaseInverter`, `PowerStage`,
`Supply`. `Amp2203` composes them.

### 3.3 Units and signal levels
- All engine-internal signals are physical quantities in SI units (volts, amperes), double precision.
- Input mapping: `v_in = sample * inputVoltsPerFullScale`, with
  `inputVoltsPerFullScale = 0.7746 · sqrt(2) · 10^(interfaceLevel_dBu / 20)`.
  Default interface level +12 dBu at 0 dBFS (4.361 V peak), the de facto amp-sim convention (NAM expects
  12 dBu; Neural DSP calibrates at 12.2 dBu; common interfaces' instrument inputs at minimum gain sit
  around +12 to +12.5 dBu). Users with a different interface enter their own value (see 6.3).
- Output mapping: `sample = v_speaker / outputVoltsPerFullScale` (16 Ω load). Default 60 V peak
  (about 112 W sine) = 0 dBFS. The plugin applies an additional output level control.
- Use `double` throughout the WDF and solvers. Convert to `float` only at the API boundary.

### 3.4 Sample rate and oversampling
- The engine runs at whatever rate `prepare()` receives. The JUCE plugin layer wraps it with
  `juce::dsp::Oversampling` and passes the oversampled rate.
- Default plugin oversampling: 16x at 44.1/48 kHz, 8x at 88.2/96 kHz, 4x at 176.4/192 kHz, so the
  internal rate is about 700–770 kHz (DECISIONS D-035; was 8x / about 384 kHz). Exposed as a parameter
  (2x/4x/8x/16x).
- Verification runs the engine directly at 768 kHz (16 x 48 kHz) on signals generated at that rate, so
  no resampler sits between SPICE and the engine. See `VERIFICATION.md`. (At 384 kHz the preamp's
  fastest cutoff transitions were not resolved: D-034, D-035.)
- Optimisation (later): linear blocks may run at the base rate. Not required for v1.

### 3.5 Real-time rules (engine `process` and plugin `processBlock`)
- No heap allocation, locks, file or console I/O, or exceptions.
- Denormals disabled (plugin: `juce::ScopedNoDenormals`; engine tests: flush-to-zero set in the test).
- Newton solvers have a fixed maximum iteration count; on failure keep the best iterate, increment
  `newtonFailures`, never output NaN/Inf (guard with a final sanitize step that also counts events).
- Parameter changes that require R-type scattering matrix recomputation are rate-limited (section 6).

### 3.6 Initial state
SPICE transient analyses start from the DC operating point. The engine must do the same:
`reset()` brings every capacitor, inductor and solver state to the DC operating point for the current
parameters. Acceptable implementations, in order of preference:
1. Solve the DC operating point explicitly (capacitors open, inductors short) with the same device
   equations, then initialise wave states from it.
2. Internal pre-roll: run the model with zero input for a fixed duration with temporarily shortened
   time constants or a large time step until node voltages settle within 1 mV, then restore.
Document the chosen method in `DECISIONS.md`. Acceptance: after `reset()`, node voltages match the
SPICE operating point table in `CIRCUIT.md` within the M-specific tolerances, and silence in produces
silence out (|output| < 1e-6 V after the first sample).

---

## 4. Circuit partition and WDF structure

The reference circuit and node names are in `reference/spice/jcm800_2203.cir`; component tables per
block are in `CIRCUIT.md`. Math for every structure below is in `WDF_DESIGN.md`.

### 4.1 Partition principle
Each block is one WDF tree with at most one nonlinear root (scalar or multiport). Blocks are connected
by **junctions**. Default junction model (J-Thevenin): the upstream block computes the open-circuit
voltage at its output node; the downstream tree contains the upstream output as a resistive voltage
source `(v_oc, R_th)`, where `R_th` is the upstream output resistance at the DC operating point. This
keeps coupling capacitors, grid-leak resistors and the downstream grid in the same tree, so grid-current
charging of coupling capacitors (blocking, bias shift) is captured, which is essential for the amp's
dynamic behaviour.

Approximations introduced by J-Thevenin: the upstream output resistance is linearised (fixed at the
operating point), and the frequency dependence of the upstream output impedance is ignored. The
harness quantifies this as **partition error** (see `VERIFICATION.md`). If a junction's partition error
exceeds tolerance, upgrade it in this order: (a) add the reactive part of the source impedance,
(b) merge the two stages into one tree with a joint multiport root. Record upgrades in `DECISIONS.md`.

### 4.2 Blocks

| # | Block | Contents | Nonlinear root | Output |
|---|---|---|---|---|
| B1234 | `Preamp` | Input source, R2, R3, V1a, R1‖C1, R4, C3, R5‖C4, C5, VR1 (Preamp Volume), V1b, R6, R7; C7, R10‖C8, R11, V2a, R9, R12, V2b (direct-coupled cathode follower), R13; C10, R15, C11, C12, VR3 Treble, VR5 Bass, VR4 Middle, VR2 Master; supplies `bpA`, `bpX` | Joint 8-port (four triodes) | `v_oc(y)`, `R_th4` (knob-dependent) |
| (B12) | `StageV1aV1b` | B12 alone, output `v_oc(p1b)` (verification of the V1a/V1b root, M2) | Joint 4-port | `v_oc(p1b)` |
| (B34) | `StageV2aV2bToneStack` | B3 + B4 behind J2 (verification, M3) | Joint 4-port | `v_oc(y)`, `R_th4` |
| (B3) | `StageV2aV2b` | B3 alone behind J2, output `v_oc(k2b)` (verification of the V2a/V2b root, M3) | Joint 4-port | `v_oc(k2b)` |
| (B4) | `ToneStackMaster` | B4 alone behind J3 (R_th3), output `v_oc(y)` (verification of the tone stack, M1) | none (R-type) | `v_oc(y)`, `R_th4` |
| B5 | `PhaseInverter` | J4 source, C9, R17, R16, R19, C15, R20, R23, C17, VR6 Presence, R22 (NFB, delayed), V3a, V3b, R18, R21 | Joint 4-port (two triodes) | `v_oc(p3a)`, `v_oc(p3b)`, `R_thA`, `R_thB` |
| B6 | `PowerStage` | J5 sources, C14, C16, C18, R24, R25, bias network (Thevenin + C19), grid stoppers, 2 EL34 equivalents, screen resistors, output transformer, 16 Ω load | Joint 6-port (two pentodes x 3 ports) | `v(out)` speaker voltage, supply currents |
| B7 | `Supply` | Source resistance, C23, choke, C22, R29, C21b, R8, C21a | none (small linear state-space) | `v(ct)`, `v(nScr)`, `v(bpX)`, `v(bpA)` |

Notes per block (details in `CIRCUIT.md` / `WDF_DESIGN.md`):
- **B12 joint root (V1a + V1b):** originally two blocks (B1 V1a, B2 V1b) joined by J1. M0 measured the
  J1 partition error in SPICE at −8.7 to −22.6 dB ESR at p1b for any constant R_th1, because B2's input
  impedance falls to 54–177 kΩ at 10 kHz while V1a's output impedance varies from 75 kΩ to 31 kΩ, so
  the two stages are solved together (4 unknowns; DECISIONS D-017). Each triode's cathode network is
  shared between its grid and plate ports, so the linear part is connected to the 4-port root through
  an R-type adaptor whose scattering matrix is derived with the MNA method. Include the inter-electrode capacitances from `tubes.lib`
  (Cgk 2.3 pF, Cgp 2.4 pF, Cpk 0.9 pF) in the linear network for parity with SPICE; Cgp (Miller) is
  audible on V1a with its 68 kΩ grid stopper.
- **B3 joint root:** V2a's plate is V2b's grid with no coupling capacitor, and V2b grid current flows
  through R12. Solve both triodes together (4 unknowns). Do not split this junction.
- **B1234 (B12 + B3 + B4 in one tree, 8-port root):** after the J3 merge (below) the remaining preamp
  junction J2 (p1b) gave −22.1 dB partition error at the PI-loaded y (Preamp Volume 1.0, 5 kHz 2 V;
  target −26 dB): V1b swings between cutoff and grid conduction, far from a constant R_th2. With B12, B3
  and B4 joined the worst sine is −36.1 dB (DECISIONS D-038, owner decision Q18). The engine's preamp is
  therefore one tree; B12, B3, B4 and B34 remain as verified classes.
- **B34 (B3 + B4 in one tree):** originally two blocks joined by J3 at the cathode follower k2b. M3
  measured the J3 partition error in SPICE at −2.2 to −7.5 dB ESR at y from 0.1 V input upwards: V2b cuts
  off on large swings into the tone stack, its output resistance jumps from 360 Ω to R13 = 100 kΩ, and a
  constant R_th3 with an unloaded upstream output cannot represent that. The tone stack and master are
  therefore part of B3's linear network under the V2a/V2b root (DECISIONS D-033, owner decision Q14).
  B3 and B4 remain as separately verified classes.
- **Tone stack (B4 part of B34):** bridge topology, cannot be expressed with series/parallel adaptors; use one
  adaptable R-type adaptor. The middle pot is wired as drawn (C12 to the Middle wiper, Bass pot's
  bottom to the Middle pot's top lug); see `CIRCUIT.md` open item O2.
- **B34 → B5 junction (J4):** compute the open-circuit wiper voltage in B34 (no PI load) and drive B5
  with `R_th4 = R_master_top ‖ R_master_bottom` plus the stack's resistance seen from the master top,
  recomputed on parameter change. Document the exact formula chosen.
- **B5 NFB:** R22 connects the speaker output to node `nG`. Use the previous sample's output voltage
  (one-sample delay at the oversampled rate). This is the only intentional delay-free-loop break in v1.
- **B6 matched pairs:** V4‖V5 and V6‖V7 are each modelled as one equivalent pentode with doubled
  current (equivalently: identical tubes with identical drive), halving the root dimension. Grid
  stoppers 5.6 kΩ‖5.6 kΩ = 2.8 kΩ and screen resistors 1 kΩ‖1 kΩ = 500 Ω are combined accordingly. Keep
  the code structured so unmatched pairs can be added later.
- **B6 output transformer:** linear equivalent circuit (two half-primaries with leakage and winding
  resistance, magnetising inductance, ideal ratio to the 16 Ω secondary). Values must match the SPICE
  coupled-inductor model (see `CIRCUIT.md`).
- **B7 supply:** updated once per sample at the oversampled rate from the previous sample's currents
  (plate, screen and preamp currents reported by B1–B6). Trees read supply node voltages as
  time-varying ideal sources. Integration: trapezoidal (bilinear), same as the WDF capacitors.

### 4.3 Processing order per sample
```
supply voltages (from state)  ->  B1234 -> B5 (uses out[n-1]) -> B6 -> output
                                   \__________ currents reported to B7, B7.update() ___________/
```

---

## 5. Device models

Must reproduce `reference/spice/tubes.lib` exactly (same equations and parameters); see
`WDF_DESIGN.md` §3 for equations and analytic Jacobians.

- **12AX7:** Koren triode, mu=100, ex=1.4, kg1=1060, kp=600, kvb=300; grid current = diode (IS=1 nA,
  n=1, series 2000 Ω + 1 Ω), solved in closed form with the Wright omega function (chowdsp_wdf ships a
  Wright omega approximation used by its diode models; reuse it or implement an equivalent).
- **EL34:** Koren pentode, mu=11, ex=1.35, kg1=650, kg2=4200, kp=60, kvb=24, grid diode with series
  1000 Ω + 1 Ω, plus this repository's screen-current modification (screen current from the same E1
  term, plus diversion of plate current to the screen at low plate voltage). This modification is not
  part of Koren's published model; it is documented in `tubes.lib` and `reference/README.md`.
- Thermal voltage: ngspice default temperature 27 °C → Vt = 25.865 mV. Diode junction capacitance and
  transit time in the SPICE diode models (CJO=1 pF, TT=1 ns) are ignored in the engine; note the
  resulting deviation in reports if measurable.
- Unit tests compare every device model against SPICE DC sweeps (see `VERIFICATION.md` §5.1) and every
  analytic Jacobian against central finite differences.

---

## 6. Parameters, potentiometers and smoothing

### 6.1 Tapers (must match the netlist)
- Log ("A") taper: `atap(x) = max((81^x − 1) / 80, 1e-4)` (10 % at mid rotation).
- Linear taper: `ltap(x) = clamp(x, 1e-4, 0.9999)`.
- A potentiometer is two adaptable resistors: `R_top = R·(1 − taper) + 1 Ω`, `R_bottom = R·taper + 1 Ω`
  (the netlist adds 1 Ω only to log pots; replicate the netlist exactly per pot, see `CIRCUIT.md`).

| Control | Part | Value | Taper | Wiring |
|---|---|---|---|---|
| Preamp Volume | VR1 | 1 MΩ | log | potentiometer, C5 1 nF top-to-wiper |
| Treble | VR3 | 220 kΩ | linear | potentiometer |
| Middle | VR4 | 22 kΩ | linear | potentiometer (as drawn: C12 on wiper) |
| Bass | VR5 | 1 MΩ | log | rheostat (wiper tied to top), resistance = R·atap(x) + 1 Ω |
| Master | VR2 | 1 MΩ | log | potentiometer, fed from Treble wiper |
| Presence | VR6 | 22 kΩ | linear | rheostat to ground, resistance = 22 kΩ·(1 − ltap(x)) + 1 Ω |

### 6.2 Smoothing and update rate
- Smooth knob rotations (not resistances) with a one-pole filter, time constant `paramSmoothingMs`.
- Blocks with R-type adaptors recompute scattering matrices at most every `kParamUpdateInterval`
  samples (default 32 at the oversampled rate, about 42 µs at 768 kHz), and only while a smoothed value
  is still moving. Measure the cost and record the chosen interval in `DECISIONS.md`.
- Abrupt full-range knob jumps must not produce NaN/Inf or audible clicks above the level of the
  real-circuit transient (unit test: sweep every knob 0→1 in 10 ms during a 1 kHz tone).

### 6.3 Plugin parameters (M5)
Preamp Volume, Treble, Middle, Bass, Master, Presence (0–10 display, rotation internally),
Interface Level (dBu at 0 dBFS, 0..+24 dBu, default +12.0 dBu; sets `inputVoltsPerFullScale`),
Input Trim (−24..+24 dB, default 0 dB), Output Level (−48..+12 dB), Oversampling (2x/4x/8x/16x),
optional Cabinet IR on/off with file chooser (only if implemented in this repo; see 1.3).

---

## 7. Milestones

Each milestone ends with: all unit tests passing, a harness report committed under `harness/reports/`
(markdown + plots), `PROGRESS.md` updated, and any deviations recorded in `DECISIONS.md`. Do not start
the next milestone until the owner has reviewed the report unless told otherwise.

### M0 — Skeleton and verification harness
Tasks:
1. Top-level CMake with targets: `amp2203_engine` (static library), `amp2203_tests` (Catch2),
   `ampwav` (CLI), `amp2203_bench`, and `amp2203_plugin` (JUCE, behind option `AMP2203_BUILD_PLUGIN`,
   default ON on macOS). Pinned FetchContent for chowdsp_wdf, Catch2, JUCE (and Eigen if used).
2. `ampwav` CLI: `ampwav --block <name|full> --in in.wav --out out.wav --rate 768000
   [--param name=value ...] [--dump-node name=file.wav ...]`. Reads/writes 64-bit float WAV.
   For M0 the engine is a pass-through so the pipeline can be tested.
3. Harness (Python): signal generator, SPICE renderer for block testbenches and the full amp,
   engine runner, comparison and report (see `VERIFICATION.md`).
4. Block SPICE testbenches for B12 and B3–B7 under `harness/testbenches/`, built from the reference netlist
   with the same partition and junction models as section 4 (J-Thevenin sources), plus a "full"
   testbench equal to the reference netlist.
5. Measure and record the junction Thevenin resistances `R_th2`, `R_th3`, `R_thA`, `R_thB` in
   SPICE (AC current injection at the open-circuited output node at 1 kHz, input AC-grounded, plus the
   DC small-signal value). Put the numbers in `CIRCUIT.md` §4.
6. A minimal GitHub Actions workflow (macOS runner): configure, build engine + tests + CLI, run tests.
   Harness in CI is optional (requires ngspice from Homebrew).
Acceptance:
- `cmake --build` succeeds from a clean checkout on macOS; `ctest` passes (placeholder tests).
- `python -m harness.render_spice --block full` produces reference files; `python -m harness.compare`
  produces a report for the pass-through engine (errors expected to be large; pipeline is what is tested).
- Block testbenches reproduce the full netlist's DC operating point at their internal nodes within
  0.5 % (sanity check that testbenches were transcribed correctly).

### M1 — B4 tone stack and master (linear R-type WDF)
Tasks:
1. Generic MNA-based scattering-matrix builder (setup-time, `engine/src/mna/`), with a Python
   (numpy) reference implementation in `harness/` and a unit test comparing the two.
2. `ToneStackMaster` block with an adaptable R-type adaptor, J3 source (`R_th3`), parameters with
   smoothing and rate-limited matrix updates, output `v_oc(y)` and `R_th4`.
Acceptance:
- Magnitude response vs SPICE AC (B4 testbench): max error ≤ 0.1 dB over 20 Hz–10 kHz and ≤ 0.3 dB
  over 10–20 kHz, for all 5 x 5 x 5 Treble/Middle/Bass combinations at Master ∈ {0.2, 0.5, 1.0}.
- Time-domain ESR vs SPICE for a log sweep: ≤ −60 dB.
- Knob-jump test passes (6.2). Cost per sample and per matrix update reported in the M1 report.

### M2 — B12: V1a and V1b with a joint triode root
Tasks:
1. Triode model (12AX7) with analytic Jacobian and Wright-omega grid current (`engine/src/devices/`).
2. Multiport-root Newton solver (generic over port count, fixed max iterations, warm start, step
   limiting), see `WDF_DESIGN.md` §4. B12 is its first user with N = 4; B3 reuses it in M3.
3. `StageV1aV1b` tree: R-type adaptor connecting the grid, plate and cathode networks of V1a and V1b
   and the coupling / Preamp Volume network (C3, R5‖C4, C5, VR1) to the 4-port root; supply node `bpA`
   as an ideal source (constant for M2, from the SPICE operating point). Preamp Volume inside the tree.
4. DC initialisation per 3.6.
Acceptance:
- Device unit tests (`VERIFICATION.md` §5.1) pass.
- DC: `v(p1a)`, `v(k1a)`, `v(p1b)` and `v(k1b)` within 0.5 % of SPICE.
- Block-exact ESR (vs B12 testbench, at `p1b`) ≤ −40 dB for sines at 0.01/0.1/0.5 V peak (100 Hz,
  1 kHz, 5 kHz), ≤ −30 dB at 2 V peak (grid current region); harmonics 2–5 within 0.5 dB where above
  −80 dBc. Default parameters; Preamp Volume 0.2 and 1.0 are also reported.
- Newton: median iterations ≤ 3, max ≤ configured limit, zero failures across the signal set.

### M3 — B3, preamp cascade
Tasks:
1. `StageV2aV2b` (B3) with a joint 4-port root.
2. Cascade B12→B3→B4; dump intermediate nodes. (Became B12→B34 after the J3 partition study, D-033, and
   one tree B1234 after the J2 study, D-038.)
Acceptance:
- Block-exact targets as in M2 for B3 (at its own nominal and overdrive levels as defined in
  `VERIFICATION.md`).
- Partition report: cascade vs full-netlist SPICE at node `y` (master wiper, with the PI load present in
  SPICE) for the standard signal set at Preamp Volume ∈ {0.2, 0.6, 1.0}. Target ESR ≤ −26 dB on sines and
  ≤ −20 dB on DI guitar clips; if a junction fails, apply the upgrade path in 4.1 and document it.

### M4 — B5, B6, B7: phase inverter, power stage and supply
Tasks:
1. Pentode model (EL34 with the screen modification) and its Jacobian.
2. `PhaseInverter` (B5) with joint 4-port root and delayed NFB input.
3. Output transformer linear network and `PowerStage` (B6) with 6-port root, bias network, J5 sources.
4. `Supply` (B7) state-space and current bookkeeping across blocks.
5. Full chain `Amp2203`.
Acceptance (vs full-netlist SPICE unless noted):
- DC: EL34 plate current per tube within ±1 mA of SPICE (34.7 mA), screen ±0.3 mA (4.65 mA),
  `v(ct)` within 1 %, bias node within 0.2 V.
- NFB: small-signal gain with and without R22 (R22 → 1 TΩ) matches SPICE within 0.5 dB at
  100 Hz / 1 kHz / 5 kHz.
- Clean power (Preamp 0.2, Master 1.0, 1 kHz): output power at 25 mV peak input within ±5 % of SPICE
  (101 W, 2.8 % THD), THD within ±0.5 percentage points.
- Stability: 60 s of silence at Preamp 1.0 / Master 1.0 / Presence 1.0 produces |out| < 1 mV, no growth.
- Overdrive (Preamp 0.8, Master 0.3, 300 mV peak @ 220 Hz): ESR ≤ −20 dB, harmonics 2–9 within 1.5 dB;
  B+ sag minimum within 2 V of SPICE (469 V).
- Partition error of the delayed-NFB break reported (compare against a variant with a 2-sample delay;
  if results differ by more than 0.5 dB anywhere in 20 Hz–20 kHz small-signal response, raise
  the issue with the owner).

### M5 — JUCE plugin
Tasks:
1. `AudioProcessor` with parameters per 6.3 (AudioProcessorValueTreeState), mono processing
   (stereo input summed or left-channel only, output duplicated to all channels), engine at the
   oversampled rate, latency reported via `setLatencySamples`.
2. Oversampling filter choice exposed (linear-phase FIR or low-latency polyphase IIR).
3. Minimal functional editor: six knobs, input calibration, output level, oversampling selector.
4. `amp2203_bench`: processes 60 s of a DI clip at 48 kHz/16x and reports the real-time factor.
Acceptance:
- `pluginval --strictness-level 5` passes for VST3 and AU; `auval -v aufx ...` passes.
- Offline render through the plugin at 16x matches `ampwav --block full` at 768 kHz followed by the
  same resampling within ESR ≤ −60 dB (proves the wrapper adds nothing unintended).
- Real-time factor ≥ 10 (single thread, owner's Mac, 48 kHz, 16x; D-035). If not met, profile and report the
  per-block cost before optimising.
- Automation of every knob over its full range at audio rate produces no NaN/Inf and no Newton failures.

### M6 — Refinements (optional, prioritised by the owner)
Reactive speaker load; OT saturation/hysteresis; rectifier and mains ripple; Low input; performance
work (lookup tables for device equations, XSIMD for R-type matrix multiplies, linear blocks at base
rate); tube parameter fitting; unmatched power tube pairs; integration into the existing amp-sim
plugin project.

---

## 8. Coding conventions
- Namespace `amp2203`; headers in `engine/include/amp2203/`; one block per header/source pair.
- `clang-format` (repository `.clang-format`), warnings as errors in engine and tests
  (`-Wall -Wextra -Wpedantic -Werror`; relax only for third-party targets).
- No `using namespace` in headers. No global mutable state.
- Every block exposes node voltages needed by the harness through a debug-only probe interface
  (compiled in for tests/CLI, compiled out or zero-cost in the plugin).
- Constants (component values) live in one place per block, named after the schematic designators
  (`R4`, `C1`, ...), with values in SI units and a comment pointing to `CIRCUIT.md`.

## 9. Definition of done (any task)
1. Builds without warnings; unit tests pass.
2. Harness report for the affected blocks regenerated; tolerances met or deviation explained and
   accepted by the owner.
3. `PROGRESS.md` and, where relevant, `DECISIONS.md` and `CIRCUIT.md` updated in the same commit.
