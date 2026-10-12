// amp2203_bench — real-time factor of the full engine, and per-block costs.
//   amp2203_bench            full engine, 60 s at 48 kHz x 8 (M0: synthetic input; DI clips in M5.4)
//   amp2203_bench b4         ToneStackMaster: ns per sample (static knobs, knobs moving) and µs per
//                            scattering-matrix update (SPEC §7 M1 acceptance, §6.2)
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <numbers>
#include <vector>

#include "amp2203/Amp2203.h"
#include "amp2203/ToneStackMaster.h"

namespace {

using Clock = std::chrono::steady_clock;
constexpr double kRate = 48000.0 * 8;

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

}  // namespace

int main(int argc, char** argv) {
    if (argc > 1 && std::strcmp(argv[1], "b4") == 0)
        return benchToneStack();
    if (argc > 1) {
        std::fprintf(stderr, "usage: amp2203_bench [b4]\n");
        return 1;
    }
    return benchFull();
}
