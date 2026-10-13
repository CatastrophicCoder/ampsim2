// amp2203_render — offline renders through the plugin's processor (SPEC §7 M5 null test, DECISIONS D-044).
//   amp2203_render plugin --in x.wav --out y.wav [--factor 16] [--filter fir|iir] [--param id=value ...]
//       the whole Amp2203Processor (oversampling, engine, levels); --param uses the plugin's parameter ids and
//       displayed values (knobs 0–10, levels in dB / dBu)
//   amp2203_render up   --in x.wav --out x_os.wav [--factor 16] [--filter fir|iir]   oversampler up stage only
//   amp2203_render down --in y_os.wav --out y.wav [--factor 16] [--filter fir|iir]   oversampler down stage only
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "../src/PluginProcessor.h"
#include "src/WavIO.h"

namespace {

constexpr int kBlock = 512;

int fail(const std::string& msg) {
    std::fprintf(stderr, "amp2203_render: %s\n", msg.c_str());
    return 1;
}

std::unique_ptr<juce::dsp::Oversampling<double>> makeOversampler(int factor, bool fir) {
    const auto type = fir ? juce::dsp::Oversampling<double>::filterHalfBandFIREquiripple
                          : juce::dsp::Oversampling<double>::filterHalfBandPolyphaseIIR;
    auto os = std::make_unique<juce::dsp::Oversampling<double>>(1, static_cast<size_t>(std::log2(factor)), type, true, true);
    os->initProcessing(kBlock);
    return os;
}

}  // namespace

int main(int argc, char** argv) {
    juce::ScopedJuceInitialiser_GUI juce;
    if (argc < 2)
        return fail("usage: amp2203_render plugin|up|down --in a.wav --out b.wav [--factor N] [--filter fir|iir] [--param id=v]");
    const std::string mode = argv[1];
    std::string inPath, outPath, filter = "fir";
    int factor = 16;
    std::map<std::string, float> params;
    for (int i = 2; i + 1 < argc; i += 2) {
        const std::string a = argv[i], v = argv[i + 1];
        if (a == "--in")
            inPath = v;
        else if (a == "--out")
            outPath = v;
        else if (a == "--factor")
            factor = std::atoi(v.c_str());
        else if (a == "--filter")
            filter = v;
        else if (a == "--param" && v.find('=') != std::string::npos)
            params[v.substr(0, v.find('='))] = std::strtof(v.substr(v.find('=') + 1).c_str(), nullptr);
        else
            return fail("unknown option " + a);
    }
    if (factor != 2 && factor != 4 && factor != 8 && factor != 16)
        return fail("--factor must be 2, 4, 8 or 16");
    const bool fir = filter == "fir";
    ampwav::MonoWav in;
    std::string err;
    if (!ampwav::readMonoWav(inPath, in, err))
        return fail(err);
    std::vector<double> out;

    if (mode == "plugin") {
        Amp2203Processor proc;
        auto& apvts = proc.state();
        auto set = [&](const std::string& id, float value) {
            auto* p = apvts.getParameter(id);
            if (p == nullptr)
                return false;
            p->setValueNotifyingHost(p->convertTo0to1(value));
            return true;
        };
        set("oversampling", static_cast<float>(std::log2(factor)));  // choice index: 1 = 2x … 4 = 16x
        set("oversamplingFilter", fir ? 0.0f : 1.0f);
        for (const auto& [id, v] : params)
            if (!set(id, v))
                return fail("unknown parameter " + id);
        proc.setPlayConfigDetails(2, 2, in.sampleRate, kBlock);
        proc.prepareToPlay(in.sampleRate, kBlock);
        if (proc.activeFactor() != factor)
            return fail("oversampling factor not applied");
        juce::AudioBuffer<double> buf(2, kBlock);
        juce::MidiBuffer midi;
        for (size_t pos = 0; pos < in.samples.size(); pos += kBlock) {
            const int n = static_cast<int>(std::min<size_t>(kBlock, in.samples.size() - pos));
            buf.setSize(2, n, false, false, true);
            for (int c = 0; c < 2; ++c)
                for (int i = 0; i < n; ++i)
                    buf.setSample(c, i, c == 0 ? in.samples[pos + static_cast<size_t>(i)] : 0.0);
            proc.processBlock(buf, midi);
            for (int i = 0; i < n; ++i)
                out.push_back(buf.getSample(0, i));
        }
        const auto st = proc.engineStats();
        std::fprintf(stderr, "amp2203_render: latency %d samples, Newton failures %llu (rescued %llu), max iterations %.0f\n",
                     proc.getLatencySamples(), static_cast<unsigned long long>(st.newtonFailures),
                     static_cast<unsigned long long>(st.newtonRescued), st.maxIterationsUsed);
        return ampwav::writeMonoWavF64(outPath, in.sampleRate, out, err) ? 0 : fail(err);
    }

    auto os = makeOversampler(factor, fir);
    juce::AudioBuffer<double> buf(1, kBlock);
    if (mode == "up") {
        for (size_t pos = 0; pos < in.samples.size(); pos += kBlock) {
            const auto n = std::min<size_t>(kBlock, in.samples.size() - pos);
            for (size_t i = 0; i < n; ++i)
                buf.setSample(0, static_cast<int>(i), in.samples[pos + i]);
            juce::dsp::AudioBlock<double> block(buf.getArrayOfWritePointers(), 1, n);
            auto up = os->processSamplesUp(block);
            for (size_t i = 0; i < up.getNumSamples(); ++i)
                out.push_back(up.getSample(0, static_cast<int>(i)));
        }
        return ampwav::writeMonoWavF64(outPath, in.sampleRate * factor, out, err) ? 0 : fail(err);
    }
    if (mode == "down") {
        // the down stage reads the oversampled block that processSamplesUp returned: fill it with the input
        const size_t total = in.samples.size() / static_cast<size_t>(factor);
        for (size_t pos = 0; pos < total; pos += kBlock) {
            const auto n = std::min<size_t>(kBlock, total - pos);
            juce::dsp::AudioBlock<double> block(buf.getArrayOfWritePointers(), 1, n);
            auto up = os->processSamplesUp(block);
            for (size_t i = 0; i < up.getNumSamples(); ++i)
                up.setSample(0, static_cast<int>(i), in.samples[pos * static_cast<size_t>(factor) + i]);
            os->processSamplesDown(block);
            for (size_t i = 0; i < n; ++i)
                out.push_back(buf.getSample(0, static_cast<int>(i)));
        }
        return ampwav::writeMonoWavF64(outPath, in.sampleRate / factor, out, err) ? 0 : fail(err);
    }
    return fail("unknown mode " + mode);
}
