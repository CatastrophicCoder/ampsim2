#include "amp2203/PhaseInverter.h"

#include "devices/TwoTriodes.h"
#include "stage/RootNetwork.h"
#include "stage5/Stage5Network.h"

namespace amp2203 {

namespace {

using namespace stage5;
using devices::TwoTriodes;

constexpr stage::RootNetwork<kNumNodes, kNumElements, kNumRootPorts> kNet{kElements, kRoot};
const solver::MultiportNewton<4> kRootSolver = TwoTriodes::solver(12);
const solver::MultiportNewton<4> kDcSolver = TwoTriodes::solver(500);  // reset() only
constexpr std::array<double, kNumRootPorts> kNoRootOffsets{};

}  // namespace

PhaseInverter::PhaseInverter() {
    Params defaults;
    setParams(defaults);
    reset(0.0);
}

void PhaseInverter::prepare(double sampleRate, double smoothingMs) {
    for (auto* c : {&c9_, &c15_, &c17_, &cgk3a_, &cgp3a_, &cpk3a_, &cgk3b_, &cgp3b_, &cpk3b_})
        c->prepare(sampleRate);
    presence_.prepare(sampleRate, smoothingMs);
    applyKnobs();
}

void PhaseInverter::setSourceResistance(double rth4) noexcept {
    if (rth4 != rth4_) {
        rth4_ = rth4;
        rthDirty_ = true;
    }
}

void PhaseInverter::setFeedbackResistance(double r) {
    src22_.setResistanceValue(r);
    applyKnobs();
}

PhaseInverter::Wave PhaseInverter::conductances(bool dc) const noexcept {
    Wave G{};
    G[eJ4] = j4_.wdf.G;
    G[eC9] = dc ? 0.0 : c9_.wdf.G;
    G[eR17] = r17_.wdf.G;
    G[eR16] = r16_.wdf.G;
    G[eR19] = r19_.wdf.G;
    G[eC15] = dc ? 0.0 : c15_.wdf.G;
    G[eR20] = r20_.wdf.G;
    G[eR23] = r23_.wdf.G;
    G[eC17VR6] = dc ? 0.0 : c17vr6_.wdf.G;
    G[eR22] = src22_.wdf.G;
    G[eR18] = src18_.wdf.G;
    G[eR21] = src21_.wdf.G;
    G[eCgk3a] = dc ? 0.0 : cgk3a_.wdf.G;
    G[eCgp3a] = dc ? 0.0 : cgp3a_.wdf.G;
    G[eCpk3a] = dc ? 0.0 : cpk3a_.wdf.G;
    G[eCgk3b] = dc ? 0.0 : cgk3b_.wdf.G;
    G[eCgp3b] = dc ? 0.0 : cgp3b_.wdf.G;
    G[eCpk3b] = dc ? 0.0 : cpk3b_.wdf.G;
    return G;
}

void PhaseInverter::applyKnobs() noexcept {
    // exactly as the netlist: RVR6 = 22k·(1 − ltap(presence)) + 1 Ω (SPEC §6.1)
    vr6_.setResistanceValue(VR6 * (1.0 - detail::ltap(presence_.value())) + 1.0);
    j4_.setResistanceValue(rth4_);
    rthDirty_ = false;
    kNet.updateMatrices(root_, conductances(false));
    ++updates_;
}

void PhaseInverter::updateOutputs(double vIn) noexcept {
    vIn_ = vIn;
    outA_ = root_.node[p3a];
    outB_ = root_.node[p3b];
    iSupply_ = (vbpX_ - root_.node[p3a]) / R18 + (vbpX_ - root_.node[p3b]) / R21;
}

void PhaseInverter::reset(double vInDc) {
    presence_.snap();
    applyKnobs();
    sinceUpdate_ = 0;

    Wave sources{};
    sources[eJ4] = vInDc;
    sources[eR22] = vFb_;
    sources[eR18] = vbpX_;
    sources[eR21] = vbpX_;
    const Wave eOff = elementOffsets();
    kNet.solveDc(root_, kDcSolver, TwoTriodes{}, conductances(true), sources, eOff, kNoRootOffsets,
                 {-1.5, 0.5 * vbpX_, -1.5, 0.5 * vbpX_});

    auto across = [&](Element e) { return kNet.elementVoltage(root_, e, eOff); };
    c9_.setDcVoltage(across(eC9));
    c15_.setDcVoltage(across(eC15));
    c17_.setDcVoltage(-across(eC17VR6));  // port stored gnd – nG; no DC current: VR6 carries 0 V
    cgk3a_.setDcVoltage(across(eCgk3a));
    cgp3a_.setDcVoltage(across(eCgp3a));
    cpk3a_.setDcVoltage(across(eCpk3a));
    cgk3b_.setDcVoltage(across(eCgk3b));
    cgp3b_.setDcVoltage(across(eCgp3b));
    cpk3b_.setDcVoltage(across(eCpk3b));
    updateOutputs(vInDc);
}

double PhaseInverter::process(double vIn) noexcept {
    const bool moving = presence_.step();
    if ((moving || rthDirty_) && ++sinceUpdate_ >= kParamUpdateInterval) {
        sinceUpdate_ = 0;
        applyKnobs();
    } else if (!moving && !rthDirty_ && sinceUpdate_ > 0) {
        sinceUpdate_ = 0;
        applyKnobs();  // settle exactly on the targets
    }

    j4_.setVoltage(vIn);
    src22_.setVoltage(vFb_);
    src18_.setVoltage(vbpX_);
    src21_.setVoltage(vbpX_);
    Wave wave;
    stage::reflected(elements(), wave);
    const bool ok = kNet.process(root_, kRootSolver, TwoTriodes{}, wave, elementOffsets(), kNoRootOffsets);
    stage::incident(elements(), wave);  // on failure wave = reflected: states unchanged
    if (ok)
        updateOutputs(vIn);
    return outA_;
}

}  // namespace amp2203
