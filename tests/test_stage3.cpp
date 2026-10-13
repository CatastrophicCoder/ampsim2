// B3 StageV2aV2b: DC start (with the bpX supply offsets checked by KCL), silence, robustness. Accuracy
// against SPICE is checked by the harness (python -m harness.compare --block b3 --milestone M3).
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <numbers>
#include <string>

#include "amp2203/StageV2aV2b.h"

using amp2203::StageV2aV2b;

namespace {
constexpr double kFs = 768000.0;  // verification rate (D-035)
constexpr double kVp1b = 352.432789;  // CIRCUIT.md §5 (J2 source DC level)

double probe(const StageV2aV2b& s, const std::string& name) {
    for (const auto& p : s.probes())
        if (name == p.name)
            return *p.value;
    FAIL("no probe " << name);
    return 0.0;
}

double sine(long n, double f, double amp) { return amp * std::sin(2.0 * std::numbers::pi * f * n / kFs); }
}  // namespace

TEST_CASE("StageV2aV2b starts at the DC operating point", "[stage3][m3]") {
    StageV2aV2b s;
    s.prepare(kFs);
    s.reset(kVp1b);
    // CIRCUIT.md §5 (F operating point); the harness checks every node against the testbench op
    REQUIRE(std::fabs(probe(s, "p2a") / 231.130 - 1.0) < 5e-3);
    REQUIRE(std::fabs(probe(s, "k2a") / 1.404 - 1.0) < 5e-3);
    REQUIRE(std::fabs(probe(s, "k2b") / 231.686 - 1.0) < 5e-3);

    const double k2b = probe(s, "k2b");
    for (int n = 0; n < 4096; ++n)
        REQUIRE(std::fabs(s.process(kVp1b) - k2b) < 1e-9);  // silence in, silence out (SPEC §3.6)
    REQUIRE(s.newtonStats().failures == 0);
    REQUIRE(s.newtonStats().maxIterations == 0);
}

TEST_CASE("StageV2aV2b DC satisfies KCL with the supply on V2b's plate", "[stage3][m3]") {
    // checks the exact supply offsets: V2b's plate port ends on bpX, which is not a network node
    for (double vbpX : {402.335999, 350.0, 450.0}) {
        StageV2aV2b s;
        s.prepare(kFs);
        s.setSupplyVoltage(vbpX);
        s.reset(kVp1b);
        const double p2a = probe(s, "p2a"), k2a = probe(s, "k2a"), k2b = probe(s, "k2b");
        const double ip2a = probe(s, "ip2a"), ig2a = probe(s, "ig2a"), ip2b = probe(s, "ip2b"),
                     ig2b = probe(s, "ig2b");
        INFO("bpX " << vbpX);
        REQUIRE(std::fabs((vbpX - p2a) / StageV2aV2b::R12 - (ip2a + ig2b)) < 1e-12);  // node p2a
        REQUIRE(std::fabs(k2a / StageV2aV2b::R9 - (ip2a + ig2a)) < 1e-12);           // node k2a
        REQUIRE(std::fabs(k2b / StageV2aV2b::R13 - (ip2b + ig2b)) < 1e-12);          // node k2b
        REQUIRE(std::fabs(s.supplyCurrent() - ((vbpX - p2a) / StageV2aV2b::R12 + ip2b)) < 1e-15);
        REQUIRE(ip2b > 1e-3);  // the cathode follower conducts (≈ 2.3 mA at the operating point)
    }
}

TEST_CASE("StageV2aV2b stays finite far beyond the signal range", "[stage3][m3]") {
    StageV2aV2b s;
    s.prepare(kFs);
    s.reset(kVp1b);
    for (long k = 0; k < static_cast<long>(0.05 * kFs); ++k)
        REQUIRE(std::isfinite(s.process(kVp1b + sine(k, 220.0, 300.0))));  // 6× the overdrive level
    REQUIRE(s.sanitizeEvents() == 0);
}
