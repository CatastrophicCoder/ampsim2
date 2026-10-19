// Multiport Newton root (WDF_DESIGN §4, DECISIONS D-028): closed-form one-port check, a coupled two-port,
// step limiting and failure counting.
#include <catch2/catch_test_macros.hpp>

#include <cmath>

#include "devices/Triode.h"
#include "solver/MultiportNewton.h"

using amp2203::detail::NewtonStats;
using amp2203::solver::Mat;
using amp2203::solver::MultiportNewton;
using amp2203::solver::Vec;

namespace {

constexpr double kIs = 1e-9;
constexpr double kVt = amp2203::devices::kThermalVoltage;

/// K independent ideal diodes on the K ports.
template <int K>
struct Diodes {
    void operator()(const Vec<K>& v, Vec<K>& i, Mat<K>& d) const noexcept {
        d = {};
        for (int k = 0; k < K; ++k) {
            const double e = std::exp(v[k] / kVt);
            i[k] = kIs * (e - 1.0);
            d[k][k] = kIs * e / kVt;
        }
    }
};

}  // namespace

TEST_CASE("Newton one-port: diode behind a resistor matches the Wright-omega closed form", "[newton][m2]") {
    MultiportNewton<1> root;
    root.maxStep = {2.0};
    for (double R : {100.0, 2001.0, 1e5}) {
        for (double vOc : {-5.0, -0.5, 0.0, 0.3, 0.6, 1.0, 2.0, 5.0}) {
            // the same circuit in closed form: series resistor + diode = the GridDiode model
            const amp2203::devices::GridDiode closed{kIs, 1.0, R};
            double iRef, unused;
            closed.eval(vOc, iRef, unused);
            const double vRef = vOc - R * iRef;

            Vec<1> v{0.0}, i{};
            NewtonStats stats;
            root.solve(Diodes<1>{}, Vec<1>{vOc}, Mat<1>{{{R}}}, v, i, stats);
            INFO("R " << R << " vOc " << vOc << " v " << v[0] << " ref " << vRef);
            REQUIRE(stats.failures == 0);
            REQUIRE(std::fabs(v[0] - vRef) <= 1e-9 * std::max(1.0, std::fabs(vRef)));
            REQUIRE(std::fabs(i[0] - iRef) <= 1e-9 * std::max(1e-6, std::fabs(iRef)));
        }
    }
}

TEST_CASE("Newton two-port: coupled Thevenin matrix", "[newton][m2]") {
    // two diodes sharing a 1 kΩ return path (off-diagonal Z), as the shared cathode of a triode
    const Mat<2> Z{{{11e3, 1e3}, {1e3, 5e3}}};
    MultiportNewton<2> root;
    root.maxStep = {2.0, 2.0};
    for (double a : {0.0, 0.5, 1.5, 4.0}) {
        const Vec<2> vOc{a, 0.7 * a};
        Vec<2> v{0.0, 0.0}, i{};
        NewtonStats stats;
        root.solve(Diodes<2>{}, vOc, Z, v, i, stats);
        REQUIRE(stats.failures == 0);
        for (int r = 0; r < 2; ++r) {
            const double f = v[r] + Z[r][0] * i[0] + Z[r][1] * i[1] - vOc[r];
            REQUIRE(std::fabs(f) < 1e-9);
        }
        // warm start at the solution: converged without an iteration
        NewtonStats warm;
        REQUIRE(root.solve(Diodes<2>{}, vOc, Z, v, i, warm) == 0);
        REQUIRE(warm.histogram[0] == 1);
    }
}

TEST_CASE("Newton step limit and failure handling", "[newton][m2]") {
    MultiportNewton<1> root;
    root.maxStep = {0.1};
    root.config.maxIter = 1;
    Vec<1> v{0.0}, i{};
    NewtonStats stats;
    root.solve(Diodes<1>{}, Vec<1>{-3.0}, Mat<1>{{{1e3}}}, v, i, stats);
    REQUIRE(std::fabs(v[0]) <= 0.1 + 1e-15);  // one limited step from 0
    REQUIRE(stats.failures == 1);
    REQUIRE(stats.solves == 1);
    REQUIRE(stats.maxIterations == 1);
    REQUIRE(std::isfinite(v[0]));
    REQUIRE(std::isfinite(i[0]));

    // a device that returns NaN: the best (initial) iterate is kept and the failure counted
    struct Broken {
        void operator()(const Vec<1>& v, Vec<1>& i, Mat<1>& d) const noexcept {
            i[0] = v[0] == 0.25 ? 0.0 : std::nan("");
            d[0][0] = 1.0;
        }
    };
    MultiportNewton<1> r2;
    r2.maxStep = {2.0};
    Vec<1> w{0.25}, j{};
    NewtonStats s2;
    r2.solve(Broken{}, Vec<1>{1.0}, Mat<1>{{{1e3}}}, w, j, s2);
    REQUIRE(w[0] == 0.25);
    REQUIRE(j[0] == 0.0);
    REQUIRE(s2.failures == 1);
}

TEST_CASE("Newton asymmetric step limits", "[newton][m3]") {
    // a single diode driven far negative: the step into cutoff may be large, a step up stays limited
    MultiportNewton<1> root;
    root.maxStep = {0.1};
    root.maxStepDown = {100.0};
    root.config.maxIter = 1;
    Vec<1> v{0.0}, i{};
    NewtonStats stats;
    root.solve(Diodes<1>{}, Vec<1>{-50.0}, Mat<1>{{{1e3}}}, v, i, stats);
    REQUIRE(v[0] < -10.0);  // one step, not limited to 0.1 V downwards
    REQUIRE(root.limitDown(0) == 100.0);
    Vec<1> w{-1.0}, j{};
    root.solve(Diodes<1>{}, Vec<1>{5.0}, Mat<1>{{{1e3}}}, w, j, stats);
    REQUIRE(w[0] <= -1.0 + 0.1 + 1e-15);  // upward step limited
    root.maxStepDown = {0.0};
    REQUIRE(root.limitDown(0) == 0.1);  // 0 = symmetric
}

TEST_CASE("Newton statistics median", "[newton][m2]") {
    NewtonStats s;
    REQUIRE(s.medianIterations() == 0);
    for (int it : {2, 3, 3, 1, 7})
        s.record(it, true);
    REQUIRE(s.medianIterations() == 3);
    s.record(12, false);
    REQUIRE(s.medianIterations() == 3);  // lower median of 1 2 3 3 7 12
    REQUIRE(s.failures == 1);
    REQUIRE(s.maxIterations == 12);
}

TEST_CASE("Kept LU factors solve a new right-hand side like a fresh elimination", "[newton][m7]") {
    // a fixed, well-conditioned 6×6 matrix that needs row swaps (small leading entries)
    Mat<6> A{};
    for (int r = 0; r < 6; ++r)
        for (int c = 0; c < 6; ++c)
            A[r][c] = std::sin(1.0 + 3.0 * r + 7.0 * c) + (r == (c + 2) % 6 ? 4.0 : 0.0);
    amp2203::detail::LuFactors<6> lu;
    lu.m = A;
    Vec<6> r1{1.0, -2.0, 0.5, 3.0, -1.0, 2.0};
    REQUIRE(amp2203::solver::solveLinear<6>(lu.m, r1, lu.piv));
    lu.valid = true;
    const Vec<6> b{0.3, 1.7, -2.2, 0.9, 4.1, -0.6};
    Vec<6> x = b;
    amp2203::solver::solveFactored<6>(lu, x);
    Mat<6> J = A;
    Vec<6> y = b;
    REQUIRE(amp2203::solver::solveLinear<6>(J, y));
    for (int k = 0; k < 6; ++k)
        CHECK(std::fabs(x[k] - y[k]) <= 1e-13 * (1.0 + std::fabs(y[k])));
    // and A·x = b
    for (int r = 0; r < 6; ++r) {
        double s = 0.0;
        for (int c = 0; c < 6; ++c)
            s += A[r][c] * x[c];
        CHECK(std::fabs(s - b[r]) <= 1e-12);
    }
}
