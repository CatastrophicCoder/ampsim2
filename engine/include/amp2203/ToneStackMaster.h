#pragma once

#include <array>
#include <span>

#include <chowdsp_wdf/chowdsp_wdf.h>

#include "amp2203/Amp2203.h"
#include "amp2203/Probe.h"
#include "amp2203/detail/DcCapacitor.h"
#include "amp2203/detail/Tapers.h"

namespace amp2203 {

namespace tonestack_detail {
/// Port resistances (index 0 = up-port, recomputed) → reflection-free up-port resistance and the 12×12
/// scattering matrix of the B4 R-type adaptor (engine/src/mna, double-double). Returns R_up, or a
/// negative value if the network is singular.
double scattering(std::array<double, 12>& R, double (&S)[12][12]) noexcept;
}  // namespace tonestack_detail

/// B4: tone stack and master volume (SPEC §4.2, CIRCUIT.md §1 B4), a linear WDF with one adaptable
/// R-type adaptor. Input: the J3 source voltage v_oc(k2b) (absolute volts) behind R_th3. Output: the
/// open-circuit master wiper voltage v_oc(y), and R_th4 for the J4 junction.
///
/// Tree: ideal voltage source (root) → polarity inverter → series(R_th3, R-type). The R-type is adapted
/// at its up-port (k2b–ground); its 11 down-ports are the B4 elements. The library's RootRtypeAdaptor is
/// not used because its compute() scatters the previous call's incident waves (one-sample delay).
class ToneStackMaster {
public:
    /// J3 Thevenin resistance (harness/junctions.py j3, |Z(1 kHz)| of B3 at k2b; DECISIONS D-018).
    static constexpr double kRth3 = 360.137;
    /// Samples between scattering-matrix updates while a knob moves (SPEC §6.2; cost in DECISIONS).
    static constexpr int kParamUpdateInterval = 32;

    // Component values, CIRCUIT.md §1 B4 (SI units).
    static constexpr double C10 = 470e-12, C11 = 0.022e-6, C12 = 0.022e-6, R15 = 33e3;
    static constexpr double VR3 = 220e3, VR4 = 22e3, VR5 = 1e6, VR2 = 1e6;

    ToneStackMaster();
    ToneStackMaster(const ToneStackMaster&) = delete;  // the tree and the probes hold references
    ToneStackMaster& operator=(const ToneStackMaster&) = delete;

    void prepare(double sampleRate, double smoothingMs);
    /// Knob targets (treble, middle, bass, master of `p`); smoothed in process().
    void setParams(const Params& p) noexcept;
    /// DC operating point for a constant input vInDc (capacitors open); knobs jump to their targets.
    void reset(double vInDc);
    /// One sample: J3 source voltage in, v_oc(y) out. Real-time safe.
    double process(double vIn) noexcept;

    /// Output resistance at the master wiper for J4 (DECISIONS D-015: VR2b ‖ (VR2a + VR3b + VR5 + VR4),
    /// capacitors open, J3 source shorted), from the current smoothed knob values.
    double rth4() const noexcept { return rth4_; }
    /// Number of scattering-matrix computations so far (for the cost measurement).
    long matrixUpdates() const noexcept { return updates_; }

    std::span<const Probe> probes() const noexcept { return probes_; }

private:
    struct ImpedanceCalc {
        template <typename RType>
        static double calcImpedance(RType& rtype) {
            const auto down = rtype.getPortImpedances();
            std::array<double, 12> R{};
            for (size_t i = 0; i < down.size(); ++i)
                R[i + 1] = down[i];
            double S[12][12];
            const double rUp = tonestack_detail::scattering(R, S);
            rtype.setSMatrixData(S);
            return rUp;
        }
    };

    void applyKnobs() noexcept;
    void updateProbes() noexcept;

    detail::DcCapacitor c10_{C10}, c11_{C11}, c12_{C12};
    chowdsp::wdft::ResistorT<double> r15_{R15};
    chowdsp::wdft::ResistorT<double> vr3a_{1.0}, vr3b_{1.0}, vr5_{1.0}, vr4a_{1.0}, vr4b_{1.0}, vr2a_{1.0}, vr2b_{1.0};

    using RType = chowdsp::wdft::RtypeAdaptor<double, 0, ImpedanceCalc, decltype(c10_), decltype(r15_),
                                              decltype(c11_), decltype(c12_), decltype(vr3a_), decltype(vr3b_),
                                              decltype(vr5_), decltype(vr4a_), decltype(vr4b_), decltype(vr2a_),
                                              decltype(vr2b_)>;
    RType rtype_{c10_, r15_, c11_, c12_, vr3a_, vr3b_, vr5_, vr4a_, vr4b_, vr2a_, vr2b_};
    chowdsp::wdft::ResistorT<double> rth3_{kRth3};
    chowdsp::wdft::WDFSeriesT<double, decltype(rth3_), RType> series_{rth3_, rtype_};
    chowdsp::wdft::PolarityInverterT<double, decltype(series_)> inverter_{series_};
    chowdsp::wdft::IdealVoltageSourceT<double, decltype(inverter_)> source_{inverter_};

    detail::KnobSmoother treble_, middle_, bass_, master_;
    int sinceUpdate_ = 0;
    long updates_ = 0;
    double rth4_ = 0.0;

    // probe values (volts)
    double vIn_ = 0.0, k2b_ = 0.0, tt_ = 0.0, tw_ = 0.0, nd_ = 0.0, ne_ = 0.0, nbb_ = 0.0, mw_ = 0.0, y_ = 0.0;
    // "rth4" is a resistance in ohms (diagnostic: the harness checks it against the testbench formula)
    Probe probes_[11] = {{"in", &vIn_}, {"out", &y_}, {"k2b", &k2b_}, {"tt", &tt_}, {"tw", &tw_}, {"nd", &nd_},
                         {"ne", &ne_},  {"nbb", &nbb_}, {"mw", &mw_}, {"y", &y_},  {"rth4", &rth4_}};
};

}  // namespace amp2203
