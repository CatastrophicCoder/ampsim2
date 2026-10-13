#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

#include "amp2203/Probe.h"
#include "amp2203/Params.h"

namespace amp2203 {

class Preamp;
class PhaseInverter;
class PowerStage;
class Supply;

/// Full amplifier (SPEC §3.2, §4.3): Preamp (B1234) → J4 → PhaseInverter (B5, NFB from the previous
/// sample's speaker voltage) → J5 → PowerStage (B6) → speaker voltage; Supply (B7) advances once per sample
/// with the currents of the sample just processed and sets the supply nodes of the next one.
class Amp2203 {
public:
    Amp2203();
    ~Amp2203();
    Amp2203(const Amp2203&) = delete;  // probes point into this object
    Amp2203& operator=(const Amp2203&) = delete;

    void prepare(const EngineConfig& cfg);  // may allocate
    void reset();                           // returns all states to the DC operating point (SPEC §3.6)
    void setParams(const Params& p);        // audio thread; values are smoothed internally

    /// In-place, real-time safe. Samples are full-scale normalised (SPEC §3.3).
    void process(float* mono, int numSamples) noexcept;
    /// Same as above in double precision (used by ampwav and the tests).
    void process(double* mono, int numSamples) noexcept;

    /// Diagnostics for the M4 NFB checks (SPEC §7 M4; DECISIONS D-012 addition), not real-time: `closed` =
    /// false sets R22 to 1 TΩ (loop open); `delaySamples` is the NFB delay (1 = the engine's design; 2 for
    /// the delayed-NFB partition check). Takes effect at the next reset().
    void setFeedbackOptions(bool closed, int delaySamples);

    // diagnostics (non-real-time)
    struct Stats {
        uint64_t newtonFailures = 0;
        double   maxIterationsUsed = 0.0;
        uint64_t newtonSolves = 0;        // DECISIONS D-012 addition (M2): Newton solves so far
        double   medianIterations = 0.0;  // median Newton iterations per solve (SPEC §7 M2 acceptance)
    };
    /// Newton statistics of all roots together (preamp, phase inverter, power stage).
    Stats getStats() const;

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
    std::unique_ptr<PhaseInverter> pi_;
    std::unique_ptr<PowerStage> power_;
    std::unique_ptr<Supply> supply_;
    bool nfbClosed_ = true;
    int nfbDelay_ = 1;
    double fb_[2] = {0.0, 0.0};  // speaker voltage of the last two samples (NFB delay line)

    double vIn_ = 0.0;   // V at the input jack
    double vOut_ = 0.0;  // V across the 16 Ω load
    std::vector<Probe> probes_;
};

}  // namespace amp2203
