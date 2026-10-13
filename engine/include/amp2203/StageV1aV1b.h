#pragma once

#include <array>
#include <span>
#include <tuple>

#include <chowdsp_wdf/chowdsp_wdf.h>

#include "amp2203/Amp2203.h"
#include "amp2203/Probe.h"
#include "amp2203/detail/DcCapacitor.h"
#include "amp2203/detail/NewtonStats.h"
#include "amp2203/detail/RootNetworkState.h"
#include "amp2203/detail/Tapers.h"

namespace amp2203 {

/// B12: input stage V1a, Preamp Volume network and V1b with a joint 4-port triode root (SPEC §4.2,
/// CIRCUIT.md §1 B12, DECISIONS D-017, D-028). Input: the guitar source voltage at node `in` (volts).
/// Output: the open-circuit plate voltage v_oc(p1b) for the J2 junction.
///
/// The 15 linear element ports (sources behind R3 / R4 / R7, R1‖C1, C3, R5‖C4, C5‖VR1a, VR1b, R6 and
/// the six inter-electrode capacitances) connect to the root through an R-type network whose root
/// ports are eliminated: the element waves give the open-root node voltages, the 4-port Newton solve
/// gives the triode currents, and the node voltages with those currents give the waves back to the
/// elements. The matrices come from the MNA (engine/src/mna, double-double) and are recomputed when
/// Preamp Volume moves (every kParamUpdateInterval samples, SPEC §6.2).
class StageV1aV1b {
public:
    /// Supply node bpA at the F operating point (CIRCUIT.md §5, as the B12 testbench); B7 sets it in M4.
    static constexpr double kSupplyBpA = 389.349603;
    static constexpr int kParamUpdateInterval = 32;
    static constexpr int kNodes = 8, kElements = 15, kRootPorts = 4;

    // Component values, CIRCUIT.md §1 B12 and tubes.lib (SI units).
    static constexpr double R1 = 2.7e3, R3 = 68e3, R4 = 100e3, R5 = 470e3, R6 = 10e3, R7 = 100e3;
    static constexpr double C1 = 0.68e-6, C3 = 0.022e-6, C4 = 470e-12, C5 = 1e-9, VR1 = 1e6;
    static constexpr double Cgk = 2.3e-12, Cgp = 2.4e-12, Cpk = 0.9e-12;

    StageV1aV1b();
    StageV1aV1b(const StageV1aV1b&) = delete;  // the adaptors and the probes hold references
    StageV1aV1b& operator=(const StageV1aV1b&) = delete;

    void prepare(double sampleRate, double smoothingMs);
    /// Preamp Volume target from `p`; smoothed in process().
    void setParams(const Params& p) noexcept;
    /// Supply voltage at bpA (V); constant in M2.
    void setSupplyVoltage(double vbpA) noexcept { vbpA_ = vbpA; }
    /// DC operating point for a constant input vInDc (capacitors open), solved with the same device
    /// equations; Preamp Volume jumps to its target (SPEC §3.6 option 1). Not real-time.
    void reset(double vInDc);
    /// One sample: input source voltage in, v_oc(p1b) out. Real-time safe.
    double process(double vIn) noexcept;

    /// The last output v_oc(p1b) (after reset(): the DC value).
    double output() const noexcept { return out_; }
    /// Current drawn from bpA (A), through R4 and R7, for the supply model (M4).
    double supplyCurrent() const noexcept { return iSupply_; }
    const detail::NewtonStats& newtonStats() const noexcept { return root_.stats; }
    void clearStats() noexcept { root_.stats.clear(); }
    /// Number of matrix computations so far (cost measurement).
    long matrixUpdates() const noexcept { return updates_; }
    /// Samples where the solution was not finite and the previous state was kept (should stay 0).
    long sanitizeEvents() const noexcept { return root_.sanitized; }

    std::span<const Probe> probes() const noexcept { return probes_; }

private:
    using Wave = std::array<double, kElements>;

    void applyKnobs() noexcept;
    Wave conductances(bool dc) const noexcept;

    // linear elements (port order of engine/src/stage12/Stage12Network.h)
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
    detail::DcCapacitor cgk1a_{Cgk}, cgp1a_{Cgp}, cpk1a_{Cpk}, cgk1b_{Cgk}, cgp1b_{Cgp}, cpk1b_{Cpk};

    auto elements() noexcept {
        return std::tie(vin_, k1a_, src4_, c3_, r5c4_, c5vr1a_, vr1b_, r6_, src7_, cgk1a_, cgp1a_, cpk1a_, cgk1b_,
                        cgp1b_, cpk1b_);
    }

    // matrices, root solution (Vgk1a, Vpk1a, Vgk1b, Vpk1b → Ig1a, Ip1a, Ig1b, Ip1b), node voltages
    detail::RootNetworkState<kNodes, kElements, kRootPorts> root_;

    detail::KnobSmoother volume_;
    int sinceUpdate_ = 0;
    long updates_ = 0;
    double vbpA_ = kSupplyBpA;

    // probe values (volts, amperes)
    double vIn_ = 0.0, out_ = 0.0, iSupply_ = 0.0;
    Probe probes_[17] = {{"in", &vIn_},          {"out", &out_},          {"g1a", &root_.node[0]},
                         {"k1a", &root_.node[1]}, {"p1a", &root_.node[2]}, {"na", &root_.node[3]},
                         {"vtop", &root_.node[4]}, {"w1", &root_.node[5]}, {"k1b", &root_.node[6]},
                         {"p1b", &root_.node[7]}, {"ig1a", &root_.i[0]},   {"ip1a", &root_.i[1]},
                         {"ig1b", &root_.i[2]},   {"ip1b", &root_.i[3]},   {"isupply", &iSupply_},
                         {"bpa", &vbpA_},         {"newton_iterations", &root_.iterations}};
};

}  // namespace amp2203
