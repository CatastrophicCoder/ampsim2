// B56 PowerAmp (phase inverter + power stage + OT in one tree, D-041): DC start against the full-netlist
// operating point (VERIFICATION §7), silence, robustness. Accuracy over the signal set: the harness.
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <numbers>
#include <string>

#include "amp2203/PowerAmp.h"

using amp2203::PowerAmp;

namespace {
constexpr double kFs = 768000.0;

double probe(const PowerAmp& s, const std::string& name) {
    for (const auto& p : s.probes())
        if (name == p.name)
            return *p.value;
    FAIL("no probe " << name);
    return 0.0;
}

void setup(PowerAmp& s) {
    s.prepare(kFs, 20.0);
    using amp2203::detail::atap;
    const double vr2b = 1e6 * atap(0.3) + 1.0, chain = (1e6 * (1.0 - atap(0.3)) + 1.0) + 110e3 + (1e6 * atap(0.5) + 1.0) + 22e3;
    s.setSourceResistance(1.0 / (1.0 / vr2b + 1.0 / chain));
    s.setSupplyVoltages(402.33599940894806, 485.64928691750646, 488.70842163204327);
    s.reset(0.0);
}
}  // namespace

TEST_CASE("PowerAmp starts at the full-netlist DC operating point", "[stage56][m4]") {
    PowerAmp s;
    setup(s);
    const std::pair<const char*, double> ref[] = {
        {"g3a", 43.52260078966601},  {"k3", 44.93336791716606},   {"ng", 13.483951021319733},
        {"p3a", 270.8809249953309},  {"p3b", 262.2705821268219},  {"ngn", -46.36312745114321},
        {"nbias", -46.36358785048281}, {"s4", 481.0819124250908}, {"pa", 487.6615211930245},
        {"pb", 487.5797411890199}};
    for (const auto& [n, v] : ref) {
        INFO(n << ": " << probe(s, n) << " (SPICE " << v << ")");
        REQUIRE(std::fabs(probe(s, n) / v - 1.0) < 5e-3);
    }
    // F's speaker node carries the NFB DC current through R22 (CIRCUIT.md §5: 42.3 µV)
    INFO("out " << s.output());
    REQUIRE(std::fabs(s.output() - 4.2302458725838655e-05) < 1e-7);
    REQUIRE(std::fabs(probe(s, "ip_tube_a") - 0.03407830267111851) < 1e-3);
    REQUIRE(std::fabs(probe(s, "is_tube_b") - 0.00456754684753321) < 0.3e-3);

    const double out = s.output();
    for (int n = 0; n < 8192; ++n)
        REQUIRE(std::fabs(s.process(0.0) - out) < 1e-6);  // SPEC §3.6
    REQUIRE(s.newtonStats().failures == 0);
}

TEST_CASE("PowerAmp overdrive and Presence jump stay finite", "[stage56][m4]") {
    PowerAmp s;
    setup(s);
    amp2203::Params p;
    double peak = 0.0;
    for (long n = 0; n < static_cast<long>(0.05 * kFs); ++n) {
        if (n == static_cast<long>(0.01 * kFs)) {
            p.presence = 1.0;
            s.setParams(p);
        }
        const double y = s.process(30.0 * std::sin(2.0 * std::numbers::pi * 220.0 * n / kFs));
        REQUIRE(std::isfinite(y));
        peak = std::max(peak, std::fabs(y));
    }
    INFO("peak " << peak << " V, Newton failures " << s.newtonStats().failures << " max it "
                 << s.newtonStats().maxIterations);
    REQUIRE(peak > 30.0);
    REQUIRE(s.sanitizeEvents() == 0);
}

TEST_CASE("PowerAmp adaptive half-steps keep DC and agree with the full step on small signals", "[stage56][m4]") {
    // every sample half-stepped: the DC point stays (state conversions T ↔ T/2 are exact at DC)
    PowerAmp a;
    setup(a);
    a.setHalfStepTrigger(1e-12);
    const double out = a.output();
    for (int n = 0; n < 4096; ++n)
        REQUIRE(std::fabs(a.process(0.0) - out) < 1e-6);
    REQUIRE(a.halfSteps() == 4096);

    // a trigger no small signal reaches: identical to no half-steps at all
    PowerAmp b, c;
    setup(b);
    setup(c);
    b.setHalfStepTrigger(0.0);
    c.setHalfStepTrigger(1e6);
    for (int n = 0; n < 4096; ++n) {
        const double x = 0.5 * std::sin(2.0 * std::numbers::pi * 1000.0 * n / kFs);
        REQUIRE(b.process(x) == c.process(x));
    }
    REQUIRE(c.halfSteps() == 0);

    // all half-steps on a 1 kHz tone: close to the full-step result (both approximate the same circuit)
    PowerAmp d, e;
    setup(d);
    setup(e);
    d.setHalfStepTrigger(1e-12);
    e.setHalfStepTrigger(0.0);
    double err = 0.0, ref = 0.0;
    for (int n = 0; n < static_cast<int>(0.01 * kFs); ++n) {
        const double x = 1.0 * std::sin(2.0 * std::numbers::pi * 1000.0 * n / kFs);
        const double yd = d.process(x), ye = e.process(x);
        err += (yd - ye) * (yd - ye);
        ref += (ye - out) * (ye - out);
    }
    INFO("half vs full step ESR " << 10.0 * std::log10(err / ref) << " dB");
    REQUIRE(10.0 * std::log10(err / ref) < -60.0);
}
