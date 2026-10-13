#include "amp2203/StageV1aV1b.h"

#include <cmath>
#include <tuple>

#include "devices/Triode.h"
#include "mna/PortNetwork.h"
#include "solver/MultiportNewton.h"
#include "stage12/Stage12Network.h"

namespace amp2203 {

namespace {

using namespace stage12;
using Root = solver::MultiportNewton<kNumRootPorts>;

/// The two 12AX7s on the root ports (Vgk1a, Vpk1a, Vgk1b, Vpk1b) → (Ig1a, Ip1a, Ig1b, Ip1b).
struct TwoTriodes {
    void operator()(const solver::Vec<4>& v, solver::Vec<4>& i, solver::Mat<4>& d) const noexcept {
        const auto a = devices::k12AX7.eval(v[rG1a], v[rP1a]);
        const auto b = devices::k12AX7.eval(v[rG1b], v[rP1b]);
        i = {a.ig, a.ip, b.ig, b.ip};
        d = {{{a.dig_dvgk, 0.0, 0.0, 0.0},
              {a.dip_dvgk, a.dip_dvpk, 0.0, 0.0},
              {0.0, 0.0, b.dig_dvgk, 0.0},
              {0.0, 0.0, b.dip_dvgk, b.dip_dvpk}}};
    }
};

Root makeRoot(int maxIter) {
    Root r;
    r.config.maxIter = maxIter;
    r.maxStep = {2.0, 20.0, 2.0, 20.0};  // WDF_DESIGN §4: grid ports 2 V, plate ports 20 V
    return r;
}

const Root kRootSolver = makeRoot(12);
const Root kDcSolver = makeRoot(500);  // reset() only

}  // namespace

StageV1aV1b::StageV1aV1b() {
    Params defaults;
    setParams(defaults);
    volume_.snap();
    applyKnobs();
    reset(0.0);
}

void StageV1aV1b::prepare(double sampleRate, double smoothingMs) {
    for (auto* c : {&c1_, &c3_, &c4_, &c5_, &cgk1a_, &cgp1a_, &cpk1a_, &cgk1b_, &cgp1b_, &cpk1b_})
        c->prepare(sampleRate);
    volume_.prepare(sampleRate, smoothingMs);
    applyKnobs();
}

void StageV1aV1b::setParams(const Params& p) noexcept { volume_.setTarget(p.preampVolume); }

StageV1aV1b::Wave StageV1aV1b::conductances(bool dc) const noexcept {
    // port resistances as the elements present them (capacitors: T/2C), or DC with capacitors open
    Wave G{};
    G[eVin] = vin_.wdf.G;
    G[eK1a] = dc ? 1.0 / R1 : k1a_.wdf.G;
    G[eR4] = src4_.wdf.G;
    G[eC3] = dc ? 0.0 : c3_.wdf.G;
    G[eR5C4] = dc ? 1.0 / R5 : r5c4_.wdf.G;
    G[eC5VR1a] = dc ? vr1a_.wdf.G : c5vr1a_.wdf.G;
    G[eVR1b] = vr1b_.wdf.G;
    G[eR6] = r6_.wdf.G;
    G[eR7] = src7_.wdf.G;
    G[eCgk1a] = dc ? 0.0 : cgk1a_.wdf.G;
    G[eCgp1a] = dc ? 0.0 : cgp1a_.wdf.G;
    G[eCpk1a] = dc ? 0.0 : cpk1a_.wdf.G;
    G[eCgk1b] = dc ? 0.0 : cgk1b_.wdf.G;
    G[eCgp1b] = dc ? 0.0 : cgp1b_.wdf.G;
    G[eCpk1b] = dc ? 0.0 : cpk1b_.wdf.G;
    return G;
}

void StageV1aV1b::applyKnobs() noexcept {
    const double t = detail::atap(volume_.value());
    // as the netlist: log pot with the extra 1 Ω on each side (SPEC §6.1)
    vr1a_.setResistanceValue(VR1 * (1.0 - t) + 1.0);
    vr1b_.setResistanceValue(VR1 * t + 1.0);
    if (mna::rootTransfer(stage12::kElements, stage12::kRoot, conductances(false), Xe_, Xr_)) {
        for (int r = 0; r < kRootPorts; ++r)
            for (int s = 0; s < kRootPorts; ++s)
                Z_[r][s] = Xr_[kRoot.ports[r][0]][s] - Xr_[kRoot.ports[r][1]][s];
    }
    ++updates_;
}

StageV1aV1b::Wave StageV1aV1b::reflectedWaves() noexcept {
    Wave b{};
    std::apply([&b](auto&... e) {
        int k = 0;
        ((b[static_cast<size_t>(k++)] = e.reflected()), ...);
    }, elements());
    return b;
}

void StageV1aV1b::incidentWaves(const Wave& a) noexcept {
    std::apply([&a](auto&... e) {
        int k = 0;
        (e.incident(a[static_cast<size_t>(k++)]), ...);
    }, elements());
}

void StageV1aV1b::reset(double vInDc) {
    volume_.snap();
    applyKnobs();
    sinceUpdate_ = 0;

    // DC operating point (capacitors open): the same network and root, sources at their DC values
    double Xe[kNodes][kElements], Xr[kNodes][kRootPorts];
    std::array<std::array<double, kRootPorts>, kRootPorts> Z{};
    if (!mna::rootTransfer(stage12::kElements, stage12::kRoot, conductances(true), Xe, Xr))
        return;
    Wave b{};
    b[eVin] = vInDc;
    b[eR4] = vbpA_;
    b[eR7] = vbpA_;
    std::array<double, kNodes> u{};
    for (int n = 0; n < kNodes; ++n)
        for (int j = 0; j < kElements; ++j)
            u[n] += Xe[n][j] * b[j];
    solver::Vec<4> vOc{};
    for (int r = 0; r < kRootPorts; ++r) {
        vOc[r] = u[kRoot.ports[r][0]] - u[kRoot.ports[r][1]];
        for (int s = 0; s < kRootPorts; ++s)
            Z[r][s] = Xr[kRoot.ports[r][0]][s] - Xr[kRoot.ports[r][1]][s];
    }
    v_ = {-1.0, 0.5 * vbpA_, -1.0, 0.5 * vbpA_};
    detail::NewtonStats dcStats;
    kDcSolver.solve(TwoTriodes{}, vOc, Z, v_, i_, dcStats);
    vPrev_ = v_;
    for (int n = 0; n < kNodes; ++n) {
        node_[n] = u[n];
        for (int r = 0; r < kRootPorts; ++r)
            node_[n] -= Xr[n][r] * i_[r];
    }

    auto across = [this](Element e) {
        const auto& p = stage12::kElements.ports[e];
        return (p[0] != mna::kGround ? node_[p[0]] : 0.0) - (p[1] != mna::kGround ? node_[p[1]] : 0.0);
    };
    c1_.setDcVoltage(across(eK1a));
    c3_.setDcVoltage(across(eC3));
    c4_.setDcVoltage(across(eR5C4));
    c5_.setDcVoltage(across(eC5VR1a));
    cgk1a_.setDcVoltage(across(eCgk1a));
    cgp1a_.setDcVoltage(across(eCgp1a));
    cpk1a_.setDcVoltage(across(eCpk1a));
    cgk1b_.setDcVoltage(across(eCgk1b));
    cgp1b_.setDcVoltage(across(eCgp1b));
    cpk1b_.setDcVoltage(across(eCpk1b));

    vIn_ = vInDc;
    out_ = node_[p1b];
    iSupply_ = (vbpA_ - node_[p1a]) / R4 + (vbpA_ - node_[p1b]) / R7;
    iters_ = 0.0;
}

double StageV1aV1b::process(double vIn) noexcept {
    const bool moving = volume_.step();
    if (moving && ++sinceUpdate_ >= kParamUpdateInterval) {
        sinceUpdate_ = 0;
        applyKnobs();
    } else if (!moving && sinceUpdate_ > 0) {
        sinceUpdate_ = 0;
        applyKnobs();  // settle exactly on the target
    }

    vin_.setVoltage(vIn);
    src4_.setVoltage(vbpA_);
    src7_.setVoltage(vbpA_);
    const Wave b = reflectedWaves();

    // open-root node voltages and the Thevenin voltages at the root ports
    std::array<double, kNodes> u{};
    for (int n = 0; n < kNodes; ++n) {
        double s = 0.0;
        for (int j = 0; j < kElements; ++j)
            s += Xe_[n][j] * b[static_cast<size_t>(j)];
        u[n] = s;
    }
    solver::Vec<4> vOc;
    for (int r = 0; r < kRootPorts; ++r)
        vOc[r] = u[kRoot.ports[r][0]] - u[kRoot.ports[r][1]];

    // predictor: linear extrapolation from the two previous solutions (DECISIONS D-029)
    auto v = v_;
    for (int r = 0; r < kRootPorts; ++r)
        v[r] += v_[r] - vPrev_[r];
    auto i = i_;
    iters_ = kRootSolver.solve(TwoTriodes{}, vOc, Z_, v, i, stats_);

    std::array<double, kNodes> node;
    bool finite = true;
    for (int n = 0; n < kNodes; ++n) {
        double s = u[n];
        for (int r = 0; r < kRootPorts; ++r)
            s -= Xr_[n][r] * i[r];
        node[n] = s;
        finite = finite && std::isfinite(s);
    }
    if (!finite) {  // keep the previous state; never output NaN/Inf (SPEC §3.5)
        ++sanitized_;
        incidentWaves(b);  // a = b: zero current into every element, states unchanged in voltage
        return out_;
    }
    vPrev_ = v_;
    v_ = v;
    i_ = i;
    node_ = node;

    Wave a;
    for (int k = 0; k < kElements; ++k) {
        const auto& p = stage12::kElements.ports[k];
        const double vk = (p[0] != mna::kGround ? node[p[0]] : 0.0) - (p[1] != mna::kGround ? node[p[1]] : 0.0);
        a[static_cast<size_t>(k)] = 2.0 * vk - b[static_cast<size_t>(k)];
    }
    incidentWaves(a);

    vIn_ = vIn;
    out_ = node[p1b];
    iSupply_ = (vbpA_ - node[p1a]) / R4 + (vbpA_ - node[p1b]) / R7;
    return out_;
}

}  // namespace amp2203
