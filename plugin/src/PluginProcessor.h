#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "amp2203/Amp2203.h"

// M0 skeleton: mono pass-through through the engine. Parameters, oversampling, latency and the editor
// arrive in M5 (SPEC §7).
class Amp2203Processor final : public juce::AudioProcessor {
public:
    Amp2203Processor();

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override;

    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

private:
    amp2203::Amp2203 engine_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Amp2203Processor)
};
