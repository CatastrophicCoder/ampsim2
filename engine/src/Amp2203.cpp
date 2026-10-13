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
    power_->prepare(cfg.sampleRate, cfg.paramSmoothingMs);
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
        power_->setSourceResistance(preamp_->rth4());
        power_->reset(preamp_->output());
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
    const double y = preamp_->process(vIn_);
    power_->setSourceResistance(preamp_->rth4());
    vOut_ = power_->process(y);
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
    return s;
}

std::span<const Probe> Amp2203::probes() const noexcept { return probes_; }

}  // namespace amp2203
