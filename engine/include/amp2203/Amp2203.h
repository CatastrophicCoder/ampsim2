#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

#include "amp2203/Probe.h"
#include "amp2203/Params.h"
#include "amp2203/detail/Halfband.h"
#include "amp2203/detail/NewtonStats.h"

namespace amp2203 {

class Preamp;
class PowerAmp;
class Supply;

/// Full amplifier (SPEC §3.2, §4.3): Preamp (B1234) → J4 → PowerAmp (B56: phase inverter, power stage and
/// output transformer in one tree, global NFB inside it; DECISIONS D-041) → speaker voltage; Supply (B7)
/// advances once per sample with the currents of the sample just processed and sets the supply nodes of
/// the next one.
class Amp2203 {
public:
    Amp2203();
    ~Amp2203();
    Amp2203(const Amp2203&) = delete;  // probes point into this object
    Amp2203& operator=(const Amp2203&) = delete;

    void prepare(const EngineConfig& cfg);  // may allocate
    void reset();                           // returns all states to the DC operating point (SPEC §3.6)
    void setParams(const Params& p);        // audio thread; values are smoothed internally
    /// Input calibration (SPEC §6.3 Interface Level): volts at the jack per full-scale sample. Real-time safe
    /// (DECISIONS D-012 addition, M5).
    void setInputVoltsPerFullScale(double v) noexcept { cfg_.inputVoltsPerFullScale = v; }

    /// In-place, real-time safe. Samples are full-scale normalised (SPEC §3.3).
    void process(float* mono, int numSamples) noexcept;
    /// Same as above in double precision (used by ampwav and the tests).
    void process(double* mono, int numSamples) noexcept;

    /// Diagnostic for the M4 NFB check (SPEC §7 M4; DECISIONS D-012 addition), not real-time: `closed` =
    /// false sets R22 to 1 TΩ (loop open). Takes effect at the next reset().
    void setFeedbackClosed(bool closed);
    /// Power-amp adaptive half-step trigger in volts (D-043; 0 = off). Not real-time.
    void setHalfStepTrigger(double volts);
    /// Newton residual tolerance of both roots in volts (default 1e-9, WDF_DESIGN §4.3; M6 study). Not real-time.
    void setNewtonResidualTolerance(double volts);

    // diagnostics (non-real-time)
    struct Stats {
        uint64_t newtonFailures = 0;
        double   maxIterationsUsed = 0.0;
        uint64_t newtonSolves = 0;        // DECISIONS D-012 addition (M2): Newton solves so far
        double   medianIterations = 0.0;  // median Newton iterations per solve (SPEC §7 M2 acceptance)
        uint64_t newtonRescued = 0;       // D-042: failures recovered by the source-stepping fallback
        uint64_t newtonUnrescued = 0;     // failures the fallback did not recover (best iterate kept)
        detail::NewtonStats preamp;       // per root, with the iteration histogram (M6)
        detail::NewtonStats powerAmp;
    };
    /// Newton statistics of both roots together (preamp, power amp).
    Stats getStats() const;

    /// Output delay in samples at the engine rate added by the power amp's rate change (0 at divisor 1; M6).
    int latencySamples() const noexcept { return cfg_.powerAmpRateDivisor == 2 ? 2 * kHalfbandM : 0; }

    /// Node probes for the harness (`ampwav --dump-node`), netlist names. Values are valid after each
    /// processed sample.
    std::span<const Probe> probes() const noexcept;

private:
    double processSample(double x) noexcept;
    void solveDc();
    void applySupplies() noexcept;

    EngineConfig cfg_{};
    Params params_{};
    std::unique_ptr<Preamp> preamp_;
    std::unique_ptr<PowerAmp> power_;
    std::unique_ptr<Supply> supply_;

    // power amp at half rate (cfg_.powerAmpRateDivisor == 2): halfband decimator on the preamp output,
    // interpolator on the power amp output (zero-stuffed, gain 2)
    static constexpr int kHalfbandM = 17;
    using Halfband = detail::HalfbandFir<kHalfbandM>;
    const Halfband halfband_{10.0};
    detail::FirWindow<Halfband::kTaps> decim_, interp_;
    bool powerPhase_ = false;  // true: the power amp runs at this sample
    double iY_ = 0.0;          // exact J4: the load current at y of the last sample (A)
    bool exactJ4() const noexcept { return cfg_.exactJ4 && cfg_.powerAmpRateDivisor == 1; }

    double vIn_ = 0.0;   // V at the input jack
    double vOut_ = 0.0;  // V across the 16 Ω load
    std::vector<Probe> probes_;
};

}  // namespace amp2203
