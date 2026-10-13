#pragma once

#include <array>
#include <span>

#include "amp2203/Probe.h"

namespace amp2203 {

/// B7, the HT supply (SPEC §4.2, WDF_DESIGN §7, CIRCUIT.md §1 B7): Vht behind Rsrc into C23 (centre tap
/// ct), the choke Lch + Rch to C22 (screen supply nScr), R29 to C21b (bpX), R8 to C21a (bpA). A
/// five-state linear system x = (v_ct, i_L, v_scr, v_X, v_A), discretised with the trapezoidal rule
/// (matrices in prepare()). Inputs are the currents the blocks draw from the four supply nodes; they are
/// those of the previous sample (one-sample coupling, SPEC §4.3) and held over the step.
class Supply {
public:
    // CIRCUIT.md §1 B7 and the netlist parameters vnl, rsrc (SI units)
    static constexpr double Vht = 505.0, Rsrc = 100.0, C23 = 50e-6, Lch = 5.0, Rch = 115.0, C22 = 50e-6;
    static constexpr double R29 = 10e3, C21b = 50e-6, R8 = 10e3, C21a = 50e-6;
    static constexpr int kStates = 5;
    enum Node : int { iCt, iScreen, iBpX, iBpA, kNodes };  // the supply nodes; current inputs in this order

    using Currents = std::array<double, kNodes>;

    void prepare(double sampleRate);
    /// DC state for constant load currents (all derivatives zero). Not real-time.
    void reset(const Currents& load) noexcept;
    /// Advances one sample with the load currents drawn during the sample just processed.
    void process(const Currents& load) noexcept;

    double ct() const noexcept { return x_[0]; }
    double screen() const noexcept { return x_[2]; }
    double bpX() const noexcept { return x_[3]; }
    double bpA() const noexcept { return x_[4]; }
    double chokeCurrent() const noexcept { return x_[1]; }
    /// Current delivered by Vht (through Rsrc).
    double sourceCurrent() const noexcept { return (Vht - x_[0]) / Rsrc; }

    std::span<const Probe> probes() const noexcept { return probes_; }

private:
    using Vec = std::array<double, kStates>;
    using Mat = std::array<Vec, kStates>;

    void continuous(Mat& A, std::array<Vec, kNodes>& B, Vec& c) const noexcept;

    Mat Ad_{};                       // x[n+1] = Ad·x[n] + Bd·u[n] + cd
    std::array<Vec, kNodes> Bd_{};   // columns per input
    Vec cd_{};
    Vec x_{};
    Currents load_{};
    Probe probes_[9] = {{"ct", &x_[0]},     {"nscr", &x_[2]},     {"bpx", &x_[3]},     {"bpa", &x_[4]},
                        {"ichoke", &x_[1]}, {"isupply_ct", &load_[0]}, {"isupply_scr", &load_[1]},
                        {"isupply_bpx", &load_[2]}, {"isupply_bpa", &load_[3]}};
};

}  // namespace amp2203
