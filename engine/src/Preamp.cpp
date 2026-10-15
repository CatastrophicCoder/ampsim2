#include "amp2203/Preamp.h"

#include "devices/FourTriodes.h"
#include "stage/RootNetwork.h"
#include "stage1234/Stage1234Network.h"

namespace amp2203 {

namespace {

using namespace stage1234;
using devices::FourTriodes;
using devices::FourTriodesLoad;

constexpr stage::RootNetwork<kNumNodes, kNumElements, kNumRootPorts> kNet{kElements, kRoot};
const solver::MultiportNewton<kNumRootPorts> kRootSolver = FourTriodes::solverK<kNumRootPorts>(32);
const solver::MultiportNewton<kNumRootPorts> kDcSolver = FourTriodes::solverK<kNumRootPorts>(500);  // reset() only

}  // namespace

Preamp::Preamp() {
    Params defaults;
    setParams(defaults);
    reset(0.0);
}

void Preamp::prepare(double sampleRate, double smoothingMs) {
    for (auto* c : {&c1_, &c3_, &c4_, &c5_, &cgk1a_, &cgp1a_, &cpk1a_, &cgk1b_, &cgp1b_, &cpk1b_, &c7_, &c8_,
                    &cgk2a_, &cgp2a_, &cpk2a_, &cgk2b_, &cgp2b_, &cpk2b_, &c10_, &c11_, &c12_})
        c->prepare(sampleRate);
    for (auto* s : {&volume_, &treble_, &middle_, &bass_, &master_})
        s->prepare(sampleRate, smoothingMs);
    applyKnobs();
}

void Preamp::setParams(const Params& p) noexcept {
    volume_.setTarget(p.preampVolume);
    treble_.setTarget(p.treble);
    middle_.setTarget(p.middle);
    bass_.setTarget(p.bass);
    master_.setTarget(p.master);
}

Preamp::Wave Preamp::conductances(bool dc) const noexcept {
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

Preamp::Wave Preamp::elementOffsets() const noexcept {
    Wave o{};
    for (int k = 0; k < kNumElements; ++k)
        o[static_cast<size_t>(k)] = kElementSupplySign[k] * vbpX_;
    return o;
}

std::array<double, Preamp::kRootPorts> Preamp::rootOffsets() const noexcept {
    std::array<double, kRootPorts> o{};
    for (int r = 0; r < kNumRootPorts; ++r)
        o[static_cast<size_t>(r)] = kRootSupplySign[r] * vbpX_;
    return o;
}

void Preamp::applyKnobs() noexcept {
    using detail::atap;
    using detail::ltap;
    const double pv = atap(volume_.value());
    const double t = treble_.value(), m = middle_.value(), b = bass_.value(), v = master_.value();
    // exactly as the netlist: linear pots without, log pots with the extra 1 Ω (SPEC §6.1)
    vr1a_.setResistanceValue(VR1 * (1.0 - pv) + 1.0);
    vr1b_.setResistanceValue(VR1 * pv + 1.0);
    vr3a_.setResistanceValue(VR3 * (1.0 - ltap(t)));
    vr3b_.setResistanceValue(VR3 * ltap(t));
    vr5_.setResistanceValue(VR5 * atap(b) + 1.0);
    vr4a_.setResistanceValue(VR4 * (1.0 - ltap(m)));
    vr4b_.setResistanceValue(VR4 * ltap(m));
    vr2a_.setResistanceValue(VR2 * (1.0 - atap(v)) + 1.0);
    vr2b_.setResistanceValue(VR2 * atap(v) + 1.0);
    // plain resistors not on bpX reflect exactly 0 with no offset: left out of the open-circuit product (M6)
    auto zeroWave = stage::plainResistors<decltype(elements())>();
    for (int k = 0; k < kNumElements; ++k)
        zeroWave[static_cast<size_t>(k)] = zeroWave[static_cast<size_t>(k)] && kElementSupplySign[k] == 0;
    kNet.updateMatrices(root_, conductances(false), {}, &zeroWave, false);  // plain double (D-057)
    ++updates_;

    // R_th4 as ToneStackMaster (D-015): VR2b ‖ (VR2a + VR3b + VR5 + VR4), capacitors open, k2b shorted
    const double chain = vr2a_.wdf.R + vr3b_.wdf.R + vr5_.wdf.R + VR4;
    rth4_ = 1.0 / (1.0 / vr2b_.wdf.R + 1.0 / chain);
}

void Preamp::updateOutputs(double vIn) noexcept {
    vIn_ = vIn;
    out_ = root_.node[y];
    iSupplyA_ = (vbpA_ - root_.node[p1a]) / R4 + (vbpA_ - root_.node[p1b]) / R7;
    iSupplyX_ = (vbpX_ - root_.node[p2a]) / R12 + root_.i[rP2b];
}

void Preamp::reset(double vInDc) {
    for (auto* s : {&volume_, &treble_, &middle_, &bass_, &master_})
        s->snap();
    applyKnobs();
    sinceUpdate_ = 0;

    Wave sources{};
    sources[eVin] = vInDc;
    sources[eR4] = vbpA_;
    sources[eR7] = vbpA_;
    sources[eR12] = vbpX_;
    const Wave eOff = elementOffsets();
    kNet.solveDc(root_, kDcSolver, FourTriodesLoad{}, conductances(true), sources, eOff, rootOffsets(),
                 {-1.0, 0.5 * vbpA_, -1.0, 0.5 * vbpA_, -1.5, 0.5 * vbpX_, -1.5, 0.5 * vbpX_, 0.0});

    auto across = [&](Element e) { return kNet.elementVoltage(root_, e, eOff); };
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

double Preamp::process(double vIn) noexcept {
    beginSample(vIn);
    solveLoaded(0.0);
    finishSample();
    return out_;
}

void Preamp::beginSample(double vIn) noexcept {
    bool moving = volume_.step();
    moving |= treble_.step();
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

    vin_.setVoltage(vIn);
    src4_.setVoltage(vbpA_);
    src7_.setVoltage(vbpA_);
    src12_.setVoltage(vbpX_);
    vInNow_ = vIn;
    solvedOnce_ = false;
    stage::reflected(elements(), reflected_);
    eOff_ = elementOffsets();
    kNet.openCircuit(root_, reflected_, eOff_, rootOffsets(), u_, vOc_);
    saved_ = {root_.v, root_.vPrev, root_.i, root_.vOcPrev};
    savedNode_ = root_.node;
}

bool Preamp::solveLoaded(double iY) noexcept {
    // the first call of a sample predicts from the last samples; a repeated call (corrected load current)
    // starts at the first call's solution with no trend, and the trend is then restored to the last sample's
    const bool repeat = solvedOnce_ && ok_;
    const auto failures = root_.stats.failures;
    if (repeat) {
        root_.vPrev = root_.v;  // no extrapolation: v is already this sample's solution for the old current
    } else {
        root_.v = saved_[0];
        root_.vPrev = saved_[1];
        root_.i = saved_[2];
        root_.vOcPrev = saved_[3];
        root_.node = savedNode_;
    }
    auto tuned = kRootSolver;  // the M6 tolerance study only (a copy per sample when not the default)
    tuned.config.residualTol = residualTol_;
    const auto& newton = residualTol_ == kRootSolver.config.residualTol ? kRootSolver : tuned;
    root_.i[rY] = iY;
    incident_ = reflected_;
    ok_ = kNet.solve(root_, newton, FourTriodesLoad{iY}, u_, vOc_, incident_, eOff_);
    if (repeat && root_.stats.failures == failures)
        root_.vPrev = saved_[0];  // the trend for the next sample: this sample's solution vs the last one
    solvedOnce_ = true;
    return ok_;
}

void Preamp::theveninAtY(double iY, double& vTh, double& rTh) const noexcept {
    // linearised at the solution: v = vOc − Z·i, i_p = i_p(v_p), i_y given. A change of i_y moves the triode
    // ports by dv_p = −(I + Z_pp·D)⁻¹·Z_py·di_y, so v_y = vTh − rTh·i_y with
    // rTh = Z_yy − Z_yp·D·(I + Z_pp·D)⁻¹·Z_py
    constexpr int P = 8;
    solver::Vec<kNumRootPorts> i;
    solver::Mat<kNumRootPorts> d9;
    FourTriodes::eval<kNumRootPorts>(root_.v, i, d9);
    solver::Mat<P> A;
    solver::Vec<P> x;
    for (int r = 0; r < P; ++r) {
        for (int c = 0; c < P; ++c) {
            double s = r == c ? 1.0 : 0.0;
            for (int m = FourTriodes::kBlockBegin[c]; m < FourTriodes::kBlockEnd[c]; ++m)
                s += root_.Z[r][m] * d9[m][c];
            A[r][c] = s;
        }
        x[r] = root_.Z[r][rY];
    }
    rTh = root_.Z[rY][rY];
    if (solver::solveLinear<P>(A, x)) {
        for (int r = 0; r < P; ++r) {
            double dx = 0.0;
            for (int m = FourTriodes::kBlockBegin[r]; m < FourTriodes::kBlockEnd[r]; ++m)
                dx += d9[r][m] * x[m];
            rTh -= root_.Z[rY][r] * dx;
        }
    }
    vTh = root_.v[rY] + rTh * iY;
}

void Preamp::finishSample() noexcept {
    // on failure incident_ = reflected: states unchanged
    stage::incident(elements(), ok_ ? incident_ : reflected_);
    if (ok_)
        updateOutputs(vInNow_);
}

}  // namespace amp2203
