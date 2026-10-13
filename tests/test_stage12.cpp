// B12 StageV1aV1b: DC start, silence, Preamp Volume handling, robustness. Accuracy against SPICE is
// checked by the harness (python -m harness.compare --block b12 --milestone M2).
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <numbers>
#include <string>

#include "amp2203/StageV1aV1b.h"

using amp2203::StageV1aV1b;

namespace {
constexpr double kFs = 768000.0;  // verification rate (D-035)

double probe(const StageV1aV1b& s, const std::string& name) {
    for (const auto& p : s.probes())
        if (name == p.name)
            return *p.value;
    FAIL("no probe " << name);
    return 0.0;
}

double sine(long n, double f, double amp) { return amp * std::sin(2.0 * std::numbers::pi * f * n / kFs); }
}  // namespace

TEST_CASE("StageV1aV1b starts at the DC operating point", "[stage12][m2]") {
    StageV1aV1b s;
    s.prepare(kFs, 20.0);
    s.reset(0.0);
    // CIRCUIT.md §5 (F operating point); the harness checks every node against the testbench op
    REQUIRE(std::fabs(probe(s, "p1a") / 296.402 - 1.0) < 5e-3);
    REQUIRE(std::fabs(probe(s, "k1a") / 2.510 - 1.0) < 5e-3);
    REQUIRE(std::fabs(probe(s, "p1b") / 352.433 - 1.0) < 5e-3);

    const double p1b = probe(s, "p1b");
    for (int n = 0; n < 4096; ++n) {
        const double y = s.process(0.0);
        REQUIRE(std::fabs(y - p1b) < 1e-9);  // silence in, silence out (SPEC §3.6: < 1e-6 V)
    }
    REQUIRE(s.newtonStats().failures == 0);
    REQUIRE(s.newtonStats().maxIterations == 0);  // the warm start is the solution
    REQUIRE(std::fabs(s.supplyCurrent() - ((389.349603 - probe(s, "p1a")) / 100e3 +
                                           (389.349603 - p1b) / 100e3)) < 1e-15);
}

TEST_CASE("StageV1aV1b Preamp Volume jump stays finite and rate-limits matrix updates", "[stage12][m2]") {
    StageV1aV1b s;
    s.prepare(kFs, 20.0);
    amp2203::Params p;
    p.preampVolume = 0.0;
    s.setParams(p);
    s.reset(0.0);
    const long u0 = s.matrixUpdates();
    const long n = static_cast<long>(0.2 * kFs);
    const long rampStart = static_cast<long>(0.02 * kFs), rampLen = static_cast<long>(0.01 * kFs);
    for (long k = 0; k < n; ++k) {
        if (k >= rampStart && k <= rampStart + rampLen) {
            p.preampVolume = static_cast<double>(k - rampStart) / static_cast<double>(rampLen);
            s.setParams(p);
        }
        const double y = s.process(sine(k, 1000.0, 0.5));
        REQUIRE(std::isfinite(y));
    }
    REQUIRE(s.newtonStats().failures == 0);
    REQUIRE(s.sanitizeEvents() == 0);
    const long updates = s.matrixUpdates() - u0;
    REQUIRE(updates > 0);
    REQUIRE(updates <= n / StageV1aV1b::kParamUpdateInterval + 2);
}

TEST_CASE("StageV1aV1b stays finite far beyond the signal range", "[stage12][m2]") {
    StageV1aV1b s;
    s.prepare(kFs, 20.0);
    s.reset(0.0);
    for (long k = 0; k < static_cast<long>(0.05 * kFs); ++k) {
        const double y = s.process(sine(k, 220.0, 50.0));  // 25× the overdrive level
        REQUIRE(std::isfinite(y));
    }
    REQUIRE(s.sanitizeEvents() == 0);
}
