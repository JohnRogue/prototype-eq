/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PrototypeEQAudioProcessor::PrototypeEQAudioProcessor()
    :
#ifndef JucePlugin_PreferredChannelConfigurations
     AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
     allpassPerChannel(getTotalNumInputChannels())
{
}

PrototypeEQAudioProcessor::~PrototypeEQAudioProcessor()
{
}

//==============================================================================
const juce::String PrototypeEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PrototypeEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PrototypeEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PrototypeEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PrototypeEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PrototypeEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PrototypeEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PrototypeEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PrototypeEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void PrototypeEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PrototypeEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void PrototypeEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PrototypeEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PrototypeEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto gain = this->gain.load();
    auto cutoffFrequency = this->cutoffFrequency.load();

    auto c = tanh(juce::MathConstants<double>::pi * cutoffFrequency / getSampleRate());
    auto v0 = pow(10.0, gain / 20.0);
    auto h0 = v0 - 1;

    auto a = gain > 0 ? (c - 1) / (c + 1) : (c - v0) / (c + v0);

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        allpassPerChannel[channel].setCoefficients(a, 1, 0, a, 0);

        auto* channelData = buffer.getWritePointer (channel);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            auto x = channelData[i];
            auto z = allpassPerChannel[channel].filter(x);
            channelData[i] = (x + z) * h0 / 2 + x;
        }
    }
}

//==============================================================================
bool PrototypeEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PrototypeEQAudioProcessor::createEditor()
{
    return new PrototypeEQAudioProcessorEditor (*this);
}

//==============================================================================
void PrototypeEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void PrototypeEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void PrototypeEQAudioProcessor::setGain(double g)
{
    gain.store(g);
}

void PrototypeEQAudioProcessor::setCutoffFrequency(double f)
{
    cutoffFrequency.store(f);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PrototypeEQAudioProcessor();
}
