#include "Runner.h"

#include "amp2203/StageV1aV1b.h"
#include "amp2203/ToneStackMaster.h"

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

/// B4 ToneStackMaster: input = J3 source voltage (absolute volts), output = v_oc(y). The block starts
/// from the DC operating point of the first input sample (every harness signal starts there).
class ToneStackRunner final : public Runner {
public:
    void prepare(double sampleRate, int) override {
        block_.prepare(sampleRate, amp2203::EngineConfig{}.paramSmoothingMs);
        started_ = false;
    }
    void setParams(const amp2203::Params& p) override { block_.setParams(p); }

    void processVolts(double* v, int numSamples) noexcept override {
        if (numSamples > 0 && !started_) {
            block_.reset(v[0]);
            started_ = true;
        }
        for (int n = 0; n < numSamples; ++n)
            v[n] = block_.process(v[n]);
    }

    std::span<const amp2203::Probe> probes() const noexcept override { return block_.probes(); }

private:
    amp2203::ToneStackMaster block_;
    bool started_ = false;
};

/// B12 StageV1aV1b: input = guitar source voltage at node `in`, output = v_oc(p1b). The block starts
/// from the DC operating point of the first input sample (every harness signal starts there).
class StageV1aV1bRunner final : public Runner {
public:
    void prepare(double sampleRate, int) override {
        block_.prepare(sampleRate, amp2203::EngineConfig{}.paramSmoothingMs);
        started_ = false;
    }
    void setParams(const amp2203::Params& p) override { block_.setParams(p); }

    void processVolts(double* v, int numSamples) noexcept override {
        if (numSamples > 0 && !started_) {
            block_.reset(v[0]);
            block_.clearStats();
            started_ = true;
        }
        for (int n = 0; n < numSamples; ++n)
            v[n] = block_.process(v[n]);
    }

    std::span<const amp2203::Probe> probes() const noexcept override { return block_.probes(); }
    amp2203::Amp2203::Stats stats() const override {
        const auto& s = block_.newtonStats();
        amp2203::Amp2203::Stats st;
        st.newtonFailures = s.failures;
        st.maxIterationsUsed = s.maxIterations;
        st.newtonSolves = s.solves;
        st.medianIterations = s.medianIterations();
        return st;
    }

private:
    amp2203::StageV1aV1b block_;
    bool started_ = false;
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
    if (id == "b12")
        return std::make_unique<StageV1aV1bRunner>();
    if (id == "b4")
        return std::make_unique<ToneStackRunner>();
    const auto& ids = blockIds();
    if (std::find(ids.begin(), ids.end(), id) != ids.end())
        return std::make_unique<PassThroughRunner>();
    return nullptr;
}

}  // namespace ampwav
