#pragma once

#include <array>
#include <span>
#include <tuple>

#include <chowdsp_wdf/chowdsp_wdf.h>

#include "amp2203/Params.h"
#include "amp2203/PhaseInverter.h"
#include "amp2203/PowerStage.h"
#include "amp2203/Probe.h"
#include "amp2203/detail/CoupledInductors.h"
#include "amp2203/detail/DcCapacitor.h"
#include "amp2203/detail/NewtonStats.h"
#include "amp2203/detail/RootNetworkState.h"
#include "amp2203/detail/Tapers.h"

namespace amp2203 {

/// B56: the phase inverter, the EL34 output stage and the output transformer in one tree with a 10-port root
/// (V3a, V3b and the two EL34 pair equivalents), DECISIONS D-041: the J5 junction was removed after the M4
/// partition study, and R22 closes the global NFB inside the tree (no delay). Inputs: the J4 source v_oc(y)
/// behind R_th4 (from the preamp) and the supply nodes bpX, nScr, ct. Output: the speaker voltage v(out).
///
/// 42 element ports: PhaseInverter's 18 (R22 now a resistor nG – out) and PowerStage's 26 without its J5
/// sources. Component values are those of the two verified sub-blocks.
class PowerAmp {
public:
    using PI = PhaseInverter;
    using PS = PowerStage;
    static constexpr int kParamUpdateInterval = 32;
    static constexpr int kNodes = 21, kElements = 42, kRootPorts = 10;
    static constexpr double kDefaultHalfStepTrigger = 100.0;  // V; D-043 (0 = off)

    PowerAmp();
    PowerAmp(const PowerAmp&) = delete;  // the adaptors and the probes hold references
    PowerAmp& operator=(const PowerAmp&) = delete;

    void prepare(double sampleRate, double smoothingMs);
    void setParams(const Params& p) noexcept { presence_.setTarget(p.presence); }
    /// J4 source resistance R_th4 (the preamp's rth4()); the matrices follow at the next update.
    void setSourceResistance(double rth4) noexcept;
    void setSupplyVoltages(double vbpX, double vScreen, double vCt) noexcept {
        vbpX_ = vbpX;
        vScr_ = vScreen;
        vCt_ = vCt;
    }
    /// R22 value (default PI::R22; 1 TΩ opens the loop for the M4 NFB check). Not real-time.
    void setFeedbackResistance(double r);
    /// DC operating point for a constant J4 source voltage (capacitors open, windings shorted). Not real-time.
    void reset(double vInDc);
    /// One sample: J4 source voltage in, speaker voltage out. Real-time safe.
    double process(double vIn) noexcept;

    double output() const noexcept { return out_; }
    /// Currents drawn from bpX (R18 + R21), from the centre tap (both plates) and from nScr (screens), A.
    double supplyCurrentX() const noexcept { return iX_; }
    double centreTapCurrent() const noexcept { return iCt_; }
    double screenCurrent() const noexcept { return iScr_; }
    /// Newton statistics of all solves (full-step samples and the half-step solves, D-043).
    detail::NewtonStats newtonStats() const noexcept {
        detail::NewtonStats all = root_.stats;
        all.add(half_.stats);
        return all;
    }
    void clearStats() noexcept {
        root_.stats.clear();
        half_.stats.clear();
        halfSteps_ = 0;
    }
    /// Adaptive half-steps (D-043): a sample whose root Thevenin voltages move by more than `volts` on any
    /// port is computed as two steps of T/2; 0 disables. Not real-time.
    void setHalfStepTrigger(double volts) noexcept { halfStepTrigger_ = volts; }
    long halfSteps() const noexcept { return halfSteps_; }
    long sanitizeEvents() const noexcept { return root_.sanitized + half_.sanitized; }
    /// Failed solves recovered / not recovered by the source-stepping fallback (D-042).
    long rescuedSolves() const noexcept { return root_.rescued + half_.rescued; }
    long unrescuedSolves() const noexcept { return root_.unrescued + half_.unrescued; }
    long matrixUpdates() const noexcept { return updates_; }
    /// Newton residual tolerance in volts (WDF_DESIGN §4.3 default 1e-9; M6 study). Not real-time.
    void setNewtonResidualTolerance(double volts) noexcept { residualTol_ = volts; }

    std::span<const Probe> probes() const noexcept { return probes_; }

private:
    using Wave = std::array<double, kElements>;

    void applyKnobs() noexcept;
    void halfStep(double vIn) noexcept;
    auto reactive() noexcept {
        return std::array<detail::DcCapacitor*, 19>{&c9_,  &c15_, &c17_, &cgk3a_, &cgp3a_, &cpk3a_, &cgk3b_,
                                                     &cgp3b_, &cpk3b_, &c14_, &c16_, &c18_, &c19_, &cgkA_,
                                                     &cgpA_, &cpkA_, &cgkB_, &cgpB_, &cpkB_};
    }
    void rescaleReactive(double ratio) noexcept;
    Wave conductances(bool dc) const noexcept;
    Wave elementOffsets() const noexcept;
    void updateOutputs(double vIn) noexcept;

    // phase inverter elements
    chowdsp::wdft::ResistiveVoltageSourceT<double> j4_{1e5};
    detail::DcCapacitor c9_{PI::C9};
    chowdsp::wdft::ResistorT<double> r17_{PI::R17}, r16_{PI::R16}, r19_{PI::R19};
    detail::DcCapacitor c15_{PI::C15};
    chowdsp::wdft::ResistorT<double> r20_{PI::R20}, r23_{PI::R23};
    detail::DcCapacitor c17_{PI::C17};
    chowdsp::wdft::ResistorT<double> vr6_{PI::VR6};
    chowdsp::wdft::WDFSeriesT<double, decltype(c17_), decltype(vr6_)> c17vr6_{c17_, vr6_};
    chowdsp::wdft::ResistorT<double> r22_{PI::R22};
    chowdsp::wdft::ResistiveVoltageSourceT<double> src18_{PI::R18}, src21_{PI::R21};
    detail::DcCapacitor cgk3a_{PI::Cgk}, cgp3a_{PI::Cgp}, cpk3a_{PI::Cpk}, cgk3b_{PI::Cgk}, cgp3b_{PI::Cgp},
        cpk3b_{PI::Cpk};
    // power stage elements
    detail::DcCapacitor c14_{PS::C14}, c16_{PS::C16}, c18_{PS::C18};
    chowdsp::wdft::ResistorT<double> r24_{PS::R24}, r25_{PS::R25};
    detail::DcCapacitor c19_{PS::C19};
    chowdsp::wdft::ResistiveVoltageSourceT<double> bias_{PS::kBiasResistance};
    chowdsp::wdft::ResistorT<double> rgA_{PS::R31 / 2}, rgB_{PS::R31 / 2};
    chowdsp::wdft::ResistiveVoltageSourceT<double> rsA_{PS::R35 / 2}, rsB_{PS::R35 / 2};
    detail::DcCapacitor cgkA_{2 * PS::Cgk}, cgpA_{2 * PS::Cgp}, cpkA_{2 * PS::Cpk}, cgkB_{2 * PS::Cgk},
        cgpB_{2 * PS::Cgp}, cpkB_{2 * PS::Cpk};
    chowdsp::wdft::ResistorT<double> rwp1_{PS::Rwp1}, rwp2_{PS::Rwp2}, rws_{PS::Rws}, rload_{PS::Rload};
    detail::CoupledInductors<3> ot_{{PS::lpt / 4, PS::lpt / 4, PS::lpt / (PS::Zpp / PS::Zs)}, PS::kot};

    auto elements() noexcept {
        return std::tie(j4_, c9_, r17_, r16_, r19_, c15_, r20_, r23_, c17vr6_, r22_, src18_, src21_, cgk3a_, cgp3a_,
                        cpk3a_, cgk3b_, cgp3b_, cpk3b_, c14_, c16_, c18_, r24_, r25_, c19_, bias_, rgA_, rgB_, rsA_, rsB_,
                        cgkA_, cgpA_, cpkA_, cgkB_, cgpB_, cpkB_, rwp1_, ot_.w[0], ot_.w[1], rwp2_, ot_.w[2], rws_,
                        rload_);
    }

    detail::RootNetworkState<kNodes, kElements, kRootPorts> root_;
    detail::RootNetworkState<kNodes, kElements, kRootPorts> half_;  // matrices at T/2 and the half-step solver state
    double halfStepTrigger_ = kDefaultHalfStepTrigger;
    double residualTol_ = 1e-9;
    long halfSteps_ = 0;
    double xPrev_ = 0.0;  // the previous sample's J4 source voltage (half-step midpoint)
    std::array<double, 22> bLast_{};  // reflected waves of the reactive elements before this sample's reflected()
    detail::KnobSmoother presence_;
    int sinceUpdate_ = 0;
    long updates_ = 0;
    bool rthDirty_ = false;
    double rth4_ = 1e5;
    double vbpX_ = PI::kSupplyBpX, vScr_ = PS::kSupplyScreen, vCt_ = PS::kSupplyCt;

    // probe values (volts, amperes); per-tube EL34 currents are half of each pair's
    double vIn_ = 0.0, out_ = 0.0, iX_ = 0.0, iCt_ = 0.0, iScr_ = 0.0, ipTubeA_ = 0.0, ipTubeB_ = 0.0, isTubeA_ = 0.0,
           isTubeB_ = 0.0;
    Probe probes_[46] = {
        {"in", &vIn_},            {"out", &out_},           {"y", &root_.node[0]},     {"g3a", &root_.node[1]},
        {"nf", &root_.node[2]},   {"k3", &root_.node[3]},   {"g3b", &root_.node[4]},   {"ng", &root_.node[5]},
        {"p3a", &root_.node[6]},  {"p3b", &root_.node[7]},  {"ngn", &root_.node[8]},   {"nor", &root_.node[9]},
        {"nbias", &root_.node[10]}, {"g4", &root_.node[11]}, {"g6", &root_.node[12]},  {"s4", &root_.node[13]},
        {"s6", &root_.node[14]},  {"pa", &root_.node[15]},  {"pa1", &root_.node[16]},  {"pb1", &root_.node[17]},
        {"pb", &root_.node[18]},  {"sl", &root_.node[20]},  {"ig3a", &root_.i[0]},     {"ip3a", &root_.i[1]},
        {"ig3b", &root_.i[2]},    {"ip3b", &root_.i[3]},    {"ig_a", &root_.i[4]},     {"is_a", &root_.i[5]},
        {"ip_a", &root_.i[6]},    {"ig_b", &root_.i[7]},    {"is_b", &root_.i[8]},     {"ip_b", &root_.i[9]},
        {"ip_tube_a", &ipTubeA_}, {"ip_tube_b", &ipTubeB_}, {"is_tube_a", &isTubeA_},  {"is_tube_b", &isTubeB_},
        {"isupply_bpx", &iX_},    {"isupply_ct", &iCt_},    {"isupply_scr", &iScr_},   {"bpx", &vbpX_},
        {"nscr", &vScr_},         {"ct", &vCt_},            {"rth4", &rth4_},          {"newton_iterations", &root_.iterations},
        {"g5", &root_.node[11]},  {"g7", &root_.node[12]}};
};

}  // namespace amp2203
