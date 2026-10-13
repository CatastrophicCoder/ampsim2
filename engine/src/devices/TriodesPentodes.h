#pragma once

#include "devices/TwoPentodes.h"
#include "devices/TwoTriodes.h"

namespace amp2203::devices {

/// The B56 root (D-041): V3a / V3b (ports 0–3, as TwoTriodes) and the two EL34 pairs (ports 4–9, as
/// TwoPentodes). The Jacobian is block-diagonal.
struct TriodesPentodes {
    void operator()(const solver::Vec<10>& v, solver::Vec<10>& i, solver::Mat<10>& d) const noexcept {
        solver::Vec<4> vt{v[0], v[1], v[2], v[3]}, it;
        solver::Mat<4> dt;
        TwoTriodes{}(vt, it, dt);
        solver::Vec<6> vp{v[4], v[5], v[6], v[7], v[8], v[9]}, ip;
        solver::Mat<6> dp;
        TwoPentodes{}(vp, ip, dp);
        d = {};
        for (int r = 0; r < 4; ++r) {
            i[r] = it[r];
            for (int c = 0; c < 4; ++c)
                d[r][c] = dt[r][c];
        }
        for (int r = 0; r < 6; ++r) {
            i[4 + r] = ip[r];
            for (int c = 0; c < 6; ++c)
                d[4 + r][4 + c] = dp[r][c];
        }
    }

    /// Step limits of both device groups (D-034, D-037, PowerStage).
    static solver::MultiportNewton<10> solver(int maxIter) noexcept {
        const auto t = TwoTriodes::solver(maxIter);
        const auto p = TwoPentodes::solver(maxIter);
        solver::MultiportNewton<10> r;
        r.config = p.config;
        for (int k = 0; k < 4; ++k) {
            r.maxStep[k] = t.maxStep[k];
            r.maxStepDown[k] = t.maxStepDown[k];
        }
        for (int k = 0; k < 6; ++k) {
            r.maxStep[4 + k] = p.maxStep[k];
            r.maxStepDown[4 + k] = p.maxStepDown[k];
        }
        return r;
    }
};

}  // namespace amp2203::devices
