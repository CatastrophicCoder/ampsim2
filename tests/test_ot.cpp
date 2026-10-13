// Output transformer network alone vs SPICE (CIRCUIT.md §3, fixtures from harness/ot_fixtures.py): the
// three coupled windings as one coupled element (detail::CoupledInductors, mna::Mutual stamps), the
// winding resistances and the 16 Ω load, plates driven by ideal current sources, centre tap at 0 V.
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <complex>
#include <cstdlib>
#include <fstream>
#include <numbers>
#include <string>
#include <tuple>
#include <vector>

#include <chowdsp_wdf/chowdsp_wdf.h>

#include "amp2203/detail/CoupledInductors.h"
#include "stage/RootNetwork.h"

namespace {

using namespace amp2203;
constexpr double kFs = 768000.0;
constexpr int G = mna::kGround;
enum Node : int { pA, pA1, pB1, pB, out, sl, kNodes };
enum Element : int { eIa, eIb, eRwp1, eLp1, eLp2, eRwp2, eLs, eRws, eRload, kElements };

constexpr mna::PortNetwork<kElements, kNodes> kNetElements{{{{pA, G}, {pB, G}, {pA, pA1}, {pA1, G}, {G, pB1}, {pB1, pB},
                                                             {out, sl}, {sl, G}, {out, G}}}};
constexpr mna::PortNetwork<1, kNodes> kNetRoot{{{{out, G}}}};  // a port that draws no current
constexpr stage::RootNetwork<kNodes, kElements, 1> kNet{kNetElements, kNetRoot};

struct Open {
    void operator()(const solver::Vec<1>&, solver::Vec<1>& i, solver::Mat<1>& d) const noexcept {
        i = {0.0};
        d = {{{0.0}}};
    }
};

// netlist values (reference/spice/jcm800_2203.cir, D-023)
constexpr double lpt = 25.61, kot = 0.9997925, Rbig = 1e9;
const double npr2 = 1700.0 / 16.0;

struct OtNetwork {
    chowdsp::wdft::ResistiveVoltageSourceT<double> ia{Rbig}, ib{Rbig};  // Norton: I·Rbig behind Rbig
    chowdsp::wdft::ResistorT<double> rwp1{15.36}, rwp2{16.56}, rws{0.320}, rload{16.0};
    detail::CoupledInductors<3> ot{{lpt / 4, lpt / 4, lpt / npr2}, kot};
    detail::RootNetworkState<kNodes, kElements, 1> s;
    solver::MultiportNewton<1> newton;
    std::array<mna::Mutual, 3> mutual{};

    explicit OtNetwork(double fs) {
        ot.prepare(fs);
        newton.maxStep = {1e12};
        const int idx[3] = {eLp1, eLp2, eLs};
        int m = 0;
        for (int i = 0; i < 3; ++i)
            for (int j = i + 1; j < 3; ++j)
                mutual[static_cast<size_t>(m++)] = {idx[i], idx[j], ot.conductance(i, j)};
        std::array<double, kElements> g{ia.wdf.G, ib.wdf.G, rwp1.wdf.G, ot.w[0].wdf.G, ot.w[1].wdf.G, rwp2.wdf.G,
                                        ot.w[2].wdf.G, rws.wdf.G, rload.wdf.G};
        REQUIRE(kNet.updateMatrices(s, g, mutual));
    }

    auto elements() { return std::tie(ia, ib, rwp1, ot.w[0], ot.w[1], rwp2, ot.w[2], rws, rload); }

    void process(double iA, double iB) {  // currents into pA and into pB
        ia.setVoltage(iA * Rbig);
        ib.setVoltage(iB * Rbig);
        std::array<double, kElements> wave{};
        stage::reflected(elements(), wave);
        const std::array<double, kElements> zero{};
        REQUIRE(kNet.process(s, newton, Open{}, wave, zero, {0.0}));
        stage::incident(elements(), wave);
    }
};

std::vector<std::vector<std::string>> readCsv(const std::string& name) {
    std::ifstream f(std::string(AMP2203_TEST_DATA_DIR) + "/" + name);
    REQUIRE(f.good());
    std::vector<std::vector<std::string>> rows;
    std::string line;
    bool header = false;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#')
            continue;
        if (!header) {
            header = true;
            continue;
        }
        std::vector<std::string> cols;
        size_t p = 0;
        for (size_t q; (q = line.find(',', p)) != std::string::npos; p = q + 1)
            cols.push_back(line.substr(p, q - p));
        cols.push_back(line.substr(p));
        rows.push_back(cols);
    }
    return rows;
}

double esrDb(const std::vector<double>& ref, const std::vector<double>& y) {
    double e = 0.0, r = 0.0;
    for (size_t n = 0; n < ref.size(); ++n) {
        e += (y[n] - ref[n]) * (y[n] - ref[n]);
        r += ref[n] * ref[n];
    }
    return 10.0 * std::log10(e / r);
}

}  // namespace

TEST_CASE("Coupled inductors: port conductance matrix is (T/2)·L⁻¹", "[ot][m4]") {
    detail::CoupledInductors<3> ot{{lpt / 4, lpt / 4, lpt / npr2}, kot};
    ot.prepare(kFs);
    const double L[3] = {lpt / 4, lpt / 4, lpt / npr2};
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) {
            double sum = 0.0;  // ((2/T)·L · G_L)_ij = δ_ij
            for (int k = 0; k < 3; ++k)
                sum += 2.0 * kFs * (i == k ? L[i] : kot * std::sqrt(L[i] * L[k])) * ot.conductance(k, j);
            REQUIRE(std::fabs(sum - (i == j ? 1.0 : 0.0)) < 1e-9);
            REQUIRE(ot.conductance(i, j) == ot.conductance(j, i));
        }
}

TEST_CASE("OT network transient matches SPICE (diff drive)", "[ot][m4]") {
    const auto rows = readCsv("ot_tran.csv");
    REQUIRE(rows.size() == 3840);
    OtNetwork net(kFs);
    std::vector<double> ref[3], y[3];
    for (const auto& r : rows) {
        const double i = std::strtod(r[0].c_str(), nullptr);
        net.process(i, -i);
        const double v[3] = {net.s.node[out], net.s.node[pA], net.s.node[pB]};
        for (int k = 0; k < 3; ++k) {
            ref[k].push_back(std::strtod(r[static_cast<size_t>(k) + 1].c_str(), nullptr));
            y[k].push_back(v[k]);
        }
    }
    if (const char* path = std::getenv("OT_DUMP")) {
        std::ofstream d(path);
        for (size_t n = 0; n < y[1].size(); ++n)
            d << y[0][n] << ',' << y[1][n] << ',' << y[2][n] << '\n';
    }
    // Scored at out (VERIFICATION §7 linear time-domain ESR ≤ −60 dB). The plate voltages are dominated by
    // L_leak·di/dt of the piecewise-linear drive, whose value at a sample instant is a matter of
    // convention: ngspice restarts integration at every sample breakpoint (backward-Euler step, the
    // left-sided slope), the trapezoidal engine uses the centred one (≈ −36 dB here, all at 15 kHz).
    // The plates are therefore compared in the AC test, where both are exact steady states.
    const double e = esrDb(ref[0], y[0]);
    INFO("out ESR " << e << " dB; pA " << esrDb(ref[1], y[1]) << " dB, pB " << esrDb(ref[2], y[2]) << " dB");
    REQUIRE(e <= -60.0);
}

TEST_CASE("OT network small-signal response matches SPICE AC", "[ot][m4]") {
    // linear-block limits (VERIFICATION §7): 0.1 dB 20 Hz–10 kHz, 0.3 dB 10–20 kHz; above 20 kHz reported
    const auto rows = readCsv("ot_ac.csv");
    int checked = 0;
    for (const auto& r : rows) {
        const double f = std::strtod(r[1].c_str(), nullptr);
        if (f < 19.0 || f > 20001.0 || (checked % 3 != 0 && f > 25.0 && f < 9000.0)) {
            ++checked;
            continue;  // a subset keeps the test short; every point above 9 kHz and at 20 Hz runs
        }
        ++checked;
        const bool diff = r[0] == "diff";
        OtNetwork net(kFs);
        const double w = 2.0 * std::numbers::pi * f / kFs;
        const int settle = static_cast<int>(0.25 * kFs);   // ≫ the 15 ms LF time constant
        const int period = static_cast<int>(std::round(kFs / f));
        const int cycles = std::max(1, static_cast<int>(0.02 * kFs) / period);
        std::complex<double> acc[3] = {0.0, 0.0, 0.0};
        for (int n = 0; n < settle + cycles * period; ++n) {
            const double i = std::sin(w * n);
            net.process(i, diff ? -i : 0.0);
            if (n >= settle) {
                const auto ph = std::exp(std::complex<double>(0.0, -w * n));
                acc[0] += net.s.node[out] * ph;
                acc[1] += net.s.node[pA] * ph;
                acc[2] += net.s.node[pB] * ph;
            }
        }
        for (int k = 0; k < 3; ++k) {
            const size_t c = 2 + 2 * static_cast<size_t>(k);
            const std::complex<double> h(std::strtod(r[c].c_str(), nullptr), std::strtod(r[c + 1].c_str(), nullptr));
            if (std::abs(h) == 0.0)
                continue;
            // the sine starts at phase 0: v = Im(H·e^{jωn}), so H = j·(2/N)·Σ v·e^{−jωn}
            const std::complex<double> he = acc[k] * 2.0 / static_cast<double>(cycles * period) * std::complex<double>(0.0, 1.0);
            const double dDb = 20.0 * std::log10(std::abs(he) / std::abs(h));
            const double dDeg = std::arg(he / h) * 180.0 / std::numbers::pi;
            INFO(r[0] << " f " << f << " Hz, node " << (k == 0 ? "out" : k == 1 ? "pA" : "pB") << ": |H| SPICE "
                      << std::abs(h) << " engine " << std::abs(he) << " (" << dDb << " dB), phase diff " << dDeg << " deg");
            REQUIRE(std::fabs(dDb) <= (f <= 10000.0 ? 0.1 : 0.3));
            REQUIRE(std::fabs(dDeg) <= 1.0);
        }
    }
}
