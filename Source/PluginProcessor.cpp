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

void AudioPluginProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate;
    delayBufferSize = static_cast<int> (kMaxDelaySeconds * sampleRate) + 1;

    int channels = getTotalNumInputChannels();
    delayBuffer.assign (channels, std::vector<float> (delayBufferSize, 0.0f));
    writePos.assign (channels, 0);
    lpfState.assign (channels, 0.0f);
}

void AudioPluginProcessor::releaseResources()
{
    delayBuffer.clear();
    lpfState.clear();
}

void AudioPluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
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

bool AudioPluginProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* AudioPluginProcessor::createEditor()
{
    return new AudioPluginEditor (*this);
}

const juce::String AudioPluginProcessor::getName() const { return JucePlugin_Name; }
bool AudioPluginProcessor::acceptsMidi() const            { return false; }
bool AudioPluginProcessor::producesMidi() const           { return false; }
bool AudioPluginProcessor::isMidiEffect() const           { return false; }
double AudioPluginProcessor::getTailLengthSeconds() const { return kMaxDelaySeconds; }

int AudioPluginProcessor::getNumPrograms()                              { return 1; }
int AudioPluginProcessor::getCurrentProgram()                           { return 0; }
void AudioPluginProcessor::setCurrentProgram (int)                      {}
const juce::String AudioPluginProcessor::getProgramName (int)           { return {}; }
void AudioPluginProcessor::changeProgramName (int, const juce::String&) {}

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
