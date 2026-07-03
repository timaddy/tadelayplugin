#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioPluginProcessor::AudioPluginProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
}

AudioPluginProcessor::~AudioPluginProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "gain", "Gain",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
        0.5f));

    return layout;
}

const juce::String AudioPluginProcessor::getName() const { return JucePlugin_Name; }
bool AudioPluginProcessor::acceptsMidi() const            { return false; }
bool AudioPluginProcessor::producesMidi() const           { return false; }
bool AudioPluginProcessor::isMidiEffect() const           { return false; }
double AudioPluginProcessor::getTailLengthSeconds() const { return 0.0; }

int AudioPluginProcessor::getNumPrograms()                              { return 1; }
int AudioPluginProcessor::getCurrentProgram()                           { return 0; }
void AudioPluginProcessor::setCurrentProgram (int)                      {}
const juce::String AudioPluginProcessor::getProgramName (int)           { return {}; }
void AudioPluginProcessor::changeProgramName (int, const juce::String&) {}

void AudioPluginProcessor::prepareToPlay (double, int) {}
void AudioPluginProcessor::releaseResources()          {}

void AudioPluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    auto gain = apvts.getRawParameterValue ("gain")->load();

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        buffer.applyGain (channel, 0, buffer.getNumSamples(), gain);
}

bool AudioPluginProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* AudioPluginProcessor::createEditor()
{
    return new AudioPluginEditor (*this);
}

void AudioPluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void AudioPluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginProcessor();
}
