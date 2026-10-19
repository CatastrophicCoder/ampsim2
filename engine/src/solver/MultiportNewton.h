#pragma once

#include <array>
#include <cmath>
#include <concepts>
#include <utility>

#include "amp2203/detail/LuFactors.h"
#include "amp2203/detail/NewtonStats.h"

namespace amp2203::solver {

template <int K>
using Vec = std::array<double, K>;
template <int K>
using Mat = std::array<std::array<double, K>, K>;
template <int K>
using Pivots = std::array<int, K>;

namespace lu {
/// Elimination step k of solveLinear, then the steps after it: every loop bound is a compile-time constant
/// (unrolled by the compiler; the same operations in the same order as the loop form; M6). The multipliers
/// are kept below the diagonal and the pivot rows in `piv`, so that J holds its LU factors afterwards (M7).
/// M ≤ K: only the leading M×M block and the first M entries of r take part (M7, D-068).
template <int K, int k, int M = K>
inline bool eliminate(Mat<K>& J, Vec<K>& r, Pivots<K>& piv) noexcept {
    if constexpr (k == M) {
        return true;
    } else {
        int p = k;
        double best = std::fabs(J[k][k]);
        for (int i = k + 1; i < M; ++i)
            if (const double a = std::fabs(J[i][k]); a > best) {
                best = a;
                p = i;
            }
        if (!(best > 0.0) || !std::isfinite(best))
            return false;
        piv[k] = p;
        if (p != k) {
            std::swap(J[p], J[k]);
            std::swap(r[p], r[k]);
        }
        const double inv = 1.0 / J[k][k];  // one division per pivot (D-046)
        for (int i = k + 1; i < M; ++i) {
            const double f = J[i][k] * inv;
            for (int j = k + 1; j < M; ++j)
                J[i][j] -= f * J[k][j];
            r[i] -= f * r[k];
            J[i][k] = f;  // the multiplier (L), not read by the elimination itself
        }
        return eliminate<K, k + 1, M>(J, r, piv);
    }
}

template <int K, int M = K>
inline void backSubstitute(const Mat<K>& U, Vec<K>& r) noexcept {
    for (int i = M - 1; i >= 0; --i) {
        double s = r[i];
        for (int j = i + 1; j < M; ++j)
            s -= U[i][j] * r[j];
        r[i] = s / U[i][i];
    }
}
}  // namespace lu

/// Solves J·x = r in place (x returned in r) by Gaussian elimination with partial pivoting; J is left
/// holding its LU factors with the pivot rows in `piv` (for solveFactored).
/// Returns false if a pivot is zero or not finite.
template <int K, int M = K>
bool solveLinear(Mat<K>& J, Vec<K>& r, Pivots<K>& piv) noexcept {
    if (!lu::eliminate<K, 0, M>(J, r, piv))
        return false;
    lu::backSubstitute<K, M>(J, r);
    return true;
}

template <int K>
bool solveLinear(Mat<K>& J, Vec<K>& r) noexcept {
    Pivots<K> piv;
    return solveLinear<K>(J, r, piv);
}

/// Solves J·x = r in place with the factors solveLinear<K, M> left in `lu` (M7); entries from M on are untouched.
template <int K, int M = K>
void solveFactored(const detail::LuFactors<K>& lu, Vec<K>& r) noexcept {
    // the elimination swapped whole rows, multipliers included: all swaps first, then L
    for (int k = 0; k < M; ++k)
        if (const int p = lu.piv[static_cast<size_t>(k)]; p != k)
            std::swap(r[static_cast<size_t>(p)], r[static_cast<size_t>(k)]);
    for (int k = 0; k < M; ++k)
        for (int i = k + 1; i < M; ++i)
            r[i] -= lu.m[i][k] * r[k];
    lu::backSubstitute<K, M>(lu.m, r);
}

/// A device whose last port is linear, i = g·v + c (the J4 load ports, M7, D-068): `linearPort(g, c)`. The Newton
/// then solves the other K − 1 ports; the linear port's voltage follows from them exactly at every evaluation.
template <class Device>
concept LinearLastPort = requires(const Device& d, double& g, double& c) { d.linearPort(g, c); };

template <class Device, int K>
inline constexpr int kNewtonPorts = LinearLastPort<Device> ? K - 1 : K;

/// The linear last port of a LinearLastPort device: given the other ports' currents i[0, K−1), sets v[K−1] and
/// i[K−1] from v = vOc − Z·i (row K−1). Returns nothing for other devices.
template <class Device, int K>
inline void solveLinearPort(const Device& device, const Vec<K>& vOc, const Mat<K>& Z, Vec<K>& v, Vec<K>& i) noexcept {
    if constexpr (LinearLastPort<Device>) {
        constexpr int y = K - 1;
        double g = 0.0, c = 0.0;
        device.linearPort(g, c);
        double s = vOc[y] - Z[y][y] * c;
        for (int m = 0; m < y; ++m)
            s -= Z[y][m] * i[m];
        v[y] = s / (1.0 + Z[y][y] * g);
        i[y] = g * v[y] + c;
    }
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
/// lowest ‖f‖∞ is kept and the failure is counted. On return `i` holds the currents at `v`. With `lu`, the
/// Jacobian of the last iteration is factored there (lu->valid; left unchanged when no iteration ran, M7); with
/// `didvOut`, the device ∂i/∂v of the last evaluation (the returned v when converged) is left there.
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
              detail::NewtonStats& stats, detail::LuFactors<K>* lu = nullptr,
              Mat<K>* didvOut = nullptr) const noexcept {
        if (!lu || !didvOut) {
            detail::LuFactors<K> localLu;
            Mat<K> localD;
            return solve(device, vOc, Z, v, i, stats, lu ? lu : &localLu, didvOut ? didvOut : &localD);
        }
        detail::LuFactors<K>& fac = *lu;
        Mat<K>& didv = *didvOut;
        // with a linear last port: J over the first M ports with Z' = Z_pp − Z_py·g·Z_yp / (1 + g·Z_yy) (D-068)
        constexpr int M = kNewtonPorts<Device, K>;
        const Mat<K>* Zj = &Z;
        Mat<K> Zr;
        if constexpr (M < K) {
            double g = 0.0, c = 0.0;
            device.linearPort(g, c);
            if (g != 0.0) {
                const double k = g / (1.0 + g * Z[M][M]);
                for (int r = 0; r < M; ++r) {
                    const double a = Z[r][M] * k;
                    for (int cc = 0; cc < M; ++cc)
                        Zr[r][cc] = Z[r][cc] - a * Z[M][cc];
                }
                Zj = &Zr;
            }
        }
        Vec<K> f;
        double fn = residual(device, vOc, Z, v, i, didv, f);
        Vec<K> best = v;
        Vec<K> bestI = i;
        double bestFn = fn;
        bool converged = false;
        int it = 0;
        while (!(converged = fn < config.residualTol) && it < config.maxIter) {
            ++it;
            Mat<K>& J = fac.m;
            const Mat<K>& Zn = *Zj;
            for (int c = 0; c < M; ++c) {  // the conductances these factors belong to (D-068)
                if constexpr (BlockDevice<Device, K>) {
                    for (int m = Device::kBlockBegin[c]; m < Device::kBlockEnd[c]; ++m)
                        fac.d[m][c] = didv[m][c];
                } else {
                    for (int m = 0; m < M; ++m)
                        fac.d[m][c] = didv[m][c];
                }
            }
            for (int r = 0; r < M; ++r)
                for (int c = 0; c < M; ++c) {
                    double s = r == c ? 1.0 : 0.0;
                    if constexpr (BlockDevice<Device, K>) {
                        // same terms in the same order as the dense sum without its exact zeros
                        for (int m = Device::kBlockBegin[c]; m < Device::kBlockEnd[c]; ++m)
                            s += Zn[r][m] * didv[m][c];
                    } else {
                        for (int m = 0; m < M; ++m)
                            s += Zn[r][m] * didv[m][c];
                    }
                    J[r][c] = s;
                }
            Vec<K> dv;
            for (int r = 0; r < M; ++r)
                dv[r] = -f[r];
            dv[K - 1] = M < K ? 0.0 : dv[K - 1];  // the linear port follows in residual()
            fac.valid = solveLinear<K, M>(J, dv, fac.piv);
            if (!fac.valid)
                break;
            double scale = 1.0;
            for (int r = 0; r < M; ++r)
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
    static double residual(const Device& device, const Vec<K>& vOc, const Mat<K>& Z, Vec<K>& v, Vec<K>& i,
                           Mat<K>& didv, Vec<K>& f) noexcept {
        constexpr int M = kNewtonPorts<Device, K>;
        device(v, i, didv);
        solveLinearPort<Device, K>(device, vOc, Z, v, i);  // exact for the linear port: its residual is 0
        if constexpr (M < K)
            f[M] = 0.0;
        double n = 0.0;
        for (int r = 0; r < M; ++r) {
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
