/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// To help with slope int expressions
enum Slope
{
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48
};

// Get plugin parameters
struct ChainSettings
{
    float peak1_freq { 0 }, peak1_gain_db { 0 }, peak1_q { 1.f};
    float peak2_freq { 0 }, peak2_gain_db { 0 }, peak2_q { 1.f};
    float peak3_freq { 0 }, peak3_gain_db { 0 }, peak3_q { 1.f};
    float low_cut_freq { 0 }, high_cut_freq { 0 };
    int low_cut_slope { Slope::Slope_12 }, high_cut_slope { Slope::Slope_12 };
};



ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

//==============================================================================
/**
*/
class ParametricEQAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    ParametricEQAudioProcessor();
    ~ParametricEQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Paramters", createParameterLayout()};

private:
    
    using Filter = juce::dsp::IIR::Filter<float>;
    
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
    
    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, Filter, Filter, CutFilter>;
    
    MonoChain left_chain, right_chain;
    
    // Order of audio chain
    enum ChainPositions
    {
        LowCut,
        Peak1,
        Peak2,
        Peak3,
        HiCut
    };
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParametricEQAudioProcessor)
};
