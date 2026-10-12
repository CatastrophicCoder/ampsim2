// B4 ToneStackMaster: element equivalence, DC start, knob handling, real-time safety of knob jumps.
// Accuracy against SPICE is checked by the harness (python -m harness.tonestack_check).
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <chowdsp_wdf/chowdsp_wdf.h>

#include <cmath>
#include <numbers>

#include "amp2203/ToneStackMaster.h"

namespace wdft = chowdsp::wdft;
using amp2203::ToneStackMaster;
using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

namespace {
constexpr double kFs = 384000.0;
constexpr double kVk2b = 231.6858;  // CIRCUIT.md §5 k2b (J3 source DC level)

double sine(int n, double f, double amp) { return amp * std::sin(2.0 * std::numbers::pi * f * n / kFs); }
}  // namespace

TEST_CASE("DcCapacitor behaves like the library's CapacitorT", "[tonestack][m1]") {
    // RC low-pass driven by a resistive source, once with each capacitor type
    wdft::ResistiveVoltageSourceT<double> s1{1e3}, s2{1e3};
    wdft::CapacitorT<double> c1{1e-6, kFs};
    amp2203::detail::DcCapacitor c2{1e-6, kFs};
    wdft::WDFParallelT<double, decltype(s1), decltype(c1)> p1{s1, c1};
    wdft::WDFParallelT<double, decltype(s2), decltype(c2)> p2{s2, c2};
    wdft::ResistorT<double> l1{10e3}, l2{10e3};
    wdft::WDFSeriesT<double, decltype(p1), decltype(l1)> root1{p1, l1};
    wdft::WDFSeriesT<double, decltype(p2), decltype(l2)> root2{p2, l2};
    c2.prepare(kFs);
    for (int n = 0; n < 2000; ++n) {
        s1.setVoltage(sine(n, 3000.0, 1.0));
        s2.setVoltage(sine(n, 3000.0, 1.0));
        // terminate both trees in a matched reflection-free load (b = 0 at the root)
        root1.reflected();
        root1.incident(0.0);
        root2.reflected();
        root2.incident(0.0);
        REQUIRE(wdft::voltage<double>(c2) == wdft::voltage<double>(c1));
    }
}

TEST_CASE("ToneStackMaster starts at the DC operating point", "[tonestack][m1]") {
    ToneStackMaster ts;
    ts.prepare(kFs, 20.0);
    ts.reset(kVk2b);
    for (int n = 0; n < 4096; ++n) {
        const double y = ts.process(kVk2b);
        REQUIRE(std::fabs(y) < 1e-9);  // silence in, silence out (SPEC §3.6: < 1e-6 V)
    }
    for (const auto& p : ts.probes()) {
        const std::string name = p.name;
        if (name == "in" || name == "k2b" || name == "nd")
            REQUIRE_THAT(*p.value, WithinRel(kVk2b, 1e-12));
        else if (name != "rth4")
            REQUIRE(std::fabs(*p.value) < 1e-9);
    }
}

TEST_CASE("ToneStackMaster: R_th4 follows the D-015 formula", "[tonestack][m1]") {
    using amp2203::detail::atap;
    using amp2203::detail::ltap;
    ToneStackMaster ts;
    ts.prepare(kFs, 20.0);
    amp2203::Params p;
    p.treble = 0.8;
    p.bass = 0.3;
    p.master = 0.6;
    ts.setParams(p);
    ts.reset(0.0);
    const double r2a = 1e6 * (1 - atap(0.6)) + 1, r2b = 1e6 * atap(0.6) + 1;
    const double chain = r2a + 220e3 * ltap(0.8) + (1e6 * atap(0.3) + 1) + 22e3;
    REQUIRE_THAT(ts.rth4(), WithinRel(1.0 / (1.0 / r2b + 1.0 / chain), 1e-12));
}

TEST_CASE("ToneStackMaster: knob jumps stay finite and settle on the targets", "[tonestack][m1]") {
    ToneStackMaster ts;
    ts.prepare(kFs, 20.0);
    amp2203::Params p;
    p.treble = p.middle = p.bass = p.master = 0.0;
    ts.setParams(p);
    ts.reset(kVk2b);
    // every knob 0 → 1 in 10 ms (SPEC §6.2) during a 1 kHz tone, then 0.5 s to settle (≈ 16 τ at 20 ms)
    const int ramp = static_cast<int>(0.010 * kFs);
    const long before = ts.matrixUpdates();
    for (int n = 0; n < ramp + static_cast<int>(0.5 * kFs); ++n) {
        const double r = std::min(1.0, static_cast<double>(n) / ramp);
        p.treble = p.middle = p.bass = p.master = r;
        ts.setParams(p);
        const double y = ts.process(kVk2b + sine(n, 1000.0, 1.0));
        REQUIRE(std::isfinite(y));
    }
    const long updates = ts.matrixUpdates() - before;
    // rate limited: at most one update per kParamUpdateInterval samples while moving (+1 final)
    REQUIRE(updates <= (ramp + static_cast<long>(0.5 * kFs)) / ToneStackMaster::kParamUpdateInterval + 1);
    REQUIRE(updates > 10);
    // settled: further samples do no matrix work
    const long settled = ts.matrixUpdates();
    for (int n = 0; n < 1000; ++n)
        ts.process(kVk2b);
    REQUIRE(ts.matrixUpdates() == settled);
}
