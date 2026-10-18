// B34 StageV2aV2bToneStack: DC start, silence, tone-knob jumps, R_th4, KCL with the bpX supply offsets.
// Accuracy against SPICE: python -m harness.compare --block b34.
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <numbers>
#include <string>

#include "amp2203/StageV2aV2bToneStack.h"
#include "amp2203/ToneStackMaster.h"

using amp2203::StageV2aV2bToneStack;

namespace {
constexpr double kFs = 768000.0;  // verification rate (D-035)
constexpr double kVp1b = 352.432789;  // CIRCUIT.md §5 (J2 source DC level)

template <class Block>
double probe(const Block& s, const std::string& name) {
    for (const auto& p : s.probes())
        if (name == p.name)
            return *p.value;
    FAIL("no probe " << name);
    return 0.0;
}

double sine(long n, double f, double amp) { return amp * std::sin(2.0 * std::numbers::pi * f * n / kFs); }
}  // namespace

TEST_CASE("StageV2aV2bToneStack starts at the DC operating point", "[stage34][m3]") {
    StageV2aV2bToneStack s;
    s.prepare(kFs, 20.0);
    s.reset(kVp1b);
    REQUIRE(std::fabs(probe(s, "p2a") / 232.577102 - 1.0) < 5e-3);  // CIRCUIT.md §5
    REQUIRE(std::fabs(probe(s, "k2b") / 233.140404 - 1.0) < 5e-3);
    REQUIRE(std::fabs(probe(s, "nd") / 233.140404 - 1.0) < 5e-3);   // R15 carries no DC current
    REQUIRE(std::fabs(probe(s, "y")) < 1e-9);                    // the tone stack is AC-coupled
    for (int n = 0; n < 4096; ++n)
        REQUIRE(std::fabs(s.process(kVp1b)) < 1e-9);  // silence in, silence out (SPEC §3.6)
    REQUIRE(s.newtonStats().failures == 0);
    REQUIRE(s.newtonStats().maxIterations == 0);

    // KCL at the triode nodes (the tone stack draws no DC current)
    const double p2a = probe(s, "p2a"), k2b = probe(s, "k2b");
    REQUIRE(std::fabs((StageV2aV2bToneStack::kSupplyBpX - p2a) / StageV2aV2bToneStack::R12 -
                      (probe(s, "ip2a") + probe(s, "ig2b"))) < 1e-12);
    REQUIRE(std::fabs(k2b / StageV2aV2bToneStack::R13 - (probe(s, "ip2b") + probe(s, "ig2b"))) < 1e-12);
}

TEST_CASE("StageV2aV2bToneStack R_th4 matches ToneStackMaster", "[stage34][m3]") {
    for (double v : {0.0, 0.3, 1.0}) {
        amp2203::Params p;
        p.master = v;
        p.treble = 0.8;
        p.bass = 0.2;
        StageV2aV2bToneStack s;
        amp2203::ToneStackMaster t;
        s.setParams(p);
        t.setParams(p);
        s.reset(kVp1b);
        t.reset(231.6858);
        REQUIRE(s.rth4() == t.rth4());
    }
}

TEST_CASE("StageV2aV2bToneStack tone-knob jumps stay finite and rate-limit matrix updates", "[stage34][m3]") {
    StageV2aV2bToneStack s;
    s.prepare(kFs, 20.0);
    amp2203::Params p;
    p.treble = p.middle = p.bass = p.master = 0.0;
    s.setParams(p);
    s.reset(kVp1b);
    const long u0 = s.matrixUpdates();
    const long n = static_cast<long>(0.2 * kFs);
    const long rampStart = static_cast<long>(0.02 * kFs), rampLen = static_cast<long>(0.01 * kFs);
    for (long k = 0; k < n; ++k) {
        if (k >= rampStart && k <= rampStart + rampLen) {
            p.treble = p.middle = p.bass = p.master = static_cast<double>(k - rampStart) / static_cast<double>(rampLen);
            s.setParams(p);
        }
        REQUIRE(std::isfinite(s.process(kVp1b + sine(k, 1000.0, 10.0))));
    }
    REQUIRE(s.newtonStats().failures == 0);
    REQUIRE(s.sanitizeEvents() == 0);
    const long updates = s.matrixUpdates() - u0;
    REQUIRE(updates > 0);
    REQUIRE(updates <= n / StageV2aV2bToneStack::kParamUpdateInterval + 2);
}

TEST_CASE("StageV2aV2bToneStack stays finite far beyond the signal range", "[stage34][m3]") {
    StageV2aV2bToneStack s;
    s.prepare(kFs, 20.0);
    s.reset(kVp1b);
    for (long k = 0; k < static_cast<long>(0.05 * kFs); ++k)
        REQUIRE(std::isfinite(s.process(kVp1b + sine(k, 220.0, 300.0))));
    REQUIRE(s.sanitizeEvents() == 0);
}
