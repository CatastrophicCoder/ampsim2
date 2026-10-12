#include "amp2203/Amp2203.h"

#include <cmath>

namespace amp2203 {

void Amp2203::prepare(const EngineConfig& cfg) {
    cfg_ = cfg;
    reset();
}

void Amp2203::reset() {
    vIn_ = 0.0;
    vOut_ = 0.0;
    stats_ = {};
}

void Amp2203::setParams(const Params& p) { params_ = p; }

double Amp2203::processSample(double x) noexcept {
    vIn_ = x * cfg_.inputVoltsPerFullScale;
    vOut_ = vIn_;  // M0 pass-through
    const double y = vOut_ / cfg_.outputVoltsPerFullScale;
    return std::isfinite(y) ? y : 0.0;
}

void Amp2203::process(float* mono, int numSamples) noexcept {
    for (int n = 0; n < numSamples; ++n)
        mono[n] = static_cast<float>(processSample(static_cast<double>(mono[n])));
}

void Amp2203::process(double* mono, int numSamples) noexcept {
    for (int n = 0; n < numSamples; ++n)
        mono[n] = processSample(mono[n]);
}

Amp2203::Stats Amp2203::getStats() const { return stats_; }

std::span<const Probe> Amp2203::probes() const noexcept { return probes_; }

}  // namespace amp2203
