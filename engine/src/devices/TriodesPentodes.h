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
        eval<10>(v, i, d);
    }

    /// The two triodes and the two pentode pairs on ports 0–9 of a K-port root.
    template <int K>
    static void eval(const solver::Vec<K>& v, solver::Vec<K>& i, solver::Mat<K>& d) noexcept {
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

    /// Step limits of both device groups; ports from 10 on (linear) without a practical limit.
    template <int K>
    static solver::MultiportNewton<K> solverK(int maxIter) noexcept {
        const auto s = solver(maxIter);
        solver::MultiportNewton<K> r;
        r.config = s.config;
        for (int k = 0; k < 10; ++k) {
            r.maxStep[k] = s.maxStep[k];
            r.maxStepDown[k] = s.maxStepDown[k];
        }
        for (int k = 10; k < K; ++k)
            r.maxStep[k] = 1e4;
        return r;
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

/// TriodesPentodes plus an eleventh port at the PI input y: the preamp as a Thevenin source (vTh behind 1/g),
/// seen as a linear device drawing i = g·(v − vTh) (M6 X1: exact J4 coupling; g = 0: no port).
struct TriodesPentodesLoad {
    static constexpr int kBlockBegin[11] = {0, 0, 2, 2, 4, 4, 4, 7, 7, 7, 10};
    static constexpr int kBlockEnd[11] = {2, 2, 4, 4, 7, 7, 7, 10, 10, 10, 11};
    double vTh = 0.0, g = 0.0;

    void operator()(const solver::Vec<11>& v, solver::Vec<11>& i, solver::Mat<11>& d) const noexcept {
        TriodesPentodes::eval<11>(v, i, d);
        i[10] = g * (v[10] - vTh);
        d[10][10] = g;
    }
};

}  // namespace amp2203::devices
