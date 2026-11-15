#include "PluginProcessor.h"
#include "PluginEditor.h"

LaPautaMicProAudioProcessor::LaPautaMicProAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    powerParam = apvts.getRawParameterValue ("power");
}

juce::AudioProcessorValueTreeState::ParameterLayout LaPautaMicProAudioProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "power", 1 }, "Power", true));
    return layout;
}

bool LaPautaMicProAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return out == layouts.getMainInputChannelSet();
}

void LaPautaMicProAudioProcessor::prepareToPlay (double sr, int samplesPerBlock)
{
    using C = juce::dsp::IIR::Coefficients<float>;
    const auto g = [] (float dB) { return juce::Decibels::decibelsToGain (dB); };

    // Curva de "carácter" de un condensador de diafragma grande, estilo clásico de estudio:
    // corte de graves muy bajos, ligero calor en graves, pequeño recorte en medios-bajos,
    // presencia en agudos medios y "aire" en agudos altos.
    auto cHP   = C::makeHighPass  (sr, 32.0f, 0.707f);
    auto cLow  = C::makeLowShelf  (sr, 110.0f, 0.7f, g (0.8f));
    auto cDip  = C::makePeakFilter (sr, 320.0f, 0.7f, g (-1.5f));
    auto cPres = C::makePeakFilter (sr, 7500.0f, 0.9f, g (2.8f));
    auto cAir  = C::makeHighShelf (sr, 11500.0f, 0.7f, g (3.5f));

    for (int c = 0; c < maxCh; ++c)
    {
        highPass[(size_t) c].coefficients  = cHP;
        lowShelf[(size_t) c].coefficients  = cLow;
        lowMidDip[(size_t) c].coefficients = cDip;
        presence[(size_t) c].coefficients  = cPres;
        air[(size_t) c].coefficients       = cAir;

        highPass[(size_t) c].reset();
        lowShelf[(size_t) c].reset();
        lowMidDip[(size_t) c].reset();
        presence[(size_t) c].reset();
        air[(size_t) c].reset();
    }

    mix.reset (sr, 0.02);
    mix.setCurrentAndTargetValue (powerParam->load() > 0.5f ? 1.0f : 0.0f);
    dry.setSize (maxCh, juce::jmax (1, samplesPerBlock));
}

void LaPautaMicProAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numCh = juce::jmin (buffer.getNumChannels(), maxCh);

    for (int c = getTotalNumInputChannels(); c < getTotalNumOutputChannels(); ++c)
        buffer.clear (c, 0, numSamples);

    mix.setTargetValue (powerParam->load() > 0.5f ? 1.0f : 0.0f);

    dry.setSize (maxCh, numSamples, false, false, true);
    for (int c = 0; c < numCh; ++c)
        dry.copyFrom (c, 0, buffer, c, 0, numSamples);

    constexpr float drive = 1.35f;
    constexpr float bias = 0.04f;
    const float biasOffset = std::tanh (bias);
    const float norm = 1.0f / drive;

    for (int c = 0; c < numCh; ++c)
    {
        auto* x = buffer.getWritePointer (c);
        const auto ci = (size_t) c;

        for (int i = 0; i < numSamples; ++i)
        {
            float s = x[i];
            s = highPass[ci].processSample (s);
            s = lowShelf[ci].processSample (s);
            s = lowMidDip[ci].processSample (s);
            s = presence[ci].processSample (s);
            s = air[ci].processSample (s);

            // saturación suave tipo válvula (armónicos pares ligeros)
            s = (std::tanh (s * drive + bias) - biasOffset) * norm;
            x[i] = s;
        }
    }

    for (int i = 0; i < numSamples; ++i)
    {
        const float m = mix.getNextValue();
        for (int c = 0; c < numCh; ++c)
        {
            auto* wet = buffer.getWritePointer (c);
            const float d = dry.getReadPointer (c)[i];
            wet[i] = d + (wet[i] - d) * m;
        }
    }
}

juce::AudioProcessorEditor* LaPautaMicProAudioProcessor::createEditor()
{
    return new LaPautaMicProAudioProcessorEditor (*this);
}

void LaPautaMicProAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void LaPautaMicProAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LaPautaMicProAudioProcessor();
}
