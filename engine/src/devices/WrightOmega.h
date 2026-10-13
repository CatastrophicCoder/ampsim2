#pragma once

#include <cmath>

namespace amp2203::devices {

/// Wright omega function for real z: the solution w > 0 of w + ln(w) = z (ω(z) = W0(e^z)), to double
/// precision. Initial guess log(1 + e^z) for z < 1 (exact as z → −∞) and z − ln z + ln z / z above,
/// then Fritsch–Shafer–Crowley iterations (fourth order; two suffice from these guesses, the loop
/// stops when the update is below the last bit). chowdsp's Omega::omega4 is a fast approximation and
/// is not accurate enough for the device tolerance (WDF_DESIGN §3.1).
inline double wrightOmega(double z) noexcept {
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

}  // namespace amp2203::devices
