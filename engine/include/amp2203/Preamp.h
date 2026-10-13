#pragma once

#include <array>
#include <span>

#include "amp2203/Amp2203.h"
#include "amp2203/Probe.h"
#include "amp2203/StageV1aV1b.h"
#include "amp2203/StageV2aV2bToneStack.h"
#include "amp2203/detail/NewtonStats.h"

namespace amp2203 {

/// Preamp cascade B12 → B34 (SPEC §7 M3, §4.3; B3 and B4 merged, DECISIONS D-033): B34's J2 source is
/// B12's v_oc(p1b). Input: guitar source voltage (volts). Output: the open-circuit master wiper voltage
/// v_oc(y) for J4.
///
/// Probes use the full-netlist names where the cascade has the same node: `p1b` is the loaded p1b of
/// B34's tree, B12's open-circuit output is `voc_p1b`; `y` is v_oc(y) (no PI load). Supply currents and
/// Newton iterations carry the supply / block name.
class Preamp {
public:
    Preamp();
    Preamp(const Preamp&) = delete;  // the blocks and the probes hold references
    Preamp& operator=(const Preamp&) = delete;

    void prepare(double sampleRate, double smoothingMs);
    void setParams(const Params& p) noexcept;
    /// DC operating point of the chain for a constant input vInDc: each block from its upstream DC output.
    void reset(double vInDc);
    double process(double vIn) noexcept {
        y_ = b34_.process(b12_.process(vIn));
        return y_;
    }

    /// Newton statistics of B12 and B34 combined.
    detail::NewtonStats newtonStats() const noexcept;
    void clearStats() noexcept;
    double rth4() const noexcept { return b34_.rth4(); }

    StageV1aV1b& b12() noexcept { return b12_; }
    StageV2aV2bToneStack& b34() noexcept { return b34_; }

    std::span<const Probe> probes() const noexcept { return {probes_.data(), numProbes_}; }

private:
    StageV1aV1b b12_;
    StageV2aV2bToneStack b34_;
    double y_ = 0.0;
    std::array<Probe, 48> probes_{};
    size_t numProbes_ = 0;
};

}  // namespace amp2203
