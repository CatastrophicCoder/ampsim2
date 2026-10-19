// 12AX7 device model vs the SPICE DC sweep tables (VERIFICATION §5.1) and the analytic Jacobian vs
// central differences (§5.2); Wright omega identity.
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>

#include "devices/Triode.h"
#include "devices/WrightOmega.h"

using amp2203::devices::k12AX7;
using amp2203::devices::wrightOmega;

namespace {

struct Row {
    double vgk, vpk, ip, ig;
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
            REQUIRE(line == "vgk,vpk,ip,ig,ip_terminal");
            header = true;
            continue;
        }
        // strtod, not a stream: the tables contain subnormal values (SPICE ~0 A), which libc++ streams
        // reject as underflow
        double v[4];
        const char* p = line.c_str();
        for (double& x : v) {
            char* end = nullptr;
            x = std::strtod(p, &end);
            REQUIRE(end != p);
            p = *end == ',' ? end + 1 : end;
        }
        rows.push_back({v[0], v[1], v[2], v[3]});
    }
    return rows;
}

// VERIFICATION §5.1: 0.1 % relative or 1 nA absolute, whichever is larger
bool withinDeviceTolerance(double value, double ref) {
    return std::fabs(value - ref) <= std::max(1e-3 * std::fabs(ref), 1e-9);
}

// VERIFICATION §5.2: analytic vs central difference within 1e-5 relative; derivatives below 1e-12 A/V
// are skipped
bool jacobianMatches(double analytic, double fd) {
    if (std::max(std::fabs(analytic), std::fabs(fd)) < 1e-12)
        return true;
    return std::fabs(analytic - fd) <= 1e-5 * std::max(std::fabs(analytic), std::fabs(fd));
}

}  // namespace

TEST_CASE("Wright omega solves w + ln w = z", "[devices][m2]") {
    for (double z = -60.0; z <= 1e4; z = z < 10.0 ? z + 0.01 : z * 1.01) {
        const double w = amp2203::devices::wrightOmegaReference(z);
        REQUIRE(w > 0.0);
        if (z < -40.0)
            REQUIRE(std::fabs(w - std::exp(z)) <= 1e-15 * w);
        else
            REQUIRE(std::fabs(w + std::log(w) - z) <= 4e-16 * std::max(1.0, std::fabs(z)));
    }
    REQUIRE(std::fabs(wrightOmega(1.0) - 1.0) < 1e-16);
    REQUIRE(std::fabs(wrightOmega(0.0) - 0.56714329040978387) < 1e-16);  // Omega constant W(1)
}

TEST_CASE("Fast Wright omega matches the reference", "[devices][m7]") {
    // D-066: series below z = −10, table start + one step above; the residual allows 2 ulp of rounding in w + ln w
    double worst = 0.0;
    for (double z = -60.0; z <= 1e4; z = z < 50.0 ? z + 1e-3 : z * 1.001) {
        const double ref = amp2203::devices::wrightOmegaReference(z);
        const double w = wrightOmega(z);
        worst = std::max(worst, std::fabs(w - ref) / ref);
        if (z >= -10.0)
            REQUIRE(std::fabs(w + std::log(w) - z) <= 8e-16 * std::max(1.0, std::fabs(z)));
    }
    REQUIRE(worst <= 1e-14);
}

TEST_CASE("12AX7 plate curves match SPICE", "[devices][m2]") {
    const auto rows = readSweep("12ax7_plate.csv");
    REQUIRE(rows.size() == 7 * 81);
    for (const auto& r : rows) {
        const auto c = k12AX7.eval(r.vgk, r.vpk);
        INFO("Vgk " << r.vgk << " Vpk " << r.vpk << " ip " << c.ip << " (SPICE " << r.ip << ") ig " << c.ig
                    << " (SPICE " << r.ig << ")");
        REQUIRE(withinDeviceTolerance(c.ip, r.ip));
        REQUIRE(withinDeviceTolerance(c.ig, r.ig));
    }
}

TEST_CASE("12AX7 grid current matches SPICE", "[devices][m2]") {
    const auto rows = readSweep("12ax7_grid.csv");
    REQUIRE(rows.size() == 8);
    for (const auto& r : rows) {
        const auto c = k12AX7.eval(r.vgk, r.vpk);
        INFO("Vgk " << r.vgk << " Vpk " << r.vpk << " ig " << c.ig << " (SPICE " << r.ig << ")");
        REQUIRE(withinDeviceTolerance(c.ig, r.ig));
        REQUIRE(withinDeviceTolerance(c.ip, r.ip));
    }
}

TEST_CASE("12AX7 analytic Jacobian matches central differences", "[devices][m2]") {
    auto points = readSweep("12ax7_plate.csv");
    const auto grid = readSweep("12ax7_grid.csv");
    points.insert(points.end(), grid.begin(), grid.end());
    int checked = 0;
    for (const auto& r : points) {
        // Vpk = 0 is the E1 = 0 boundary: Ip ∝ E1^1.4 is once but not twice differentiable there, so a
        // central difference across it is not accurate (its error ∝ h^0.4); the analytic value is 0.
        if (r.vpk == 0.0)
            continue;
        const auto c = k12AX7.eval(r.vgk, r.vpk);
        const double hg = 1e-6 * std::max(1.0, std::fabs(r.vgk));
        const double hp = 1e-6 * std::max(1.0, std::fabs(r.vpk));
        const auto gm = k12AX7.eval(r.vgk - hg, r.vpk), gp = k12AX7.eval(r.vgk + hg, r.vpk);
        const auto pm = k12AX7.eval(r.vgk, r.vpk - hp), pp = k12AX7.eval(r.vgk, r.vpk + hp);
        const double fd_ig_g = (gp.ig - gm.ig) / (2 * hg);
        const double fd_ip_g = (gp.ip - gm.ip) / (2 * hg);
        const double fd_ip_p = (pp.ip - pm.ip) / (2 * hp);
        const double fd_ig_p = (pp.ig - pm.ig) / (2 * hp);
        INFO("Vgk " << r.vgk << " Vpk " << r.vpk);
        REQUIRE(jacobianMatches(c.dig_dvgk, fd_ig_g));
        REQUIRE(jacobianMatches(c.dip_dvgk, fd_ip_g));
        REQUIRE(jacobianMatches(c.dip_dvpk, fd_ip_p));
        REQUIRE(fd_ig_p == 0.0);  // the grid diode does not depend on Vpk
        ++checked;
    }
    REQUIRE(checked == 7 * 80 + 8);
}
