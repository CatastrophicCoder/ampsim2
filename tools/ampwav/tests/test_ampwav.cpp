#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <dr_wav.h>

#include <chrono>
#include <cmath>
#include <filesystem>
#include <numbers>
#include <sstream>

#include "src/Cli.h"
#include "src/Runner.h"
#include "src/WavIO.h"

namespace fs = std::filesystem;
using Catch::Matchers::WithinAbs;

namespace {

struct TempDir {
    fs::path path;
    TempDir() {
        path = fs::temp_directory_path() /
               ("ampwav_test_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }
    ~TempDir() { fs::remove_all(path); }
    std::string file(const char* name) const { return (path / name).string(); }
};

std::vector<double> testSignal(size_t n) {
    std::vector<double> x(n);
    for (size_t i = 0; i < n; ++i)
        x[i] = 0.3 * std::sin(2.0 * std::numbers::pi * 1000.0 * static_cast<double>(i) / 384000.0) + 1e-17 * static_cast<double>(i);
    return x;
}

std::optional<ampwav::Options> parse(std::vector<std::string> args, std::string& err) {
    return ampwav::parseArgs(args, err);
}

}  // namespace

TEST_CASE("WAV: 64-bit float round trip is bit-exact", "[ampwav]") {
    TempDir dir;
    const auto x = testSignal(4096);
    std::string err;
    REQUIRE(ampwav::writeMonoWavF64(dir.file("a.wav"), 384000.0, x, err));

    ampwav::MonoWav in;
    REQUIRE(ampwav::readMonoWav(dir.file("a.wav"), in, err));
    REQUIRE(in.sampleRate == 384000.0);
    REQUIRE(in.samples == x);
}

TEST_CASE("WAV: 16-bit PCM is read and scaled to +-1", "[ampwav]") {
    TempDir dir;
    drwav_data_format fmt{drwav_container_riff, DR_WAVE_FORMAT_PCM, 1, 48000, 16};
    drwav wav;
    REQUIRE(drwav_init_file_write(&wav, dir.file("p.wav").c_str(), &fmt, nullptr));
    const drwav_int16 pcm[] = {0, 16384, -32768};
    REQUIRE(drwav_write_pcm_frames(&wav, 3, pcm) == 3);
    drwav_uninit(&wav);

    ampwav::MonoWav in;
    std::string err;
    REQUIRE(ampwav::readMonoWav(dir.file("p.wav"), in, err));
    REQUIRE(in.sampleRate == 48000.0);
    REQUIRE(in.samples == std::vector<double>{0.0, 0.5, -1.0});
}

TEST_CASE("WAV: stereo input is rejected", "[ampwav]") {
    TempDir dir;
    drwav_data_format fmt{drwav_container_riff, DR_WAVE_FORMAT_IEEE_FLOAT, 2, 384000, 32};
    drwav wav;
    REQUIRE(drwav_init_file_write(&wav, dir.file("s.wav").c_str(), &fmt, nullptr));
    const float frames[] = {0.f, 0.f, 0.1f, 0.2f};
    REQUIRE(drwav_write_pcm_frames(&wav, 2, frames) == 2);
    drwav_uninit(&wav);

    ampwav::MonoWav in;
    std::string err;
    REQUIRE_FALSE(ampwav::readMonoWav(dir.file("s.wav"), in, err));
    REQUIRE(err.find("mono") != std::string::npos);
}

TEST_CASE("CLI: valid command line", "[ampwav]") {
    std::string err;
    const auto o = parse({"--block", "B12", "--in", "i.wav", "--out", "o.wav", "--rate", "384000", "--param",
                          "preampVolume=0.8", "--param", "presence=1", "--dump-node", "out=d.wav"},
                         err);
    REQUIRE(o);
    REQUIRE(o->block == "B12");
    REQUIRE(o->rate == 384000.0);
    REQUIRE(o->params.preampVolume == 0.8);
    REQUIRE(o->params.presence == 1.0);
    REQUIRE(o->params.master == amp2203::Params{}.master);
    REQUIRE(o->dumps.size() == 1);
    REQUIRE(o->dumps[0].node == "out");
    REQUIRE(o->dumps[0].path == "d.wav");
}

TEST_CASE("CLI: invalid command lines are rejected with a message", "[ampwav]") {
    const std::vector<std::vector<std::string>> bad = {
        {"--in", "i.wav"},                                                  // missing --out
        {"--block", "b9", "--in", "i.wav", "--out", "o.wav"},              // unknown block
        {"--in", "i.wav", "--out", "o.wav", "--param", "gain=0.5"},       // unknown knob
        {"--in", "i.wav", "--out", "o.wav", "--param", "treble=1.5"},     // out of range
        {"--in", "i.wav", "--out", "o.wav", "--param", "treble"},         // no '='
        {"--in", "i.wav", "--out", "o.wav", "--dump-node", "p1a"},        // no '='
        {"--in", "i.wav", "--out", "o.wav", "--rate", "fast"},            // not a number
        {"--in", "i.wav", "--out", "o.wav", "--frobnicate"},              // unknown option
        {"--block", "b4", "--in", "i.wav", "--out", "o.wav", "--units", "fs"},  // fs only for full
        {"--in", "i.wav", "--out"},                                         // missing value
    };
    for (const auto& args : bad) {
        std::string err;
        CHECK_FALSE(parse(args, err));
        CHECK_FALSE(err.empty());
    }
}

TEST_CASE("Runner: every block id resolves; full is the identity in volts", "[ampwav]") {
    for (const auto& id : ampwav::blockIds())
        REQUIRE(ampwav::makeRunner(id));
    REQUIRE_FALSE(ampwav::makeRunner("b8"));

    auto r = ampwav::makeRunner("full");
    r->prepare(384000.0, 64);
    double v[] = {0.1, -2.0, 0.0};
    r->processVolts(v, 3);
    REQUIRE_THAT(v[0], WithinAbs(0.1, 1e-15));
    REQUIRE_THAT(v[1], WithinAbs(-2.0, 1e-15));
}

TEST_CASE("CLI end to end: output, node dump and stats", "[ampwav]") {
    TempDir dir;
    const auto x = testSignal(2000);
    std::string err;
    REQUIRE(ampwav::writeMonoWavF64(dir.file("in.wav"), 384000.0, x, err));

    for (const char* block : {"full", "b12"}) {
        INFO("block " << block);
        const auto o = parse({"--block", block, "--in", dir.file("in.wav"), "--out", dir.file("out.wav"), "--rate",
                              "384000", "--dump-node", "in=" + dir.file("node_in.wav"), "--stats",
                              dir.file("stats.json")},
                             err);
        REQUIRE(o);
        std::ostringstream log;
        REQUIRE(ampwav::run(*o, log) == 0);

        ampwav::MonoWav out, nodeIn;
        REQUIRE(ampwav::readMonoWav(dir.file("out.wav"), out, err));
        REQUIRE(ampwav::readMonoWav(dir.file("node_in.wav"), nodeIn, err));
        REQUIRE(out.samples.size() == x.size());
        for (size_t i = 0; i < x.size(); ++i) {
            REQUIRE_THAT(out.samples[i], WithinAbs(x[i], 1e-15));
            REQUIRE_THAT(nodeIn.samples[i], WithinAbs(x[i], 1e-15));
        }
        REQUIRE(fs::file_size(dir.file("stats.json")) > 0);
        REQUIRE(log.str().find("ns/sample=") != std::string::npos);
    }
}

TEST_CASE("CLI end to end: rate mismatch and unknown node fail", "[ampwav]") {
    TempDir dir;
    std::string err;
    REQUIRE(ampwav::writeMonoWavF64(dir.file("in.wav"), 48000.0, testSignal(100), err));
    std::ostringstream log;

    auto o = parse({"--in", dir.file("in.wav"), "--out", dir.file("o.wav"), "--rate", "384000"}, err);
    REQUIRE(o);
    REQUIRE(ampwav::run(*o, log) == 1);
    REQUIRE(log.str().find("does not resample") != std::string::npos);

    o = parse({"--in", dir.file("in.wav"), "--out", dir.file("o.wav"), "--dump-node", "p9=x.wav"}, err);
    REQUIRE(o);
    REQUIRE(ampwav::run(*o, log) == 1);
    REQUIRE(log.str().find("no node 'p9'") != std::string::npos);
}

TEST_CASE("CLI end to end: full-scale units apply the engine calibration", "[ampwav]") {
    TempDir dir;
    std::string err;
    REQUIRE(ampwav::writeMonoWavF64(dir.file("in.wav"), 384000.0, {0.5}, err));
    const auto o = parse({"--in", dir.file("in.wav"), "--out", dir.file("o.wav"), "--units", "fs"}, err);
    REQUIRE(o);
    std::ostringstream log;
    REQUIRE(ampwav::run(*o, log) == 0);

    ampwav::MonoWav out;
    REQUIRE(ampwav::readMonoWav(dir.file("o.wav"), out, err));
    const amp2203::EngineConfig cfg;
    REQUIRE_THAT(out.samples[0], WithinAbs(0.5 * cfg.inputVoltsPerFullScale / cfg.outputVoltsPerFullScale, 1e-15));
}

TEST_CASE("CLI: knob ramps", "[ampwav]") {
    std::string err;
    auto o = parse({"--block", "b4", "--in", "i.wav", "--out", "o.wav", "--knob-ramp", "treble=0.02,0.01,0,1"}, err);
    REQUIRE(o);
    REQUIRE(o->ramps.size() == 1);
    REQUIRE(o->ramps[0].knob == "treble");
    REQUIRE(o->ramps[0].start == 0.02);
    REQUIRE(o->ramps[0].duration == 0.01);
    REQUIRE(o->ramps[0].to == 1.0);
    for (const char* bad : {"treble=0.02,0.01,0", "treble=0.02,0.01,0,2", "nope=0,0.01,0,1", "treble=a,b,c,d"}) {
        INFO(bad);
        REQUIRE_FALSE(parse({"--block", "b4", "--in", "i.wav", "--out", "o.wav", "--knob-ramp", bad}, err));
    }
}
