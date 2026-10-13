#pragma once

#include <memory>
#include <vector>

#include <juce_audio_processors/juce_audio_processors.h>

class Amp2203Processor;

/// Minimal functional editor (SPEC §7 M5.3): the six knobs, Interface Level, Input Trim, Output Level and the
/// oversampling factor and filter.
class Amp2203Editor final : public juce::AudioProcessorEditor {
public:
    explicit Amp2203Editor(Amp2203Processor& p);
    ~Amp2203Editor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    struct Knob {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };
    struct Choice {
        juce::ComboBox box;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
    };

    std::vector<std::unique_ptr<Knob>> knobs_;
    std::vector<std::unique_ptr<Choice>> choices_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Amp2203Editor)
};
