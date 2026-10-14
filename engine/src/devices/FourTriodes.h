#pragma once

#include "devices/Triode.h"
#include "solver/MultiportNewton.h"

namespace amp2203::devices {

/// Four 12AX7s on an 8-port root (grid and plate port of each, against its own cathode): the B1234 preamp
/// (V1a, V1b, V2a, V2b; DECISIONS D-038). Block-diagonal Jacobian of 2×2 blocks.
struct FourTriodes {
    static constexpr int kBlockBegin[8] = {0, 0, 2, 2, 4, 4, 6, 6};  // 2×2 blocks (solver::BlockDevice)
    static constexpr int kBlockEnd[8] = {2, 2, 4, 4, 6, 6, 8, 8};

    void operator()(const solver::Vec<8>& v, solver::Vec<8>& i, solver::Mat<8>& d) const noexcept { eval<8>(v, i, d); }

    /// The four triodes on ports 0–7 of a K-port root.
    template <int K>
    static void eval(const solver::Vec<K>& v, solver::Vec<K>& i, solver::Mat<K>& d) noexcept {
        for (int t = 0; t < 4; ++t) {
            const int g = 2 * t, p = 2 * t + 1;
            const auto c = k12AX7.eval(v[g], v[p]);
            i[g] = c.ig;
            i[p] = c.ip;
            d[g][g] = c.dig_dvgk;
            d[g][p] = 0.0;
            d[p][g] = c.dip_dvgk;
            d[p][p] = c.dip_dvpk;
        }
    }

    /// Newton solver with the triode step limits of D-037 on every grid / plate port.
    template <int K = 8>
    static solver::MultiportNewton<K> solverK(int maxIter) noexcept {
        const auto s = solver(maxIter);
        solver::MultiportNewton<K> r;
        r.config = s.config;
        for (int k = 0; k < 8; ++k) {
            r.maxStep[k] = s.maxStep[k];
            r.maxStepDown[k] = s.maxStepDown[k];
        }
        for (int k = 8; k < K; ++k)
            r.maxStep[k] = 1e4;  // linear ports: no limit in practice
        return r;
    }
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

/// FourTriodes plus a ninth port at the master wiper y that draws a given current (the power amp's input,
/// M6 X1: exact J4 coupling): i = iY, ∂i/∂v = 0.
struct FourTriodesLoad {
    static constexpr int kBlockBegin[9] = {0, 0, 2, 2, 4, 4, 6, 6, 8};
    static constexpr int kBlockEnd[9] = {2, 2, 4, 4, 6, 6, 8, 8, 9};
    double iY = 0.0;

    void operator()(const solver::Vec<9>& v, solver::Vec<9>& i, solver::Mat<9>& d) const noexcept {
        FourTriodes::eval<9>(v, i, d);
        i[8] = iY;
        d[8][8] = 0.0;
    }
};

}  // namespace amp2203::devices
