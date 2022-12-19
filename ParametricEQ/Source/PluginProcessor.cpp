/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ParametricEQAudioProcessor::ParametricEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

ParametricEQAudioProcessor::~ParametricEQAudioProcessor()
{
}

//==============================================================================
const juce::String ParametricEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ParametricEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ParametricEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ParametricEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ParametricEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ParametricEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ParametricEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ParametricEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ParametricEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void ParametricEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ParametricEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;
    
    left_chain.prepare(spec);
    right_chain.prepare(spec);
}

void ParametricEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ParametricEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
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

void ParametricEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // audio blocks
    juce::dsp::AudioBlock<float> block(buffer);
    auto left_block = block.getSingleChannelBlock(0);
    auto right_block = block.getSingleChannelBlock(1);
    
    // audio contexts
    juce::dsp::ProcessContextReplacing<float> left_context(left_block);
    juce::dsp::ProcessContextReplacing<float> right_context(right_block);

    left_chain.process(left_context);
    right_chain.process(right_context);
    
    
}

//==============================================================================
bool ParametricEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ParametricEQAudioProcessor::createEditor()
{
    //return new ParametricEQAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void ParametricEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ParametricEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

juce::AudioProcessorValueTreeState::ParameterLayout ParametricEQAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // Low Cut
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Freq",
                                                           "LowCut Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f), 20.f));
    
    // High Cut
    layout.add(std::make_unique<juce::AudioParameterFloat>("HiCut Freq",
                                                           "HiCut Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f), 20000.f));
    
    // Peak 1
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak1 Freq",
                                                           "Peak1 Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f), 500.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak1 Gain",
                                                           "Peak1 Gain",
                                                           juce::NormalisableRange<float>(-24.f, 24, 0.5f, 1.f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak1 Q",
                                                           "Peak1 Q",
                                                           juce::NormalisableRange<float>(0.1f, 10, 0.05f, 1.f), 1.f));
    
    // Peak 2
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak2 Freq",
                                                           "Peak2 Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f), 750.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak2 Gain",
                                                           "Peak2 Gain",
                                                           juce::NormalisableRange<float>(-24.f, 24, 0.5f, 1.f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak2 Q",
                                                           "Peak2 Q",
                                                           juce::NormalisableRange<float>(0.1f, 10, 0.05f, 1.f), 1.f));
    
    // Peak 3
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak3 Freq",
                                                           "Peak3 Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f), 1200.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak3 Gain",
                                                           "Peak3 Gain",
                                                           juce::NormalisableRange<float>(-24.f, 24, 0.5f, 1.f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak3 Q",
                                                           "Peak3 Q",
                                                           juce::NormalisableRange<float>(0.1f, 10, 0.05f, 1.f), 1.f));
    

    // Cut Slope String Array
    juce::StringArray string_array;
    for (int i = 0; i < 4; i++)
    {
        juce::String string;
        string << (12 + i * 12);
        string << " db/Oct";
        string_array.add(string);
    }
    
    // Cut Slope Choices
    layout.add(std::make_unique<juce::AudioParameterChoice>("LowCut Slope", "LowCut Slope", string_array, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>("HighCut Slope", "HighCut Slope", string_array, 0));

    
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ParametricEQAudioProcessor();
}
