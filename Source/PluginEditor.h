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

    juce::Slider gainSlider;
    juce::Label  gainLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginEditor)
};
