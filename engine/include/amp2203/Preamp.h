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

/// B1234, the preamp: V1a, the Preamp Volume network, V1b, V2a, the V2b cathode follower, the tone stack
/// and the master volume in one tree with an 8-port root of the four triodes (SPEC §4.2, DECISIONS D-038:
/// the J2 and J3 junctions were removed after the M3 partition studies). Input: guitar source voltage at
/// node `in` (volts). Output: the open-circuit master wiper voltage v_oc(y) and R_th4 for J4.
///
/// The 38 element ports are StageV1aV1b's 15 and StageV2aV2bToneStack's 23 (without its J2 source); both
/// classes stay as separately verified blocks. Preamp Volume and the tone knobs are smoothed (SPEC §6.2);
/// while one moves, the matrices are recomputed every kParamUpdateInterval samples.
class Preamp {
public:
    static constexpr double kSupplyBpA = 391.921506;  // CIRCUIT.md §5 (after D-058); B7 sets them in M4
    static constexpr double kSupplyBpX = 404.998185;
    static constexpr int kParamUpdateInterval = 128;  // samples between matrix updates while a knob moves (D-057; was 32)
    static constexpr int kNodes = 20, kElements = 38, kRootPorts = 9;  // 8 triode ports + the load at y

    // Component values, CIRCUIT.md §1 and tubes.lib (SI units).
    static constexpr double R1 = 2.7e3, R3 = 68e3, R4 = 100e3, R5 = 470e3, R6 = 10e3, R7 = 100e3;
    static constexpr double C1 = 0.68e-6, C2 = 100e-12, C3 = 0.022e-6, C4 = 470e-12, C5 = 1e-9, VR1 = 1e6;
    static constexpr double R9 = 820.0, R10 = 470e3, R11 = 470e3, R12 = 100e3, R13 = 100e3, R15 = 33e3;
    static constexpr double C7 = 0.022e-6, C8 = 470e-12, C10 = 470e-12, C11 = 0.022e-6, C12 = 0.022e-6;
    static constexpr double VR3 = 220e3, VR4 = 22e3, VR5 = 1e6, VR2 = 1e6;
    static constexpr double Cgk = 2.3e-12, Cgp = 2.4e-12, Cpk = 0.9e-12;

    Preamp();
    Preamp(const Preamp&) = delete;  // the adaptors and the probes hold references
    Preamp& operator=(const Preamp&) = delete;

    void prepare(double sampleRate, double smoothingMs);
    /// Knob targets (preampVolume, treble, middle, bass, master of `p`); smoothed in process().
    void setParams(const Params& p) noexcept;
    void setSupplyVoltages(double vbpA, double vbpX) noexcept {
        vbpA_ = vbpA;
        vbpX_ = vbpX;
    }
    /// DC operating point for a constant input vInDc (capacitors open); knobs jump to their targets.
    /// Not real-time.
    void reset(double vInDc);
    /// One sample: guitar source voltage in, v_oc(y) out. Real-time safe.
    double process(double vIn) noexcept;
    /// The same sample in steps, with a load current drawn at y (M6 X1, exact J4 coupling): beginSample, then
    /// solveLoaded (repeatable: each call restarts from the state before the sample), then finishSample.
    void beginSample(double vIn) noexcept;
    /// `final` = false: a trial solve whose node voltages and waves another call replaces (M7: not computed).
    bool solveLoaded(double iY, bool final = true) noexcept;
    /// Linearised Thevenin source at y around the last solveLoaded(iY): v_y = vTh − rTh·i_y.
    void theveninAtY(double iY, double& vTh, double& rTh) noexcept;
    /// After theveninAtY: the solution for a new load current from the linearisation, dv = −J⁻¹·Z[:, y]·Δi_y
    /// with the triode currents moved by their conductances (M7, D-067); a full solveLoaded when any port would
    /// move by more than `maxCorrection` volts or no linearisation is at hand. Returns as solveLoaded.
    bool correctLoad(double iY, double maxCorrection) noexcept;
    void finishSample() noexcept;

    double output() const noexcept { return out_; }
    /// Output resistance at the master wiper for J4 (DECISIONS D-015 formula, as ToneStackMaster).
    double rth4() const noexcept { return rth4_; }
    double supplyCurrentA() const noexcept { return iSupplyA_; }  // through R4 + R7
    double supplyCurrentX() const noexcept { return iSupplyX_; }  // through R12 + V2b's plate
    const detail::NewtonStats& newtonStats() const noexcept { return root_.stats; }
    void clearStats() noexcept { root_.stats.clear(); }
    long sanitizeEvents() const noexcept { return root_.sanitized; }
    /// Failed solves recovered / not recovered by the source-stepping fallback (D-042).
    long rescuedSolves() const noexcept { return root_.rescued; }
    long unrescuedSolves() const noexcept { return root_.unrescued; }
    long matrixUpdates() const noexcept { return updates_; }
    /// Newton residual tolerance in volts (WDF_DESIGN §4.3 default 1e-9; M6 study). Not real-time.
    void setNewtonResidualTolerance(double volts) noexcept { residualTol_ = volts; }

    std::span<const Probe> probes() const noexcept { return probes_; }

private:
    using Wave = std::array<double, kElements>;
    double residualTol_ = 1e-9;
    // a sample in steps (beginSample / solveLoaded / finishSample)
    Wave reflected_{}, incident_{}, eOff_{};
    std::array<double, kNodes> u_{};
    std::array<double, kRootPorts> vOc_{};
    std::array<std::array<double, kRootPorts>, 4> saved_{};  // v, vPrev, i, vOcPrev before the sample
    std::array<double, kNodes> savedNode_{};
    std::array<double, kRootPorts> loadSens_{};  // J⁻¹·Z[:, y] from theveninAtY (M7)
    bool loadSensValid_ = false;
    double vInNow_ = 0.0;
    bool ok_ = true;
    bool solvedOnce_ = false;  // solveLoaded() ran in this sample

    void applyKnobs() noexcept;
    Wave conductances(bool dc) const noexcept;
    Wave elementOffsets() const noexcept;
    std::array<double, kRootPorts> rootOffsets() const noexcept;
    void updateOutputs(double vIn) noexcept;

    // B12 elements
    chowdsp::wdft::ResistiveVoltageSourceT<double> vin_{R3};
    chowdsp::wdft::ResistorT<double> r1_{R1};
    detail::DcCapacitor c1_{C1};
    chowdsp::wdft::WDFParallelT<double, decltype(r1_), decltype(c1_)> k1a_{r1_, c1_};
    chowdsp::wdft::ResistiveVoltageSourceT<double> src4_{R4};
    detail::DcCapacitor c3_{C3};
    chowdsp::wdft::ResistorT<double> r5_{R5};
    detail::DcCapacitor c4_{C4};
    chowdsp::wdft::WDFParallelT<double, decltype(r5_), decltype(c4_)> r5c4_{r5_, c4_};
    detail::DcCapacitor c5_{C5};
    chowdsp::wdft::ResistorT<double> vr1a_{1.0};
    chowdsp::wdft::WDFParallelT<double, decltype(c5_), decltype(vr1a_)> c5vr1a_{c5_, vr1a_};
    chowdsp::wdft::ResistorT<double> vr1b_{1.0};
    chowdsp::wdft::ResistorT<double> r6_{R6};
    chowdsp::wdft::ResistiveVoltageSourceT<double> src7_{R7};
    // cpk1a_: V1a's Cpk in parallel with C2 (100 pF plate–cathode, D-062): one p1a – k1a port
    detail::DcCapacitor cgk1a_{Cgk}, cgp1a_{Cgp}, cpk1a_{Cpk + C2}, cgk1b_{Cgk}, cgp1b_{Cgp}, cpk1b_{Cpk};
    // B3 elements
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
        return std::tie(vin_, k1a_, src4_, c3_, r5c4_, c5vr1a_, vr1b_, r6_, src7_, cgk1a_, cgp1a_, cpk1a_, cgk1b_,
                        cgp1b_, cpk1b_, c7_, r10c8_, r11_, r9_, src12_, r13_, cgk2a_, cgp2a_, cpk2a_, cgk2b_, cgp2b_,
                        cpk2b_, c10_, r15_, c11_, c12_, vr3a_, vr3b_, vr5_, vr4a_, vr4b_, vr2a_, vr2b_);
    }

    detail::RootNetworkState<kNodes, kElements, kRootPorts> root_;
    detail::KnobSmoother volume_, treble_, middle_, bass_, master_;
    int sinceUpdate_ = 0;
    long updates_ = 0;
    double vbpA_ = kSupplyBpA, vbpX_ = kSupplyBpX;

    // probe values (volts, amperes, ohms)
    double vIn_ = 0.0, out_ = 0.0, iSupplyA_ = 0.0, iSupplyX_ = 0.0, rth4_ = 0.0;
    Probe probes_[40] = {
        {"in", &vIn_},          {"out", &out_},          {"g1a", &root_.node[0]},  {"k1a", &root_.node[1]},
        {"p1a", &root_.node[2]}, {"na", &root_.node[3]},  {"vtop", &root_.node[4]}, {"w1", &root_.node[5]},
        {"k1b", &root_.node[6]}, {"p1b", &root_.node[7]}, {"nb", &root_.node[8]},   {"g2a", &root_.node[9]},
        {"k2a", &root_.node[10]}, {"p2a", &root_.node[11]}, {"k2b", &root_.node[12]}, {"tt", &root_.node[13]},
        {"nd", &root_.node[14]}, {"ne", &root_.node[15]}, {"mw", &root_.node[16]},  {"tw", &root_.node[17]},
        {"nbb", &root_.node[18]}, {"y", &root_.node[19]}, {"ig1a", &root_.i[0]},    {"ip1a", &root_.i[1]},
        {"ig1b", &root_.i[2]},   {"ip1b", &root_.i[3]},   {"ig2a", &root_.i[4]},    {"ip2a", &root_.i[5]},
        {"ig2b", &root_.i[6]},   {"ip2b", &root_.i[7]},   {"isupply_bpa", &iSupplyA_}, {"isupply_bpx", &iSupplyX_},
        {"bpa", &vbpA_},         {"bpx", &vbpX_},         {"newton_iterations", &root_.iterations},
        {"rth4", &rth4_},        {"vgk1b", &root_.v[2]},  {"vgk2a", &root_.v[4]},   {"vgk2b", &root_.v[6]},
        {"vpk2b", &root_.v[7]}};
};

}  // namespace amp2203
