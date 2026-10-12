#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <vector>

#include "amp2203/Amp2203.h"

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

TEST_CASE("M0 pass-through: silence in gives silence out", "[amp2203][m0]") {
    amp2203::Amp2203 amp;
    amp.prepare({});
    std::vector<double> x(1024, 0.0);
    amp.process(x.data(), static_cast<int>(x.size()));
    for (double y : x)
        REQUIRE(y == 0.0);
}

TEST_CASE("M0 pass-through is the identity in volts", "[amp2203][m0]") {
    amp2203::EngineConfig cfg;
    amp2203::Amp2203 amp;
    amp.prepare(cfg);

    double x[] = {0.5, -0.25, 1.0};
    amp.process(x, 3);
    REQUIRE_THAT(x[0], WithinRel(0.5 * cfg.inputVoltsPerFullScale / cfg.outputVoltsPerFullScale, 1e-15));

    // after the last sample, probes report the jack and speaker voltages
    for (const auto& p : amp.probes())
        REQUIRE_THAT(*p.value, WithinAbs(cfg.inputVoltsPerFullScale, 1e-12));
}
