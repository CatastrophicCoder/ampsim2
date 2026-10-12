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
    /// Volts per full-scale sample at the input and output (only meaningful for the full amp).
    virtual double inputVoltsPerFullScale() const { return 1.0; }
    virtual double outputVoltsPerFullScale() const { return 1.0; }
};

/// Block ids accepted by `--block` (VERIFICATION.md §1 naming).
const std::vector<std::string>& blockIds();

/// Returns nullptr for an unknown id. Ids are case-insensitive.
std::unique_ptr<Runner> makeRunner(std::string_view blockId);

}  // namespace ampwav
