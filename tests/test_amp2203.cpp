// Amp2203 full chain (SPEC §4.3): DC start against the full-netlist operating point (CIRCUIT.md §5,
// VERIFICATION §7: 0.5 % per node), silence, probe names, robustness. Accuracy against SPICE over the
// signal set is checked by the harness (M4 acceptance).
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <cstring>
#include <numbers>
#include <set>
#include <string>
#include <vector>

#include "amp2203/Amp2203.h"

namespace {
double probe(const amp2203::Amp2203& a, const std::string& name) {
    for (const auto& p : a.probes())
        if (name == p.name)
            return *p.value;
    FAIL("no probe " << name);
    return 0.0;
}
}  // namespace

TEST_CASE("Amp2203 starts at the full-netlist DC operating point", "[amp2203][m4]") {
    amp2203::Amp2203 amp;
    amp.prepare({});
    // full-netlist op at default parameters (ngspice 47, EL34 D-058)
    const std::pair<const char*, double> ref[] = {
        {"ct", 491.3466370439178}, {"nscr", 488.8804667528911}, {"bpx", 404.9981846270138},
        {"bpa", 391.9215063373393}, {"p1a", 298.3259948720179},             {"p1b", 354.7502349059159},
        {"p2a", 232.5771020001565},           {"k2b", 233.1404038063239},             {"p3a", 272.6112625407083},
        {"p3b", 263.9511987003812}, {"k3", 45.25019514420279},    {"pa", 490.4627514780715},
        {"pb", 490.3937077859763},  {"nbias", -46.36358785048281}};
    for (const auto& [n, v] : ref) {
        INFO(n << ": " << probe(amp, n) << " (SPICE " << v << ")");
        REQUIRE(std::fabs(probe(amp, n) / v - 1.0) < 5e-3);
    }
    REQUIRE(std::fabs(probe(amp, "ip_tube_a") - 0.0287723165965592) < 1e-3);  // ±1 mA per tube
    REQUIRE(std::fabs(probe(amp, "is_tube_a") - 0.0032641827099544754) < 0.3e-3);

    // silence in, silence out (SPEC §3.6: within 1 µV of the DC output; F's speaker node carries 42.4 µV
    // of NFB DC current through R22, and so does the engine)
    const double outDc = probe(amp, "out");
    REQUIRE(std::fabs(outDc - 4.240884065521802e-05) < 1e-7);
    std::vector<double> x(8192, 0.0);
    amp.process(x.data(), static_cast<int>(x.size()));
    for (double y : x)
        REQUIRE(std::fabs(y * amp2203::EngineConfig{}.outputVoltsPerFullScale - outDc) < 1e-6);
    REQUIRE(amp.getStats().newtonFailures == 0);
}

TEST_CASE("Amp2203 probe names are unique", "[amp2203][m4]") {
    amp2203::Amp2203 amp;
    std::set<std::string> names;
    for (const auto& p : amp.probes())
        REQUIRE(names.insert(p.name).second);
    for (const char* n : {"in", "out", "y", "y_oc", "p1b", "p3a", "pa", "ct", "bpa"})
        REQUIRE(names.count(n) == 1);
}

TEST_CASE("Amp2203 overdrive stays finite", "[amp2203][m4]") {
    amp2203::Amp2203 amp;
    amp2203::EngineConfig cfg;
    amp.prepare(cfg);
    amp2203::Params p;
    p.preampVolume = 1.0;
    p.master = 1.0;
    p.presence = 1.0;
    amp.setParams(p);
    amp.reset();
    std::vector<double> x(static_cast<size_t>(0.03 * cfg.sampleRate));
    for (size_t n = 0; n < x.size(); ++n)
        x[n] = 0.5 * std::sin(2.0 * std::numbers::pi * 220.0 * static_cast<double>(n) / cfg.sampleRate);
    amp.process(x.data(), static_cast<int>(x.size()));
    double peak = 0.0;
    for (double y : x) {
        REQUIRE(std::isfinite(y));
        peak = std::max(peak, std::fabs(y));
    }
    INFO("peak " << peak * cfg.outputVoltsPerFullScale << " V, Newton failures " << amp.getStats().newtonFailures);
    REQUIRE(peak > 0.1);
}
