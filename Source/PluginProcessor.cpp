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

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        "delayMode", "Delay Mode",
        juce::StringArray { "Digital", "Analogue" }, 0));

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
    // Extra headroom for analogue LFO modulation
    delayBufferSize = static_cast<int> ((kMaxDelaySeconds + kModDepthSeconds) * sampleRate) + 1;

    int channels = getTotalNumInputChannels();
    delayBuffer.assign (channels, std::vector<float> (delayBufferSize, 0.0f));
    writePos.assign (channels, 0);
    lpfState.assign (channels, 0.0f);
    lfoPhase.assign (channels, 0.0f);
    satState.assign (channels, 0.0f);
}

void TaDelayPluginProcessor::releaseResources()
{
    delayBuffer.clear();
    lpfState.clear();
    lfoPhase.clear();
    satState.clear();
}

// Soft-clip used in analogue mode feedback path
static inline float softClip (float x)
{
    return x / (1.0f + std::abs (x));
}

// Linear interpolation for fractional delay read
static inline float readInterpolated (const std::vector<float>& buf, float readPos, int bufSize)
{
    int pos0 = static_cast<int> (readPos);
    float frac = readPos - static_cast<float> (pos0);
    int pos1 = (pos0 + 1) % bufSize;
    pos0 = ((pos0 % bufSize) + bufSize) % bufSize;
    return buf[pos0] + frac * (buf[pos1] - buf[pos0]);
}

void TaDelayPluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int   mode      = static_cast<int> (apvts.getRawParameterValue ("delayMode")->load());
    const float delayTime = apvts.getRawParameterValue ("delayTime")->load();
    const float feedback  = apvts.getRawParameterValue ("feedback")->load();
    const float cutoff    = apvts.getRawParameterValue ("lpfCutoff")->load();
    const float mix       = apvts.getRawParameterValue ("mix")->load();

    const float fs = static_cast<float> (currentSampleRate);

    // In analogue mode: tighter LPF, LFO at 0.6 Hz, ±8 ms pitch wobble
    const float effectiveCutoff = (mode == Analogue) ? std::min (cutoff, 6000.0f) : cutoff;
    const float lpfCoeff = std::exp (-juce::MathConstants<float>::twoPi * effectiveCutoff / fs);

    const float lfoRate      = 0.6f;
    const float lfoDepth     = (mode == Analogue) ? kModDepthSeconds * fs : 0.0f;
    const float lfoIncrement = juce::MathConstants<float>::twoPi * lfoRate / fs;

    const float baseDelaySamples = delayTime * fs;

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer (ch);
        auto& buf  = delayBuffer[ch];
        auto& wPos = writePos[ch];
        auto& lpf  = lpfState[ch];
        auto& lfo  = lfoPhase[ch];

        // Offset LFO phase between channels for stereo width in analogue mode
        float lfoOffset = (ch == 1 && mode == Analogue)
                          ? juce::MathConstants<float>::pi * 0.5f : 0.0f;

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float modulation = (mode == Analogue)
                               ? lfoDepth * std::sin (lfo + lfoOffset) : 0.0f;
            lfo += lfoIncrement;
            if (lfo > juce::MathConstants<float>::twoPi)
                lfo -= juce::MathConstants<float>::twoPi;

            float delaySamples = baseDelaySamples + modulation;
            delaySamples = juce::jlimit (1.0f, static_cast<float> (delayBufferSize - 2), delaySamples);

            float readPos = static_cast<float> (wPos) - delaySamples;
            if (readPos < 0.0f) readPos += static_cast<float> (delayBufferSize);

            float delayed = readInterpolated (buf, readPos, delayBufferSize);

            // LPF on feedback
            lpf = delayed + lpfCoeff * (lpf - delayed);

            float fbSignal = (mode == Analogue) ? softClip (lpf * feedback * 1.2f)
                                                : lpf * feedback;

            buf[wPos] = data[i] + fbSignal;
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
