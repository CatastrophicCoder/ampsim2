#include "Runner.h"

#include <algorithm>
#include <cctype>

namespace ampwav {
namespace {

class FullRunner final : public Runner {
public:
    void prepare(double sampleRate, int maxBlockSize) override {
        cfg_.sampleRate = sampleRate;
        cfg_.maxBlockSize = maxBlockSize;
        amp_.prepare(cfg_);
    }
    void setParams(const amp2203::Params& p) override { amp_.setParams(p); }

    void processVolts(double* v, int numSamples) noexcept override {
        for (int n = 0; n < numSamples; ++n)
            v[n] /= cfg_.inputVoltsPerFullScale;
        amp_.process(v, numSamples);
        for (int n = 0; n < numSamples; ++n)
            v[n] *= cfg_.outputVoltsPerFullScale;
    }

    std::span<const amp2203::Probe> probes() const noexcept override { return amp_.probes(); }
    amp2203::Amp2203::Stats stats() const override { return amp_.getStats(); }
    double inputVoltsPerFullScale() const override { return cfg_.inputVoltsPerFullScale; }
    double outputVoltsPerFullScale() const override { return cfg_.outputVoltsPerFullScale; }

private:
    amp2203::EngineConfig cfg_{};
    amp2203::Amp2203 amp_;
};

/// M0 stand-in for a block that is not implemented yet: output = input (volts).
class PassThroughRunner final : public Runner {
public:
    void prepare(double, int) override { vIn_ = vOut_ = 0.0; }
    void setParams(const amp2203::Params&) override {}

    void processVolts(double* v, int numSamples) noexcept override {
        if (numSamples > 0)
            vIn_ = vOut_ = v[numSamples - 1];
    }

    std::span<const amp2203::Probe> probes() const noexcept override { return probes_; }

private:
    double vIn_ = 0.0;
    double vOut_ = 0.0;
    amp2203::Probe probes_[2] = {{"in", &vIn_}, {"out", &vOut_}};
};

std::string lower(std::string_view s) {
    std::string r(s);
    std::transform(r.begin(), r.end(), r.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return r;
}

}  // namespace

const std::vector<std::string>& blockIds() {
    static const std::vector<std::string> ids = {"full", "b12", "b3", "b4", "b5", "b6", "b7", "b56"};
    return ids;
}

std::unique_ptr<Runner> makeRunner(std::string_view blockId) {
    const std::string id = lower(blockId);
    if (id == "full")
        return std::make_unique<FullRunner>();
    const auto& ids = blockIds();
    if (std::find(ids.begin(), ids.end(), id) != ids.end())
        return std::make_unique<PassThroughRunner>();
    return nullptr;
}

}  // namespace ampwav
