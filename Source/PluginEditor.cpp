#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace {
    const juce::Colour kBg        { 0xff1a1a2e };
    const juce::Colour kActive    { 0xff3b82f6 };
    const juce::Colour kInactive  { 0xff2d3748 };
    const juce::Colour kTextMuted { 0xff94a3b8 };

    void setupKnob (juce::Slider& s, juce::Label& l, const juce::String& text,
                    juce::Component& parent)
    {
        s.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 18);
        parent.addAndMakeVisible (s);

        l.setText (text, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
        l.setFont (juce::Font (12.0f));
        l.setColour (juce::Label::textColourId, kTextMuted);
        parent.addAndMakeVisible (l);
    }

    void styleBtn (juce::TextButton& btn)
    {
        btn.setColour (juce::TextButton::buttonColourId,   kInactive);
        btn.setColour (juce::TextButton::buttonOnColourId, kActive);
        btn.setColour (juce::TextButton::textColourOffId,  kTextMuted);
        btn.setColour (juce::TextButton::textColourOnId,   juce::Colours::white);
    }
}

TaDelayPluginEditor::TaDelayPluginEditor (TaDelayPluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    styleBtn (digitalBtn);
    styleBtn (analogueBtn);
    addAndMakeVisible (digitalBtn);
    addAndMakeVisible (analogueBtn);

    digitalBtn.setClickingTogglesState (false);
    analogueBtn.setClickingTogglesState (false);

    digitalBtn.onClick = [this]
    {
        processorRef.apvts.getParameter ("delayMode")->setValueNotifyingHost (0.0f);
        updateModeButtons();
    };

    analogueBtn.onClick = [this]
    {
        processorRef.apvts.getParameter ("delayMode")->setValueNotifyingHost (1.0f);
        updateModeButtons();
    };

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

    apvts.addParameterListener ("delayMode", this);

    updateModeButtons();
    setSize (480, 260);
}

TaDelayPluginEditor::~TaDelayPluginEditor()
{
    processorRef.apvts.removeParameterListener ("delayMode", this);
}

void TaDelayPluginEditor::parameterChanged (const juce::String&, float)
{
    juce::MessageManager::callAsync ([this] { updateModeButtons(); });
}

void TaDelayPluginEditor::updateModeButtons()
{
    int mode = static_cast<int> (processorRef.apvts.getRawParameterValue ("delayMode")->load());
    digitalBtn.setToggleState  (mode == TaDelayPluginProcessor::Digital,  juce::dontSendNotification);
    analogueBtn.setToggleState (mode == TaDelayPluginProcessor::Analogue, juce::dontSendNotification);
}

void TaDelayPluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (kBg);
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (18.0f, juce::Font::bold));
    g.drawFittedText ("TaDelay", getLocalBounds().removeFromTop (36),
                      juce::Justification::centred, 1);
}

void TaDelayPluginEditor::resized()
{
    auto area = getLocalBounds().reduced (16);
    area.removeFromTop (36);

    // Mode buttons row
    auto btnRow = area.removeFromTop (32);
    area.removeFromTop (8);
    int btnW = (btnRow.getWidth() - 8) / 2;
    digitalBtn.setBounds  (btnRow.removeFromLeft (btnW));
    btnRow.removeFromLeft (8);
    analogueBtn.setBounds (btnRow);

    // Knobs row
    const int knobW = (area.getWidth() - 24) / 4;

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
