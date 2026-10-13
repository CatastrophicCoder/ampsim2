#pragma once

#include <cstdint>
#include <span>

#include "amp2203/Probe.h"

namespace amp2203 {

/// Knob rotations, 0.0 .. 1.0 (0 = fully counter-clockwise). SPEC §3.2.
struct Params {
    double preampVolume = 0.6;
    double treble       = 0.5;
    double middle       = 0.5;
    double bass         = 0.5;
    double master       = 0.3;
    double presence     = 0.5;
};

struct EngineConfig {
    double sampleRate = 384000.0;           // the rate process() is called at (already oversampled)
    int    maxBlockSize = 512;
    double inputVoltsPerFullScale  = 4.361; // 0 dBFS input = +12 dBu at the jack (DECISIONS D-010)
    double outputVoltsPerFullScale = 60.0;  // 60 V peak across 16 Ω = 0 dBFS output
    double paramSmoothingMs        = 20.0;
};

/// Full amplifier. M0: pass-through in volts (v_speaker = v_in) so the CLI and harness pipeline can be
/// tested; the circuit blocks replace this from M1 on.
class Amp2203 {
public:
    Amp2203() = default;
    Amp2203(const Amp2203&) = delete;  // probes point into this object
    Amp2203& operator=(const Amp2203&) = delete;

    void prepare(const EngineConfig& cfg);  // may allocate
    void reset();                           // returns all states to the DC operating point (SPEC §3.6)
    void setParams(const Params& p);        // audio thread; values are smoothed internally

    /// In-place, real-time safe. Samples are full-scale normalised (SPEC §3.3).
    void process(float* mono, int numSamples) noexcept;
    /// Same as above in double precision (used by ampwav and the tests).
    void process(double* mono, int numSamples) noexcept;

    // diagnostics (non-real-time)
    struct Stats {
        uint64_t newtonFailures = 0;
        double   maxIterationsUsed = 0.0;
        uint64_t newtonSolves = 0;        // DECISIONS D-012 addition (M2): Newton solves so far
        double   medianIterations = 0.0;  // median Newton iterations per solve (SPEC §7 M2 acceptance)
    };
    Stats getStats() const;

    /// Node probes for the harness (`ampwav --dump-node`). Values are valid after each processed sample.
    std::span<const Probe> probes() const noexcept;

private:
    double processSample(double x) noexcept;

    EngineConfig cfg_{};
    Params params_{};
    Stats stats_{};

    double vIn_ = 0.0;   // V at the input jack
    double vOut_ = 0.0;  // V across the 16 Ω load
    Probe probes_[2] = {{"in", &vIn_}, {"out", &vOut_}};
};

}  // namespace amp2203
