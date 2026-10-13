// B6 PowerStage: DC start against the SPICE operating point (VERIFICATION §7: 0.5 % per node, EL34
// currents ±1 mA plate / ±0.3 mA screen per tube), silence, centre-tap current bookkeeping, robustness.
// Accuracy against SPICE over the signal set is checked by the harness (compare --block b6).
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <numbers>
#include <string>

#include "amp2203/PowerStage.h"

using amp2203::PowerStage;

namespace {
constexpr double kFs = 768000.0;
constexpr double kP3a = 270.8809249953309, kP3b = 262.2705821268219;  // F op (ngspice 47)

double probe(const PowerStage& s, const std::string& name) {
    for (const auto& p : s.probes())
        if (name == p.name)
            return *p.value;
    FAIL("no probe " << name);
    return 0.0;
}
}  // namespace

TEST_CASE("PowerStage starts at the DC operating point", "[stage6][m4]") {
    PowerStage s;
    s.prepare(kFs);
    s.setSupplyVoltages(488.70842163204327, 485.64928691750646);
    s.reset(kP3a, kP3b);
    const std::pair<const char*, double> ref[] = {{"ngn", -46.36312745114321}, {"nor", -46.36312745114321},
                                                  {"nbias", -46.36358785048281}, {"g4", -46.36312159151525},
                                                  {"s4", 481.0819124250908},   {"pa", 487.6615211930245},
                                                  {"pb", 487.5797411890199},   {"s6", 481.08174006997325}};
    for (const auto& [n, v] : ref) {
        INFO(n << ": " << probe(s, n) << " (SPICE " << v << ")");
        REQUIRE(std::fabs(probe(s, n) / v - 1.0) < 5e-3);
    }
    INFO("ip tube A " << probe(s, "ip_tube_a") << " B " << probe(s, "ip_tube_b") << " is A " << probe(s, "is_tube_a"));
    REQUIRE(std::fabs(probe(s, "ip_tube_a") - 0.03407830267111851) < 1e-3);
    REQUIRE(std::fabs(probe(s, "ip_tube_b") - 0.03407802821202568) < 1e-3);
    REQUIRE(std::fabs(probe(s, "is_tube_a") - 0.0045673744924156524) < 0.3e-3);
    REQUIRE(std::fabs(probe(s, "is_tube_b") - 0.00456754684753321) < 0.3e-3);
    // the centre tap supplies both plates (KCL at DC)
    REQUIRE(std::fabs(s.centreTapCurrent() - probe(s, "ip_a") - probe(s, "ip_b")) < 1e-9);
    REQUIRE(std::fabs(s.screenCurrent() - probe(s, "is_a") - probe(s, "is_b")) < 1e-12);

    const double out = s.output();
    for (int n = 0; n < 8192; ++n)
        REQUIRE(std::fabs(s.process(kP3a, kP3b) - out) < 1e-9);  // silence in, silence out (SPEC §3.6)
    REQUIRE(std::fabs(probe(s, "pa") - 487.6615211930245) < 5e-3 * 487.66);
    REQUIRE(s.newtonStats().failures == 0);
}

TEST_CASE("PowerStage overdrive stays finite", "[stage6][m4]") {
    PowerStage s;
    s.prepare(kFs);
    s.reset(kP3a, kP3b);
    double peak = 0.0;
    for (long n = 0; n < static_cast<long>(0.05 * kFs); ++n) {
        const double x = 150.0 * std::sin(2.0 * std::numbers::pi * 220.0 * n / kFs);  // PI plates far beyond clipping
        const double y = s.process(kP3a + x, kP3b - x);
        REQUIRE(std::isfinite(y));
        peak = std::max(peak, std::fabs(y));
    }
    INFO("peak speaker voltage " << peak << " V, Newton failures " << s.newtonStats().failures << ", max iterations "
                                 << s.newtonStats().maxIterations);
    REQUIRE(peak > 30.0);
    REQUIRE(s.sanitizeEvents() == 0);
}
