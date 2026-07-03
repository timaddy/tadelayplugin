#include "PluginProcessor.h"
#include "PluginEditor.h"

TaDelayPluginProcessor::TaDelayPluginProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
}

TaDelayPluginProcessor::~TaDelayPluginProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout TaDelayPluginProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "delayTime", "Delay Time",
        juce::NormalisableRange<float> (0.01f, kMaxDelaySeconds, 0.001f, 0.4f),
        0.5f,
        juce::AudioParameterFloatAttributes().withLabel ("s")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "feedback", "Feedback",
        juce::NormalisableRange<float> (0.0f, 0.95f, 0.01f),
        0.4f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "lpfCutoff", "LPF Cutoff",
        juce::NormalisableRange<float> (200.0f, 20000.0f, 1.0f, 0.3f),
        4000.0f,
        juce::AudioParameterFloatAttributes().withLabel ("Hz")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "mix", "Dry/Wet",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
        0.5f));

    return layout;
}

void TaDelayPluginProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate;
    delayBufferSize = static_cast<int> (kMaxDelaySeconds * sampleRate) + 1;

    int channels = getTotalNumInputChannels();
    delayBuffer.assign (channels, std::vector<float> (delayBufferSize, 0.0f));
    writePos.assign (channels, 0);
    lpfState.assign (channels, 0.0f);
}

void TaDelayPluginProcessor::releaseResources()
{
    delayBuffer.clear();
    lpfState.clear();
}

void TaDelayPluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const float delayTime = apvts.getRawParameterValue ("delayTime")->load();
    const float feedback  = apvts.getRawParameterValue ("feedback")->load();
    const float cutoff    = apvts.getRawParameterValue ("lpfCutoff")->load();
    const float mix       = apvts.getRawParameterValue ("mix")->load();

    // 1-pole LPF coefficient: c = exp(-2π * fc / fs)
    const float lpfCoeff = std::exp (-juce::MathConstants<float>::twoPi * cutoff
                                     / static_cast<float> (currentSampleRate));

    const int delaySamples = static_cast<int> (delayTime * static_cast<float> (currentSampleRate));

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer (ch);
        auto& buf  = delayBuffer[ch];
        auto& wPos = writePos[ch];
        auto& lpf  = lpfState[ch];

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            int readPos = (wPos - delaySamples + delayBufferSize) % delayBufferSize;
            float delayed = buf[readPos];

            // 1-pole LPF on the feedback signal
            lpf = delayed + lpfCoeff * (lpf - delayed);

            buf[wPos] = data[i] + lpf * feedback;
            wPos = (wPos + 1) % delayBufferSize;

            data[i] = data[i] * (1.0f - mix) + delayed * mix;
        }
    }
}

bool TaDelayPluginProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* TaDelayPluginProcessor::createEditor()
{
    return new TaDelayPluginEditor (*this);
}

const juce::String TaDelayPluginProcessor::getName() const { return JucePlugin_Name; }
bool TaDelayPluginProcessor::acceptsMidi() const            { return false; }
bool TaDelayPluginProcessor::producesMidi() const           { return false; }
bool TaDelayPluginProcessor::isMidiEffect() const           { return false; }
double TaDelayPluginProcessor::getTailLengthSeconds() const { return kMaxDelaySeconds; }

int TaDelayPluginProcessor::getNumPrograms()                              { return 1; }
int TaDelayPluginProcessor::getCurrentProgram()                           { return 0; }
void TaDelayPluginProcessor::setCurrentProgram (int)                      {}
const juce::String TaDelayPluginProcessor::getProgramName (int)           { return {}; }
void TaDelayPluginProcessor::changeProgramName (int, const juce::String&) {}

void TaDelayPluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void TaDelayPluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TaDelayPluginProcessor();
}
