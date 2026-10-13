// B7 Supply: DC operating point vs SPICE (full netlist op, CIRCUIT.md §5) for the F load currents, and the
// discrete system settling to the new DC point after a load step.
#include <catch2/catch_test_macros.hpp>

#include <cmath>

#include "amp2203/Supply.h"

using amp2203::Supply;

namespace {
// full-netlist op at default parameters (ngspice 47): currents drawn from ct (EL34 plates), nScr
// (screens), bpX (V2a, V2b, PI) and bpA (V1a, V1b), and the supply node voltages
constexpr Supply::Currents kLoad{0.13631461224881197, 0.018269842679899527, 0.007032689141320076,
                                 0.0012986396095357635};
constexpr double kCt = 488.70842163204327, kScr = 485.64928691750646, kBpX = 402.33599940894806,
                 kBpA = 389.3496033135904;
}  // namespace

TEST_CASE("Supply DC matches the SPICE operating point", "[supply][m4]") {
    Supply s;
    s.prepare(768000.0);
    s.reset(kLoad);
    REQUIRE(std::fabs(s.ct() - kCt) < 1e-6);
    REQUIRE(std::fabs(s.screen() - kScr) < 1e-6);
    REQUIRE(std::fabs(s.bpX() - kBpX) < 1e-6);
    REQUIRE(std::fabs(s.bpA() - kBpA) < 1e-6);
    REQUIRE(std::fabs(s.chokeCurrent() - 0.02660117143075537) < 1e-9);
    // constant load: the discrete system stays at the DC point
    for (int n = 0; n < 1000; ++n)
        s.process(kLoad);
    REQUIRE(std::fabs(s.ct() - kCt) < 1e-9);
    REQUIRE(std::fabs(s.bpA() - kBpA) < 1e-9);
}

TEST_CASE("Supply settles to the new DC point after a load step", "[supply][m4]") {
    Supply s;
    s.prepare(768000.0);
    s.reset(kLoad);
    Supply::Currents more = kLoad;
    more[Supply::iCt] += 0.1;  // 100 mA more plate current
    Supply ref;
    ref.prepare(768000.0);
    ref.reset(more);
    double minCt = s.ct();
    for (int n = 0; n < static_cast<int>(20.0 * 768000.0); ++n) {  // slowest mode τ = 1.33 s (bpX / bpA chain)
        s.process(more);
        minCt = std::min(minCt, s.ct());
        REQUIRE(std::isfinite(s.ct()));
    }
    REQUIRE(std::fabs(s.ct() - ref.ct()) < 1e-3);
    REQUIRE(std::fabs(s.bpA() - ref.bpA()) < 1e-3);
    REQUIRE(ref.ct() < kCt - 9.0);  // ≈ 100 mA · 100 Ω sag
    REQUIRE(minCt >= ref.ct() - 5.0);  // the LC (choke, C22) rings, but the centre tap stays near its sag
}
