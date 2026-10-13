#pragma once

#include <array>
#include <span>
#include <tuple>

#include <chowdsp_wdf/chowdsp_wdf.h>

#include "amp2203/Probe.h"
#include "amp2203/detail/CoupledInductors.h"
#include "amp2203/detail/DcCapacitor.h"
#include "amp2203/detail/NewtonStats.h"
#include "amp2203/detail/RootNetworkState.h"

namespace amp2203 {

/// B6: the push-pull EL34 output stage and the output transformer with a joint 6-port pentode root (SPEC
/// §4.2, CIRCUIT.md §1 B6, §3). The matched pairs V4 ‖ V5 and V6 ‖ V7 are pair equivalents (doubled
/// currents and capacitances, grid stoppers 2.8 kΩ, screen resistors 500 Ω). Inputs: the J5 sources
/// v_oc(p3a), v_oc(p3b) behind R_thA / R_thB and the supply nodes ct (OT centre tap) and nScr. Output: the
/// speaker voltage v(out) across the 16 Ω load.
///
/// 26 element ports (J5 sources, C14, C16, C18, R24, R25, C19, the bias Thevenin source, grid stoppers,
/// screen resistors as nScr sources, six inter-electrode capacitances, the OT windings with their
/// resistances as one coupled element, the load); root ports (g, s, p) of each pair against ground.
class PowerStage {
public:
    static constexpr double kSupplyCt = 488.708422, kSupplyScreen = 485.649287;  // CIRCUIT.md §5
    /// J5 Thevenin resistances (harness/junctions.py j5a / j5b, provisional; CIRCUIT.md §4).
    static constexpr double kRthA = 58067.6, kRthB = 56937.4;
    static constexpr int kNodes = 15, kElements = 26, kRootPorts = 6;

    // Component values, CIRCUIT.md §1 B6, §3 and tubes.lib (SI units).
    static constexpr double C14 = 0.022e-6, C16 = 0.022e-6, C18 = 47e-12, C19 = 10e-6, R24 = 220e3, R25 = 220e3;
    static constexpr double Vc20 = -60.0, R27 = 15e3, R26 = 47e3, RV1 = 4e3;  // bias supply (netlist rv1 = 4k)
    /// Its Thevenin equivalent at nbias (CIRCUIT.md §1 B6: −46.364 V behind 11.59 kΩ).
    static constexpr double kBiasVoltage = Vc20 * (R26 + RV1) / (R27 + R26 + RV1);
    static constexpr double kBiasResistance = R27 * (R26 + RV1) / (R27 + R26 + RV1);
    static constexpr double R31 = 5.6e3, R35 = 1e3;                           // one tube's stopper / screen R
    static constexpr double Cgk = 15e-12, Cgp = 1e-12, Cpk = 8e-12;            // one EL34 (tubes.lib)
    static constexpr double Rwp1 = 15.36, Rwp2 = 16.56, Rws = 0.320, Rload = 16.0;
    static constexpr double lpt = 25.61, kot = 0.9997925, Zpp = 1700.0, Zs = 16.0;  // OT (D-023)

    PowerStage();
    PowerStage(const PowerStage&) = delete;  // the adaptors and the probes hold references
    PowerStage& operator=(const PowerStage&) = delete;

    void prepare(double sampleRate);
    void setSupplyVoltages(double vCt, double vScreen) noexcept {
        vCt_ = vCt;
        vScr_ = vScreen;
    }
    /// J5 source resistances (default kRthA / kRthB); recomputes the matrices. Not real-time.
    void setSourceResistances(double rthA, double rthB);
    /// DC operating point for constant J5 source voltages (capacitors open, windings shorted). Not real-time.
    void reset(double vocA, double vocB);
    /// One sample: J5 source voltages in, speaker voltage out. Real-time safe.
    double process(double vocA, double vocB) noexcept;

    double output() const noexcept { return out_; }
    /// Currents drawn from the centre tap (both plates through the OT) and from nScr (both screens), A.
    double centreTapCurrent() const noexcept { return iCt_; }
    double screenCurrent() const noexcept { return iScr_; }
    const detail::NewtonStats& newtonStats() const noexcept { return root_.stats; }
    void clearStats() noexcept { root_.stats.clear(); }
    long sanitizeEvents() const noexcept { return root_.sanitized; }

    std::span<const Probe> probes() const noexcept { return probes_; }

private:
    using Wave = std::array<double, kElements>;

    Wave conductances(bool dc) const noexcept;
    Wave elementOffsets() const noexcept;
    void updateMatrices() noexcept;
    void updateOutputs(double vocA, double vocB) noexcept;

    chowdsp::wdft::ResistiveVoltageSourceT<double> j5a_{kRthA}, j5b_{kRthB};
    detail::DcCapacitor c14_{C14}, c16_{C16}, c18_{C18};
    chowdsp::wdft::ResistorT<double> r24_{R24}, r25_{R25};
    detail::DcCapacitor c19_{C19};
    chowdsp::wdft::ResistiveVoltageSourceT<double> bias_{kBiasResistance};
    chowdsp::wdft::ResistorT<double> rgA_{R31 / 2}, rgB_{R31 / 2};
    chowdsp::wdft::ResistiveVoltageSourceT<double> rsA_{R35 / 2}, rsB_{R35 / 2};
    detail::DcCapacitor cgkA_{2 * Cgk}, cgpA_{2 * Cgp}, cpkA_{2 * Cpk}, cgkB_{2 * Cgk}, cgpB_{2 * Cgp}, cpkB_{2 * Cpk};
    chowdsp::wdft::ResistorT<double> rwp1_{Rwp1}, rwp2_{Rwp2}, rws_{Rws}, rload_{Rload};
    detail::CoupledInductors<3> ot_{{lpt / 4, lpt / 4, lpt / (Zpp / Zs)}, kot};

    auto elements() noexcept {
        return std::tie(j5a_, j5b_, c14_, c16_, c18_, r24_, r25_, c19_, bias_, rgA_, rgB_, rsA_, rsB_, cgkA_, cgpA_, cpkA_,
                        cgkB_, cgpB_, cpkB_, rwp1_, ot_.w[0], ot_.w[1], rwp2_, ot_.w[2], rws_, rload_);
    }

    detail::RootNetworkState<kNodes, kElements, kRootPorts> root_;
    double vCt_ = kSupplyCt, vScr_ = kSupplyScreen;

    // probe values (volts, amperes); per-tube currents are half of each pair's
    double vInA_ = 0.0, vInB_ = 0.0, out_ = 0.0, iCt_ = 0.0, iScr_ = 0.0, ipTubeA_ = 0.0, ipTubeB_ = 0.0,
           isTubeA_ = 0.0, isTubeB_ = 0.0;
    Probe probes_[34] = {
        {"in", &vInA_},          {"in_b", &vInB_},          {"out", &out_},             {"p3a", &root_.node[0]},
        {"p3b", &root_.node[1]}, {"ngn", &root_.node[2]},   {"nor", &root_.node[3]},    {"nbias", &root_.node[4]},
        {"g4", &root_.node[5]},  {"g6", &root_.node[6]},    {"s4", &root_.node[7]},     {"s6", &root_.node[8]},
        {"pa", &root_.node[9]},  {"pa1", &root_.node[10]},  {"pb1", &root_.node[11]},   {"pb", &root_.node[12]},
        {"sl", &root_.node[14]}, {"ig_a", &root_.i[0]},     {"is_a", &root_.i[1]},      {"ip_a", &root_.i[2]},
        {"ig_b", &root_.i[3]},   {"is_b", &root_.i[4]},     {"ip_b", &root_.i[5]},      {"ip_tube_a", &ipTubeA_},
        {"ip_tube_b", &ipTubeB_}, {"is_tube_a", &isTubeA_}, {"is_tube_b", &isTubeB_},   {"isupply_ct", &iCt_},
        {"isupply_scr", &iScr_}, {"ct", &vCt_},             {"nscr", &vScr_},           {"newton_iterations", &root_.iterations},
        {"g5", &root_.node[5]},  {"g7", &root_.node[6]}};
};

}  // namespace amp2203
