#pragma once

#include <cmath>

#include "devices/Triode.h"

namespace amp2203::devices {

/// Currents of one pentode (or a matched pair) and their derivatives with respect to the port voltages
/// (control grid G1 = g−k, screen G2 = s−k, plate P = p−k). Currents flow into the grid / screen / plate
/// and out of the cathode.
struct PentodeCurrents {
    double ig = 0.0, is = 0.0, ip = 0.0;
    double dig_dvgk = 0.0;                                   // dIg/dVsk = dIg/dVpk = 0
    double dis_dvgk = 0.0, dis_dvsk = 0.0, dis_dvpk = 0.0;
    double dip_dvgk = 0.0, dip_dvsk = 0.0, dip_dvpk = 0.0;
};

/// Koren pentode with this repository's screen-current modification, as in reference/spice/tubes.lib
/// (subckt EL34, WDF_DESIGN §3.2):
///   Vs' = max(Vsk, 1 mV), u = kp·(1/mu + Vgk/Vs'), E1 = Vs'·softplus(u)/kp, F = 2·E1^ex,
///   Ip = (F/kg1)·atan(Vpk/kvb),  Is = F·(1/kg2 + (1.5707963 − atan(max(Vpk, 0)/kvb))/kg1),
///   Ig = grid diode (Rs = rgi + 1 Ω).
/// As in SPICE, atan(Vpk/kvb) in Ip is not clamped (Ip < 0 below the cathode). The 1 GΩ Re1 / Rcp
/// helpers, the diode's CJO / TT and the inter-electrode capacitances are not part of the device.
/// `count` scales every current and derivative: 2 = a matched pair with identical drive (SPEC §4.2 B6).
struct Pentode {
    double mu = 11.0, ex = 1.35, kg1 = 650.0, kg2 = 4200.0, kp = 60.0, kvb = 24.0;
    GridDiode grid{1e-9, 1.0, 1000.0 + 1.0};
    double count = 1.0;

    PentodeCurrents eval(double vgk, double vsk, double vpk) const noexcept {
        PentodeCurrents c;
        grid.eval(vgk, c.ig, c.dig_dvgk);

        const bool clamped = !(vsk > 1e-3);
        const double vs = clamped ? 1e-3 : vsk;
        const double u = kp * (1.0 / mu + vgk / vs);
        const double sp = softplus(u);
        const double e1 = vs * sp / kp;
        if (e1 > 0.0) {
            const double f = 2.0 * std::pow(e1, ex);
            const double df_de1 = ex * f / e1;
            const double sg = sigmoid(u);
            const double df_dvgk = df_de1 * sg;
            const double df_dvsk = clamped ? 0.0 : df_de1 * (sp / kp - sg * vgk / vs);

            const double x = vpk / kvb;
            const double a = std::atan(x);
            const double da = 1.0 / (kvb * (1.0 + x * x));
            c.ip = f / kg1 * a;
            c.dip_dvgk = df_dvgk / kg1 * a;
            c.dip_dvsk = df_dvsk / kg1 * a;
            c.dip_dvpk = f / kg1 * da;

            const double knee = vpk > 0.0 ? a : 0.0;
            const double sfac = 1.0 / kg2 + (1.5707963 - knee) / kg1;
            c.is = f * sfac;
            c.dis_dvgk = df_dvgk * sfac;
            c.dis_dvsk = df_dvsk * sfac;
            c.dis_dvpk = vpk > 0.0 ? -f * da / kg1 : 0.0;
        }
        if (count != 1.0) {
            for (double* p : {&c.ig, &c.is, &c.ip, &c.dig_dvgk, &c.dis_dvgk, &c.dis_dvsk, &c.dis_dvpk, &c.dip_dvgk,
                              &c.dip_dvsk, &c.dip_dvpk})
                *p *= count;
        }
        return c;
    }
};

/// EL34 with the tubes.lib parameters (mu=11 ex=1.35 kg1=650 kg2=4200 kp=60 kvb=24 rgi=1000).
inline constexpr Pentode kEL34{};
/// Matched EL34 pair with identical drive (V4 ‖ V5, V6 ‖ V7): every current doubled.
inline constexpr Pentode kEL34Pair{11.0, 1.35, 650.0, 4200.0, 60.0, 24.0, {1e-9, 1.0, 1000.0 + 1.0}, 2.0};

}  // namespace amp2203::devices
