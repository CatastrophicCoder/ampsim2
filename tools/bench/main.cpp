// amp2203_bench — real-time factor of the full engine, and per-block costs.
//   amp2203_bench            full engine, 60 s at 48 kHz x 16 (M0: synthetic input; DI clips in M5.4)
//   amp2203_bench b4         ToneStackMaster: ns per sample (static knobs, knobs moving) and µs per
//                            scattering-matrix update (SPEC §7 M1 acceptance, §6.2)
//   amp2203_bench b12        StageV1aV1b: ns per sample at several input levels, with Preamp Volume
//                            moving, and µs per matrix update; Newton iterations
//   amp2203_bench b3         StageV2aV2b: ns per sample at the B3 levels; Newton iterations
//   amp2203_bench b34        StageV2aV2bToneStack: as b3, plus tone knobs moving and µs per matrix update
//   amp2203_bench preamp     Preamp cascade B12 -> B34 at the B12 levels
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <numbers>
#include <utility>
#include <vector>

#include "amp2203/Amp2203.h"
#include "amp2203/Preamp.h"
#include "amp2203/StageV1aV1b.h"
#include "amp2203/StageV2aV2b.h"
#include "amp2203/StageV2aV2bToneStack.h"
#include "amp2203/ToneStackMaster.h"

namespace {

using Clock = std::chrono::steady_clock;
constexpr double kRate = 48000.0 * 16;  // verification / default internal rate (D-035)

double seconds(Clock::time_point t0) { return std::chrono::duration<double>(Clock::now() - t0).count(); }

int benchFull() {
    constexpr double secs = 60.0;
    constexpr int blockSize = 512;
    amp2203::EngineConfig cfg;
    cfg.sampleRate = kRate;
    cfg.maxBlockSize = blockSize;
    amp2203::Amp2203 amp;
    amp.prepare(cfg);

    const auto total = static_cast<long long>(secs * cfg.sampleRate);
    std::vector<float> block(blockSize);
    double phase = 0.0;
    const double dphi = 2.0 * std::numbers::pi * 220.0 / cfg.sampleRate;
    double sink = 0.0;
    const auto t0 = Clock::now();
    for (long long done = 0; done < total; done += blockSize) {
        for (auto& s : block) {
            s = static_cast<float>(0.1 * std::sin(phase));
            phase += dphi;
        }
        amp.process(block.data(), blockSize);
        sink += block[0];
    }
    const double elapsed = seconds(t0);
    std::printf("processed %.1f s at %.0f Hz in %.3f s: real-time factor %.1f (checksum %g)\n", secs,
                cfg.sampleRate, elapsed, secs / elapsed, sink);
    return 0;
}

int benchToneStack() {
    amp2203::ToneStackMaster ts;
    ts.prepare(kRate, amp2203::EngineConfig{}.paramSmoothingMs);
    ts.reset(231.6858);
    const long n = static_cast<long>(10.0 * kRate);
    const double dphi = 2.0 * std::numbers::pi * 1000.0 / kRate;
    double sink = 0.0;

    auto t0 = Clock::now();
    for (long i = 0; i < n; ++i)
        sink += ts.process(231.6858 + std::sin(dphi * static_cast<double>(i)));
    const double nsStatic = seconds(t0) / static_cast<double>(n) * 1e9;

    // knobs moving all the time: alternate targets every 50 ms so the smoothers never settle
    amp2203::Params p;
    const long u0 = ts.matrixUpdates();
    t0 = Clock::now();
    for (long i = 0; i < n; ++i) {
        if (i % static_cast<long>(0.05 * kRate) == 0) {
            const double v = (i / static_cast<long>(0.05 * kRate)) % 2 ? 0.9 : 0.1;
            p.treble = p.middle = p.bass = p.master = v;
            ts.setParams(p);
        }
        sink += ts.process(231.6858 + std::sin(dphi * static_cast<double>(i)));
    }
    const double nsMoving = seconds(t0) / static_cast<double>(n) * 1e9;
    const long updates = ts.matrixUpdates() - u0;

    // isolated matrix updates: reset() performs one (plus a DC solve); measure many
    constexpr int kResets = 20000;
    t0 = Clock::now();
    for (int i = 0; i < kResets; ++i) {
        p.treble = (i % 100) / 99.0;
        ts.setParams(p);
        ts.reset(231.6858);
    }
    const double usReset = seconds(t0) / kResets * 1e6;

    std::printf("ToneStackMaster at %.0f Hz:\n", kRate);
    std::printf("  static knobs:  %.1f ns/sample (real-time factor %.0f)\n", nsStatic, 1e9 / nsStatic / kRate);
    std::printf("  moving knobs:  %.1f ns/sample, %ld matrix updates in %ld samples (interval %d)\n", nsMoving,
                updates, n, amp2203::ToneStackMaster::kParamUpdateInterval);
    std::printf("  matrix update: %.2f us (reset(): update + DC solve)\n", usReset);
    std::printf("  checksum %g\n", sink);
    return 0;
}

int benchStage12() {
    const long n = static_cast<long>(5.0 * kRate);
    double sink = 0.0;
    std::printf("StageV1aV1b at %.0f Hz (Preamp Volume 0.6 unless moving):\n", kRate);
    double nsRef = 0.0;
    for (const auto& [freq, amp] : {std::pair{1000.0, 0.01}, {1000.0, 0.1}, {1000.0, 0.5}, {1000.0, 2.0}, {5000.0, 2.0}}) {
        amp2203::StageV1aV1b st;
        st.prepare(kRate, amp2203::EngineConfig{}.paramSmoothingMs);
        st.reset(0.0);
        const double dphi = 2.0 * std::numbers::pi * freq / kRate;
        const auto t0 = Clock::now();
        for (long i = 0; i < n; ++i)
            sink += st.process(amp * std::sin(dphi * static_cast<double>(i)));
        const double ns = seconds(t0) / static_cast<double>(n) * 1e9;
        if (freq == 1000.0 && amp == 0.1)
            nsRef = ns;
        const auto& s = st.newtonStats();
        std::printf("  %5.0f Hz %4.2f V: %6.1f ns/sample, Newton median %d max %d failures %llu\n", freq, amp, ns,
                    s.medianIterations(), s.maxIterations, static_cast<unsigned long long>(s.failures));
    }

    // Preamp Volume moving all the time (alternate targets every 50 ms), 1 kHz 0.1 V
    amp2203::StageV1aV1b st;
    st.prepare(kRate, amp2203::EngineConfig{}.paramSmoothingMs);
    st.reset(0.0);
    amp2203::Params p;
    const long u0 = st.matrixUpdates();
    const double dphi = 2.0 * std::numbers::pi * 1000.0 / kRate;
    const auto t0 = Clock::now();
    for (long i = 0; i < n; ++i) {
        if (i % static_cast<long>(0.05 * kRate) == 0) {
            p.preampVolume = (i / static_cast<long>(0.05 * kRate)) % 2 ? 0.7 : 0.5;
            st.setParams(p);
        }
        sink += st.process(0.1 * std::sin(dphi * static_cast<double>(i)));
    }
    const double nsMoving = seconds(t0) / static_cast<double>(n) * 1e9;
    const long updates = st.matrixUpdates() - u0;
    std::printf("  moving volume:  %.1f ns/sample, %ld matrix updates in %ld samples (interval %d)\n", nsMoving,
                updates, n, amp2203::StageV1aV1b::kParamUpdateInterval);
    std::printf("  matrix update:  %.2f us (moving minus static 1 kHz 0.1 V, per update)\n",
                (nsMoving - nsRef) * static_cast<double>(n) / static_cast<double>(updates) * 1e-3);
    std::printf("  checksum %g\n", sink);
    return 0;
}

/// ns per sample of `make()`'s block on sines at (frequency, amplitude) around `dc`, with its Newton stats.
template <class Make>
double levelSweep(const char* title, Make make, double dc, std::initializer_list<std::pair<double, double>> cases,
                  double& sink) {
    const long n = static_cast<long>(5.0 * kRate);
    std::printf("%s at %.0f Hz:\n", title, kRate);
    double first = 0.0;
    for (const auto& [freq, amp] : cases) {
        auto block = make();
        const double dphi = 2.0 * std::numbers::pi * freq / kRate;
        const auto t0 = Clock::now();
        for (long i = 0; i < n; ++i)
            sink += block->process(dc + amp * std::sin(dphi * static_cast<double>(i)));
        const double ns = seconds(t0) / static_cast<double>(n) * 1e9;
        first = first == 0.0 ? ns : first;
        const auto s = block->newtonStats();
        std::printf("  %5.0f Hz %5.2f V: %6.1f ns/sample, Newton median %d max %d failures %llu\n", freq, amp, ns,
                    s.medianIterations(), s.maxIterations, static_cast<unsigned long long>(s.failures));
    }
    return first;
}

constexpr double kVp1b = 352.432789;  // CIRCUIT.md §5: J2 source DC level

int benchStage3() {
    double sink = 0.0;
    levelSweep("StageV2aV2b", [] {
        auto b = std::make_unique<amp2203::StageV2aV2b>();
        b->prepare(kRate);
        b->reset(kVp1b);
        return b;
    }, kVp1b, {{1000.0, 0.1}, {1000.0, 1.0}, {1000.0, 10.0}, {1000.0, 50.0}, {5000.0, 50.0}}, sink);
    std::printf("  checksum %g\n", sink);
    return 0;
}

int benchStage34() {
    double sink = 0.0;
    auto make = [] {
        auto b = std::make_unique<amp2203::StageV2aV2bToneStack>();
        b->prepare(kRate, amp2203::EngineConfig{}.paramSmoothingMs);
        b->reset(kVp1b);
        return b;
    };
    const double nsRef = levelSweep("StageV2aV2bToneStack", make, kVp1b,
                                    {{1000.0, 1.0}, {1000.0, 0.1}, {1000.0, 10.0}, {1000.0, 50.0}, {5000.0, 50.0}}, sink);
    // tone knobs moving all the time (alternate targets every 50 ms), 1 kHz 1 V
    auto b = make();
    amp2203::Params p;
    const long n = static_cast<long>(5.0 * kRate);
    const long u0 = b->matrixUpdates();
    const double dphi = 2.0 * std::numbers::pi * 1000.0 / kRate;
    const auto t0 = Clock::now();
    for (long i = 0; i < n; ++i) {
        if (i % static_cast<long>(0.05 * kRate) == 0) {
            const double v = (i / static_cast<long>(0.05 * kRate)) % 2 ? 0.9 : 0.1;
            p.treble = p.middle = p.bass = p.master = v;
            b->setParams(p);
        }
        sink += b->process(kVp1b + std::sin(dphi * static_cast<double>(i)));
    }
    const double nsMoving = seconds(t0) / static_cast<double>(n) * 1e9;
    const long updates = b->matrixUpdates() - u0;
    std::printf("  moving knobs:   %.1f ns/sample, %ld matrix updates in %ld samples (interval %d)\n", nsMoving, updates,
                n, amp2203::StageV2aV2bToneStack::kParamUpdateInterval);
    std::printf("  matrix update:  %.2f us (moving minus static 1 kHz 1 V, per update)\n",
                (nsMoving - nsRef) * static_cast<double>(n) / static_cast<double>(updates) * 1e-3);
    std::printf("  checksum %g\n", sink);
    return 0;
}

int benchPreamp() {
    double sink = 0.0;
    levelSweep("Preamp (B12 -> B34)", [] {
        auto b = std::make_unique<amp2203::Preamp>();
        b->prepare(kRate, amp2203::EngineConfig{}.paramSmoothingMs);
        b->reset(0.0);
        return b;
    }, 0.0, {{1000.0, 0.01}, {1000.0, 0.1}, {1000.0, 0.5}, {1000.0, 2.0}, {5000.0, 2.0}}, sink);
    std::printf("  checksum %g\n", sink);
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc > 1 && std::strcmp(argv[1], "b4") == 0)
        return benchToneStack();
    if (argc > 1 && std::strcmp(argv[1], "b12") == 0)
        return benchStage12();
    if (argc > 1 && std::strcmp(argv[1], "b3") == 0)
        return benchStage3();
    if (argc > 1 && std::strcmp(argv[1], "b34") == 0)
        return benchStage34();
    if (argc > 1 && std::strcmp(argv[1], "preamp") == 0)
        return benchPreamp();
    if (argc > 1) {
        std::fprintf(stderr, "usage: amp2203_bench [b4|b12|b3|b34|preamp]\n");
        return 1;
    }
    return benchFull();
}
