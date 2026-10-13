// B1234 Preamp (B12 + B3 + B4 in one tree, 8-port root, D-038): DC start and KCL, silence, knob jumps,
// R_th4, supply inputs, robustness. Accuracy against SPICE: python -m harness.cascade / compare --block b1234.
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <numbers>
#include <string>

#include "amp2203/Preamp.h"
#include "amp2203/StageV1aV1b.h"
#include "amp2203/ToneStackMaster.h"

using amp2203::Preamp;

namespace {
constexpr double kFs = 768000.0;  // verification rate (D-035)

double probe(const Preamp& s, const std::string& name) {
    for (const auto& p : s.probes())
        if (name == p.name)
            return *p.value;
    FAIL("no probe " << name);
    return 0.0;
}

double sine(long n, double f, double amp) { return amp * std::sin(2.0 * std::numbers::pi * f * n / kFs); }
}  // namespace

TEST_CASE("Preamp starts at the DC operating point", "[preamp][m3]") {
    Preamp s;
    s.prepare(kFs, 20.0);
    s.reset(0.0);
    // CIRCUIT.md §5 (F operating point); the harness checks every node against the testbench op
    for (const auto& [node, v] : {std::pair{"p1a", 296.402}, {"k1a", 2.510}, {"p1b", 352.433}, {"p2a", 231.130},
                                  {"k2a", 1.404}, {"k2b", 231.686}}) {
        INFO(node);
        REQUIRE(std::fabs(probe(s, node) / v - 1.0) < 5e-3);
    }
    // the same DC as the separately verified B12 (J2 carries no DC current)
    amp2203::StageV1aV1b b12;
    b12.prepare(kFs, 20.0);
    b12.reset(0.0);
    REQUIRE(std::fabs(probe(s, "p1b") - b12.output()) < 1e-6);

    for (int n = 0; n < 4096; ++n)
        REQUIRE(std::fabs(s.process(0.0)) < 1e-9);  // silence in, silence out (SPEC §3.6)
    REQUIRE(s.newtonStats().failures == 0);
    REQUIRE(s.newtonStats().maxIterations == 0);

    // KCL at the triode nodes with both supplies (V2b's plate port ends on bpX)
    const double p2a = probe(s, "p2a"), k2b = probe(s, "k2b"), p1b = probe(s, "p1b");
    REQUIRE(std::fabs((Preamp::kSupplyBpX - p2a) / Preamp::R12 - (probe(s, "ip2a") + probe(s, "ig2b"))) < 1e-12);
    REQUIRE(std::fabs(k2b / Preamp::R13 - (probe(s, "ip2b") + probe(s, "ig2b"))) < 1e-12);
    REQUIRE(std::fabs((Preamp::kSupplyBpA - p1b) / Preamp::R7 - probe(s, "ip1b")) < 1e-12);
    REQUIRE(std::fabs(s.supplyCurrentX() - ((Preamp::kSupplyBpX - p2a) / Preamp::R12 + probe(s, "ip2b"))) < 1e-15);
}

TEST_CASE("Preamp R_th4 matches ToneStackMaster", "[preamp][m3]") {
    for (double v : {0.0, 0.3, 1.0}) {
        amp2203::Params p;
        p.master = v;
        p.treble = 0.8;
        p.bass = 0.2;
        Preamp s;
        amp2203::ToneStackMaster t;
        s.setParams(p);
        t.setParams(p);
        s.reset(0.0);
        t.reset(231.6858);
        REQUIRE(s.rth4() == t.rth4());
    }
}

TEST_CASE("Preamp knob jumps stay finite and rate-limit matrix updates", "[preamp][m3]") {
    Preamp s;
    s.prepare(kFs, 20.0);
    amp2203::Params p;
    p.preampVolume = p.treble = p.middle = p.bass = p.master = 0.0;
    s.setParams(p);
    s.reset(0.0);
    const long u0 = s.matrixUpdates();
    const long n = static_cast<long>(0.2 * kFs);
    const long rampStart = static_cast<long>(0.02 * kFs), rampLen = static_cast<long>(0.01 * kFs);
    for (long k = 0; k < n; ++k) {
        if (k >= rampStart && k <= rampStart + rampLen) {
            const double u = static_cast<double>(k - rampStart) / static_cast<double>(rampLen);
            p.preampVolume = p.treble = p.middle = p.bass = p.master = u;
            s.setParams(p);
        }
        REQUIRE(std::isfinite(s.process(sine(k, 1000.0, 0.5))));
    }
    REQUIRE(s.newtonStats().failures == 0);
    REQUIRE(s.sanitizeEvents() == 0);
    const long updates = s.matrixUpdates() - u0;
    REQUIRE(updates > 0);
    REQUIRE(updates <= n / Preamp::kParamUpdateInterval + 2);
}

TEST_CASE("Preamp follows its supply inputs and stays finite far beyond the signal range", "[preamp][m3]") {
    Preamp s;
    s.prepare(kFs, 20.0);
    s.setSupplyVoltages(380.0, 395.0);
    s.reset(0.0);
    REQUIRE(probe(s, "bpa") == 380.0);
    REQUIRE(probe(s, "p1b") < 352.433);  // lower supply, lower plate voltage
    for (long k = 0; k < static_cast<long>(0.05 * kFs); ++k)
        REQUIRE(std::isfinite(s.process(sine(k, 220.0, 20.0))));  // 10× the overdrive level
    REQUIRE(s.sanitizeEvents() == 0);
}
