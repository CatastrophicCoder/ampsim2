// amp2203_bench — real-time factor of the full engine, and per-block costs.
//   amp2203_bench [di.wav]   full engine, 60 s at 48 kHz x 16 on a DI clip (SPEC §7 M5.4; default: the first
//                            WAV in reference/di/, else a 220 Hz sine), linearly interpolated to 768 kHz while running
//   options: --factor N (engine rate 48 kHz × N instead of × 16), --hot (full engine: Master, Presence 1.0)
//   amp2203_bench b4         ToneStackMaster: ns per sample (static knobs, knobs moving) and µs per
//                            scattering-matrix update (SPEC §7 M1 acceptance, §6.2)
//   amp2203_bench b12        StageV1aV1b: ns per sample at several input levels, with Preamp Volume
//                            moving, and µs per matrix update; Newton iterations
//   amp2203_bench b3         StageV2aV2b: ns per sample at the B3 levels; Newton iterations
//   amp2203_bench b34        StageV2aV2bToneStack: as b3, plus tone knobs moving and µs per matrix update
//   amp2203_bench preamp     Preamp B1234 (one tree, 8-port root) at the B12 levels
//   amp2203_bench b5         PhaseInverter at the B56 levels (J4 source), NFB input at its DC value
//   amp2203_bench b6         PowerStage driven push-pull at the PI plates
//   amp2203_bench b56        PowerAmp (B56, the engine's PI + power stage, D-041) at the B56 levels
#include <chrono>
#include <filesystem>
#include <string>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <numbers>
#include <utility>
#include <vector>

#include "amp2203/Amp2203.h"
#include "amp2203/PhaseInverter.h"
#include "amp2203/PowerAmp.h"
#include "amp2203/PowerStage.h"
#include "amp2203/Preamp.h"
#include "amp2203/StageV1aV1b.h"
#include "amp2203/StageV2aV2b.h"
#include "amp2203/StageV2aV2bToneStack.h"
#include "amp2203/ToneStackMaster.h"
#include "src/WavIO.h"

namespace {

using Clock = std::chrono::steady_clock;
double kRate = 48000.0 * 16;  // verification / default internal rate (D-035); --factor N: 48 kHz × N
bool gHot = false;            // --hot: Master and Presence at 1.0 (full engine)

double seconds(Clock::time_point t0) { return std::chrono::duration<double>(Clock::now() - t0).count(); }

int benchFull(const std::string& diPath) {
    constexpr double secs = 60.0;
    constexpr int blockSize = 512;
    amp2203::EngineConfig cfg;
    cfg.sampleRate = kRate;
    cfg.maxBlockSize = blockSize;
    amp2203::Amp2203 amp;
    amp.prepare(cfg);
    if (gHot) {
        amp2203::Params p;
        p.master = 1.0;
        p.presence = 1.0;
        amp.setParams(p);
        amp.reset();
    }

    ampwav::MonoWav di;
    std::string err, source = "220 Hz sine at 0.1 FS";
    if (!diPath.empty()) {
        if (!ampwav::readMonoWav(diPath, di, err)) {
            std::fprintf(stderr, "amp2203_bench: %s\n", err.c_str());
            return 1;
        }
        source = diPath + " (" + std::to_string(static_cast<int>(di.sampleRate)) + " Hz, looped if shorter than 60 s)";
    }
    const auto total = static_cast<long long>(secs * cfg.sampleRate);
    std::vector<double> block(blockSize);
    double sink = 0.0;
    long long n = 0;
    const auto t0 = Clock::now();
    for (long long done = 0; done < total; done += blockSize) {
        for (auto& s : block) {
            const double t = static_cast<double>(n++) / cfg.sampleRate;
            if (di.samples.empty()) {
                s = 0.1 * std::sin(2.0 * std::numbers::pi * 220.0 * t);
            } else {  // linear interpolation of the clip to the engine rate
                const double pos = std::fmod(t * di.sampleRate, static_cast<double>(di.samples.size() - 1));
                const auto k = static_cast<size_t>(pos);
                s = di.samples[k] + (pos - static_cast<double>(k)) * (di.samples[k + 1] - di.samples[k]);
            }
        }
        amp.process(block.data(), blockSize);
        sink += block[0];
    }
    const double elapsed = seconds(t0);
    const auto st = amp.getStats();
    std::printf("full engine at %.0f Hz on %s:\n", cfg.sampleRate, source.c_str());
    std::printf("  processed %.1f s in %.3f s: real-time factor %.2f (%.0f ns per sample)\n", secs, elapsed, secs / elapsed,
                elapsed / static_cast<double>(total) * 1e9);
    std::printf("  Newton: %llu solves, median %.0f, max %.0f, failures %llu (rescued %llu)  (checksum %g)\n",
                static_cast<unsigned long long>(st.newtonSolves), st.medianIterations, st.maxIterationsUsed,
                static_cast<unsigned long long>(st.newtonFailures), static_cast<unsigned long long>(st.newtonRescued), sink);
    for (const auto& [name, ns] : {std::pair{"preamp", &st.preamp}, std::pair{"power amp", &st.powerAmp}}) {
        std::printf("  %s iterations:", name);
        for (int k = 0; k <= ns->maxIterations; ++k)
            std::printf(" %d:%.1f%%", k, 100.0 * static_cast<double>(ns->histogram[static_cast<size_t>(k)]) /
                                          static_cast<double>(ns->solves));
        std::printf("\n");
    }
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
    auto make = [] {
        auto b = std::make_unique<amp2203::Preamp>();
        b->prepare(kRate, amp2203::EngineConfig{}.paramSmoothingMs);
        b->reset(0.0);
        return b;
    };
    const double nsRef = levelSweep("Preamp (B1234, 8-port root)", make, 0.0,
                                    {{1000.0, 0.1}, {1000.0, 0.01}, {1000.0, 0.5}, {1000.0, 2.0}, {5000.0, 2.0}}, sink);
    // all five knobs moving all the time (alternate targets every 50 ms), 1 kHz 0.1 V
    auto b = make();
    amp2203::Params p;
    const long n = static_cast<long>(5.0 * kRate);
    const long u0 = b->matrixUpdates();
    const double dphi = 2.0 * std::numbers::pi * 1000.0 / kRate;
    const auto t0 = Clock::now();
    for (long i = 0; i < n; ++i) {
        if (i % static_cast<long>(0.05 * kRate) == 0) {
            const double v = (i / static_cast<long>(0.05 * kRate)) % 2 ? 0.7 : 0.5;
            p.preampVolume = p.treble = p.middle = p.bass = p.master = v;
            b->setParams(p);
        }
        sink += b->process(0.1 * std::sin(dphi * static_cast<double>(i)));
    }
    const double nsMoving = seconds(t0) / static_cast<double>(n) * 1e9;
    const long updates = b->matrixUpdates() - u0;
    std::printf("  moving knobs:   %.1f ns/sample, %ld matrix updates in %ld samples (interval %d)\n", nsMoving, updates,
                n, amp2203::Preamp::kParamUpdateInterval);
    std::printf("  matrix update:  %.2f us (moving minus static 1 kHz 0.1 V, per update)\n",
                (nsMoving - nsRef) * static_cast<double>(n) / static_cast<double>(updates) * 1e-3);
    std::printf("  checksum %g\n", sink);
    return 0;
}

}  // namespace

int benchPhaseInverter() {
    const long n = static_cast<long>(5.0 * kRate);
    double sink = 0.0;
    std::printf("PhaseInverter at %.0f Hz (default knobs):\n", kRate);
    for (const auto& [freq, amp] : {std::pair{1000.0, 0.1}, {1000.0, 1.0}, {1000.0, 5.0}, {1000.0, 20.0}, {5000.0, 20.0}}) {
        amp2203::PhaseInverter st;
        st.prepare(kRate, amp2203::EngineConfig{}.paramSmoothingMs);
        st.setSourceResistance(43000.0);
        st.reset(0.0);
        const double dphi = 2.0 * std::numbers::pi * freq / kRate;
        const auto t0 = Clock::now();
        for (long i = 0; i < n; ++i)
            sink += st.process(amp * std::sin(dphi * static_cast<double>(i)));
        const double ns = seconds(t0) / static_cast<double>(n) * 1e9;
        const auto& s = st.newtonStats();
        std::printf("  %5.0f Hz %5.1f V: %6.1f ns/sample, Newton median %d max %d failures %llu\n", freq, amp, ns,
                    s.medianIterations(), s.maxIterations, static_cast<unsigned long long>(s.failures));
    }
    std::printf("  checksum %g\n", sink);
    return 0;
}

int benchPowerStage() {
    const long n = static_cast<long>(5.0 * kRate);
    double sink = 0.0;
    std::printf("PowerStage at %.0f Hz (push-pull drive at the PI plates):\n", kRate);
    for (const auto& [freq, amp] : {std::pair{1000.0, 1.0}, {1000.0, 20.0}, {1000.0, 60.0}, {220.0, 120.0}, {5000.0, 60.0}}) {
        amp2203::PowerStage st;
        st.prepare(kRate);
        st.reset(270.880925, 262.270582);
        const double dphi = 2.0 * std::numbers::pi * freq / kRate;
        const auto t0 = Clock::now();
        for (long i = 0; i < n; ++i) {
            const double x = amp * std::sin(dphi * static_cast<double>(i));
            sink += st.process(270.880925 + x, 262.270582 - x);
        }
        const double ns = seconds(t0) / static_cast<double>(n) * 1e9;
        const auto& s = st.newtonStats();
        std::printf("  %5.0f Hz %5.1f V: %6.1f ns/sample, Newton median %d max %d failures %llu\n", freq, amp, ns,
                    s.medianIterations(), s.maxIterations, static_cast<unsigned long long>(s.failures));
    }
    std::printf("  checksum %g\n", sink);
    return 0;
}

int benchPowerAmp() {
    const long n = static_cast<long>(5.0 * kRate);
    double sink = 0.0;
    std::printf("PowerAmp (B56, 10-port root) at %.0f Hz (default knobs):\n", kRate);
    for (const auto& [freq, amp] : {std::pair{1000.0, 0.1}, {1000.0, 1.0}, {1000.0, 5.0}, {1000.0, 20.0}, {5000.0, 20.0}}) {
        amp2203::PowerAmp st;
        st.prepare(kRate, amp2203::EngineConfig{}.paramSmoothingMs);
        st.setSourceResistance(43000.0);
        st.reset(0.0);
        const double dphi = 2.0 * std::numbers::pi * freq / kRate;
        const auto t0 = Clock::now();
        for (long i = 0; i < n; ++i)
            sink += st.process(amp * std::sin(dphi * static_cast<double>(i)));
        const double ns = seconds(t0) / static_cast<double>(n) * 1e9;
        const auto& s = st.newtonStats();
        std::printf("  %5.0f Hz %5.1f V: %6.1f ns/sample, Newton median %d max %d failures %llu (rescued %ld)\n", freq,
                    amp, ns, s.medianIterations(), s.maxIterations, static_cast<unsigned long long>(s.failures),
                    st.rescuedSolves());
    }
    std::printf("  checksum %g\n", sink);
    return 0;
}

int main(int argc, char** argv) {
    // options --factor N and --hot, anywhere; the rest as before
    std::vector<char*> args{argv[0]};
    for (int k = 1; k < argc; ++k) {
        if (std::strcmp(argv[k], "--factor") == 0 && k + 1 < argc)
            kRate = 48000.0 * std::atoi(argv[++k]);
        else if (std::strcmp(argv[k], "--hot") == 0)
            gHot = true;
        else
            args.push_back(argv[k]);
    }
    argc = static_cast<int>(args.size());
    argv = args.data();
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
    if (argc > 1 && std::strcmp(argv[1], "b5") == 0)
        return benchPhaseInverter();
    if (argc > 1 && std::strcmp(argv[1], "b6") == 0)
        return benchPowerStage();
    if (argc > 1 && std::strcmp(argv[1], "b56") == 0)
        return benchPowerAmp();
    if (argc > 1 && std::string(argv[1]).find(".wav") == std::string::npos) {
        std::fprintf(stderr, "usage: amp2203_bench [--factor N] [--hot] [di.wav|b4|b12|b3|b34|preamp|b5|b6|b56]\n");
        return 1;
    }
    std::string di;
    if (argc > 1) {
        di = argv[1];
    } else {
        std::error_code ec;
        for (const auto& e : std::filesystem::directory_iterator(AMP2203_DI_DIR, ec))
            if (e.path().extension() == ".wav" && (di.empty() || e.path().string() < di))
                di = e.path().string();
    }
    return benchFull(di);
}
