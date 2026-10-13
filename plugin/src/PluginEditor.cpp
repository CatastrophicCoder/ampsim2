#include "PluginEditor.h"

#include "PluginProcessor.h"

namespace {
constexpr const char* kKnobIds[] = {"preampVolume", "treble", "middle", "bass", "master", "presence",
                                    "interfaceLevel", "inputTrim", "outputLevel"};
constexpr const char* kChoiceIds[] = {"oversampling", "oversamplingFilter"};
constexpr int kKnobSize = 90, kLabelHeight = 20, kMargin = 12;
}  // namespace

Amp2203Editor::Amp2203Editor(Amp2203Processor& p) : AudioProcessorEditor(p) {
    auto& apvts = p.state();
    for (const char* id : kKnobIds) {
        auto k = std::make_unique<Knob>();
        k->slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        k->slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, kKnobSize, 18);
        k->label.setText(apvts.getParameter(id)->getName(32), juce::dontSendNotification);
        k->label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(k->slider);
        addAndMakeVisible(k->label);
        k->attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, id, k->slider);
        knobs_.push_back(std::move(k));
    }
    for (const char* id : kChoiceIds) {
        auto c = std::make_unique<Choice>();
        if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(id)))
            c->box.addItemList(param->choices, 1);
        c->label.setText(apvts.getParameter(id)->getName(32), juce::dontSendNotification);
        addAndMakeVisible(c->box);
        addAndMakeVisible(c->label);
        c->attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, id, c->box);
        choices_.push_back(std::move(c));
    }
    setSize(6 * (kKnobSize + kMargin) + kMargin, 2 * (kKnobSize + kLabelHeight + 18 + kMargin) + 60);
}

Amp2203Editor::~Amp2203Editor() = default;

void Amp2203Editor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff1c1c1c));
    g.setColour(juce::Colours::goldenrod);
    g.setFont(18.0f);
    g.drawText("JCM800 2203", getLocalBounds().removeFromTop(30), juce::Justification::centred);
}

void Amp2203Editor::resized() {
    auto area = getLocalBounds().reduced(kMargin);
    area.removeFromTop(24);
    auto place = [&](juce::Rectangle<int>& row, Knob& k) {
        auto cell = row.removeFromLeft(kKnobSize);
        row.removeFromLeft(kMargin);
        k.label.setBounds(cell.removeFromTop(kLabelHeight));
        k.slider.setBounds(cell);
    };
    auto row1 = area.removeFromTop(kKnobSize + kLabelHeight + 18);
    for (int k = 0; k < 6; ++k)
        place(row1, *knobs_[static_cast<size_t>(k)]);
    area.removeFromTop(kMargin);
    auto row2 = area.removeFromTop(kKnobSize + kLabelHeight + 18);
    for (int k = 6; k < 9; ++k)
        place(row2, *knobs_[static_cast<size_t>(k)]);
    for (auto& c : choices_) {
        auto cell = row2.removeFromLeft(kKnobSize * 2);
        row2.removeFromLeft(kMargin);
        c->label.setBounds(cell.removeFromTop(kLabelHeight));
        c->box.setBounds(cell.removeFromTop(26));
    }
}
