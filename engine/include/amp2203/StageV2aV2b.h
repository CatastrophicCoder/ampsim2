#pragma once

#include <array>
#include <span>
#include <tuple>

#include <chowdsp_wdf/chowdsp_wdf.h>

#include "amp2203/Probe.h"
#include "amp2203/detail/DcCapacitor.h"
#include "amp2203/detail/NewtonStats.h"
#include "amp2203/detail/RootNetworkState.h"

namespace amp2203 {

/// B3: V2a gain stage and the direct-coupled V2b cathode follower with a joint 4-port triode root (SPEC
/// §4.2, CIRCUIT.md §1 B3, DECISIONS D-032). Input: the J2 source voltage v_oc(p1b) (absolute volts) behind
/// R_th2. Output: the open-circuit cathode voltage v_oc(k2b) for the J3 junction.
///
/// Same structure as StageV1aV1b (D-028): 13 linear element ports (J2 source, C7, R10‖C8, R11, R9,
/// (bpX, R12), R13 and the six inter-electrode capacitances) and the root ports V2a (g2a−k2a, p2a−k2a),
/// V2b (p2a−k2b, bpX−k2b). bpX is an ideal supply; the ports that end on it carry it as an exact offset.
class StageV2aV2b {
public:
    /// J2 Thevenin resistance (harness/junctions.py j2, |Z(1 kHz)| of B12 at p1b; DECISIONS D-018).
    static constexpr double kRth2 = 89732.8;
    /// Supply node bpX at the F operating point (CIRCUIT.md §5, as the B3 testbench); B7 sets it in M4.
    static constexpr double kSupplyBpX = 404.998185;
    static constexpr int kNodes = 6, kElements = 13, kRootPorts = 4;

    // Component values, CIRCUIT.md §1 B3 and tubes.lib (SI units).
    static constexpr double R9 = 820.0, R10 = 470e3, R11 = 470e3, R12 = 100e3, R13 = 100e3;
    static constexpr double C7 = 0.022e-6, C8 = 470e-12;
    static constexpr double Cgk = 2.3e-12, Cgp = 2.4e-12, Cpk = 0.9e-12;

    StageV2aV2b();
    StageV2aV2b(const StageV2aV2b&) = delete;  // the adaptors and the probes hold references
    StageV2aV2b& operator=(const StageV2aV2b&) = delete;

    void prepare(double sampleRate);
    /// Supply voltage at bpX (V); constant in M2/M3.
    void setSupplyVoltage(double vbpX) noexcept { vbpX_ = vbpX; }
    /// DC operating point for a constant J2 source voltage vInDc (capacitors open). Not real-time.
    void reset(double vInDc);
    /// One sample: J2 source voltage in, v_oc(k2b) out. Real-time safe.
    double process(double vIn) noexcept;

    /// The last output v_oc(k2b) (after reset(): the DC value).
    double output() const noexcept { return out_; }
    /// Current drawn from bpX (A): through R12 and V2b's plate, for the supply model (M4).
    double supplyCurrent() const noexcept { return iSupply_; }
    const detail::NewtonStats& newtonStats() const noexcept { return root_.stats; }
    void clearStats() noexcept { root_.stats.clear(); }
    long sanitizeEvents() const noexcept { return root_.sanitized; }
    /// Failed solves recovered / not recovered by the source-stepping fallback (D-042).
    long rescuedSolves() const noexcept { return root_.rescued; }
    long unrescuedSolves() const noexcept { return root_.unrescued; }

    std::span<const Probe> probes() const noexcept { return probes_; }

private:
    using Wave = std::array<double, kElements>;

    Wave conductances(bool dc) const noexcept;
    Wave elementOffsets() const noexcept;
    std::array<double, kRootPorts> rootOffsets() const noexcept;
    void updateOutputs(double vIn) noexcept;

    // linear elements (port order of engine/src/stage3/Stage3Network.h)
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

    auto elements() noexcept {
        return std::tie(j2_, c7_, r10c8_, r11_, r9_, src12_, r13_, cgk2a_, cgp2a_, cpk2a_, cgk2b_, cgp2b_, cpk2b_);
    }

    // matrices, root solution (Vgk2a, Vpk2a, Vgk2b, Vpk2b → Ig2a, Ip2a, Ig2b, Ip2b), node voltages
    detail::RootNetworkState<kNodes, kElements, kRootPorts> root_;
    double vbpX_ = kSupplyBpX;

    // probe values (volts, amperes)
    double vIn_ = 0.0, out_ = 0.0, iSupply_ = 0.0;
    Probe probes_[15] = {{"in", &vIn_},           {"out", &out_},          {"p1b", &root_.node[0]},
                         {"nb", &root_.node[1]},   {"g2a", &root_.node[2]}, {"k2a", &root_.node[3]},
                         {"p2a", &root_.node[4]},  {"k2b", &root_.node[5]}, {"ig2a", &root_.i[0]},
                         {"ip2a", &root_.i[1]},    {"ig2b", &root_.i[2]},   {"ip2b", &root_.i[3]},
                         {"isupply", &iSupply_},   {"bpx", &vbpX_},         {"newton_iterations", &root_.iterations}};
};

}  // namespace amp2203
