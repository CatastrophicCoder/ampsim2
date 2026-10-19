#pragma once

#include <cmath>

#include "devices/WrightOmega.h"

namespace amp2203::devices {

/// Numerically stable softplus ln(1 + e^u) and its derivative (WDF_DESIGN §3).
inline double softplus(double u) noexcept {
    return u > 30.0 ? u + std::log1p(std::exp(-u)) : (u < -30.0 ? std::exp(u) : std::log1p(std::exp(u)));
}
inline double sigmoid(double u) noexcept { return 1.0 / (1.0 + std::exp(-u)); }

/// softplus(u) and sigmoid(u) with one exponential (D-046); the same branches as softplus().
inline void softplusSigmoid(double u, double& sp, double& sg) noexcept {
    if (u > 30.0) {
        const double e = std::exp(-u);
        sp = u + std::log1p(e);
        sg = 1.0 / (1.0 + e);
    } else {
        const double e = std::exp(u);
        sp = u < -30.0 ? e : std::log1p(e);
        sg = e / (1.0 + e);
    }
}

/// Thermal voltage at 27 °C with ngspice's constants (WDF_DESIGN §3.1): 25.8649170 mV.
inline constexpr double kThermalVoltage = 1.38064852e-23 * 300.15 / 1.6021766208e-19;

/// Grid diode of tubes.lib (Rg in series with D(IS, RS = 1 Ω), grid → cathode) in closed form with the
/// Wright omega function: Ig = (n·Vt/Rs)·ω(z) − Is, z = ln(Is·Rs/(n·Vt)) + (Vgk + Is·Rs)/(n·Vt),
/// dIg/dVgk = ω / (Rs·(1 + ω)) (= 1 / (Rs + n·Vt/(Ig + Is))).
struct GridDiode {
    GridDiode(double saturation, double emission, double series) noexcept
        : is(saturation), n(emission), rs(series), nvt(n * kThermalVoltage), z0(std::log(is * rs / nvt) + is * rs / nvt) {}

    double is;   // A
    double n;
    double rs;   // Ω: rgi + the diode's RS
    double nvt;  // n·Vt
    double z0;   // ln(Is·Rs/(n·Vt)) + Is·Rs/(n·Vt): the constant part of z (D-046)

    void eval(double vgk, double& ig, double& dig) const noexcept {
        const double z = z0 + vgk / nvt;
        const double w = wrightOmega(z);
        ig = nvt / rs * w - is;
        dig = w / (rs * (1.0 + w));
    }
};

/// Currents of one triode and their derivatives with respect to the port voltages (grid port G = g−k,
/// plate port P = p−k). Currents flow into the grid / plate and out of the cathode.
struct TriodeCurrents {
    double ig = 0.0, ip = 0.0;
    double dig_dvgk = 0.0;                  // dIg/dVpk = 0
    double dip_dvgk = 0.0, dip_dvpk = 0.0;
};

/// Koren triode as in reference/spice/tubes.lib (subckt 12AX7, WDF_DESIGN §3.1). The 1 GΩ Re1 / Rcp
/// helpers are omitted; the diode's CJO and TT and the inter-electrode capacitances are not part of
/// the device (the capacitances belong to the block's linear network, SPEC §4.2 and §5).
struct Triode {
    double mu = 100.0, ex = 1.4, kg1 = 1060.0, kp = 600.0, kvb = 300.0;
    GridDiode grid{1e-9, 1.0, 2000.0 + 1.0};

    TriodeCurrents eval(double vgk, double vpk) const noexcept {
        TriodeCurrents c;
        grid.eval(vgk, c.ig, c.dig_dvgk);

        const double s = std::sqrt(kvb + vpk * vpk);
        const double u = kp * (1.0 / mu + vgk / s);
        double sp, sg;
        softplusSigmoid(u, sp, sg);
        const double e1 = vpk * sp / kp;
        if (e1 > 0.0) {
            c.ip = 2.0 * std::exp(ex * std::log(e1)) / kg1;
            const double dip_de1 = ex * c.ip / e1;  // 2·ex·E1^(ex−1)/kg1
            c.dip_dvgk = dip_de1 * vpk * sg / s;
            c.dip_dvpk = dip_de1 * (sp / kp - sg * vgk * vpk * vpk / (s * s * s));
        }
        return c;
    }
};

/// 12AX7 with the tubes.lib parameters (mu=100 ex=1.4 kg1=1060 kp=600 kvb=300 rgi=2000).
inline const Triode k12AX7{};

}  // namespace amp2203::devices
