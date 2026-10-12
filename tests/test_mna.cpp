// MNA scattering builder (engine/src/mna) against the exact-arithmetic Python reference (harness/mna.py
// fixtures) and the WDF_DESIGN §5.5 sanity checks.
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <chowdsp_wdf/chowdsp_wdf.h>

#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "mna/PortNetwork.h"
#include "tonestack/ToneStackNetwork.h"

namespace wdft = chowdsp::wdft;
namespace mna = amp2203::mna;
using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

namespace {

struct Fixture {
    std::string name;
    std::vector<double> R;
    std::vector<std::vector<double>> S;
};

std::vector<Fixture> readFixtures() {
    std::ifstream in(std::string(AMP2203_TEST_DATA_DIR) + "/rtype_fixtures.txt");
    REQUIRE(in.good());
    std::vector<Fixture> out;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#')
            continue;
        std::istringstream h(line);
        std::string tag, name;
        int ports = 0, up = 0;
        h >> tag >> name >> ports >> up;
        REQUIRE(tag == "case");
        Fixture f{name, std::vector<double>(ports), std::vector<std::vector<double>>(ports, std::vector<double>(ports))};
        std::getline(in, line);
        std::istringstream r(line);
        r >> tag;
        for (auto& x : f.R)
            r >> x;
        for (auto& row : f.S) {
            std::getline(in, line);
            std::istringstream s(line);
            for (auto& x : row)
                s >> x;
        }
        out.push_back(f);
    }
    return out;
}

// The bridge of harness/mna.py random_network(): nodes a..e
constexpr mna::PortNetwork<9, 5> kBridge{{{{0, mna::kGround}, {0, 1}, {0, 2}, {1, 2}, {1, 3}, {2, 4}, {3, 4},
                                           {3, mna::kGround}, {4, mna::kGround}}}};

template <int P, int N>
void checkAgainstFixture(const mna::PortNetwork<P, N>& net, const Fixture& f) {
    std::array<double, P> R{};
    for (int k = 0; k < P; ++k)
        R[k] = f.R[k];
    R[0] = 1.0;  // recomputed below
    R[0] = mna::upPortResistance(net, R, 0);
    REQUIRE_THAT(R[0], WithinRel(f.R[0], 1e-15));
    double S[P][P];
    REQUIRE(mna::scattering(net, R, S));
    for (int i = 0; i < P; ++i)
        for (int j = 0; j < P; ++j)
            REQUIRE_THAT(S[i][j], WithinAbs(f.S[i][j], 1e-12));
    REQUIRE(std::fabs(S[0][0]) < 1e-12);  // up-port reflection-free (WDF_DESIGN §5.5)
    // losslessness: Sᵀ·G·S = G (WDF_DESIGN §5.5)
    for (int i = 0; i < P; ++i)
        for (int j = 0; j < P; ++j) {
            double m = 0.0;
            for (int k = 0; k < P; ++k)
                m += S[k][i] * S[k][j] / R[k];
            const double g = i == j ? 1.0 / R[i] : 0.0;
            REQUIRE(std::fabs(m - g) <= 1e-9 * (1.0 / R[i] + 1.0 / R[j]));
        }
}

}  // namespace

TEST_CASE("MNA: double-double arithmetic", "[mna][m1]") {
    const mna::DD third = mna::DD(1.0) / mna::DD(3.0);
    const mna::DD back = third * mna::DD(3.0) - mna::DD(1.0);
    REQUIRE(std::fabs(back.toDouble()) < 1e-30);
    // 1 + 1e-20 survives in the low word and cancels exactly
    const mna::DD x = (mna::DD(1.0) + mna::DD(1e-20)) - mna::DD(1.0);
    REQUIRE_THAT(x.toDouble(), WithinRel(1e-20, 1e-12));
}

TEST_CASE("MNA: scattering matches the exact Python reference", "[mna][m1]") {
    const auto fixtures = readFixtures();
    REQUIRE(fixtures.size() == 5);
    for (const auto& f : fixtures) {
        INFO(f.name);
        if (f.name == "random_bridge")
            checkAgainstFixture(kBridge, f);
        else
            checkAgainstFixture(amp2203::tonestack::kNetwork, f);
    }
}

TEST_CASE("MNA: R-type 3-ports match the library's parallel and series adaptors", "[mna][m1]") {
    constexpr double R1 = 220.0, R2 = 4.7e3;
    constexpr double V1 = 0.3, V2 = -1.1, X = 0.7;  // child waves and the incident wave from the parent
    wdft::ResistiveVoltageSourceT<double> s1{R1}, s2{R2};
    s1.setVoltage(V1);
    s2.setVoltage(V2);

    auto check = [&](auto& adaptor, const auto& net) {
        std::array<double, 3> R{1.0, R1, R2};
        R[0] = mna::upPortResistance(net, R, 0);
        REQUIRE_THAT(R[0], WithinRel(adaptor.wdf.R, 1e-12));
        double S[3][3];
        REQUIRE(mna::scattering(net, R, S));
        const double up = adaptor.reflected();
        adaptor.incident(X);
        const double a[3] = {X, V1, V2};
        double b[3] = {};
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                b[i] += S[i][j] * a[j];
        REQUIRE_THAT(b[0], WithinAbs(up, 1e-12));
        REQUIRE_THAT(b[1], WithinAbs(s1.wdf.a, 1e-12));
        REQUIRE_THAT(b[2], WithinAbs(s2.wdf.a, 1e-12));
    };

    SECTION("parallel") {
        wdft::WDFParallelT<double, decltype(s1), decltype(s2)> p{s1, s2};
        check(p, mna::PortNetwork<3, 1>{{{{0, mna::kGround}, {0, mna::kGround}, {0, mna::kGround}}}});
    }
    SECTION("series") {
        // loop orientation: v_up + v1 + v2 = 0 (the library's series adaptor presents −(v1 + v2) upwards)
        wdft::WDFSeriesT<double, decltype(s1), decltype(s2)> s{s1, s2};
        check(s, mna::PortNetwork<3, 2>{{{{0, mna::kGround}, {1, 0}, {mna::kGround, 1}}}});
    }
}
