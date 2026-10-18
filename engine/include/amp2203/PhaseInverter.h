#pragma once

#include <array>
#include <span>
#include <tuple>

#include <chowdsp_wdf/chowdsp_wdf.h>

#include "amp2203/Params.h"
#include "amp2203/Probe.h"
#include "amp2203/detail/DcCapacitor.h"
#include "amp2203/detail/NewtonStats.h"
#include "amp2203/detail/RootNetworkState.h"
#include "amp2203/detail/Tapers.h"

namespace amp2203 {

/// B5: the long-tail-pair phase inverter V3a / V3b with global negative feedback and Presence, joint
/// 4-port triode root (SPEC §4.2, CIRCUIT.md §1 B5). Inputs: the J4 source v_oc(y) behind R_th4 (from the
/// preamp), the speaker voltage driving R22 (the previous sample's output in the full amp, SPEC §4.2;
/// a recording in the standalone test) and bpX. Outputs: the open-circuit plate voltages v_oc(p3a),
/// v_oc(p3b) for J5.
///
/// 18 element ports (J4 source, C9, R17, R16, R19, C15, R20, R23, C17 + VR6 in series, R22 source, the
/// bpX sources behind R18 / R21, six inter-electrode capacitances); root ports V3a (g3a−k3, p3a−k3), V3b
/// (g3b−k3, p3b−k3). Presence is smoothed (SPEC §6.2); R_th4 follows the preamp's knobs.
class PhaseInverter {
public:
    static constexpr double kSupplyBpX = 404.998185;  // CIRCUIT.md §5
    static constexpr int kParamUpdateInterval = 32;
    static constexpr int kNodes = 8, kElements = 18, kRootPorts = 4;

    // Component values, CIRCUIT.md §1 B5 and tubes.lib (SI units).
    static constexpr double R16 = 470.0, R17 = 1e6, R18 = 82e3, R19 = 1e6, R20 = 10e3, R21 = 100e3, R22 = 100e3,
                            R23 = 4.7e3, VR6 = 22e3;
    static constexpr double C9 = 0.022e-6, C15 = 0.1e-6, C17 = 0.1e-6;
    static constexpr double Cgk = 2.3e-12, Cgp = 2.4e-12, Cpk = 0.9e-12;

    PhaseInverter();
    PhaseInverter(const PhaseInverter&) = delete;  // the adaptors and the probes hold references
    PhaseInverter& operator=(const PhaseInverter&) = delete;

    void prepare(double sampleRate, double smoothingMs);
    /// Presence target (smoothed in process()).
    void setParams(const Params& p) noexcept { presence_.setTarget(p.presence); }
    /// J4 source resistance R_th4 (the preamp's rth4()); the matrices follow at the next update.
    void setSourceResistance(double rth4) noexcept;
    void setSupplyVoltage(double vbpX) noexcept { vbpX_ = vbpX; }
    /// R22 value (default R22; e.g. 1 TΩ opens the feedback loop for the M4 NFB check). Not real-time.
    void setFeedbackResistance(double r);
    /// Speaker voltage that drives R22 for the next sample.
    void setFeedbackVoltage(double vOut) noexcept { vFb_ = vOut; }
    /// DC operating point for a constant J4 source voltage (capacitors open). Not real-time.
    void reset(double vInDc);
    /// One sample: J4 source voltage in, v_oc(p3a) out (v_oc(p3b) in outputB()). Real-time safe.
    double process(double vIn) noexcept;

    double outputA() const noexcept { return outA_; }
    double outputB() const noexcept { return outB_; }
    /// Current drawn from bpX through R18 and R21 (A).
    double supplyCurrent() const noexcept { return iSupply_; }
    const detail::NewtonStats& newtonStats() const noexcept { return root_.stats; }
    void clearStats() noexcept { root_.stats.clear(); }
    long sanitizeEvents() const noexcept { return root_.sanitized; }
    /// Failed solves recovered / not recovered by the source-stepping fallback (D-042).
    long rescuedSolves() const noexcept { return root_.rescued; }
    long unrescuedSolves() const noexcept { return root_.unrescued; }
    long matrixUpdates() const noexcept { return updates_; }

    std::span<const Probe> probes() const noexcept { return probes_; }

private:
    using Wave = std::array<double, kElements>;

    void applyKnobs() noexcept;
    Wave conductances(bool dc) const noexcept;
    Wave elementOffsets() const noexcept { return {}; }  // no port ends on bpX: R18 / R21 are sources
    void updateOutputs(double vIn) noexcept;

    chowdsp::wdft::ResistiveVoltageSourceT<double> j4_{1e5};
    detail::DcCapacitor c9_{C9};
    chowdsp::wdft::ResistorT<double> r17_{R17}, r16_{R16}, r19_{R19};
    detail::DcCapacitor c15_{C15};
    chowdsp::wdft::ResistorT<double> r20_{R20}, r23_{R23};
    detail::DcCapacitor c17_{C17};
    chowdsp::wdft::ResistorT<double> vr6_{VR6};
    chowdsp::wdft::WDFSeriesT<double, decltype(c17_), decltype(vr6_)> c17vr6_{c17_, vr6_};
    chowdsp::wdft::ResistiveVoltageSourceT<double> src22_{R22}, src18_{R18}, src21_{R21};
    detail::DcCapacitor cgk3a_{Cgk}, cgp3a_{Cgp}, cpk3a_{Cpk}, cgk3b_{Cgk}, cgp3b_{Cgp}, cpk3b_{Cpk};

    auto elements() noexcept {
        return std::tie(j4_, c9_, r17_, r16_, r19_, c15_, r20_, r23_, c17vr6_, src22_, src18_, src21_, cgk3a_, cgp3a_,
                        cpk3a_, cgk3b_, cgp3b_, cpk3b_);
    }

    detail::RootNetworkState<kNodes, kElements, kRootPorts> root_;
    detail::KnobSmoother presence_;
    int sinceUpdate_ = 0;
    long updates_ = 0;
    bool rthDirty_ = false;
    double vbpX_ = kSupplyBpX, vFb_ = 0.0, rth4_ = 1e5;

    double vIn_ = 0.0, outA_ = 0.0, outB_ = 0.0, iSupply_ = 0.0;
    Probe probes_[20] = {{"in", &vIn_},          {"out", &outA_},         {"y", &root_.node[0]},  {"g3a", &root_.node[1]},
                         {"nf", &root_.node[2]}, {"k3", &root_.node[3]},  {"g3b", &root_.node[4]}, {"ng", &root_.node[5]},
                         {"p3a", &root_.node[6]}, {"p3b", &root_.node[7]}, {"ig3a", &root_.i[0]}, {"ip3a", &root_.i[1]},
                         {"ig3b", &root_.i[2]},  {"ip3b", &root_.i[3]},   {"isupply_bpx", &iSupply_}, {"bpx", &vbpX_},
                         {"nfb", &vFb_},         {"rth4", &rth4_},        {"newton_iterations", &root_.iterations},
                         {"outb", &outB_}};
};

}  // namespace amp2203
