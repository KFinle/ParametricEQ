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
    
    // get peak coefficients
    auto chain_settings = getChainSettings(apvts);
   
    updatePeakFilter(chain_settings);
    
    
    
    
    auto cut_coefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chain_settings.low_cut_freq, sampleRate, 2 * (chain_settings.low_cut_slope + 1));
    
    // Setup low cut left channel
    auto& left_low_cut = left_chain.get<ChainPositions::LowCut>();
    left_low_cut.setBypassed<0>(true);
    left_low_cut.setBypassed<1>(true);
    left_low_cut.setBypassed<2>(true);
    left_low_cut.setBypassed<3>(true);
    
    switch (chain_settings.low_cut_slope)
    {
        case Slope_12:
            *left_low_cut.get<0>().coefficients = *cut_coefficients[0];
            left_low_cut.setBypassed<0>(false);
            break;
        case Slope_24:
            *left_low_cut.get<0>().coefficients = *cut_coefficients[0];
            left_low_cut.setBypassed<0>(false);
            *left_low_cut.get<1>().coefficients = *cut_coefficients[0];
            left_low_cut.setBypassed<1>(false);
            break;
        case Slope_36:
            *left_low_cut.get<0>().coefficients = *cut_coefficients[0];
            left_low_cut.setBypassed<0>(false);
            *left_low_cut.get<1>().coefficients = *cut_coefficients[0];
            left_low_cut.setBypassed<1>(false);
            *left_low_cut.get<2>().coefficients = *cut_coefficients[0];
            left_low_cut.setBypassed<2>(false);
            break;
        case Slope_48:
            *left_low_cut.get<0>().coefficients = *cut_coefficients[0];
            left_low_cut.setBypassed<0>(false);
            *left_low_cut.get<1>().coefficients = *cut_coefficients[0];
            left_low_cut.setBypassed<1>(false);
            *left_low_cut.get<2>().coefficients = *cut_coefficients[0];
            left_low_cut.setBypassed<2>(false);
            *left_low_cut.get<3>().coefficients = *cut_coefficients[0];
            left_low_cut.setBypassed<3>(false);
            break;
    }
    
    // Setup low cut right channel
    auto& right_low_cut = right_chain.get<ChainPositions::LowCut>();
    right_low_cut.setBypassed<0>(true);
    right_low_cut.setBypassed<1>(true);
    right_low_cut.setBypassed<2>(true);
    right_low_cut.setBypassed<3>(true);
    
    switch (chain_settings.low_cut_slope)
    {
        case Slope_12:
            *right_low_cut.get<0>().coefficients = *cut_coefficients[0];
            right_low_cut.setBypassed<0>(false);
            break;
        case Slope_24:
            *right_low_cut.get<0>().coefficients = *cut_coefficients[0];
            right_low_cut.setBypassed<0>(false);
            *right_low_cut.get<1>().coefficients = *cut_coefficients[0];
            right_low_cut.setBypassed<1>(false);
            break;
        case Slope_36:
            *right_low_cut.get<0>().coefficients = *cut_coefficients[0];
            right_low_cut.setBypassed<0>(false);
            *right_low_cut.get<1>().coefficients = *cut_coefficients[0];
            right_low_cut.setBypassed<1>(false);
            *right_low_cut.get<2>().coefficients = *cut_coefficients[0];
            right_low_cut.setBypassed<2>(false);
            break;
        case Slope_48:
            *right_low_cut.get<0>().coefficients = *cut_coefficients[0];
            right_low_cut.setBypassed<0>(false);
            *right_low_cut.get<1>().coefficients = *cut_coefficients[0];
            right_low_cut.setBypassed<1>(false);
            *right_low_cut.get<2>().coefficients = *cut_coefficients[0];
            right_low_cut.setBypassed<2>(false);
            *right_low_cut.get<3>().coefficients = *cut_coefficients[0];
            right_low_cut.setBypassed<3>(false);
            break;
    }

    


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
    
    
    // peak settings
    auto chain_settings = getChainSettings(apvts);
    
    updatePeakFilter(chain_settings);
    
    
    auto cut_coefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chain_settings.low_cut_freq, getSampleRate(), 2 * (chain_settings.low_cut_slope + 1));
    
    // Setup low cut left channel
    auto& left_low_cut = left_chain.get<ChainPositions::LowCut>();
    left_low_cut.setBypassed<0>(true);
    left_low_cut.setBypassed<1>(true);
    left_low_cut.setBypassed<2>(true);
    left_low_cut.setBypassed<3>(true);
    
    switch (chain_settings.low_cut_slope)
    {
        case Slope_12:
        {
            *left_low_cut.get<0>().coefficients = *cut_coefficients[0];
            left_low_cut.setBypassed<0>(false);
            break;
        }
        case Slope_24:
        {
            *left_low_cut.get<0>().coefficients = *cut_coefficients[0];
            left_low_cut.setBypassed<0>(false);
            *left_low_cut.get<1>().coefficients = *cut_coefficients[0];
            left_low_cut.setBypassed<1>(false);
            break;
        }
        case Slope_36:
        {
            *left_low_cut.get<0>().coefficients = *cut_coefficients[0];
            left_low_cut.setBypassed<0>(false);
            *left_low_cut.get<1>().coefficients = *cut_coefficients[0];
            left_low_cut.setBypassed<1>(false);
            *left_low_cut.get<2>().coefficients = *cut_coefficients[0];
            left_low_cut.setBypassed<2>(false);
            break;
        }
        case Slope_48:
        {
            *left_low_cut.get<0>().coefficients = *cut_coefficients[0];
            left_low_cut.setBypassed<0>(false);
            *left_low_cut.get<1>().coefficients = *cut_coefficients[0];
            left_low_cut.setBypassed<1>(false);
            *left_low_cut.get<2>().coefficients = *cut_coefficients[0];
            left_low_cut.setBypassed<2>(false);
            *left_low_cut.get<3>().coefficients = *cut_coefficients[0];
            left_low_cut.setBypassed<3>(false);
            break;
        }
    }
    
    // Setup low cut right channel
    auto& right_low_cut = right_chain.get<ChainPositions::LowCut>();
    right_low_cut.setBypassed<0>(true);
    right_low_cut.setBypassed<1>(true);
    right_low_cut.setBypassed<2>(true);
    right_low_cut.setBypassed<3>(true);
    
    switch (chain_settings.low_cut_slope)
    {
        case Slope_12:
        {
            *right_low_cut.get<0>().coefficients = *cut_coefficients[0];
            right_low_cut.setBypassed<0>(false);
            break;
        }
        case Slope_24:
        {
            *right_low_cut.get<0>().coefficients = *cut_coefficients[0];
            right_low_cut.setBypassed<0>(false);
            *right_low_cut.get<1>().coefficients = *cut_coefficients[0];
            right_low_cut.setBypassed<1>(false);
            break;
        }
        case Slope_36:
        {
            *right_low_cut.get<0>().coefficients = *cut_coefficients[0];
            right_low_cut.setBypassed<0>(false);
            *right_low_cut.get<1>().coefficients = *cut_coefficients[0];
            right_low_cut.setBypassed<1>(false);
            *right_low_cut.get<2>().coefficients = *cut_coefficients[0];
            right_low_cut.setBypassed<2>(false);
            break;
        }
        case Slope_48:
        {
            *right_low_cut.get<0>().coefficients = *cut_coefficients[0];
            right_low_cut.setBypassed<0>(false);
            *right_low_cut.get<1>().coefficients = *cut_coefficients[0];
            right_low_cut.setBypassed<1>(false);
            *right_low_cut.get<2>().coefficients = *cut_coefficients[0];
            right_low_cut.setBypassed<2>(false);
            *right_low_cut.get<3>().coefficients = *cut_coefficients[0];
            right_low_cut.setBypassed<3>(false);
            break;
        }
    }


    
    
    
    
    
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


// Chain Settings for parameter layout
ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;
    
    settings.low_cut_freq = apvts.getRawParameterValue("LowCut Freq")->load();
    settings.high_cut_freq = apvts.getRawParameterValue("HiCut Freq")->load();
    settings.peak1_freq = apvts.getRawParameterValue("Peak1 Freq")->load();
    settings.peak1_gain_db = apvts.getRawParameterValue("Peak1 Gain")->load();
    settings.peak1_q = apvts.getRawParameterValue("Peak1 Q")->load();
    settings.peak2_freq = apvts.getRawParameterValue("Peak2 Freq")->load();
    settings.peak2_gain_db = apvts.getRawParameterValue("Peak2 Gain")->load();
    settings.peak2_q = apvts.getRawParameterValue("Peak2 Q")->load();
    settings.peak3_freq = apvts.getRawParameterValue("Peak3 Freq")->load();
    settings.peak3_gain_db = apvts.getRawParameterValue("Peak3 Gain")->load();
    settings.peak3_q = apvts.getRawParameterValue("Peak3 Q")->load();
    settings.low_cut_slope = static_cast<Slope>(apvts.getRawParameterValue("LowCut Slope")->load());
    settings.high_cut_slope = static_cast<Slope>(apvts.getRawParameterValue("HiCut Slope")->load());

    return settings;
}

// Update filter with chain settings
void ParametricEQAudioProcessor::updatePeakFilter(const ChainSettings &chain_settings)
{
    auto peak1_coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chain_settings.peak1_freq, chain_settings.peak1_q, juce::Decibels::decibelsToGain(chain_settings.peak1_gain_db));
    
    auto peak2_coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chain_settings.peak2_freq, chain_settings.peak2_q, juce::Decibels::decibelsToGain(chain_settings.peak2_gain_db));
    
    auto peak3_coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chain_settings.peak3_freq, chain_settings.peak3_q, juce::Decibels::decibelsToGain(chain_settings.peak3_gain_db));
    
    updateCoefficients(left_chain.get<ChainPositions::Peak1>().coefficients, peak1_coefficients);
    updateCoefficients(left_chain.get<ChainPositions::Peak2>().coefficients, peak2_coefficients);
    updateCoefficients(left_chain.get<ChainPositions::Peak3>().coefficients, peak3_coefficients);
    updateCoefficients(right_chain.get<ChainPositions::Peak1>().coefficients, peak1_coefficients);
    updateCoefficients(right_chain.get<ChainPositions::Peak2>().coefficients, peak2_coefficients);
    updateCoefficients(right_chain.get<ChainPositions::Peak3>().coefficients, peak3_coefficients);

}

// Update coefficients helper
void ParametricEQAudioProcessor::updateCoefficients(Coefficients &old, const Coefficients &replace)
{
    *old = *replace;
}


juce::AudioProcessorValueTreeState::ParameterLayout ParametricEQAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // Low Cut
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Freq",
                                                           "LowCut Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.2f), 20.f));
    
    // High Cut
    layout.add(std::make_unique<juce::AudioParameterFloat>("HiCut Freq",
                                                           "HiCut Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.2f), 20000.f));
    
    // Peak 1
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak1 Freq",
                                                           "Peak1 Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.2f), 500.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak1 Gain",
                                                           "Peak1 Gain",
                                                           juce::NormalisableRange<float>(-24.f, 24, 0.5f, 1.f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak1 Q",
                                                           "Peak1 Q",
                                                           juce::NormalisableRange<float>(0.1f, 10, 0.05f, 1.f), 1.f));
    
    // Peak 2
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak2 Freq",
                                                           "Peak2 Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.2f), 750.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak2 Gain",
                                                           "Peak2 Gain",
                                                           juce::NormalisableRange<float>(-24.f, 24, 0.5f, 1.f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak2 Q",
                                                           "Peak2 Q",
                                                           juce::NormalisableRange<float>(0.1f, 10, 0.05f, 1.f), 1.f));
    
    // Peak 3
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak3 Freq",
                                                           "Peak3 Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.2f), 1200.f));
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
    layout.add(std::make_unique<juce::AudioParameterChoice>("HiCut Slope", "HiCut Slope", string_array, 0));

    
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ParametricEQAudioProcessor();
}
