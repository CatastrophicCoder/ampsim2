#include "PluginProcessor.h"

#include <algorithm>
#include <cmath>

#include "PluginEditor.h"

namespace {

// parameter ids (SPEC §6.3)
constexpr const char* kKnobs[] = {"preampVolume", "treble", "middle", "bass", "master", "presence"};
constexpr const char* kKnobNames[] = {"Preamp Volume", "Treble", "Middle", "Bass", "Master", "Presence"};

juce::String tenths(float v, int) { return juce::String(v, 1); }

constexpr double kNewtonTolerance = 1e-4;  // V, D-055 (engine default 1e-9 for verification)

}  // namespace

int Amp2203Processor::factorFor(int choice) noexcept {
    return 2 << std::clamp(choice, 0, 3);
}

double Amp2203Processor::interfaceVoltsPerFullScale(double dBu) noexcept {
    return 0.7746 * std::sqrt(2.0) * std::pow(10.0, dBu / 20.0);
}

juce::AudioProcessorValueTreeState::ParameterLayout Amp2203Processor::layout() {
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    const amp2203::Params defaults;
    const double def[] = {defaults.preampVolume, defaults.treble, defaults.middle,
                          defaults.bass,         defaults.master, defaults.presence};
    for (int k = 0; k < 6; ++k)  // 0–10 shown, rotation 0–1 inside (value / 10)
        l.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{kKnobs[k], 1}, kKnobNames[k], juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f),
            static_cast<float>(10.0 * def[k]), juce::AudioParameterFloatAttributes().withStringFromValueFunction(tenths)));
    l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"interfaceLevel", 1}, "Interface Level",
                                                      juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f), 12.0f,
                                                      juce::AudioParameterFloatAttributes().withLabel("dBu")));
    l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"inputTrim", 1}, "Input Trim",
                                                      juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f,
                                                      juce::AudioParameterFloatAttributes().withLabel("dB")));
    l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"outputLevel", 1}, "Output Level",
                                                      juce::NormalisableRange<float>(-48.0f, 12.0f, 0.1f), 0.0f,
                                                      juce::AudioParameterFloatAttributes().withLabel("dB")));
    l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"oversampling", 1}, "Oversampling",
                                                       juce::StringArray{"2x", "4x", "8x", "16x"}, 0));  // 2x (D-054)
    l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"oversamplingFilter", 1}, "Oversampling Filter",
                                                       juce::StringArray{"Linear phase (FIR)", "Low latency (IIR)"}, 0));
    // power amp at the oversampled rate or at half of it (M6, D-049, D-053; default Half, D-054)
    l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"powerAmpRate", 1}, "Power Amp Rate",
                                                       juce::StringArray{"Full", "Half"}, 1));
    return l;
}

Amp2203Processor::Amp2203Processor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_(*this, nullptr, "Amp2203", layout()) {
    startTimerHz(10);
}

Amp2203Processor::~Amp2203Processor() { stopTimer(); }

void Amp2203Processor::rebuild(double hostRate, int maxBlock) {
    activeChoice_ = static_cast<int>(apvts_.getRawParameterValue("oversampling")->load());
    activeFilter_ = static_cast<int>(apvts_.getRawParameterValue("oversamplingFilter")->load());
    activePowerRate_ = static_cast<int>(apvts_.getRawParameterValue("powerAmpRate")->load());
    activeFactor_ = factorFor(activeChoice_);
    const auto order = static_cast<size_t>(std::log2(activeFactor_));
    const auto type = activeFilter_ == 0 ? juce::dsp::Oversampling<double>::filterHalfBandFIREquiripple
                                         : juce::dsp::Oversampling<double>::filterHalfBandPolyphaseIIR;
    oversampler_ = std::make_unique<juce::dsp::Oversampling<double>>(1, order, type, true, true);
    oversampler_->initProcessing(static_cast<size_t>(maxBlock));
    mono_.setSize(1, maxBlock);

    amp2203::EngineConfig cfg;
    cfg.sampleRate = hostRate * activeFactor_;
    cfg.maxBlockSize = maxBlock * activeFactor_;
    cfg.powerAmpRateDivisor = activePowerRate_ == 1 ? 2 : 1;
    cfg.exactJ4 = true;  // preamp → power amp junction solved exactly (M6 X1, D-053, D-054)
    engine_.prepare(cfg);
    // D-055: Newton residual tolerance 1e-4 V; adaptive half-steps (D-043) at both power-amp rates (D-056)
    engine_.setNewtonResidualTolerance(kNewtonTolerance);
    engine_.setAdaptiveHalfSteps(true);
    pushParams();
    engine_.reset();  // DC operating point at the current knobs (SPEC §3.6)
    trim_.reset(hostRate, 0.02);
    outLevel_.reset(hostRate, 0.02);
    trim_.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(apvts_.getRawParameterValue("inputTrim")->load()));
    outLevel_.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(apvts_.getRawParameterValue("outputLevel")->load()));
    // oversampler plus the engine's rate-change filters (half-rate power amp), in host samples
    setLatencySamples(static_cast<int>(std::lround(oversampler_->getLatencyInSamples() +
                                                   static_cast<double>(engine_.latencySamples()) / activeFactor_)));
}

void Amp2203Processor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    prepared_ = false;
    hostRate_ = sampleRate;
    maxBlock_ = samplesPerBlock;
    rebuild(sampleRate, samplesPerBlock);
    prepared_ = true;
}

void Amp2203Processor::timerCallback() {
    // oversampling factor / filter and power-amp rate changes rebuild the oversampler and re-prepare the engine: allocation,
    // so on the message thread with processing suspended
    if (!prepared_)
        return;
    const int choice = static_cast<int>(apvts_.getRawParameterValue("oversampling")->load());
    const int filter = static_cast<int>(apvts_.getRawParameterValue("oversamplingFilter")->load());
    const int powerRate = static_cast<int>(apvts_.getRawParameterValue("powerAmpRate")->load());
    if (choice == activeChoice_ && filter == activeFilter_ && powerRate == activePowerRate_)
        return;
    suspendProcessing(true);
    rebuild(hostRate_, maxBlock_);
    suspendProcessing(false);
}

bool Amp2203Processor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    const auto out = layouts.getMainOutputChannelSet();
    const auto in = layouts.getMainInputChannelSet();
    const bool outOk = out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
    const bool inOk = in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo();
    return outOk && inOk;
}

void Amp2203Processor::pushParams() noexcept {
    amp2203::Params p;
    double* knob[] = {&p.preampVolume, &p.treble, &p.middle, &p.bass, &p.master, &p.presence};
    for (int k = 0; k < 6; ++k)
        *knob[k] = apvts_.getRawParameterValue(kKnobs[k])->load() / 10.0;
    engine_.setParams(p);
    engine_.setInputVoltsPerFullScale(interfaceVoltsPerFullScale(apvts_.getRawParameterValue("interfaceLevel")->load()));
}

template <class T>
void Amp2203Processor::process(juce::AudioBuffer<T>& buffer) {
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    const int numIn = getTotalNumInputChannels();
    const int numOut = getTotalNumOutputChannels();
    if (!prepared_ || n > maxBlock_ || oversampler_ == nullptr) {
        buffer.clear();
        return;
    }
    pushParams();
    trim_.setTargetValue(juce::Decibels::decibelsToGain(apvts_.getRawParameterValue("inputTrim")->load()));
    outLevel_.setTargetValue(juce::Decibels::decibelsToGain(apvts_.getRawParameterValue("outputLevel")->load()));

    // mono: the left (or only) input channel, input trim
    double* m = mono_.getWritePointer(0);
    const T* in = numIn > 0 ? buffer.getReadPointer(0) : nullptr;
    for (int i = 0; i < n; ++i)
        m[i] = (in != nullptr ? static_cast<double>(in[i]) : 0.0) * trim_.getNextValue();

    juce::dsp::AudioBlock<double> block(mono_.getArrayOfWritePointers(), 1, static_cast<size_t>(n));
    auto up = oversampler_->processSamplesUp(block);
    engine_.process(up.getChannelPointer(0), static_cast<int>(up.getNumSamples()));
    oversampler_->processSamplesDown(block);

    for (int i = 0; i < n; ++i)
        m[i] *= outLevel_.getNextValue();
    for (int ch = 0; ch < numOut; ++ch) {
        T* out = buffer.getWritePointer(ch);
        for (int i = 0; i < n; ++i)
            out[i] = static_cast<T>(m[i]);
    }
}

void Amp2203Processor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) { process(buffer); }
void Amp2203Processor::processBlock(juce::AudioBuffer<double>& buffer, juce::MidiBuffer&) { process(buffer); }

juce::AudioProcessorEditor* Amp2203Processor::createEditor() { return new Amp2203Editor(*this); }

void Amp2203Processor::getStateInformation(juce::MemoryBlock& dest) {
    if (auto xml = apvts_.copyState().createXml())
        copyXmlToBinary(*xml, dest);
}

void Amp2203Processor::setStateInformation(const void* data, int size) {
    if (auto xml = getXmlFromBinary(data, size))
        if (xml->hasTagName(apvts_.state.getType()))
            apvts_.replaceState(juce::ValueTree::fromXml(*xml));
}

#ifndef AMP2203_NO_PLUGIN_ENTRY
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new Amp2203Processor(); }
#endif
