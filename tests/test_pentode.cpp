// EL34 device model (with the repository screen modification) vs the SPICE DC sweep tables
// (VERIFICATION §5.1) and the analytic Jacobian vs central differences (§5.2).
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

#include "devices/Pentode.h"

using amp2203::devices::kEL34;
using amp2203::devices::kEL34Pair;
using amp2203::devices::PentodeCurrents;

namespace {

struct Row {
    double vsk, vgk, vpk, ip, is, ig;
};

std::vector<Row> readSweep(const std::string& name) {
    std::ifstream f(std::string(AMP2203_TEST_DATA_DIR) + "/" + name);
    REQUIRE(f.good());
    std::vector<Row> rows;
    std::string line;
    bool header = false;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#')
            continue;
        if (!header) {
            REQUIRE(line == "vsk,vgk,vpk,ip,is,ig,ip_terminal");
            header = true;
            continue;
        }
        double v[6];  // strtod: the tables contain subnormals (see test_triode.cpp)
        const char* p = line.c_str();
        for (double& x : v) {
            char* end = nullptr;
            x = std::strtod(p, &end);
            REQUIRE(end != p);
            p = *end == ',' ? end + 1 : end;
        }
        rows.push_back({v[0], v[1], v[2], v[3], v[4], v[5]});
    }
    return rows;
}

bool withinDeviceTolerance(double value, double ref) {  // 0.1 % or 1 nA
    return std::fabs(value - ref) <= std::max(1e-3 * std::fabs(ref), 1e-9);
}

bool jacobianMatches(double analytic, double fd) {  // 1e-5 relative; below 1e-12 A/V skipped
    if (std::max(std::fabs(analytic), std::fabs(fd)) < 1e-12)
        return true;
    return std::fabs(analytic - fd) <= 1e-5 * std::max(std::fabs(analytic), std::fabs(fd));
}

}  // namespace

TEST_CASE("EL34 plate and screen curves match SPICE", "[devices][m4]") {
    const auto rows = readSweep("el34_plate.csv");
    REQUIRE(rows.size() == 3 * 8 * 61);
    for (const auto& r : rows) {
        const auto c = kEL34.eval(r.vgk, r.vsk, r.vpk);
        INFO("Vsk " << r.vsk << " Vgk " << r.vgk << " Vpk " << r.vpk << " ip " << c.ip << " (SPICE " << r.ip
                    << ") is " << c.is << " (SPICE " << r.is << ") ig " << c.ig << " (SPICE " << r.ig << ")");
        REQUIRE(withinDeviceTolerance(c.ip, r.ip));
        REQUIRE(withinDeviceTolerance(c.is, r.is));
        REQUIRE(withinDeviceTolerance(c.ig, r.ig));
    }
}

TEST_CASE("EL34 grid current matches SPICE", "[devices][m4]") {
    const auto rows = readSweep("el34_grid.csv");
    REQUIRE(rows.size() == 5);
    for (const auto& r : rows) {
        const auto c = kEL34.eval(r.vgk, r.vsk, r.vpk);
        INFO("Vgk " << r.vgk << " ig " << c.ig << " (SPICE " << r.ig << ")");
        REQUIRE(withinDeviceTolerance(c.ig, r.ig));
        REQUIRE(withinDeviceTolerance(c.ip, r.ip));
        REQUIRE(withinDeviceTolerance(c.is, r.is));
    }
}

TEST_CASE("EL34 analytic Jacobian matches central differences", "[devices][m4]") {
    auto points = readSweep("el34_plate.csv");
    const auto grid = readSweep("el34_grid.csv");
    points.insert(points.end(), grid.begin(), grid.end());
    int checked = 0;
    for (const auto& r : points) {
        const auto c = kEL34.eval(r.vgk, r.vsk, r.vpk);
        const double hg = 1e-6 * std::max(1.0, std::fabs(r.vgk));
        const double hs = 1e-6 * std::max(1.0, std::fabs(r.vsk));
        const double hp = 1e-6 * std::max(1.0, std::fabs(r.vpk));
        auto fd = [](const PentodeCurrents& m, const PentodeCurrents& p, double h, double PentodeCurrents::* x) {
            return (p.*x - m.*x) / (2 * h);
        };
        const auto gm = kEL34.eval(r.vgk - hg, r.vsk, r.vpk), gp = kEL34.eval(r.vgk + hg, r.vsk, r.vpk);
        const auto sm = kEL34.eval(r.vgk, r.vsk - hs, r.vpk), sp = kEL34.eval(r.vgk, r.vsk + hs, r.vpk);
        const auto pm = kEL34.eval(r.vgk, r.vsk, r.vpk - hp), pp = kEL34.eval(r.vgk, r.vsk, r.vpk + hp);
        INFO("Vsk " << r.vsk << " Vgk " << r.vgk << " Vpk " << r.vpk);
        REQUIRE(jacobianMatches(c.dig_dvgk, fd(gm, gp, hg, &PentodeCurrents::ig)));
        REQUIRE(jacobianMatches(c.dip_dvgk, fd(gm, gp, hg, &PentodeCurrents::ip)));
        REQUIRE(jacobianMatches(c.dis_dvgk, fd(gm, gp, hg, &PentodeCurrents::is)));
        REQUIRE(jacobianMatches(c.dip_dvsk, fd(sm, sp, hs, &PentodeCurrents::ip)));
        REQUIRE(jacobianMatches(c.dis_dvsk, fd(sm, sp, hs, &PentodeCurrents::is)));
        REQUIRE(jacobianMatches(c.dip_dvpk, fd(pm, pp, hp, &PentodeCurrents::ip)));
        // Vpk = 0 is the kink of max(Vpk, 0) in the screen equation: a central difference across it is
        // the mean of both one-sided slopes, not a derivative
        if (r.vpk != 0.0)
            REQUIRE(jacobianMatches(c.dis_dvpk, fd(pm, pp, hp, &PentodeCurrents::is)));
        REQUIRE(fd(sm, sp, hs, &PentodeCurrents::ig) == 0.0);
        REQUIRE(fd(pm, pp, hp, &PentodeCurrents::ig) == 0.0);
        ++checked;
    }
    REQUIRE(checked == 3 * 8 * 61 + 5);
}

TEST_CASE("EL34 below the screen clamp and the matched-pair equivalent", "[devices][m4]") {
    // below 1 mV the screen voltage is clamped as in SPICE: currents constant, d/dVsk = 0
    const auto a = kEL34.eval(-10.0, 0.0, 300.0), b = kEL34.eval(-10.0, -5.0, 300.0);
    REQUIRE(a.ip == b.ip);
    REQUIRE(a.dip_dvsk == 0.0);
    REQUIRE(a.dis_dvsk == 0.0);
    // the pair is exactly twice one tube
    for (double vgk : {-60.0, -46.0, -20.0, 0.0, 5.0})
        for (double vpk : {-10.0, 0.0, 50.0, 480.0}) {
            const auto one = kEL34.eval(vgk, 470.0, vpk), two = kEL34Pair.eval(vgk, 470.0, vpk);
            REQUIRE(two.ip == 2.0 * one.ip);
            REQUIRE(two.is == 2.0 * one.is);
            REQUIRE(two.ig == 2.0 * one.ig);
            REQUIRE(two.dip_dvpk == 2.0 * one.dip_dvpk);
            REQUIRE(two.dis_dvgk == 2.0 * one.dis_dvgk);
            REQUIRE(two.dig_dvgk == 2.0 * one.dig_dvgk);
        }
    // no plate current below the cathode (5 V softplus, D-061): positive and small just below, vanishing far
    // below, continuous and increasing through Vpk = 0; the screen takes what the plate does not
    const auto below = kEL34.eval(-20.0, 470.0, -5.0), far = kEL34.eval(-20.0, 470.0, -500.0);
    REQUIRE(below.ip > 0.0);
    REQUIRE(far.ip >= 0.0);
    REQUIRE(far.ip < 1e-12);
    REQUIRE(far.is > below.is);
    double last = -1.0;
    for (double vpk = -50.0; vpk <= 50.0; vpk += 0.5) {
        const double ip = kEL34.eval(-20.0, 470.0, vpk).ip;
        REQUIRE(ip >= last);
        last = ip;
    }
}
