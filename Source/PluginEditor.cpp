#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace {
    void setupKnob (juce::Slider& s, juce::Label& l, const juce::String& text,
                    juce::Component& parent)
    {
        s.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 18);
        parent.addAndMakeVisible (s);

        l.setText (text, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
        l.setFont (juce::Font (12.0f));
        l.setColour (juce::Label::textColourId, juce::Colour (0xff94a3b8));
        parent.addAndMakeVisible (l);
    }
}

TaDelayPluginEditor::TaDelayPluginEditor (TaDelayPluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setupKnob (delaySlider,    delayLabel,    "Delay",    *this);
    setupKnob (feedbackSlider, feedbackLabel, "Feedback", *this);
    setupKnob (lpfSlider,      lpfLabel,      "LPF",      *this);
    setupKnob (mixSlider,      mixLabel,      "Mix",      *this);

    using A = juce::AudioProcessorValueTreeState::SliderAttachment;
    auto& apvts = processorRef.apvts;
    delayAttachment    = std::make_unique<A> (apvts, "delayTime", delaySlider);
    feedbackAttachment = std::make_unique<A> (apvts, "feedback",  feedbackSlider);
    lpfAttachment      = std::make_unique<A> (apvts, "lpfCutoff", lpfSlider);
    mixAttachment      = std::make_unique<A> (apvts, "mix",       mixSlider);

    setSize (480, 220);
}

TaDelayPluginEditor::~TaDelayPluginEditor() {}

void TaDelayPluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1a1a2e));
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (18.0f, juce::Font::bold));
    g.drawFittedText ("Delay + LPF", getLocalBounds().removeFromTop (36),
                      juce::Justification::centred, 1);
}

void TaDelayPluginEditor::resized()
{
    auto area = getLocalBounds().reduced (16);
    area.removeFromTop (36);

    const int knobW = (area.getWidth() - 24) / 4;
    const int knobH = area.getHeight();

    auto placeKnob = [&] (juce::Slider& s, juce::Label& l)
    {
        auto col = area.removeFromLeft (knobW);
        area.removeFromLeft (8);
        l.setBounds (col.removeFromBottom (20));
        s.setBounds (col);
    };

    placeKnob (delaySlider,    delayLabel);
    placeKnob (feedbackSlider, feedbackLabel);
    placeKnob (lpfSlider,      lpfLabel);
    placeKnob (mixSlider,      mixLabel);
}
