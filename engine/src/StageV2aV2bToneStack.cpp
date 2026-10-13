#include "amp2203/StageV2aV2bToneStack.h"

#include "devices/TwoTriodes.h"
#include "stage/RootNetwork.h"
#include "stage34/Stage34Network.h"

namespace amp2203 {

namespace {

using namespace stage34;
using devices::TwoTriodes;

constexpr stage::RootNetwork<kNumNodes, kNumElements, kNumRootPorts> kNet{kElements, kRoot};
const solver::MultiportNewton<4> kRootSolver = TwoTriodes::solver(12);
const solver::MultiportNewton<4> kDcSolver = TwoTriodes::solver(500);  // reset() only

}  // namespace

StageV2aV2bToneStack::StageV2aV2bToneStack() {
    Params defaults;
    setParams(defaults);
    reset(352.432789);  // CIRCUIT.md §5 p1b
}

void StageV2aV2bToneStack::prepare(double sampleRate, double smoothingMs) {
    for (auto* c : {&c7_, &c8_, &cgk2a_, &cgp2a_, &cpk2a_, &cgk2b_, &cgp2b_, &cpk2b_, &c10_, &c11_, &c12_})
        c->prepare(sampleRate);
    for (auto* s : {&treble_, &middle_, &bass_, &master_})
        s->prepare(sampleRate, smoothingMs);
    applyKnobs();
}

void StageV2aV2bToneStack::setParams(const Params& p) noexcept {
    treble_.setTarget(p.treble);
    middle_.setTarget(p.middle);
    bass_.setTarget(p.bass);
    master_.setTarget(p.master);
}

StageV2aV2bToneStack::Wave StageV2aV2bToneStack::conductances(bool dc) const noexcept {
    // port resistances as the elements present them (capacitors: T/2C), or DC with capacitors open
    Wave G{};
    G[eJ2] = j2_.wdf.G;
    G[eC7] = dc ? 0.0 : c7_.wdf.G;
    G[eR10C8] = dc ? 1.0 / R10 : r10c8_.wdf.G;
    G[eR11] = r11_.wdf.G;
    G[eR9] = r9_.wdf.G;
    G[eR12] = src12_.wdf.G;
    G[eR13] = r13_.wdf.G;
    G[eCgk2a] = dc ? 0.0 : cgk2a_.wdf.G;
    G[eCgp2a] = dc ? 0.0 : cgp2a_.wdf.G;
    G[eCpk2a] = dc ? 0.0 : cpk2a_.wdf.G;
    G[eCgk2b] = dc ? 0.0 : cgk2b_.wdf.G;
    G[eCgp2b] = dc ? 0.0 : cgp2b_.wdf.G;
    G[eCpk2b] = dc ? 0.0 : cpk2b_.wdf.G;
    G[eC10] = dc ? 0.0 : c10_.wdf.G;
    G[eR15] = r15_.wdf.G;
    G[eC11] = dc ? 0.0 : c11_.wdf.G;
    G[eC12] = dc ? 0.0 : c12_.wdf.G;
    G[eVR3a] = vr3a_.wdf.G;
    G[eVR3b] = vr3b_.wdf.G;
    G[eVR5] = vr5_.wdf.G;
    G[eVR4a] = vr4a_.wdf.G;
    G[eVR4b] = vr4b_.wdf.G;
    G[eVR2a] = vr2a_.wdf.G;
    G[eVR2b] = vr2b_.wdf.G;
    return G;
}

StageV2aV2bToneStack::Wave StageV2aV2bToneStack::elementOffsets() const noexcept {
    Wave o{};
    for (int k = 0; k < kNumElements; ++k)
        o[static_cast<size_t>(k)] = kElementSupplySign[k] * vbpX_;
    return o;
}

std::array<double, StageV2aV2bToneStack::kRootPorts> StageV2aV2bToneStack::rootOffsets() const noexcept {
    std::array<double, kRootPorts> o{};
    for (int r = 0; r < kNumRootPorts; ++r)
        o[static_cast<size_t>(r)] = kRootSupplySign[r] * vbpX_;
    return o;
}

void StageV2aV2bToneStack::applyKnobs() noexcept {
    using detail::atap;
    using detail::ltap;
    const double t = treble_.value(), m = middle_.value(), b = bass_.value(), v = master_.value();
    // exactly as the netlist: linear pots without, log pots with the extra 1 Ω (SPEC §6.1)
    vr3a_.setResistanceValue(VR3 * (1.0 - ltap(t)));
    vr3b_.setResistanceValue(VR3 * ltap(t));
    vr5_.setResistanceValue(VR5 * atap(b) + 1.0);
    vr4a_.setResistanceValue(VR4 * (1.0 - ltap(m)));
    vr4b_.setResistanceValue(VR4 * ltap(m));
    vr2a_.setResistanceValue(VR2 * (1.0 - atap(v)) + 1.0);
    vr2b_.setResistanceValue(VR2 * atap(v) + 1.0);
    kNet.updateMatrices(root_, conductances(false));
    ++updates_;

    // R_th4 as ToneStackMaster (D-015): VR2b ‖ (VR2a + VR3b + VR5 + VR4), capacitors open, k2b shorted
    const double chain = vr2a_.wdf.R + vr3b_.wdf.R + vr5_.wdf.R + VR4;
    rth4_ = 1.0 / (1.0 / vr2b_.wdf.R + 1.0 / chain);
}

void StageV2aV2bToneStack::updateOutputs(double vIn) noexcept {
    vIn_ = vIn;
    out_ = root_.node[y];
    iSupply_ = (vbpX_ - root_.node[p2a]) / R12 + root_.i[rP2b];
}

void StageV2aV2bToneStack::reset(double vInDc) {
    for (auto* s : {&treble_, &middle_, &bass_, &master_})
        s->snap();
    applyKnobs();
    sinceUpdate_ = 0;

    Wave sources{};
    sources[eJ2] = vInDc;
    sources[eR12] = vbpX_;
    const Wave eOff = elementOffsets();
    kNet.solveDc(root_, kDcSolver, TwoTriodes{}, conductances(true), sources, eOff, rootOffsets(),
                 {-1.5, 0.5 * vbpX_, -1.5, 0.5 * vbpX_});

    auto across = [&](Element e) { return kNet.elementVoltage(root_, e, eOff); };
    c7_.setDcVoltage(across(eC7));
    c8_.setDcVoltage(across(eR10C8));
    cgk2a_.setDcVoltage(across(eCgk2a));
    cgp2a_.setDcVoltage(across(eCgp2a));
    cpk2a_.setDcVoltage(across(eCpk2a));
    cgk2b_.setDcVoltage(across(eCgk2b));
    cgp2b_.setDcVoltage(across(eCgp2b));
    cpk2b_.setDcVoltage(across(eCpk2b));
    c10_.setDcVoltage(across(eC10));
    c11_.setDcVoltage(across(eC11));
    c12_.setDcVoltage(across(eC12));
    updateOutputs(vInDc);
}

double StageV2aV2bToneStack::process(double vIn) noexcept {
    bool moving = treble_.step();
    moving |= middle_.step();
    moving |= bass_.step();
    moving |= master_.step();
    if (moving && ++sinceUpdate_ >= kParamUpdateInterval) {
        sinceUpdate_ = 0;
        applyKnobs();
    } else if (!moving && sinceUpdate_ > 0) {
        sinceUpdate_ = 0;
        applyKnobs();  // settle exactly on the targets
    }

    j2_.setVoltage(vIn);
    src12_.setVoltage(vbpX_);
    Wave wave;
    stage::reflected(elements(), wave);
    const bool ok = kNet.process(root_, kRootSolver, TwoTriodes{}, wave, elementOffsets(), rootOffsets());
    stage::incident(elements(), wave);  // on failure wave = reflected: states unchanged
    if (ok)
        updateOutputs(vIn);
    return out_;
}

}  // namespace amp2203
