#include "amp2203/PowerStage.h"

#include "devices/TwoPentodes.h"
#include "stage/RootNetwork.h"
#include "stage6/Stage6Network.h"

namespace amp2203 {

namespace {

using namespace stage6;
using devices::TwoPentodes;

constexpr stage::RootNetwork<kNumNodes, kNumElements, kNumRootPorts> kNet{kElements, kRoot};
const solver::MultiportNewton<6> kRootSolver = TwoPentodes::solver(12);
const solver::MultiportNewton<6> kDcSolver = TwoPentodes::solver(500);  // reset() only
constexpr std::array<double, kNumRootPorts> kNoRootOffsets{};
constexpr double kWindingDcConductance = 1e6;  // S: the windings are shorts at DC (≈ 70 nV at 70 mA)
constexpr Element kWindings[3] = {eLp1, eLp2, eLs};

}  // namespace

PowerStage::PowerStage() {
    bias_.setVoltage(kBiasVoltage);
    prepare(768000.0);
    reset(270.880925, 262.270582);  // CIRCUIT.md §5 p3a, p3b
}

void PowerStage::prepare(double sampleRate) {
    for (auto* c : {&c14_, &c16_, &c18_, &c19_, &cgkA_, &cgpA_, &cpkA_, &cgkB_, &cgpB_, &cpkB_})
        c->prepare(sampleRate);
    ot_.prepare(sampleRate);
    updateMatrices();
}

void PowerStage::updateMatrices() noexcept {
    std::array<mna::Mutual, 3> mutual{};
    int m = 0;
    for (int i = 0; i < 3; ++i)
        for (int j = i + 1; j < 3; ++j)
            mutual[static_cast<size_t>(m++)] = {kWindings[i], kWindings[j], ot_.conductance(i, j)};
    kNet.updateMatrices(root_, conductances(false), mutual);
}

void PowerStage::setSourceResistances(double rthA, double rthB) {
    j5a_.setResistanceValue(rthA);
    j5b_.setResistanceValue(rthB);
    updateMatrices();
}

PowerStage::Wave PowerStage::conductances(bool dc) const noexcept {
    Wave G{};
    G[eJ5a] = j5a_.wdf.G;
    G[eJ5b] = j5b_.wdf.G;
    G[eC14] = dc ? 0.0 : c14_.wdf.G;
    G[eC16] = dc ? 0.0 : c16_.wdf.G;
    G[eC18] = dc ? 0.0 : c18_.wdf.G;
    G[eR24] = r24_.wdf.G;
    G[eR25] = r25_.wdf.G;
    G[eC19] = dc ? 0.0 : c19_.wdf.G;
    G[eBias] = bias_.wdf.G;
    G[eRgA] = rgA_.wdf.G;
    G[eRgB] = rgB_.wdf.G;
    G[eRsA] = rsA_.wdf.G;
    G[eRsB] = rsB_.wdf.G;
    G[eCgkA] = dc ? 0.0 : cgkA_.wdf.G;
    G[eCgpA] = dc ? 0.0 : cgpA_.wdf.G;
    G[eCpkA] = dc ? 0.0 : cpkA_.wdf.G;
    G[eCgkB] = dc ? 0.0 : cgkB_.wdf.G;
    G[eCgpB] = dc ? 0.0 : cgpB_.wdf.G;
    G[eCpkB] = dc ? 0.0 : cpkB_.wdf.G;
    G[eRwp1] = rwp1_.wdf.G;
    G[eRwp2] = rwp2_.wdf.G;
    G[eRws] = rws_.wdf.G;
    G[eRload] = rload_.wdf.G;
    for (int w = 0; w < 3; ++w)
        G[kWindings[w]] = dc ? kWindingDcConductance : ot_.conductance(w, w);
    return G;
}

PowerStage::Wave PowerStage::elementOffsets() const noexcept {
    Wave o{};
    for (int k = 0; k < kNumElements; ++k)
        o[static_cast<size_t>(k)] = kElementCtSign[k] * vCt_;
    return o;
}

void PowerStage::updateOutputs(double vocA, double vocB) noexcept {
    const auto& v = root_.node;
    vInA_ = vocA;
    vInB_ = vocB;
    out_ = v[out];
    // plate A draws from ct through Lp1 (current pA1 → pA is −i(Lp1)), plate B through Lp2
    iCt_ = (v[pB1] - v[pB]) / Rwp2 + (v[pA1] - v[pA]) / Rwp1;
    iScr_ = root_.i[rSA] + root_.i[rSB];
    ipTubeA_ = 0.5 * root_.i[rPA];
    ipTubeB_ = 0.5 * root_.i[rPB];
    isTubeA_ = 0.5 * root_.i[rSA];
    isTubeB_ = 0.5 * root_.i[rSB];
}

void PowerStage::reset(double vocA, double vocB) {
    updateMatrices();
    Wave sources{};
    sources[eJ5a] = vocA;
    sources[eJ5b] = vocB;
    sources[eBias] = kBiasVoltage;
    sources[eRsA] = vScr_;
    sources[eRsB] = vScr_;
    const Wave eOff = elementOffsets();
    const double vb = kBiasVoltage;
    kNet.solveDc(root_, kDcSolver, TwoPentodes{}, conductances(true), sources, eOff, kNoRootOffsets,
                 {vb, vScr_ - 5.0, vCt_ - 1.0, vb, vScr_ - 5.0, vCt_ - 1.0});

    auto across = [&](Element e) { return kNet.elementVoltage(root_, e, eOff); };
    c14_.setDcVoltage(across(eC14));
    c16_.setDcVoltage(across(eC16));
    c18_.setDcVoltage(across(eC18));
    c19_.setDcVoltage(across(eC19));
    cgkA_.setDcVoltage(across(eCgkA));
    cgpA_.setDcVoltage(across(eCgpA));
    cpkA_.setDcVoltage(across(eCpkA));
    cgkB_.setDcVoltage(across(eCgkB));
    cgpB_.setDcVoltage(across(eCgpB));
    cpkB_.setDcVoltage(across(eCpkB));
    // winding currents into each + terminal from the series resistances (not from the 1 MS DC "short")
    const auto& v = root_.node;
    ot_.setDcCurrents({(v[pA] - v[pA1]) / Rwp1, (v[pB1] - v[pB]) / Rwp2, v[sl] / Rws});
    updateOutputs(vocA, vocB);
}

double PowerStage::process(double vocA, double vocB) noexcept {
    j5a_.setVoltage(vocA);
    j5b_.setVoltage(vocB);
    rsA_.setVoltage(vScr_);
    rsB_.setVoltage(vScr_);
    Wave wave;
    stage::reflected(elements(), wave);
    const bool ok = kNet.process(root_, kRootSolver, TwoPentodes{}, wave, elementOffsets(), kNoRootOffsets);
    stage::incident(elements(), wave);  // on failure wave = reflected: states unchanged
    if (ok)
        updateOutputs(vocA, vocB);
    return out_;
}

}  // namespace amp2203
