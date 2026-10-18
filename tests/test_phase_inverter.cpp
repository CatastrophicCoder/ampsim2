// B5 PhaseInverter: DC start against the SPICE operating point, silence, KCL at the tail, Presence
// knob jump, robustness. Accuracy against SPICE is checked by the harness (compare --block b5open).
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <numbers>
#include <string>

#include "amp2203/PhaseInverter.h"

using amp2203::PhaseInverter;

namespace {
constexpr double kFs = 768000.0;
constexpr double kOutDc = 4.2302458725838655e-05;  // F op v(out): R22 carries the NFB DC current
// R_th4 at the default knobs (D-015): VR2b ‖ (VR2a + VR3b + VR5 + VR4), master 0.3, treble / bass 0.5
const double kRth4Default = [] {
    using amp2203::detail::atap;
    const double vr2b = 1e6 * atap(0.3) + 1.0, chain = (1e6 * (1.0 - atap(0.3)) + 1.0) + 110e3 + (1e6 * atap(0.5) + 1.0) + 22e3;
    return 1.0 / (1.0 / vr2b + 1.0 / chain);
}();

double probe(const PhaseInverter& s, const std::string& name) {
    for (const auto& p : s.probes())
        if (name == p.name)
            return *p.value;
    FAIL("no probe " << name);
    return 0.0;
}

void setup(PhaseInverter& s) {
    s.prepare(kFs, 20.0);
    s.setSourceResistance(kRth4Default);
    s.setFeedbackVoltage(kOutDc);
    s.reset(0.0);  // v(y) = 0 at DC (C9 blocks)
}
}  // namespace

TEST_CASE("PhaseInverter starts at the DC operating point", "[stage5][m4]") {
    PhaseInverter s;
    setup(s);
    // full-netlist op (ngspice 47), DC tolerance 0.5 % (VERIFICATION §7); the engine is ≈ 1 mV off at
    // the tail because tubes.lib's 1 GΩ plate–cathode resistors are omitted (WDF_DESIGN §3.1)
    const std::pair<const char*, double> ref[] = {{"g3a", 43.8294735664613}, {"nf", 43.82847215398154},
                                                  {"k3", 45.25019513654581},  {"g3b", 43.82947356646135},
                                                  {"ng", 13.57902696479027}, {"p3a", 272.6112625373205},
                                                  {"p3b", 263.9511986968434}};
    for (const auto& [n, v] : ref) {
        INFO(n << ": " << probe(s, n) << " (SPICE " << v << ")");
        REQUIRE(std::fabs(probe(s, n) / v - 1.0) < 5e-3);
    }
    // KCL at the tail node nF: R16 from k3 = R17 + R19 + R20 out of nF (grid currents ≈ 0)
    const double nf = probe(s, "nf"), k3 = probe(s, "k3"), ng = probe(s, "ng");
    const double iTail = (k3 - nf) / PhaseInverter::R16;
    const double iOut = (nf - probe(s, "g3a")) / PhaseInverter::R17 + (nf - probe(s, "g3b")) / PhaseInverter::R19 +
                        (nf - ng) / PhaseInverter::R20;
    REQUIRE(std::fabs(iTail - iOut) < 1e-9);
    REQUIRE(std::fabs(iTail - (probe(s, "ip3a") + probe(s, "ip3b") + probe(s, "ig3a") + probe(s, "ig3b"))) < 1e-9);

    const double a = s.outputA(), b = s.outputB();
    for (int n = 0; n < 4096; ++n) {
        REQUIRE(std::fabs(s.process(0.0) - a) < 1e-9);
        REQUIRE(std::fabs(s.outputB() - b) < 1e-9);
    }
    REQUIRE(s.newtonStats().failures == 0);
}

TEST_CASE("PhaseInverter Presence jump and overdrive stay finite", "[stage5][m4]") {
    PhaseInverter s;
    setup(s);
    amp2203::Params p;
    for (long n = 0; n < static_cast<long>(0.05 * kFs); ++n) {
        if (n == static_cast<long>(0.01 * kFs)) {
            p.presence = 1.0;
            s.setParams(p);
        }
        const double x = 60.0 * std::sin(2.0 * std::numbers::pi * 1000.0 * n / kFs);  // far beyond 20 V
        const double y = s.process(x);
        REQUIRE(std::isfinite(y));
        REQUIRE(std::isfinite(s.outputB()));
    }
    REQUIRE(s.matrixUpdates() > 2);
    REQUIRE(s.sanitizeEvents() == 0);
}
