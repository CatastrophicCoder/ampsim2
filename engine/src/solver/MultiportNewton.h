#pragma once

#include <array>
#include <cmath>
#include <concepts>
#include <utility>

#include "amp2203/detail/NewtonStats.h"

namespace amp2203::solver {

template <int K>
using Vec = std::array<double, K>;
template <int K>
using Mat = std::array<std::array<double, K>, K>;

/// Solves J·x = r in place (x returned in r) by Gaussian elimination with partial pivoting.
/// Returns false if a pivot is zero or not finite.
template <int K>
bool solveLinear(Mat<K>& J, Vec<K>& r) noexcept {
    for (int k = 0; k < K; ++k) {
        int p = k;
        double best = std::fabs(J[k][k]);
        for (int i = k + 1; i < K; ++i)
            if (const double a = std::fabs(J[i][k]); a > best) {
                best = a;
                p = i;
            }
        if (!(best > 0.0) || !std::isfinite(best))
            return false;
        if (p != k) {
            std::swap(J[p], J[k]);
            std::swap(r[p], r[k]);
        }
        const double inv = 1.0 / J[k][k];  // one division per pivot (D-046)
        for (int i = k + 1; i < K; ++i) {
            const double f = J[i][k] * inv;
            for (int j = k + 1; j < K; ++j)  // column k below the pivot is not used again
                J[i][j] -= f * J[k][j];
            r[i] -= f * r[k];
        }
    }
    for (int i = K - 1; i >= 0; --i) {
        double s = r[i];
        for (int j = i + 1; j < K; ++j)
            s -= J[i][j] * r[j];
        r[i] = s / J[i][i];
    }
    return true;
}

struct NewtonConfig {
    int maxIter = 12;          // WDF_DESIGN §4 default
    int maxHalvings = 4;
    double stepTol = 1e-9;     // ‖Δv‖∞ < stepTol·max(1, ‖v‖∞)
    double residualTol = 1e-9;  // ‖f‖∞ < residualTol (V)
};

/// Multiport nonlinear root (WDF_DESIGN §4) for K ports whose linear surroundings are given as an exact
/// K-port Thevenin equivalent v = vOc − Z·i(v): the R-type adaptor below the root with the root ports
/// eliminated (DECISIONS D-028). Solves f(v) = v + Z·i(v) − vOc = 0 with J = I + Z·∂i/∂v.
///
/// Device: `void operator()(const Vec<K>& v, Vec<K>& i, Mat<K>& didv) const noexcept`, with i the
/// currents into the + terminal of each port.
///
/// Warm start from `v` (in/out). Each step is scaled uniformly so that no port moves by more than its
/// `maxStep` (keeps the Newton direction); if ‖f‖∞ grows, the step is halved up to maxHalvings times.
/// Converged when ‖f‖∞ < residualTol or ‖Δv‖∞ < stepTol·max(1, ‖v‖∞). Otherwise the iterate with the
/// lowest ‖f‖∞ is kept and the failure is counted. On return `i` holds the currents at `v`.
/// A device whose ∂i/∂v is block-diagonal declares, per column c, the row range [kBlockBegin[c], kBlockEnd[c])
/// that can be nonzero; the Jacobian J = I + Z·∂i/∂v is then built from those entries only (D-046). Entries
/// outside the blocks are never read, so such a device need not clear them.
template <class Device, int K>
concept BlockDevice = requires {
    { Device::kBlockBegin[0] } -> std::convertible_to<int>;
    { Device::kBlockEnd[0] } -> std::convertible_to<int>;
};

template <int K>
class MultiportNewton {
public:
    NewtonConfig config;
    Vec<K> maxStep{};      // largest increase of each port voltage per iteration
    Vec<K> maxStepDown{};  // largest decrease (0: same as maxStep)

    double limitDown(int r) const noexcept { return maxStepDown[r] > 0.0 ? maxStepDown[r] : maxStep[r]; }

    template <class Device>
    int solve(const Device& device, const Vec<K>& vOc, const Mat<K>& Z, Vec<K>& v, Vec<K>& i,
              detail::NewtonStats& stats) const noexcept {
        Mat<K> didv;
        Vec<K> f;
        double fn = residual(device, vOc, Z, v, i, didv, f);
        Vec<K> best = v;
        Vec<K> bestI = i;
        double bestFn = fn;
        bool converged = false;
        int it = 0;
        while (!(converged = fn < config.residualTol) && it < config.maxIter) {
            ++it;
            Mat<K> J;
            for (int r = 0; r < K; ++r)
                for (int c = 0; c < K; ++c) {
                    double s = r == c ? 1.0 : 0.0;
                    if constexpr (BlockDevice<Device, K>) {
                        // same terms in the same order as the dense sum without its exact zeros
                        for (int m = Device::kBlockBegin[c]; m < Device::kBlockEnd[c]; ++m)
                            s += Z[r][m] * didv[m][c];
                    } else {
                        for (int m = 0; m < K; ++m)
                            s += Z[r][m] * didv[m][c];
                    }
                    J[r][c] = s;
                }
            Vec<K> dv;
            for (int r = 0; r < K; ++r)
                dv[r] = -f[r];
            if (!solveLinear<K>(J, dv))
                break;
            double scale = 1.0;
            for (int r = 0; r < K; ++r)
                if (const double lim = dv[r] >= 0.0 ? maxStep[r] : limitDown(r); std::fabs(dv[r]) * scale > lim)
                    scale = lim / std::fabs(dv[r]);
            for (auto& d : dv)
                d *= scale;

            const Vec<K> v0 = v;
            double fnNew = 0.0;
            for (int h = 0;; ++h) {
                for (int r = 0; r < K; ++r)
                    v[r] = v0[r] + dv[r];
                fnNew = residual(device, vOc, Z, v, i, didv, f);
                if (fnNew <= fn || h == config.maxHalvings)
                    break;
                for (auto& d : dv)
                    d *= 0.5;
            }
            fn = fnNew;
            if (fn < bestFn) {
                bestFn = fn;
                best = v;
                bestI = i;
            }
            double dvn = 0.0, vn = 0.0;
            for (int r = 0; r < K; ++r) {
                dvn = std::fmax(dvn, std::fabs(dv[r]));
                vn = std::fmax(vn, std::fabs(v[r]));
            }
            if (dvn < config.stepTol * std::fmax(1.0, vn) && std::isfinite(fn)) {
                converged = true;
                break;
            }
        }
        if (!converged) {
            v = best;
            i = bestI;
        }
        stats.record(it, converged);
        return it;
    }

private:
    template <class Device>
    static double residual(const Device& device, const Vec<K>& vOc, const Mat<K>& Z, const Vec<K>& v, Vec<K>& i,
                           Mat<K>& didv, Vec<K>& f) noexcept {
        device(v, i, didv);
        double n = 0.0;
        for (int r = 0; r < K; ++r) {
            double s = v[r] - vOc[r];
            for (int m = 0; m < K; ++m)
                s += Z[r][m] * i[m];
            f[r] = s;
            n = std::isfinite(s) ? std::fmax(n, std::fabs(s)) : HUGE_VAL;
        }
        return n;
    }
};

}  // namespace amp2203::solver
