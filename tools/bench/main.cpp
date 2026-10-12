// amp2203_bench — real-time factor of the full engine. M0: synthetic input (the DI clip input of
// SPEC §7 M5.4 comes later).
#include <chrono>
#include <cmath>
#include <cstdio>
#include <numbers>
#include <vector>

#include "amp2203/Amp2203.h"

int main() {
    constexpr double baseRate = 48000.0;
    constexpr int oversampling = 8;
    constexpr double seconds = 60.0;
    constexpr int blockSize = 512;

    amp2203::EngineConfig cfg;
    cfg.sampleRate = baseRate * oversampling;
    cfg.maxBlockSize = blockSize;

    amp2203::Amp2203 amp;
    amp.prepare(cfg);

    const auto total = static_cast<long long>(seconds * cfg.sampleRate);
    std::vector<float> block(blockSize);
    double phase = 0.0;
    const double dphi = 2.0 * std::numbers::pi * 220.0 / cfg.sampleRate;
    double sink = 0.0;

    const auto t0 = std::chrono::steady_clock::now();
    for (long long done = 0; done < total; done += blockSize) {
        for (auto& s : block) {
            s = static_cast<float>(0.1 * std::sin(phase));
            phase += dphi;
        }
        amp.process(block.data(), blockSize);
        sink += block[0];
    }
    const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();

    std::printf("processed %.1f s at %.0f Hz in %.3f s: real-time factor %.1f (checksum %g)\n", seconds,
                cfg.sampleRate, elapsed, seconds / elapsed, sink);
    return 0;
}
