#pragma once

#include <array>

#include "amp2203/detail/NewtonStats.h"

namespace amp2203::detail {

/// State of a block whose linear network (P element ports between N nodes) feeds a K-port nonlinear root
/// (WDF_DESIGN §4, DECISIONS D-028). Data only; the per-sample algorithm is in engine/src/stage/RootNetwork.h.
template <int N, int P, int K>
struct RootNetworkState {
    // open-root transfer: node voltages = Xe·b − Xr·i; root-port Thevenin resistance matrix Z
    double Xe[N][P]{};
    double Xr[N][K]{};
    std::array<std::array<double, K>, K> Z{};

    std::array<double, K> v{};      // root port voltages (solution of the last sample)
    std::array<double, K> vPrev{};  // the sample before (predictor, D-029)
    std::array<double, K> i{};      // root port currents
    std::array<double, N> node{};   // node voltages
    double iterations = 0.0;        // Newton iterations of the last sample (probe)
    long sanitized = 0;             // samples with a non-finite solution (state kept)
    NewtonStats stats;
};

}  // namespace amp2203::detail
