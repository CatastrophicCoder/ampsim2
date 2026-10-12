#pragma once

#include <array>
#include <type_traits>

#include "mna/DoubleDouble.h"

namespace amp2203::mna {

/// A port network for an R-type adaptor (WDF_DESIGN §5): P ports between N non-ground nodes. Each port is
/// (node+, node−) with node indices 0..N−1 and kGround for ground. Each port is modelled as a Thevenin
/// source (a_k in series with R_k, + terminal at node+): v_k = v(node+) − v(node−), b_k = 2·v_k − a_k.
/// Counterpart of harness/mna.py (cross-checked against its exact-arithmetic fixtures).
inline constexpr int kGround = -1;

template <int P, int N>
struct PortNetwork {
    std::array<std::array<int, 2>, P> ports{};
};

namespace detail {

/// Gaussian elimination with partial pivoting on an N×N system with M right-hand sides, in place.
/// Returns false if a pivot is exactly zero (floating node).
template <int N, int M>
bool solve(std::type_identity_t<std::array<std::array<DD, N>, N>>& Y,
           std::type_identity_t<std::array<std::array<DD, N>, M>>& rhs) noexcept {
    for (int k = 0; k < N; ++k) {
        int p = k;
        for (int i = k + 1; i < N; ++i)
            if (abs(Y[i][k]) > abs(Y[p][k]))
                p = i;
        if (Y[p][k].hi == 0.0)
            return false;
        if (p != k) {
            std::swap(Y[p], Y[k]);
            for (auto& r : rhs)
                std::swap(r[p], r[k]);
        }
        for (int i = k + 1; i < N; ++i) {
            const DD f = Y[i][k] / Y[k][k];
            if (f.hi == 0.0)
                continue;
            for (int j = k; j < N; ++j)
                Y[i][j] -= f * Y[k][j];
            for (auto& r : rhs)
                r[i] -= f * r[k];
        }
    }
    for (auto& r : rhs)
        for (int i = N - 1; i >= 0; --i) {
            DD s = r[i];
            for (int j = i + 1; j < N; ++j)
                s -= Y[i][j] * r[j];
            r[i] = s / Y[i][i];
        }
    return true;
}

template <int P, int N>
std::array<std::array<DD, N>, N> nodal(const PortNetwork<P, N>& net,
                                       const std::type_identity_t<std::array<DD, P>>& G) noexcept {
    std::array<std::array<DD, N>, N> Y{};
    for (int k = 0; k < P; ++k) {
        const int a = net.ports[k][0];
        const int b = net.ports[k][1];
        if (a != kGround)
            Y[a][a] += G[k];
        if (b != kGround)
            Y[b][b] += G[k];
        if (a != kGround && b != kGround) {
            Y[a][b] -= G[k];
            Y[b][a] -= G[k];
        }
    }
    return Y;
}

template <int N>
DD portVoltage(const std::array<int, 2>& port, const std::type_identity_t<std::array<DD, N>>& v) noexcept {
    DD u = port[0] != kGround ? v[port[0]] : DD(0.0);
    if (port[1] != kGround)
        u -= v[port[1]];
    return u;
}

}  // namespace detail

/// Resistance seen into the network at port `up` with every other port replaced by its resistance
/// (WDF_DESIGN §5.2): 1 A injected at node+ of `up` with that port's own resistance removed.
/// Returns a negative value if the network is singular.
template <int P, int N>
double upPortResistance(const PortNetwork<P, N>& net, const std::type_identity_t<std::array<double, P>>& R,
                        int up) noexcept {
    std::array<DD, P> G{};
    for (int k = 0; k < P; ++k)
        G[k] = k == up ? DD(0.0) : DD(1.0) / DD(R[k]);
    auto Y = detail::nodal(net, G);
    std::array<std::array<DD, N>, 1> rhs{};
    if (net.ports[up][0] != kGround)
        rhs[0][net.ports[up][0]] += DD(1.0);
    if (net.ports[up][1] != kGround)
        rhs[0][net.ports[up][1]] -= DD(1.0);
    if (!detail::solve<N, 1>(Y, rhs))
        return -1.0;
    return detail::portVoltage<N>(net.ports[up], rhs[0]).toDouble();
}

/// Definition-based scattering matrix (WDF_DESIGN §5.1), S[n][j] = 2·v_n − δ_nj for a = e_j.
/// Returns false if the network is singular.
template <int P, int N>
bool scattering(const PortNetwork<P, N>& net, const std::type_identity_t<std::array<double, P>>& R,
                std::type_identity_t<double[P][P]>& S) noexcept {
    std::array<DD, P> G{};
    for (int k = 0; k < P; ++k)
        G[k] = DD(1.0) / DD(R[k]);
    auto Y = detail::nodal(net, G);
    std::array<std::array<DD, N>, P> rhs{};  // column j: Norton injection of a_j = 1 (G_j into node+)
    for (int j = 0; j < P; ++j) {
        if (net.ports[j][0] != kGround)
            rhs[j][net.ports[j][0]] += G[j];
        if (net.ports[j][1] != kGround)
            rhs[j][net.ports[j][1]] -= G[j];
    }
    if (!detail::solve<N, P>(Y, rhs))
        return false;
    for (int j = 0; j < P; ++j)
        for (int n = 0; n < P; ++n) {
            DD s = DD(2.0) * detail::portVoltage<N>(net.ports[n], rhs[j]);
            if (n == j)
                s -= DD(1.0);
            S[n][j] = s.toDouble();
        }
    return true;
}

/// DC node voltages for conductances G[k] on the ports plus Norton current injections I[k] (A, from
/// node− to node+ of port k). Used for `reset()` with capacitors open (G = 0, I = 0) and resistive
/// sources as Norton equivalents. Returns false if singular.
template <int P, int N>
bool solveDc(const PortNetwork<P, N>& net, const std::type_identity_t<std::array<double, P>>& G,
             const std::type_identity_t<std::array<double, P>>& I,
             std::type_identity_t<std::array<double, N>>& v) noexcept {
    std::array<DD, P> g{};
    for (int k = 0; k < P; ++k)
        g[k] = DD(G[k]);
    auto Y = detail::nodal(net, g);
    std::array<std::array<DD, N>, 1> rhs{};
    for (int k = 0; k < P; ++k) {
        if (net.ports[k][0] != kGround)
            rhs[0][net.ports[k][0]] += DD(I[k]);
        if (net.ports[k][1] != kGround)
            rhs[0][net.ports[k][1]] -= DD(I[k]);
    }
    if (!detail::solve<N, 1>(Y, rhs))
        return false;
    for (int i = 0; i < N; ++i)
        v[i] = rhs[0][i].toDouble();
    return true;
}

}  // namespace amp2203::mna
