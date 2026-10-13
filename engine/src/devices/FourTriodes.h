#pragma once

#include "devices/Triode.h"
#include "solver/MultiportNewton.h"

namespace amp2203::devices {

/// Four 12AX7s on an 8-port root (grid and plate port of each, against its own cathode): the B1234 preamp
/// (V1a, V1b, V2a, V2b; DECISIONS D-038). Block-diagonal Jacobian of 2×2 blocks.
struct FourTriodes {
    void operator()(const solver::Vec<8>& v, solver::Vec<8>& i, solver::Mat<8>& d) const noexcept {
        d = {};
        for (int t = 0; t < 4; ++t) {
            const int g = 2 * t, p = 2 * t + 1;
            const auto c = k12AX7.eval(v[g], v[p]);
            i[g] = c.ig;
            i[p] = c.ip;
            d[g][g] = c.dig_dvgk;
            d[p][g] = c.dip_dvgk;
            d[p][p] = c.dip_dvpk;
        }
    }

    /// Newton solver with the triode step limits of D-037 on every grid / plate port.
    static solver::MultiportNewton<8> solver(int maxIter) noexcept {
        solver::MultiportNewton<8> r;
        r.config.maxIter = maxIter;
        for (int t = 0; t < 4; ++t) {
            r.maxStep[2 * t] = 50.0;
            r.maxStepDown[2 * t] = 1000.0;
            r.maxStep[2 * t + 1] = 50.0;
            r.maxStepDown[2 * t + 1] = 50.0;
        }
        return r;
    }
};

}  // namespace amp2203::devices
