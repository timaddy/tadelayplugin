#pragma once

#include "PluginProcessor.h"

class TaDelayPluginEditor : public juce::AudioProcessorEditor,
                            private juce::AudioProcessorValueTreeState::Listener
{
public:
    explicit TaDelayPluginEditor (TaDelayPluginProcessor&);
    ~TaDelayPluginEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void parameterChanged (const juce::String& paramID, float newValue) override;
    void updateModeButtons();

    TaDelayPluginProcessor& processorRef;

    // Mode toggle buttons
    juce::TextButton digitalBtn { "Digital" };
    juce::TextButton analogueBtn { "Analogue" };

    juce::Slider delaySlider, feedbackSlider, lpfSlider, mixSlider;
    juce::Label  delayLabel,  feedbackLabel,  lpfLabel,  mixLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> delayAttachment, feedbackAttachment,
                                      lpfAttachment,   mixAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TaDelayPluginEditor)
};
