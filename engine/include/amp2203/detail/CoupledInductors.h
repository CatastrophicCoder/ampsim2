#pragma once

#include <array>
#include <cmath>

#include <chowdsp_wdf/chowdsp_wdf.h>

namespace amp2203::detail {

/// W magnetically coupled inductors with one coupling coefficient k for every pair (as the SPICE OT model:
/// K1 = K2 = K3 = kot), discretised with the trapezoidal rule (WDF_DESIGN §5.6).
///
/// The inductance matrix L_ij = k·sqrt(L_i·L_j) (i ≠ j), L_ii = L_i, is a W-port element whose port
/// resistance matrix is R_L = (2/T)·L. Each winding is a one-port wave element like a WDF inductor
/// (b[n] = −a[n−1]); the coupling lives only in the port conductance matrix G_L = (T/2)·L⁻¹, which the
/// network stamps with mna::Mutual entries for i ≠ j and the windings' own G for i = j. Since
/// L = (1−k)·D + k·l·lᵀ with D = diag(L_i), l_i = sqrt(L_i), Sherman–Morrison gives the inverse in closed
/// form: L⁻¹_ij = δ_ij / ((1−k)·L_i) − k / ((1−k)·(1 − k + k·W)) / sqrt(L_i·L_j).
template <int W>
class CoupledInductors {
public:
    /// One winding as a wave element. `wdf.G` is the diagonal entry of G_L.
    struct Winding {
        inline double reflected() noexcept {
            wdf.b = -z;
            return wdf.b;
        }
        inline void incident(double a) noexcept {
            wdf.a = a;
            z = a;
        }
        chowdsp::wdft::WDFMembers<double> wdf;
        double z = 0.0;  // a[n−1]
    };

    CoupledInductors(const std::array<double, W>& inductance, double k) : L_(inductance), k_(k) { prepare(48000.0); }

    void prepare(double sampleRate) {
        const double T = 1.0 / sampleRate;
        const double c = k_ / ((1.0 - k_) * (1.0 - k_ + k_ * W));
        for (int i = 0; i < W; ++i)
            for (int j = 0; j < W; ++j) {
                const double inv = (i == j ? 1.0 / ((1.0 - k_) * L_[i]) : 0.0) - c / std::sqrt(L_[i] * L_[j]);
                G_[i][j] = 0.5 * T * inv;
                R_[i][j] = 2.0 / T * (i == j ? L_[i] : k_ * std::sqrt(L_[i] * L_[j]));
            }
        for (int i = 0; i < W; ++i) {
            w[i].wdf.G = G_[i][i];
            w[i].wdf.R = 1.0 / G_[i][i];
            w[i].z = 0.0;
        }
    }

    /// Port conductance matrix entry G_L[i][j] (siemens).
    double conductance(int i, int j) const noexcept { return G_[i][j]; }

    /// DC state for winding currents i (amperes, into each winding's + terminal) with zero voltage:
    /// a = v + R_L·i = R_L·i, so the next reflected wave −a keeps v = 0 at that current.
    void setDcCurrents(const std::array<double, W>& i) noexcept {
        for (int r = 0; r < W; ++r) {
            double a = 0.0;
            for (int c = 0; c < W; ++c)
                a += R_[r][c] * i[c];
            w[r].z = a;
            w[r].wdf.a = a;
            w[r].wdf.b = -a;
        }
    }

    std::array<Winding, W> w{};

private:
    std::array<double, W> L_;
    double k_;
    double G_[W][W]{};
    double R_[W][W]{};
};

}  // namespace amp2203::detail
