#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioPluginEditor::AudioPluginEditor (AudioPluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    gainLabel.setText ("Gain", juce::dontSendNotification);
    gainLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (gainLabel);

    gainSlider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    gainSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible (gainSlider);

    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processorRef.apvts, "gain", gainSlider);

    setSize (300, 200);
}

AudioPluginEditor::~AudioPluginEditor() {}

void AudioPluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1a1a2e));
    g.setColour (juce::Colours::white);
    g.setFont (16.0f);
    g.drawFittedText ("Audio Plugin", getLocalBounds().removeFromTop (40),
                      juce::Justification::centred, 1);
}

void AudioPluginEditor::resized()
{
    auto area = getLocalBounds().reduced (20);
    area.removeFromTop (40);
    gainLabel.setBounds (area.removeFromTop (20));
    gainSlider.setBounds (area.withSizeKeepingCentre (120, 120));
}
