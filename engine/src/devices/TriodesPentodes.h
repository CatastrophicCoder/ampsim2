#pragma once

#include "devices/TwoPentodes.h"
#include "devices/TwoTriodes.h"

namespace amp2203::devices {

/// The B56 root (D-041): V3a / V3b (ports 0–3, as TwoTriodes) and the two EL34 pairs (ports 4–9, as
/// TwoPentodes). The Jacobian is block-diagonal.
struct TriodesPentodes {
    // 2×2 triode blocks, then 3×3 pentode-pair blocks (solver::BlockDevice)
    static constexpr int kBlockBegin[10] = {0, 0, 2, 2, 4, 4, 4, 7, 7, 7};
    static constexpr int kBlockEnd[10] = {2, 2, 4, 4, 7, 7, 7, 10, 10, 10};

    void operator()(const solver::Vec<10>& v, solver::Vec<10>& i, solver::Mat<10>& d) const noexcept {
        for (int t = 0; t < 2; ++t) {
            const int g = 2 * t, p = 2 * t + 1;
            const auto c = k12AX7.eval(v[g], v[p]);
            i[g] = c.ig;
            i[p] = c.ip;
            d[g][g] = c.dig_dvgk;
            d[g][p] = 0.0;
            d[p][g] = c.dip_dvgk;
            d[p][p] = c.dip_dvpk;
        }
        for (int side = 0; side < 2; ++side) {
            const int o = 4 + 3 * side;
            const auto c = kEL34Pair.eval(v[o], v[o + 1], v[o + 2]);
            i[o] = c.ig;
            i[o + 1] = c.is;
            i[o + 2] = c.ip;
            d[o][o] = c.dig_dvgk;
            d[o][o + 1] = 0.0;
            d[o][o + 2] = 0.0;
            d[o + 1][o] = c.dis_dvgk;
            d[o + 1][o + 1] = c.dis_dvsk;
            d[o + 1][o + 2] = c.dis_dvpk;
            d[o + 2][o] = c.dip_dvgk;
            d[o + 2][o + 1] = c.dip_dvsk;
            d[o + 2][o + 2] = c.dip_dvpk;
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
