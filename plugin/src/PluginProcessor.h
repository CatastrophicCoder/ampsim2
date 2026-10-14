#pragma once

#include <atomic>
#include <memory>

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "amp2203/Amp2203.h"

/// The JCM800 2203 engine as a JUCE effect (SPEC §7 M5): parameters per SPEC §6.3, mono processing (left
/// channel of a stereo bus, output to every channel), the engine at the host rate × the oversampling factor
/// inside juce::dsp::Oversampling, latency reported to the host (DECISIONS D-044).
class Amp2203Processor final : public juce::AudioProcessor, private juce::Timer {
public:
    /// Oversampling choice: 0 = 2x, 1 = 4x, 2 = 8x (default), 3 = 16x, at any host rate (SPEC §3.4, D-048).
    static int factorFor(int choice) noexcept;
    static double interfaceVoltsPerFullScale(double dBu) noexcept;  // SPEC §3.3

    Amp2203Processor();
    ~Amp2203Processor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override;
    void processBlock(juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midi) override;
    bool supportsDoublePrecisionProcessing() const override { return true; }

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Amp2203"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& dest) override;
    void setStateInformation(const void* data, int size) override;

    juce::AudioProcessorValueTreeState& state() noexcept { return apvts_; }
    /// The oversampling factor and filter in use (after a change, the timer rebuilds within ~0.1 s).
    int activeFactor() const noexcept { return activeFactor_; }
    amp2203::Amp2203::Stats engineStats() const { return engine_.getStats(); }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout layout();
    void timerCallback() override;
    void rebuild(double hostRate, int maxBlock);
    template <class T>
    void process(juce::AudioBuffer<T>& buffer);
    void pushParams() noexcept;

    juce::AudioProcessorValueTreeState apvts_;
    amp2203::Amp2203 engine_;
    std::unique_ptr<juce::dsp::Oversampling<double>> oversampler_;
    juce::AudioBuffer<double> mono_;
    juce::SmoothedValue<double, juce::ValueSmoothingTypes::Multiplicative> trim_, outLevel_;
    double hostRate_ = 48000.0;
    int maxBlock_ = 512;
    int activeFactor_ = 16, activeChoice_ = 0, activeFilter_ = 0, activePowerRate_ = 0;
    std::atomic<bool> prepared_{false};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Amp2203Processor)
};
