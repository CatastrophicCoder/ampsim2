#include "amp2203/StageV1aV1b.h"

#include "devices/TwoTriodes.h"
#include "stage/RootNetwork.h"
#include "stage12/Stage12Network.h"

namespace amp2203 {

namespace {

using namespace stage12;
using devices::TwoTriodes;

constexpr stage::RootNetwork<kNumNodes, kNumElements, kNumRootPorts> kNet{kElements, kRoot};
const solver::MultiportNewton<4> kRootSolver = TwoTriodes::solver(12);
const solver::MultiportNewton<4> kDcSolver = TwoTriodes::solver(500);  // reset() only
constexpr std::array<double, kNumElements> kNoElementOffset{};         // no port ends on a supply node
constexpr std::array<double, kNumRootPorts> kNoRootOffset{};

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
    kNet.updateMatrices(root_, conductances(false));
    ++updates_;
}

void StageV1aV1b::reset(double vInDc) {
    volume_.snap();
    applyKnobs();
    sinceUpdate_ = 0;

    // DC operating point (capacitors open): the same network and root, sources at their DC values
    Wave sources{};
    sources[eVin] = vInDc;
    sources[eR4] = vbpA_;
    sources[eR7] = vbpA_;
    kNet.solveDc(root_, kDcSolver, TwoTriodes{}, conductances(true), sources, kNoElementOffset, kNoRootOffset,
                 {-1.0, 0.5 * vbpA_, -1.0, 0.5 * vbpA_});

    auto across = [this](Element e) { return kNet.elementVoltage(root_, e, kNoElementOffset); };
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
    out_ = root_.node[p1b];
    iSupply_ = (vbpA_ - root_.node[p1a]) / R4 + (vbpA_ - root_.node[p1b]) / R7;
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
    Wave wave;
    stage::reflected(elements(), wave);
    const bool ok = kNet.process(root_, kRootSolver, TwoTriodes{}, wave, kNoElementOffset, kNoRootOffset);
    stage::incident(elements(), wave);  // on failure wave = reflected: states unchanged
    if (!ok)
        return out_;

    vIn_ = vIn;
    out_ = root_.node[p1b];
    iSupply_ = (vbpA_ - root_.node[p1a]) / R4 + (vbpA_ - root_.node[p1b]) / R7;
    return out_;
}

}  // namespace amp2203
