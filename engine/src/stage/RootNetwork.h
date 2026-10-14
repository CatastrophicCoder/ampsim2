#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <span>
#include <tuple>
#include <type_traits>

#include <chowdsp_wdf/chowdsp_wdf.h>

#include "amp2203/detail/RootNetworkState.h"
#include "mna/PortNetwork.h"
#include "solver/MultiportNewton.h"

namespace amp2203::stage {

/// Reflected waves of a tuple of element references (the block's port order) into `wave`.
template <class Tuple, size_t P>
void reflected(Tuple&& elements, std::array<double, P>& wave) noexcept {
    std::apply([&wave](auto&... e) {
        size_t k = 0;
        ((wave[k++] = e.reflected()), ...);
    }, elements);
}

/// Incident waves from `wave` into a tuple of element references.
template <class Tuple, size_t P>
void incident(Tuple&& elements, const std::array<double, P>& wave) noexcept {
    std::apply([&wave](auto&... e) {
        size_t k = 0;
        (e.incident(wave[k++]), ...);
    }, elements);
}

/// Ports of a tuple of element references that are plain resistors (reflected wave always 0, M6).
template <class Tuple>
constexpr auto plainResistors() noexcept {
    using T = std::remove_cvref_t<Tuple>;
    return []<size_t... k>(std::index_sequence<k...>) {
        return std::array<bool, sizeof...(k)>{
            std::is_same_v<std::remove_cvref_t<std::tuple_element_t<k, T>>, chowdsp::wdft::ResistorT<double>>...};
    }(std::make_index_sequence<std::tuple_size_v<T>>{});
}

/// Per-sample algorithm of a linear network with a K-port nonlinear root (WDF_DESIGN §4, D-028), shared by
/// the triode and pentode blocks.
///
/// Ports with a terminal on an ideal supply node (a known voltage, not a network node; e.g. V2b's plate on
/// bpX) are stored against ground, and `offset` gives the exact correction: the port voltage is
/// v_port = v(node+) − v(node−) + offset, where offset = (supply at +) − (supply at −). For an element
/// port, its Thevenin source b behind R becomes b − offset in the ground-referenced network; for a root
/// port, the Thevenin voltage gains + offset.
template <int N, int P, int K>
struct RootNetwork {
    const mna::PortNetwork<P, N>& elements;
    const mna::PortNetwork<K, N>& root;

    using State = detail::RootNetworkState<N, P, K>;
    using EVec = std::array<double, P>;
    using RVec = std::array<double, K>;

    static double across(const std::array<int, 2>& port, const std::array<double, N>& node) noexcept {
        return (port[0] != mna::kGround ? node[port[0]] : 0.0) - (port[1] != mna::kGround ? node[port[1]] : 0.0);
    }

    /// Element port voltage k (with its supply offset).
    double elementVoltage(const State& s, int k, const EVec& offset) const noexcept {
        return across(elements.ports[k], s.node) + offset[k];
    }

    /// Z[r][c] = voltage across root port r per unit current drawn at root port c (root-port rows of Xr).
    void resistanceMatrix(const double (&Xr)[N][K], std::array<std::array<double, K>, K>& Z) const noexcept {
        for (int r = 0; r < K; ++r)
            for (int c = 0; c < K; ++c)
                Z[r][c] = (root.ports[r][0] != mna::kGround ? Xr[root.ports[r][0]][c] : 0.0) -
                          (root.ports[r][1] != mna::kGround ? Xr[root.ports[r][1]][c] : 0.0);
    }

    /// Recomputes Xe, Xr and Z for the element conductances G (plus the mutual conductances of coupled
    /// elements). `zeroWave` (optional) marks element ports whose wave minus offset is always exactly 0 (plain
    /// resistors without a supply offset): their Xe columns are left out of the open-circuit product (M6; the
    /// sum then skips exact zeros and is grouped differently, so results change at the rounding level).
    /// Returns false (state unchanged) if singular.
    bool updateMatrices(State& s, const EVec& G, std::span<const mna::Mutual> mutual = {},
                        const std::array<bool, P>* zeroWave = nullptr) const noexcept {
        double Xe[N][P], Xr[N][K];
        if (!mna::rootTransfer(elements, root, G, Xe, Xr, mutual))
            return false;
        s.activeCount = 0;
        for (int j = 0; j < P; ++j)
            if (zeroWave == nullptr || !(*zeroWave)[static_cast<size_t>(j)])
                s.active[static_cast<size_t>(s.activeCount++)] = j;
        for (int n = 0; n < N; ++n) {
            for (int c = 0; c < P; ++c)
                s.Xe[n][c] = c < s.activeCount ? Xe[n][s.active[static_cast<size_t>(c)]] : 0.0;
            for (int r = 0; r < K; ++r)
                s.Xr[n][r] = Xr[n][r];
        }
        resistanceMatrix(Xr, s.Z);
        return true;
    }

    /// Open-root node voltages u = Xe·(b − offset) and the root Thevenin voltages (the first part of process()).
    void openCircuit(const State& s, const EVec& wave, const EVec& eOffset, const RVec& rOffset,
                     std::array<double, N>& u, RVec& vOc) const noexcept {
        std::array<double, P> b;
        const int m = s.activeCount;
        for (int c = 0; c < m; ++c) {
            const auto j = static_cast<size_t>(s.active[static_cast<size_t>(c)]);
            b[static_cast<size_t>(c)] = wave[j] - eOffset[j];
        }
        for (int n = 0; n < N; ++n)
            u[n] = dot(s.Xe[n], b.data(), m);
        for (int r = 0; r < K; ++r)
            vOc[r] = across(root.ports[r], u) + rOffset[r];
    }

    /// Σ a[j]·b[j] with four independent partial sums (lets the compiler use SIMD without reassociating a
    /// single chain; the rounding differs from the plain loop at the last-bit level, D-047).
    template <int M>
    static double dot(const double* a, const double* b) noexcept {
        return dot(a, b, M);
    }
    static double dot(const double* a, const double* b, int M) noexcept {
        double s0 = 0.0, s1 = 0.0, s2 = 0.0, s3 = 0.0;
        int j = 0;
        for (; j + 3 < M; j += 4) {
            s0 += a[j] * b[j];
            s1 += a[j + 1] * b[j + 1];
            s2 += a[j + 2] * b[j + 2];
            s3 += a[j + 3] * b[j + 3];
        }
        for (; j < M; ++j)
            s0 += a[j] * b[j];
        return (s0 + s1) + (s2 + s3);
    }

    /// One sample. `wave` holds the elements' reflected waves on entry and the incident waves for them on
    /// return. Returns false if the solution was not finite: the state is kept and `wave` is left as is
    /// (incident = reflected: no current into any element, states unchanged).
    template <class Device>
    bool process(State& s, const solver::MultiportNewton<K>& newton, const Device& device, EVec& wave,
                 const EVec& eOffset, const RVec& rOffset) const noexcept {
        std::array<double, N> u;
        RVec vOc;
        openCircuit(s, wave, eOffset, rOffset, u, vOc);
        return solve(s, newton, device, u, vOc, wave, eOffset);
    }

    /// The rest of process() for open-circuit voltages from openCircuit(): Newton, node voltages, incident
    /// waves.
    template <class Device>
    bool solve(State& s, const solver::MultiportNewton<K>& newton, const Device& device, const std::array<double, N>& u,
               const RVec& vOc, EVec& wave, const EVec& eOffset) const noexcept {
        // predictor: linear extrapolation from the two previous solutions (D-029), each port's step
        // clamped to its Newton step limit; after a failed sample the trend restarts at zero (D-034)
        RVec v = s.v;
        for (int r = 0; r < K; ++r)
            v[r] += std::clamp(s.v[r] - s.vPrev[r], -newton.limitDown(r), newton.maxStep[r]);
        RVec i = s.i;
        const auto failures = s.stats.failures;
        int iterations = newton.solve(device, vOc, s.Z, v, i, s.stats);
        bool failed = s.stats.failures != failures;
        if (failed)
            iterations += rescue(s, newton, device, vOc, v, i, failed);
        s.iterations = iterations;
        s.vOcPrev = vOc;

        std::array<double, N> node;
        bool finite = true;
        for (int n = 0; n < N; ++n) {
            const double acc = u[n] - dot<K>(s.Xr[n], i.data());
            node[n] = acc;
            finite = finite && std::isfinite(acc);
        }
        if (!finite) {  // never output NaN/Inf (SPEC §3.5)
            ++s.sanitized;
            return false;
        }
        s.vPrev = failed ? v : s.v;
        s.v = v;
        s.i = i;
        s.node = node;
        for (int k = 0; k < P; ++k)
            wave[static_cast<size_t>(k)] = 2.0 * elementVoltage(s, k, eOffset) - wave[static_cast<size_t>(k)];
        return true;
    }

    /// Source-stepping fallback after a failed solve (DECISIONS D-042): from the previous sample's solution,
    /// the Thevenin voltages move from vOcPrev to vOc in m equal substeps (m = 2, 4, 8 until one converges),
    /// each a full Newton solve. Bounded: at most 14 more solves. On success v, i take the result, `failed`
    /// is cleared and `rescued` counted; otherwise v, i keep the best iterate of the first solve and
    /// `unrescued` is counted. The first solve's failure stays in `stats` either way.
    template <class Device>
    int rescue(State& s, const solver::MultiportNewton<K>& newton, const Device& device, const RVec& vOc, RVec& v,
               RVec& i, bool& failed) const noexcept {
        int iterations = 0;
        for (const int m : {2, 4, 8}) {
            RVec vs = s.v, is = s.i;
            bool ok = true;
            for (int k = 1; k <= m && ok; ++k) {
                RVec vk;
                for (int r = 0; r < K; ++r)
                    vk[r] = s.vOcPrev[r] + (vOc[r] - s.vOcPrev[r]) * (static_cast<double>(k) / m);
                detail::NewtonStats sub;
                iterations += newton.solve(device, vk, s.Z, vs, is, sub);
                ok = sub.failures == 0;
            }
            if (ok) {
                v = vs;
                i = is;
                failed = false;
                ++s.rescued;
                return iterations;
            }
        }
        ++s.unrescued;
        return iterations;
    }

    /// DC operating point: G with the capacitors open (G = 0), `sources` the DC Thevenin voltages of the
    /// element ports (resistive sources; 0 elsewhere), `guess` the initial root voltages. Writes node, v,
    /// vPrev and i. Returns false if the network is singular or the solve did not converge.
    template <class Device>
    bool solveDc(State& s, const solver::MultiportNewton<K>& newton, const Device& device, const EVec& G,
                 const EVec& sources, const EVec& eOffset, const RVec& rOffset, const RVec& guess) const noexcept {
        double Xe[N][P], Xr[N][K];
        if (!mna::rootTransfer(elements, root, G, Xe, Xr))
            return false;
        std::array<double, N> u{};
        for (int n = 0; n < N; ++n)
            for (int j = 0; j < P; ++j)
                u[n] += Xe[n][j] * (sources[static_cast<size_t>(j)] - eOffset[static_cast<size_t>(j)]);
        RVec vOc;
        for (int r = 0; r < K; ++r)
            vOc[r] = across(root.ports[r], u) + rOffset[r];
        std::array<std::array<double, K>, K> Z{};
        resistanceMatrix(Xr, Z);
        s.v = guess;
        detail::NewtonStats dcStats;
        newton.solve(device, vOc, Z, s.v, s.i, dcStats);
        for (int n = 0; n < N; ++n) {
            s.node[n] = u[n];
            for (int r = 0; r < K; ++r)
                s.node[n] -= Xr[n][r] * s.i[r];
        }
        s.vPrev = s.v;
        s.vOcPrev = vOc;
        s.iterations = 0.0;
        return dcStats.failures == 0;
    }
};

}  // namespace amp2203::stage
