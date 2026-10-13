#pragma once

#include "devices/Triode.h"
#include "solver/MultiportNewton.h"

namespace amp2203::devices {

/// Two 12AX7s on a 4-port root with ports (grid a, plate a, grid b, plate b), each against its own
/// cathode: (Vgk_a, Vpk_a, Vgk_b, Vpk_b) → (Ig_a, Ip_a, Ig_b, Ip_b) and the block-diagonal Jacobian.
/// Used by B12 (V1a, V1b), B3 (V2a, V2b) and B5 (V3a, V3b).
struct TwoTriodes {
    enum Port : int { gA, pA, gB, pB };

    void operator()(const solver::Vec<4>& v, solver::Vec<4>& i, solver::Mat<4>& d) const noexcept {
        const auto a = k12AX7.eval(v[gA], v[pA]);
        const auto b = k12AX7.eval(v[gB], v[pB]);
        i = {a.ig, a.ip, b.ig, b.ip};
        d = {{{a.dig_dvgk, 0.0, 0.0, 0.0},
              {a.dip_dvgk, a.dip_dvpk, 0.0, 0.0},
              {0.0, 0.0, b.dig_dvgk, 0.0},
              {0.0, 0.0, b.dip_dvgk, b.dip_dvpk}}};
    }

    /// Newton solver with the WDF_DESIGN §4 step limits (DECISIONS D-034, D-037): grid ports +50 V / −1000 V,
    /// plate ports ±50 V per iteration (the device equations cannot overflow; the limits shape convergence).
    static solver::MultiportNewton<4> solver(int maxIter) noexcept {
        solver::MultiportNewton<4> r;
        r.config.maxIter = maxIter;
        r.maxStep = {50.0, 50.0, 50.0, 50.0};
        r.maxStepDown = {1000.0, 50.0, 1000.0, 50.0};
        return r;
    }
};

}  // namespace amp2203::devices
