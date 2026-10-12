#include "amp2203/ToneStackMaster.h"

#include <cmath>

#include "mna/PortNetwork.h"
#include "tonestack/ToneStackNetwork.h"

namespace amp2203 {

namespace tonestack_detail {
double scattering(std::array<double, 12>& R, double (&S)[12][12]) noexcept {
    R[0] = mna::upPortResistance(tonestack::kNetwork, R, tonestack::pUp);
    if (R[0] <= 0.0 || !mna::scattering(tonestack::kNetwork, R, S))
        return -1.0;
    return R[0];
}
}  // namespace tonestack_detail

ToneStackMaster::ToneStackMaster() {
    Params defaults;
    setParams(defaults);
    treble_.snap();
    middle_.snap();
    bass_.snap();
    master_.snap();
    applyKnobs();
}

void ToneStackMaster::prepare(double sampleRate, double smoothingMs) {
    c10_.prepare(sampleRate);
    c11_.prepare(sampleRate);
    c12_.prepare(sampleRate);
    for (auto* s : {&treble_, &middle_, &bass_, &master_})
        s->prepare(sampleRate, smoothingMs);
    applyKnobs();
}

void ToneStackMaster::setParams(const Params& p) noexcept {
    treble_.setTarget(p.treble);
    middle_.setTarget(p.middle);
    bass_.setTarget(p.bass);
    master_.setTarget(p.master);
}

void ToneStackMaster::applyKnobs() noexcept {
    using detail::atap;
    using detail::ltap;
    const double t = treble_.value(), m = middle_.value(), b = bass_.value(), v = master_.value();
    {
        // one scattering-matrix computation for all pots (the R-type recomputes when the scope ends)
        chowdsp::wdft::ScopedDeferImpedancePropagation<RType> defer{rtype_};
        // exactly as the netlist: linear pots without, log pots with the extra 1 Ω (SPEC §6.1)
        vr3a_.setResistanceValue(VR3 * (1.0 - ltap(t)));
        vr3b_.setResistanceValue(VR3 * ltap(t));
        vr5_.setResistanceValue(VR5 * atap(b) + 1.0);
        vr4a_.setResistanceValue(VR4 * (1.0 - ltap(m)));
        vr4b_.setResistanceValue(VR4 * ltap(m));
        vr2a_.setResistanceValue(VR2 * (1.0 - atap(v)) + 1.0);
        vr2b_.setResistanceValue(VR2 * atap(v) + 1.0);
    }
    series_.propagateImpedanceChange();
    ++updates_;

    const double r2a = VR2 * (1.0 - atap(v)) + 1.0, r2b = VR2 * atap(v) + 1.0;
    const double chain = r2a + VR3 * ltap(t) + (VR5 * atap(b) + 1.0) + VR4;
    rth4_ = 1.0 / (1.0 / r2b + 1.0 / chain);
}

void ToneStackMaster::reset(double vInDc) {
    treble_.snap();
    middle_.snap();
    bass_.snap();
    master_.snap();
    applyKnobs();
    sinceUpdate_ = 0;

    // DC operating point: J3 source as a Norton equivalent on the up-port, capacitors open
    using namespace tonestack;
    std::array<double, kNumPorts> G{}, I{};
    G[pUp] = 1.0 / kRth3;
    I[pUp] = vInDc / kRth3;
    G[pR15] = 1.0 / R15;
    G[pVR3a] = 1.0 / vr3a_.wdf.R;
    G[pVR3b] = 1.0 / vr3b_.wdf.R;
    G[pVR5] = 1.0 / vr5_.wdf.R;
    G[pVR4a] = 1.0 / vr4a_.wdf.R;
    G[pVR4b] = 1.0 / vr4b_.wdf.R;
    G[pVR2a] = 1.0 / vr2a_.wdf.R;
    G[pVR2b] = 1.0 / vr2b_.wdf.R;
    std::array<double, kNumNodes> v{};
    mna::solveDc(kNetwork, G, I, v);
    c10_.setDcVoltage(v[Node::k2b] - v[Node::tt]);
    c11_.setDcVoltage(v[Node::nD] - v[Node::nE]);
    c12_.setDcVoltage(v[Node::nD] - v[Node::mw]);
}

double ToneStackMaster::process(double vIn) noexcept {
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

    vIn_ = vIn;
    source_.setVoltage(vIn);
    source_.incident(inverter_.reflected());
    inverter_.incident(source_.reflected());
    updateProbes();
    return y_;
}

void ToneStackMaster::updateProbes() noexcept {
    using chowdsp::wdft::voltage;
    k2b_ = voltage<double>(rtype_);
    tt_ = k2b_ - voltage<double>(c10_);
    nd_ = k2b_ - voltage<double>(r15_);
    ne_ = nd_ - voltage<double>(c11_);
    mw_ = voltage<double>(vr4b_);
    tw_ = tt_ - voltage<double>(vr3a_);
    nbb_ = mw_ + voltage<double>(vr4a_);
    y_ = voltage<double>(vr2b_);
}

}  // namespace amp2203
