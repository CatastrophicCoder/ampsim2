#include "amp2203/PowerAmp.h"

#include <cmath>

#include "devices/TriodesPentodes.h"
#include "stage/RootNetwork.h"
#include "stage56/Stage56Network.h"

namespace amp2203 {

namespace {

using namespace stage56;
using devices::TriodesPentodes;
using devices::TriodesPentodesLoad;

constexpr stage::RootNetwork<kNumNodes, kNumElements, kNumRootPorts> kNet{kElements, kRoot};
// D-045 (owner decision Q21)
const solver::MultiportNewton<kNumRootPorts> kRootSolver = TriodesPentodes::solverK<kNumRootPorts>(32);
const solver::MultiportNewton<kNumRootPorts> kDcSolver = TriodesPentodes::solverK<kNumRootPorts>(500);  // reset() only
constexpr std::array<double, kNumRootPorts> kNoRootOffsets{};
constexpr double kWindingDcConductance = 1e6;  // S: the windings are shorts at DC (as PowerStage)
constexpr Element kWindings[3] = {eLp1, eLp2, eLs};

}  // namespace

PowerAmp::PowerAmp() {
    bias_.setVoltage(PS::kBiasVoltage);
    Params defaults;
    setParams(defaults);
    prepare(768000.0, 20.0);
    reset(0.0);
}

void PowerAmp::prepare(double sampleRate, double smoothingMs) {
    for (auto* c : {&c9_, &c15_, &c17_, &cgk3a_, &cgp3a_, &cpk3a_, &cgk3b_, &cgp3b_, &cpk3b_, &c14_, &c16_, &c18_,
                    &c19_, &cgkA_, &cgpA_, &cpkA_, &cgkB_, &cgpB_, &cpkB_})
        c->prepare(sampleRate);
    ot_.prepare(sampleRate);
    presence_.prepare(sampleRate, smoothingMs);
    applyKnobs();
}

void PowerAmp::setSourceResistance(double rth4) noexcept {
    if (rth4 != rth4_) {
        rth4_ = rth4;
        rthDirty_ = true;
    }
}

void PowerAmp::setFeedbackResistance(double r) {
    r22_.setResistanceValue(r);
    applyKnobs();
}

PowerAmp::Wave PowerAmp::conductances(bool dc) const noexcept {
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
    G[eR22] = r22_.wdf.G;
    G[eR18] = src18_.wdf.G;
    G[eR21] = src21_.wdf.G;
    G[eCgk3a] = dc ? 0.0 : cgk3a_.wdf.G;
    G[eCgp3a] = dc ? 0.0 : cgp3a_.wdf.G;
    G[eCpk3a] = dc ? 0.0 : cpk3a_.wdf.G;
    G[eCgk3b] = dc ? 0.0 : cgk3b_.wdf.G;
    G[eCgp3b] = dc ? 0.0 : cgp3b_.wdf.G;
    G[eCpk3b] = dc ? 0.0 : cpk3b_.wdf.G;
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

PowerAmp::Wave PowerAmp::elementOffsets() const noexcept {
    Wave o{};
    o[eLp1] = -vCt_;
    o[eLp2] = vCt_;
    return o;
}

void PowerAmp::rescaleReactive(double ratio) noexcept {
    // capacitors: R = T/(2C) scales with the step; the windings' R_L = 2L/T inversely (their conductances
    // enter only the matrices, which exist for both steps)
    for (auto* c : reactive())
        c->rescale(ratio, true);
    ot_.rescaleState(ratio);
}

void PowerAmp::applyKnobs() noexcept {
    vr6_.setResistanceValue(PI::VR6 * (1.0 - detail::ltap(presence_.value())) + 1.0);  // as PhaseInverter
    j4_.setResistanceValue(rth4_);
    rthDirty_ = false;
    for (const double step : {1.0, 0.5}) {  // full step and half step (D-043)
        std::array<mna::Mutual, 3> mutual{};
        int m = 0;
        for (int i = 0; i < 3; ++i)
            for (int j = i + 1; j < 3; ++j)
                mutual[static_cast<size_t>(m++)] = {kWindings[i], kWindings[j], step * ot_.conductance(i, j)};
        Wave G = conductances(false);
        if (step != 1.0) {
            for (auto* c : reactive())
                c->rescale(step, false);
            G = conductances(false);
            for (int w = 0; w < 3; ++w)
                G[kWindings[w]] = step * ot_.conductance(w, w);
            for (auto* c : reactive())
                c->rescale(1.0 / step, false);
        }
        // plain resistors reflect exactly 0 and have no offset (only the windings do): left out of the
        // open-circuit product (M6)
        constexpr auto kZeroWave = stage::plainResistors<decltype(elements())>();
        kNet.updateMatrices(step == 1.0 ? root_ : half_, G, mutual, &kZeroWave);
    }
    ++updates_;
}

void PowerAmp::halfStep(double vIn) noexcept {
    // the trapezoidal states move to T/2, two steps with the J4 source at the linearly interpolated midpoint
    // and at vIn, then back to T (D-043). The solver state goes along with the step-scaled trend. The
    // conversion needs the last sample's reflected waves, which this sample's reflected() overwrote.
    {
        size_t k = 0;
        for (auto* c : reactive())
            c->wdf.b = bLast_[k++];
        for (auto& w : ot_.w)
            w.wdf.b = bLast_[k++];
    }
    rescaleReactive(0.5);
    auto& h = half_;
    for (int r = 0; r < kNumRootPorts; ++r) {
        h.v[r] = root_.v[r];
        h.vPrev[r] = root_.v[r] - 0.5 * (root_.v[r] - root_.vPrev[r]);
        h.i[r] = root_.i[r];
        double voc = root_.v[r];  // the Thevenin voltages for which the last solution is exact at T/2
        for (int c = 0; c < kNumRootPorts; ++c)
            voc += h.Z[r][c] * root_.i[c];
        h.vOcPrev[r] = voc;
    }
    auto tuned = kRootSolver;  // the M6 tolerance study only (a copy per sample when not the default)
    tuned.config.residualTol = residualTol_;
    const auto& newton = residualTol_ == kRootSolver.config.residualTol ? kRootSolver : tuned;
    double iterations = 0.0;
    for (const int step : {0, 1}) {
        j4_.setVoltage(step == 0 ? 0.5 * (xPrev_ + vIn) : vIn);
        const TriodesPentodesLoad dev{step == 0 ? 0.5 * (vThPrev_ + portVTh_) : portVTh_, portG_};
        Wave wave;
        stage::reflected(elements(), wave);
        kNet.process(h, newton, dev, wave, elementOffsets(), kNoRootOffsets);
        stage::incident(elements(), wave);
        iterations += h.iterations;
    }
    rescaleReactive(2.0);
    for (int r = 0; r < kNumRootPorts; ++r) {
        root_.vPrev[r] = 2.0 * h.vPrev[r] - h.v[r];  // full-step trend
        root_.v[r] = h.v[r];
        root_.i[r] = h.i[r];
        double voc = h.v[r];
        for (int c = 0; c < kNumRootPorts; ++c)
            voc += root_.Z[r][c] * h.i[c];
        root_.vOcPrev[r] = voc;
    }
    root_.node = h.node;
    root_.iterations = iterations;
    ++halfSteps_;
}

void PowerAmp::updateOutputs(double vIn) noexcept {
    const auto& v = root_.node;
    vIn_ = vIn;
    out_ = v[out];
    iX_ = (vbpX_ - v[p3a]) / PI::R18 + (vbpX_ - v[p3b]) / PI::R21;
    iCt_ = (v[pB1] - v[pB]) / PS::Rwp2 + (v[pA1] - v[pA]) / PS::Rwp1;
    iScr_ = root_.i[rSA] + root_.i[rSB];
    ipTubeA_ = 0.5 * root_.i[rPA];
    ipTubeB_ = 0.5 * root_.i[rPB];
    isTubeA_ = 0.5 * root_.i[rSA];
    isTubeB_ = 0.5 * root_.i[rSB];
}

void PowerAmp::reset(double vInDc) {
    presence_.snap();
    applyKnobs();
    sinceUpdate_ = 0;

    Wave sources{};
    sources[eJ4] = vInDc;
    sources[eR18] = vbpX_;
    sources[eR21] = vbpX_;
    sources[eBias] = PS::kBiasVoltage;
    sources[eRsA] = vScr_;
    sources[eRsB] = vScr_;
    const Wave eOff = elementOffsets();
    const double vb = PS::kBiasVoltage;
    kNet.solveDc(root_, kDcSolver, TriodesPentodesLoad{portVTh_, portG_}, conductances(true), sources, eOff,
                 kNoRootOffsets,
                 {-1.5, 0.5 * vbpX_, -1.5, 0.5 * vbpX_, vb, vScr_ - 5.0, vCt_ - 1.0, vb, vScr_ - 5.0, vCt_ - 1.0,
                  portVTh_});

    auto across = [&](Element e) { return kNet.elementVoltage(root_, e, eOff); };
    c9_.setDcVoltage(across(eC9));
    c15_.setDcVoltage(across(eC15));
    c17_.setDcVoltage(-across(eC17VR6));  // port stored gnd – nG; no DC current through VR6
    for (auto [c, e] : {std::pair{&cgk3a_, eCgk3a}, {&cgp3a_, eCgp3a}, {&cpk3a_, eCpk3a}, {&cgk3b_, eCgk3b},
                        {&cgp3b_, eCgp3b}, {&cpk3b_, eCpk3b}, {&c14_, eC14}, {&c16_, eC16}, {&c18_, eC18},
                        {&c19_, eC19}, {&cgkA_, eCgkA}, {&cgpA_, eCgpA}, {&cpkA_, eCpkA}, {&cgkB_, eCgkB},
                        {&cgpB_, eCgpB}, {&cpkB_, eCpkB}})
        c->setDcVoltage(across(e));
    const auto& v = root_.node;
    ot_.setDcCurrents({(v[pA] - v[pA1]) / PS::Rwp1, (v[pB1] - v[pB]) / PS::Rwp2, v[sl] / PS::Rws});
    xPrev_ = vInDc;
    vThPrev_ = portVTh_;
    updateOutputs(vInDc);
}

double PowerAmp::process(double vIn) noexcept {
    const bool moving = presence_.step();
    if ((moving || rthDirty_) && ++sinceUpdate_ >= kParamUpdateInterval) {
        sinceUpdate_ = 0;
        applyKnobs();
    } else if (!moving && !rthDirty_ && sinceUpdate_ > 0) {
        sinceUpdate_ = 0;
        applyKnobs();  // settle exactly on the targets
    }

    j4_.setVoltage(vIn);
    src18_.setVoltage(vbpX_);
    src21_.setVoltage(vbpX_);
    rsA_.setVoltage(vScr_);
    rsB_.setVoltage(vScr_);
    if (halfStepTrigger_ > 0.0) {
        size_t k = 0;
        for (auto* c : reactive())
            bLast_[k++] = c->wdf.b;
        for (const auto& w : ot_.w)
            bLast_[k++] = w.wdf.b;
    }
    Wave wave;
    stage::reflected(elements(), wave);
    const Wave eOff = elementOffsets();
    std::array<double, kNumNodes> u;
    std::array<double, kNumRootPorts> vOc;
    kNet.openCircuit(root_, wave, eOff, kNoRootOffsets, u, vOc);
    bool half = false;
    if (halfStepTrigger_ > 0.0)
        for (int r = 0; r < kNumRootPorts; ++r)
            half = half || std::fabs(vOc[r] - root_.vOcPrev[r]) > halfStepTrigger_;
    auto tuned = kRootSolver;  // the M6 tolerance study only (a copy per sample when not the default)
    tuned.config.residualTol = residualTol_;
    const auto& newton = residualTol_ == kRootSolver.config.residualTol ? kRootSolver : tuned;
    if (half) {
        halfStep(vIn);
        updateOutputs(vIn);
    } else {
        const bool ok = kNet.solve(root_, newton, TriodesPentodesLoad{portVTh_, portG_}, u, vOc, wave, eOff);
        stage::incident(elements(), wave);  // on failure wave = reflected: states unchanged
        if (ok)
            updateOutputs(vIn);
    }
    xPrev_ = vIn;
    vThPrev_ = portVTh_;
    return out_;
}

}  // namespace amp2203
