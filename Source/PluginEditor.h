#pragma once

#include "PluginProcessor.h"

class AudioPluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit AudioPluginEditor (AudioPluginProcessor&);
    ~AudioPluginEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AudioPluginProcessor& processorRef;

    juce::Slider delaySlider, feedbackSlider, lpfSlider, mixSlider;
    juce::Label  delayLabel,  feedbackLabel,  lpfLabel,  mixLabel;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> delayAttachment, feedbackAttachment,
                                lpfAttachment,   mixAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginEditor)
};
