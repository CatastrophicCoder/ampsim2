#include "amp2203/Preamp.h"

#include <cstring>

namespace amp2203 {

namespace {

struct Rename {
    const char* from;
    const char* to;  // nullptr: not exposed by the cascade
};

const char* renamed(const char* name, std::span<const Rename> table) noexcept {
    for (const auto& r : table)
        if (std::strcmp(name, r.from) == 0)
            return r.to;
    return name;
}

constexpr Rename kB12[] = {{"out", nullptr}, {"p1b", "voc_p1b"}, {"isupply", "isupply_bpa"},
                           {"newton_iterations", "newton_iterations_b12"}};
constexpr Rename kB34[] = {{"in", nullptr}, {"out", nullptr}, {"isupply", "isupply_bpx"},
                           {"newton_iterations", "newton_iterations_b34"}};

}  // namespace

Preamp::Preamp() {
    auto add = [this](std::span<const Probe> probes, std::span<const Rename> table) {
        for (const auto& p : probes)
            if (const char* name = renamed(p.name, table); name != nullptr && numProbes_ < probes_.size())
                probes_[numProbes_++] = {name, p.value};
    };
    add(b12_.probes(), kB12);
    add(b34_.probes(), kB34);
    probes_[numProbes_++] = {"out", &y_};
}

void Preamp::prepare(double sampleRate, double smoothingMs) {
    b12_.prepare(sampleRate, smoothingMs);
    b34_.prepare(sampleRate, smoothingMs);
}

void Preamp::setParams(const Params& p) noexcept {
    b12_.setParams(p);
    b34_.setParams(p);
}

void Preamp::reset(double vInDc) {
    b12_.reset(vInDc);
    b34_.reset(b12_.output());
    y_ = b34_.output();
    clearStats();
}

detail::NewtonStats Preamp::newtonStats() const noexcept {
    detail::NewtonStats s = b12_.newtonStats();
    const auto& t = b34_.newtonStats();
    for (size_t k = 0; k < s.histogram.size(); ++k)
        s.histogram[k] += t.histogram[k];
    s.solves += t.solves;
    s.failures += t.failures;
    s.maxIterations = s.maxIterations > t.maxIterations ? s.maxIterations : t.maxIterations;
    return s;
}

void Preamp::clearStats() noexcept {
    b12_.clearStats();
    b34_.clearStats();
}

}  // namespace amp2203
