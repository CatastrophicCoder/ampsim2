#pragma once

#include <array>
#include <cmath>

namespace amp2203::devices {

/// Wright omega function for real z: the solution w > 0 of w + ln(w) = z (ω(z) = W0(e^z)), to double
/// precision. Initial guess log(1 + e^z) for z < 1 (exact as z → −∞) and z − ln z + ln z / z above,
/// then Fritsch–Shafer–Crowley iterations (fourth order; two suffice from these guesses, the loop
/// stops when the update is below the last bit). chowdsp's Omega::omega4 is a fast approximation and
/// is not accurate enough for the device tolerance (WDF_DESIGN §3.1).
inline double wrightOmegaReference(double z) noexcept {
    if (z < -40.0)
        return std::exp(z);  // ω = e^z·(1 − e^z + ...), relative error < 5e-18
    double w;
    if (z < 1.0) {
        w = std::log1p(std::exp(z));
    } else {
        const double lz = std::log(z);
        w = z - lz + lz / z;
    }
    for (int it = 0; it < 4; ++it) {
        const double r = z - w - std::log(w);
        const double wp1 = 1.0 + w;
        const double t = wp1 * (wp1 + (2.0 / 3.0) * r);
        const double dw = w * (r / wp1) * (t - 0.5 * r) / (t - r);
        w += dw;
        if (std::fabs(dw) <= 1e-16 * w)
            break;
    }
    return w;
}

namespace detail {
/// Fritsch–Shafer–Crowley step for w + ln w = z (fourth order).
inline double omegaStep(double z, double w) noexcept {
    const double r = z - w - std::log(w);
    const double wp1 = 1.0 + w;
    const double t = wp1 * (wp1 + (2.0 / 3.0) * r);
    return w + w * (r / wp1) * (t - 0.5 * r) / (t - r);
}

/// Cubic Hermite start for the fast omega on [kZ0, kZ1] (M7, D-066): values and slopes ω' = ω/(1 + ω) of the
/// reference at a 1/8 grid; the start is within 1e-6 relative, so one step reaches double precision.
struct OmegaTable {
    static constexpr double kZ0 = -10.0, kZ1 = 40.0, kH = 0.125;
    static constexpr int kIntervals = 400;  // (kZ1 − kZ0) / kH
    std::array<double, kIntervals + 1> w{}, d{};
    OmegaTable() noexcept {
        for (int k = 0; k <= kIntervals; ++k) {
            const double v = wrightOmegaReference(kZ0 + kH * k);
            w[static_cast<size_t>(k)] = v;
            d[static_cast<size_t>(k)] = kH * v / (1.0 + v);
        }
    }
};
inline const OmegaTable kOmegaTable{};
}  // namespace detail

/// Wright omega for the device evaluations (M7, D-066): the same function as wrightOmegaReference to a few
/// ulp, about four times faster. z < −10: the series e^z·(1 − y + 3/2·y² − 8/3·y³), y = e^z (next term
/// 125/24·y⁴ < 3e-18 relative); −10 ≤ z < 40: cubic Hermite start from a table, then one Fritsch–Shafer–
/// Crowley step; z ≥ 40: the asymptotic start z − ln z + ln z / z, then one step.
inline double wrightOmega(double z) noexcept {
    using detail::kOmegaTable;
    using T = detail::OmegaTable;
    if (z < T::kZ0) {
        const double y = std::exp(z);
        return y * (1.0 - y * (1.0 - y * (1.5 - y * (8.0 / 3.0))));
    }
    double w;
    if (z < T::kZ1) {
        const double x = (z - T::kZ0) * (1.0 / T::kH);
        const int k = static_cast<int>(x);
        const double t = x - k;
        const auto i = static_cast<size_t>(k);
        const double u = 1.0 - t;
        w = (1.0 + 2.0 * t) * u * u * kOmegaTable.w[i] + t * u * u * kOmegaTable.d[i] +
            t * t * (3.0 - 2.0 * t) * kOmegaTable.w[i + 1] - t * t * u * kOmegaTable.d[i + 1];
    } else {
        const double lz = std::log(z);
        w = z - lz + lz / z;
    }
    return detail::omegaStep(z, w);
}

}  // namespace amp2203::devices
