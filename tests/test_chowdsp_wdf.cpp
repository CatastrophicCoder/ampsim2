// Confirms the pinned chowdsp_wdf builds under our warning flags and that its wdft API behaves as used
// in docs/WDF_DESIGN.md §1 (checked against the library's README example at the pinned commit).
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <chowdsp_wdf/chowdsp_wdf.h>

namespace wdft = chowdsp::wdft;
using Catch::Matchers::WithinRel;

TEST_CASE("chowdsp_wdf: resistive divider", "[chowdsp_wdf][m0]") {
    constexpr double R1 = 10.0e3;
    constexpr double R2 = 30.0e3;
    constexpr double Vs = 2.0;

    wdft::ResistorT<double> r1{R1};
    wdft::ResistorT<double> r2{R2};
    wdft::WDFSeriesT<double, decltype(r1), decltype(r2)> s1{r1, r2};
    wdft::PolarityInverterT<double, decltype(s1)> i1{s1};
    wdft::IdealVoltageSourceT<double, decltype(i1)> vs{i1};

    vs.setVoltage(Vs);
    vs.incident(i1.reflected());
    i1.incident(vs.reflected());

    REQUIRE_THAT(wdft::voltage<double>(r2), WithinRel(Vs * R2 / (R1 + R2), 1e-12));
    REQUIRE_THAT(wdft::current<double>(r2), WithinRel(Vs / (R1 + R2), 1e-12));
}
