#pragma once

#include <cmath>

#include "devices/WrightOmega.h"

namespace amp2203::devices {

/// Numerically stable softplus ln(1 + e^u) and its derivative (WDF_DESIGN §3).
inline double softplus(double u) noexcept {
    return u > 30.0 ? u + std::log1p(std::exp(-u)) : (u < -30.0 ? std::exp(u) : std::log1p(std::exp(u)));
}
inline double sigmoid(double u) noexcept { return 1.0 / (1.0 + std::exp(-u)); }

/// Thermal voltage at 27 °C with ngspice's constants (WDF_DESIGN §3.1): 25.8649170 mV.
inline constexpr double kThermalVoltage = 1.38064852e-23 * 300.15 / 1.6021766208e-19;

/// Grid diode of tubes.lib (Rg in series with D(IS, RS = 1 Ω), grid → cathode) in closed form with the
/// Wright omega function: Ig = (n·Vt/Rs)·ω(z) − Is, z = ln(Is·Rs/(n·Vt)) + (Vgk + Is·Rs)/(n·Vt),
/// dIg/dVgk = ω / (Rs·(1 + ω)) (= 1 / (Rs + n·Vt/(Ig + Is))).
struct GridDiode {
    double is = 1e-9;   // A
    double n = 1.0;
    double rs = 2001.0;  // Ω: rgi + the diode's RS

    void eval(double vgk, double& ig, double& dig) const noexcept {
        const double nvt = n * kThermalVoltage;
        const double z = std::log(is * rs / nvt) + (vgk + is * rs) / nvt;
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
        const double sp = softplus(u);
        const double e1 = vpk * sp / kp;
        if (e1 > 0.0) {
            c.ip = 2.0 * std::pow(e1, ex) / kg1;
            const double dip_de1 = ex * c.ip / e1;  // 2·ex·E1^(ex−1)/kg1
            const double sg = sigmoid(u);
            c.dip_dvgk = dip_de1 * vpk * sg / s;
            c.dip_dvpk = dip_de1 * (sp / kp - sg * vgk * vpk * vpk / (s * s * s));
        }
        return c;
    }
};

/// 12AX7 with the tubes.lib parameters (mu=100 ex=1.4 kg1=1060 kp=600 kvb=300 rgi=2000).
inline constexpr Triode k12AX7{};

}  // namespace amp2203::devices
