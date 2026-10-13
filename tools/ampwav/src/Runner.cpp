#include "Runner.h"

#include "amp2203/PhaseInverter.h"
#include "amp2203/PowerStage.h"
#include "amp2203/Preamp.h"
#include "amp2203/StageV1aV1b.h"
#include "amp2203/StageV2aV2b.h"
#include "amp2203/StageV2aV2bToneStack.h"
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
        started_ = false;
    }
    void setParams(const amp2203::Params& p) override { amp_.setParams(p); }
    bool setFeedback(bool closed, int delaySamples) override {
        amp_.setFeedbackOptions(closed, delaySamples);
        return true;
    }

    void processVolts(double* v, int numSamples) noexcept override {
        if (numSamples > 0 && !started_) {
            amp_.reset();  // DC operating point at the knobs given (SPICE starts there too)
            started_ = true;
        }
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
    bool started_ = false;
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

/// Newton statistics of a block in the ampwav --stats form.
amp2203::Amp2203::Stats toStats(const amp2203::detail::NewtonStats& s) {
    amp2203::Amp2203::Stats st;
    st.newtonFailures = s.failures;
    st.maxIterationsUsed = s.maxIterations;
    st.newtonSolves = s.solves;
    st.medianIterations = s.medianIterations();
    return st;
}

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
    amp2203::Amp2203::Stats stats() const override { return toStats(block_.newtonStats()); }

    bool setSupply(std::string_view node, double v) noexcept override {
        if (node != "bpa")
            return false;
        block_.setSupplyVoltage(v);
        return true;
    }

private:
    amp2203::StageV1aV1b block_;
    bool started_ = false;
};

/// B3 StageV2aV2b: input = J2 source voltage v_oc(p1b) (absolute volts), output = v_oc(k2b). The block
/// starts from the DC operating point of the first input sample.
class StageV2aV2bRunner final : public Runner {
public:
    void prepare(double sampleRate, int) override {
        block_.prepare(sampleRate);
        started_ = false;
    }
    void setParams(const amp2203::Params&) override {}

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
    amp2203::Amp2203::Stats stats() const override { return toStats(block_.newtonStats()); }

    bool setSupply(std::string_view node, double v) noexcept override {
        if (node != "bpx")
            return false;
        block_.setSupplyVoltage(v);
        return true;
    }

private:
    amp2203::StageV2aV2b block_;
    bool started_ = false;
};

/// B34 StageV2aV2bToneStack (B3 + B4 merged, D-033): input = J2 source voltage v_oc(p1b), output =
/// v_oc(y). Starts from the DC operating point of the first input sample.
class StageV2aV2bToneStackRunner final : public Runner {
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
    amp2203::Amp2203::Stats stats() const override { return toStats(block_.newtonStats()); }

    bool setSupply(std::string_view node, double v) noexcept override {
        if (node != "bpx")
            return false;
        block_.setSupplyVoltage(v);
        return true;
    }

private:
    amp2203::StageV2aV2bToneStack block_;
    bool started_ = false;
};

/// Preamp B1234 (B12 + B3 + B4 in one tree, D-038): input = guitar source voltage, output = v_oc(y).
/// Starts from the DC operating point of the first input sample. Block ids `preamp` and `b1234`.
class PreampRunner final : public Runner {
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
    amp2203::Amp2203::Stats stats() const override { return toStats(block_.newtonStats()); }

    bool setSupply(std::string_view node, double v) noexcept override {
        if (node == "bpa")
            bpA_ = v;
        else if (node == "bpx")
            bpX_ = v;
        else
            return false;
        block_.setSupplyVoltages(bpA_, bpX_);
        return true;
    }

private:
    amp2203::Preamp block_;
    bool started_ = false;
    double bpA_ = amp2203::Preamp::kSupplyBpA, bpX_ = amp2203::Preamp::kSupplyBpX;
};

/// B5 PhaseInverter standalone (the b5open testbench): input = J4 source voltage (absolute volts), output =
/// v_oc(p3a). R_th4 from the knobs with the testbench formula (DECISIONS D-015); `--supply bpx=` and
/// `--supply out=` (the speaker voltage driving R22, a recording from a B56 run) set the other inputs.
class PhaseInverterRunner final : public Runner {
public:
    void prepare(double sampleRate, int) override {
        block_.prepare(sampleRate, amp2203::EngineConfig{}.paramSmoothingMs);
        started_ = false;
    }
    void setParams(const amp2203::Params& p) override {
        using amp2203::detail::atap;
        using amp2203::detail::ltap;
        block_.setParams(p);
        const double vr2b = 1e6 * atap(p.master) + 1.0;
        const double chain = (1e6 * (1.0 - atap(p.master)) + 1.0) + 220e3 * ltap(p.treble) + (1e6 * atap(p.bass) + 1.0) + 22e3;
        block_.setSourceResistance(1.0 / (1.0 / vr2b + 1.0 / chain));
    }

    void processVolts(double* v, int numSamples) noexcept override {
        if (numSamples > 0 && !started_) {
            block_.reset(v[0]);
            started_ = true;
        }
        for (int n = 0; n < numSamples; ++n)
            v[n] = block_.process(v[n]);
    }

    std::span<const amp2203::Probe> probes() const noexcept override { return block_.probes(); }
    amp2203::Amp2203::Stats stats() const override { return toStats(block_.newtonStats()); }

    bool setSupply(std::string_view node, double v) noexcept override {
        if (node == "bpx")
            block_.setSupplyVoltage(v);
        else if (node == "out")
            block_.setFeedbackVoltage(v);
        else
            return false;
        return true;
    }

private:
    amp2203::PhaseInverter block_;
    bool started_ = false;
};

/// B6 PowerStage standalone (the b6 testbench): input = J5 source voltage v_oc(p3a) (absolute volts),
/// output = speaker voltage. `--supply p3b=` (the other PI output), `ct=` and `nscr=` set the other inputs.
class PowerStageRunner final : public Runner {
public:
    void prepare(double sampleRate, int) override {
        block_.prepare(sampleRate);
        started_ = false;
    }
    void setParams(const amp2203::Params&) override {}

    void processVolts(double* v, int numSamples) noexcept override {
        if (numSamples > 0 && !started_) {
            block_.reset(v[0], vocB_);
            started_ = true;
        }
        for (int n = 0; n < numSamples; ++n)
            v[n] = block_.process(v[n], vocB_);
    }

    std::span<const amp2203::Probe> probes() const noexcept override { return block_.probes(); }
    amp2203::Amp2203::Stats stats() const override { return toStats(block_.newtonStats()); }

    bool setSupply(std::string_view node, double v) noexcept override {
        if (node == "p3b")
            vocB_ = v;
        else if (node == "ct")
            ct_ = v;
        else if (node == "nscr")
            scr_ = v;
        else
            return false;
        block_.setSupplyVoltages(ct_, scr_);
        return true;
    }

private:
    amp2203::PowerStage block_;
    bool started_ = false;
    double vocB_ = 262.270582, ct_ = amp2203::PowerStage::kSupplyCt, scr_ = amp2203::PowerStage::kSupplyScreen;
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
    static const std::vector<std::string> ids = {"full", "preamp", "b1234", "b12", "b3", "b34", "b4", "b5", "b6", "b7", "b56"};
    return ids;
}

std::unique_ptr<Runner> makeRunner(std::string_view blockId) {
    const std::string id = lower(blockId);
    if (id == "full")
        return std::make_unique<FullRunner>();
    if (id == "b12")
        return std::make_unique<StageV1aV1bRunner>();
    if (id == "preamp" || id == "b1234")
        return std::make_unique<PreampRunner>();
    if (id == "b34")
        return std::make_unique<StageV2aV2bToneStackRunner>();
    if (id == "b3")
        return std::make_unique<StageV2aV2bRunner>();
    if (id == "b4")
        return std::make_unique<ToneStackRunner>();
    if (id == "b5")
        return std::make_unique<PhaseInverterRunner>();
    if (id == "b6")
        return std::make_unique<PowerStageRunner>();
    const auto& ids = blockIds();
    if (std::find(ids.begin(), ids.end(), id) != ids.end())
        return std::make_unique<PassThroughRunner>();
    return nullptr;
}

}  // namespace ampwav
