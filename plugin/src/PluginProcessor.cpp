#include "PluginProcessor.h"

Amp2203Processor::Amp2203Processor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)) {}

void Amp2203Processor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    amp2203::EngineConfig cfg;
    cfg.sampleRate = sampleRate;
    cfg.maxBlockSize = samplesPerBlock;
    engine_.prepare(cfg);
}

bool Amp2203Processor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    const auto out = layouts.getMainOutputChannelSet();
    const auto in = layouts.getMainInputChannelSet();
    const bool outOk = out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
    const bool inOk = in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo();
    return outOk && inOk;
}

void Amp2203Processor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;

    const int numIn = getTotalNumInputChannels();
    const int numOut = getTotalNumOutputChannels();
    const int n = buffer.getNumSamples();

    // Mono engine: left input channel only, output duplicated to all channels (SPEC §7 M5.1).
    if (numIn == 0)
        buffer.clear(0, 0, n);
    float* mono = buffer.getWritePointer(0);
    engine_.process(mono, n);
    for (int ch = 1; ch < numOut; ++ch)
        buffer.copyFrom(ch, 0, mono, n);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new Amp2203Processor(); }
