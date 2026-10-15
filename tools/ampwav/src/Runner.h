#pragma once

#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "amp2203/Amp2203.h"

namespace ampwav {

/// Runs one engine block (or the full amp) on signals in volts at the block's input and output.
class Runner {
public:
    virtual ~Runner() = default;
    virtual void prepare(double sampleRate, int maxBlockSize) = 0;
    virtual void setParams(const amp2203::Params& p) = 0;
    virtual void processVolts(double* v, int numSamples) noexcept = 0;
    virtual std::span<const amp2203::Probe> probes() const noexcept = 0;
    virtual amp2203::Amp2203::Stats stats() const { return {}; }
    /// Sets an ideal supply node (e.g. "bpa", "bpx") for the next samples; false if the block has none.
    virtual bool setSupply(std::string_view, double) noexcept { return false; }
    /// NFB diagnostic of the full amp (SPEC §7 M4): loop closed or open (R22 → 1 TΩ). False if the block has
    /// no NFB loop.
    virtual bool setFeedback(bool) { return false; }
    /// Adaptive half-step trigger of the power amp in volts (D-043; 0 = off). False if the block has none.
    virtual bool setHalfStepTrigger(double) { return false; }
    /// Newton residual tolerance in volts (full amp, M6 study). False if the block has none.
    virtual bool setNewtonResidualTolerance(double) { return false; }
    /// Exact J4 coupling (full amp, M6 X1); call before prepare(). False if the block has none.
    virtual bool setExactJ4(bool) { return false; }
    /// Power amp at sampleRate / divisor (full amp, M6); call before prepare(). False if the block has none.
    virtual bool setPowerRateDivisor(int) { return false; }
    /// Output delay in samples added by the block (full amp at divisor 2); ampwav removes it.
    virtual int latency() const { return 0; }
    /// Volts per full-scale sample at the input and output (only meaningful for the full amp).
    virtual double inputVoltsPerFullScale() const { return 1.0; }
    /// Input volts per full-scale sample (full amp, `--volts-per-fs`). False if the block has none.
    virtual bool setInputVoltsPerFullScale(double) { return false; }
    /// Full-scale input samples in, volts out. The full amp passes them to the engine unchanged (bit-exact as the
    /// plugin); other blocks scale to volts first.
    virtual void processFullScale(double* v, int numSamples) noexcept {
        for (int n = 0; n < numSamples; ++n)
            v[n] *= inputVoltsPerFullScale();
        processVolts(v, numSamples);
    }
    virtual double outputVoltsPerFullScale() const { return 1.0; }
};

/// Block ids accepted by `--block` (VERIFICATION.md §1 naming).
const std::vector<std::string>& blockIds();

/// Returns nullptr for an unknown id. Ids are case-insensitive.
std::unique_ptr<Runner> makeRunner(std::string_view blockId);

}  // namespace ampwav
