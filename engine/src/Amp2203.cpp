#include "amp2203/Amp2203.h"

#include <cmath>
#include <cstring>
#include <initializer_list>
#include <utility>

#include "amp2203/PowerAmp.h"
#include "amp2203/Preamp.h"
#include "amp2203/Supply.h"

namespace amp2203 {

namespace {

constexpr int kDcIterations = 50;
constexpr double kDcTolerance = 1e-9;  // V, supply node change between fixed-point rounds
constexpr double kOpenLoopR22 = 1e12;  // Ω: "R22 → 1 TΩ" (SPEC §7 M4 NFB check)
constexpr double kOpenJ4 = 1e12;       // Ω: the J4 element source in the exact-J4 mode (M6 X1)

/// Appends `src` to `dst`, skipping `skip` and renaming per `rename` ({from, to} pairs).
void appendProbes(std::vector<Probe>& dst, std::span<const Probe> src, std::initializer_list<const char*> skip,
                  std::initializer_list<std::pair<const char*, const char*>> rename = {}) {
    for (const auto& p : src) {
        bool skipped = false;
        for (const char* s : skip)
            skipped = skipped || std::strcmp(p.name, s) == 0;
        if (skipped)
            continue;
        Probe q = p;
        for (const auto& [from, to] : rename)
            if (std::strcmp(p.name, from) == 0)
                q.name = to;
        dst.push_back(q);
    }
}

}  // namespace

Amp2203::Amp2203()
    : preamp_(std::make_unique<Preamp>()), power_(std::make_unique<PowerAmp>()), supply_(std::make_unique<Supply>()) {
    // one probe list with netlist names: the power amp's y is the loaded master wiper (as F's y), the
    // preamp's open-circuit wiper is y_oc; supply nodes come from the supply; per-block Newton iterations
    // and supply currents get a block suffix
    probes_.push_back({"in", &vIn_});
    probes_.push_back({"out", &vOut_});
    appendProbes(probes_, preamp_->probes(), {"in", "out", "bpa", "bpx"},
                 {{"y", "y_oc"}, {"newton_iterations", "newton_iterations_preamp"},
                  {"isupply_bpa", "isupply_bpa_preamp"}, {"isupply_bpx", "isupply_bpx_preamp"}});
    appendProbes(probes_, power_->probes(), {"in", "out", "bpx", "nscr", "ct", "isupply_ct", "isupply_scr"},
                 {{"newton_iterations", "newton_iterations_power"}, {"isupply_bpx", "isupply_bpx_power"},
                  {"rth4", "rth4_power"}});
    appendProbes(probes_, supply_->probes(), {});
    prepare(cfg_);
}

Amp2203::~Amp2203() = default;

void Amp2203::prepare(const EngineConfig& cfg) {
    cfg_ = cfg;
    preamp_->prepare(cfg.sampleRate, cfg.paramSmoothingMs);
    if (cfg_.powerAmpRateDivisor != 2)
        cfg_.powerAmpRateDivisor = 1;
    power_->prepare(cfg.sampleRate / cfg_.powerAmpRateDivisor, cfg.paramSmoothingMs);
    supply_->prepare(cfg.sampleRate);
    reset();
}

void Amp2203::setParams(const Params& p) {
    params_ = p;
    preamp_->setParams(p);
    power_->setParams(p);
}

void Amp2203::setFeedbackClosed(bool closed) { power_->setFeedbackResistance(closed ? PhaseInverter::R22 : kOpenLoopR22); }

void Amp2203::setHalfStepTrigger(double volts) { power_->setHalfStepTrigger(volts); }

void Amp2203::setNewtonResidualTolerance(double volts) {
    preamp_->setNewtonResidualTolerance(volts);
    power_->setNewtonResidualTolerance(volts);
}

void Amp2203::applySupplies() noexcept {
    preamp_->setSupplyVoltages(supply_->bpA(), supply_->bpX());
    power_->setSupplyVoltages(supply_->bpX(), supply_->screen(), supply_->ct());
}

void Amp2203::solveDc() {
    // fixed point (WDF_DESIGN §7): blocks at the supply voltages → their DC currents → supply DC
    supply_->reset({0.136, 0.018, 0.007, 0.0013});  // CIRCUIT.md §5 currents as the start
    for (int k = 0; k < kDcIterations; ++k) {
        const double before[4] = {supply_->ct(), supply_->screen(), supply_->bpX(), supply_->bpA()};
        applySupplies();
        preamp_->reset(0.0);
        if (exactJ4()) {  // J4 element open; the preamp on the root port (at DC no current through C9)
            power_->setSourceResistance(kOpenJ4);
            power_->setPreampPort(preamp_->output(), 1.0 / preamp_->rth4());
            power_->reset(0.0);
        } else {
            power_->setPreampPort(0.0, 0.0);
            power_->setSourceResistance(preamp_->rth4());
            power_->reset(preamp_->output());
        }
        supply_->reset({power_->centreTapCurrent(), power_->screenCurrent(),
                        preamp_->supplyCurrentX() + power_->supplyCurrentX(), preamp_->supplyCurrentA()});
        const double after[4] = {supply_->ct(), supply_->screen(), supply_->bpX(), supply_->bpA()};
        double change = 0.0;
        for (int i = 0; i < 4; ++i)
            change = std::fmax(change, std::fabs(after[i] - before[i]));
        if (change < kDcTolerance)
            break;
    }
    applySupplies();
    vOut_ = power_->output();
    iY_ = 0.0;
    // rate-change filters at the DC point: the decimator holds the preamp output, the interpolator the
    // zero-stuffed power amp output (2·v at the samples where the power amp runs, 0 between; both polyphase
    // branches have unity DC gain)
    decim_.fill(preamp_->output());
    for (int n = 0; n < Halfband::kTaps; ++n)  // newest (n = kTaps − 1, even) nonzero: the next sample is a 0
        interp_.push(n % 2 == 0 ? 2.0 * vOut_ : 0.0);
    powerPhase_ = false;
}

void Amp2203::reset() {
    vIn_ = 0.0;
    solveDc();
    preamp_->clearStats();
    power_->clearStats();
}

double Amp2203::processSample(double x) noexcept {
    vIn_ = x * cfg_.inputVoltsPerFullScale;
    applySupplies();
    if (exactJ4()) {
        // preamp with the last load current, its linearised Thevenin source at y, the power amp against it,
        // then the preamp again with the power amp's actual load current. Power amp at half rate: the same at
        // the samples where it runs (the Thevenin source taken directly, no decimation filter in the loop),
        // the preamp alone with the held load current in between; the output through the interpolator.
        preamp_->beginSample(vIn_);
        preamp_->solveLoaded(iY_);
        const bool runPower = cfg_.powerAmpRateDivisor == 1 || powerPhase_;
        double u = 0.0;
        if (runPower) {
            double vTh = 0.0, rTh = 0.0;
            preamp_->theveninAtY(iY_, vTh, rTh);
            power_->setPreampPort(vTh, 1.0 / rTh);
            u = power_->process(0.0);
            iY_ = power_->preampPortCurrent();
            preamp_->solveLoaded(iY_);
        }
        preamp_->finishSample();
        if (cfg_.powerAmpRateDivisor == 1) {
            vOut_ = u;
        } else {
            interp_.push(runPower ? 2.0 * u : 0.0);
            vOut_ = interp_.dot(halfband_.h);
            powerPhase_ = !powerPhase_;
        }
        supply_->process({power_->centreTapCurrent(), power_->screenCurrent(),
                          preamp_->supplyCurrentX() + power_->supplyCurrentX(), preamp_->supplyCurrentA()});
        const double o = vOut_ / cfg_.outputVoltsPerFullScale;
        return std::isfinite(o) ? o : 0.0;
    }
    const double y = preamp_->process(vIn_);
    if (cfg_.powerAmpRateDivisor == 1) {
        power_->setSourceResistance(preamp_->rth4());
        vOut_ = power_->process(y);
    } else {
        decim_.push(y);
        double u = 0.0;
        if (powerPhase_) {
            power_->setSourceResistance(preamp_->rth4());
            u = 2.0 * power_->process(decim_.dot(halfband_.h));
        }
        interp_.push(u);
        vOut_ = interp_.dot(halfband_.h);
        powerPhase_ = !powerPhase_;
    }
    supply_->process({power_->centreTapCurrent(), power_->screenCurrent(),
                      preamp_->supplyCurrentX() + power_->supplyCurrentX(), preamp_->supplyCurrentA()});
    const double out = vOut_ / cfg_.outputVoltsPerFullScale;
    return std::isfinite(out) ? out : 0.0;
}

void Amp2203::process(float* mono, int numSamples) noexcept {
    for (int n = 0; n < numSamples; ++n)
        mono[n] = static_cast<float>(processSample(static_cast<double>(mono[n])));
}

void Amp2203::process(double* mono, int numSamples) noexcept {
    for (int n = 0; n < numSamples; ++n)
        mono[n] = processSample(mono[n]);
}

Amp2203::Stats Amp2203::getStats() const {
    detail::NewtonStats all = preamp_->newtonStats();
    all.add(power_->newtonStats());
    Stats s;
    s.newtonFailures = all.failures;
    s.maxIterationsUsed = all.maxIterations;
    s.newtonSolves = all.solves;
    s.medianIterations = all.medianIterations();
    s.newtonRescued = static_cast<uint64_t>(preamp_->rescuedSolves() + power_->rescuedSolves());
    s.newtonUnrescued = static_cast<uint64_t>(preamp_->unrescuedSolves() + power_->unrescuedSolves());
    s.preamp = preamp_->newtonStats();
    s.powerAmp = power_->newtonStats();
    return s;
}

std::span<const Probe> Amp2203::probes() const noexcept { return probes_; }

}  // namespace amp2203
