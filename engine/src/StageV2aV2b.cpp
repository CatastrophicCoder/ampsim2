#include "amp2203/StageV2aV2b.h"

#include "devices/TwoTriodes.h"
#include "stage/RootNetwork.h"
#include "stage3/Stage3Network.h"

namespace amp2203 {

namespace {

using namespace stage3;
using devices::TwoTriodes;

constexpr stage::RootNetwork<kNumNodes, kNumElements, kNumRootPorts> kNet{kElements, kRoot};
const solver::MultiportNewton<4> kRootSolver = TwoTriodes::solver(12);
const solver::MultiportNewton<4> kDcSolver = TwoTriodes::solver(500);  // reset() only

}  // namespace

StageV2aV2b::StageV2aV2b() {
    kNet.updateMatrices(root_, conductances(false));
    reset(352.432789);  // CIRCUIT.md §5 p1b
}

void StageV2aV2b::prepare(double sampleRate) {
    for (auto* c : {&c7_, &c8_, &cgk2a_, &cgp2a_, &cpk2a_, &cgk2b_, &cgp2b_, &cpk2b_})
        c->prepare(sampleRate);
    kNet.updateMatrices(root_, conductances(false));
}

StageV2aV2b::Wave StageV2aV2b::conductances(bool dc) const noexcept {
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
    return G;
}

StageV2aV2b::Wave StageV2aV2b::elementOffsets() const noexcept {
    Wave o{};
    for (int k = 0; k < kNumElements; ++k)
        o[static_cast<size_t>(k)] = kElementSupplySign[k] * vbpX_;
    return o;
}

std::array<double, StageV2aV2b::kRootPorts> StageV2aV2b::rootOffsets() const noexcept {
    std::array<double, kRootPorts> o{};
    for (int r = 0; r < kNumRootPorts; ++r)
        o[static_cast<size_t>(r)] = kRootSupplySign[r] * vbpX_;
    return o;
}

void StageV2aV2b::updateOutputs(double vIn) noexcept {
    vIn_ = vIn;
    out_ = root_.node[k2b];
    iSupply_ = (vbpX_ - root_.node[p2a]) / R12 + root_.i[rP2b];
}

void StageV2aV2b::reset(double vInDc) {
    kNet.updateMatrices(root_, conductances(false));
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
    updateOutputs(vInDc);
}

double StageV2aV2b::process(double vIn) noexcept {
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
