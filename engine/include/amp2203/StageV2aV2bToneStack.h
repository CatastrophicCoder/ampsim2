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

/// B34: V2a, the V2b cathode follower, the tone stack and the master volume in one tree with the V2a/V2b
/// 4-port root (DECISIONS D-033: J3 as a J-Thevenin junction failed because V2b cuts off into the tone
/// stack). Input: the J2 source voltage v_oc(p1b) (absolute volts) behind R_th2. Output: the open-circuit
/// master wiper voltage v_oc(y) and R_th4 for J4.
///
/// The 24 element ports are B3's 13 (StageV2aV2b) and B4's 11 (ToneStackMaster). Treble, Middle, Bass
/// and Master are smoothed as in B4; while one moves, the matrices are recomputed every
/// kParamUpdateInterval samples.
class StageV2aV2bToneStack {
public:
    static constexpr double kRth2 = 89732.8;          // harness/junctions.py j2 (D-018)
    static constexpr double kSupplyBpX = 402.335999;  // CIRCUIT.md §5; B7 sets it in M4
    static constexpr int kParamUpdateInterval = 32;
    static constexpr int kNodes = 13, kElements = 24, kRootPorts = 4;

    // Component values, CIRCUIT.md §1 B3 / B4 and tubes.lib (SI units).
    static constexpr double R9 = 820.0, R10 = 470e3, R11 = 470e3, R12 = 100e3, R13 = 100e3, R15 = 33e3;
    static constexpr double C7 = 0.022e-6, C8 = 470e-12, C10 = 470e-12, C11 = 0.022e-6, C12 = 0.022e-6;
    static constexpr double VR3 = 220e3, VR4 = 22e3, VR5 = 1e6, VR2 = 1e6;
    static constexpr double Cgk = 2.3e-12, Cgp = 2.4e-12, Cpk = 0.9e-12;

    StageV2aV2bToneStack();
    StageV2aV2bToneStack(const StageV2aV2bToneStack&) = delete;  // the adaptors and the probes hold references
    StageV2aV2bToneStack& operator=(const StageV2aV2bToneStack&) = delete;

    void prepare(double sampleRate, double smoothingMs);
    /// Knob targets (treble, middle, bass, master of `p`); smoothed in process().
    void setParams(const Params& p) noexcept;
    void setSupplyVoltage(double vbpX) noexcept { vbpX_ = vbpX; }
    /// DC operating point for a constant J2 source voltage vInDc (capacitors open); knobs jump to their
    /// targets. Not real-time.
    void reset(double vInDc);
    /// One sample: J2 source voltage in, v_oc(y) out. Real-time safe.
    double process(double vIn) noexcept;

    double output() const noexcept { return out_; }
    /// Output resistance at the master wiper for J4 (DECISIONS D-015 formula, as ToneStackMaster).
    double rth4() const noexcept { return rth4_; }
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
    Wave elementOffsets() const noexcept;
    std::array<double, kRootPorts> rootOffsets() const noexcept;
    void updateOutputs(double vIn) noexcept;

    // B3 elements
    chowdsp::wdft::ResistiveVoltageSourceT<double> j2_{kRth2};
    detail::DcCapacitor c7_{C7};
    chowdsp::wdft::ResistorT<double> r10_{R10};
    detail::DcCapacitor c8_{C8};
    chowdsp::wdft::WDFParallelT<double, decltype(r10_), decltype(c8_)> r10c8_{r10_, c8_};
    chowdsp::wdft::ResistorT<double> r11_{R11};
    chowdsp::wdft::ResistorT<double> r9_{R9};
    chowdsp::wdft::ResistiveVoltageSourceT<double> src12_{R12};
    chowdsp::wdft::ResistorT<double> r13_{R13};
    detail::DcCapacitor cgk2a_{Cgk}, cgp2a_{Cgp}, cpk2a_{Cpk}, cgk2b_{Cgk}, cgp2b_{Cgp}, cpk2b_{Cpk};
    // B4 elements
    detail::DcCapacitor c10_{C10}, c11_{C11}, c12_{C12};
    chowdsp::wdft::ResistorT<double> r15_{R15};
    chowdsp::wdft::ResistorT<double> vr3a_{1.0}, vr3b_{1.0}, vr5_{1.0}, vr4a_{1.0}, vr4b_{1.0}, vr2a_{1.0}, vr2b_{1.0};

    auto elements() noexcept {
        return std::tie(j2_, c7_, r10c8_, r11_, r9_, src12_, r13_, cgk2a_, cgp2a_, cpk2a_, cgk2b_, cgp2b_, cpk2b_,
                        c10_, r15_, c11_, c12_, vr3a_, vr3b_, vr5_, vr4a_, vr4b_, vr2a_, vr2b_);
    }

    detail::RootNetworkState<kNodes, kElements, kRootPorts> root_;
    detail::KnobSmoother treble_, middle_, bass_, master_;
    int sinceUpdate_ = 0;
    long updates_ = 0;
    double vbpX_ = kSupplyBpX;

    // probe values (volts, amperes, ohms)
    double vIn_ = 0.0, out_ = 0.0, iSupply_ = 0.0, rth4_ = 0.0;
    Probe probes_[23] = {{"in", &vIn_},          {"out", &out_},           {"p1b", &root_.node[0]},
                         {"nb", &root_.node[1]},  {"g2a", &root_.node[2]},  {"k2a", &root_.node[3]},
                         {"p2a", &root_.node[4]}, {"k2b", &root_.node[5]},  {"tt", &root_.node[6]},
                         {"nd", &root_.node[7]},  {"ne", &root_.node[8]},   {"mw", &root_.node[9]},
                         {"tw", &root_.node[10]}, {"nbb", &root_.node[11]}, {"y", &root_.node[12]},
                         {"ig2a", &root_.i[0]},   {"ip2a", &root_.i[1]},    {"ig2b", &root_.i[2]},
                         {"ip2b", &root_.i[3]},   {"isupply", &iSupply_},   {"bpx", &vbpX_},
                         {"newton_iterations", &root_.iterations},          {"rth4", &rth4_}};
};

}  // namespace amp2203
