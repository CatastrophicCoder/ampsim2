#pragma once

#include "devices/Pentode.h"
#include "solver/MultiportNewton.h"

namespace amp2203::devices {

/// The two matched EL34 pairs of B6 (V4 ‖ V5 and V6 ‖ V7, each one pair equivalent, SPEC §4.2) on a 6-port
/// root with ports (grid A, screen A, plate A, grid B, screen B, plate B), cathodes at ground:
/// (Vgk, Vsk, Vpk) per side → (Ig, Is, Ip) and the block-diagonal 3×3 Jacobians.
struct TwoPentodes {
    enum Port : int { gA, sA, pA, gB, sB, pB };
    static constexpr int kBlockBegin[6] = {0, 0, 0, 3, 3, 3};  // 3×3 blocks (solver::BlockDevice)
    static constexpr int kBlockEnd[6] = {3, 3, 3, 6, 6, 6};

    void operator()(const solver::Vec<6>& v, solver::Vec<6>& i, solver::Mat<6>& d) const noexcept {
        for (int side = 0; side < 2; ++side) {
            const int o = 3 * side;
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

    /// Newton solver (DECISIONS D-042): grid ports +50 V / −1000 V per iteration as the triode blocks
    /// (D-037); screen and plate ports unlimited (the OT flyback moves a plate by hundreds of volts in one
    /// sample); at most 2 step halvings (with 4, the 1/16 steps stall at the flyback).
    static solver::MultiportNewton<6> solver(int maxIter) noexcept {
        constexpr double kFree = 1e12;
        solver::MultiportNewton<6> r;
        r.config.maxIter = maxIter;
        r.config.maxHalvings = 2;
        r.maxStep = {50.0, kFree, kFree, 50.0, kFree, kFree};
        r.maxStepDown = {1000.0, kFree, kFree, 1000.0, kFree, kFree};
        return r;
    }
};

}  // namespace amp2203::devices
