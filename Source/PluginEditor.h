#pragma once

#include "PluginProcessor.h"

class TaDelayPluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit TaDelayPluginEditor (TaDelayPluginProcessor&);
    ~TaDelayPluginEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    TaDelayPluginProcessor& processorRef;

    juce::Slider delaySlider, feedbackSlider, lpfSlider, mixSlider;
    juce::Label  delayLabel,  feedbackLabel,  lpfLabel,  mixLabel;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> delayAttachment, feedbackAttachment,
                                lpfAttachment,   mixAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TaDelayPluginEditor)
};
