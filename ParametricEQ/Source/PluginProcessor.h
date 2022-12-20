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
    
    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Parameters", createParameterLayout()};

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
    
    
    void updatePeakFilter(const ChainSettings& chain_settings);
    
    using Coefficients = Filter::CoefficientsPtr;
    static void updateCoefficients(Coefficients& old, const Coefficients& replace);
    
    // Update cut filter coefficients and setbypassed
    template<int Index, typename ChainType, typename CoefficientType>
    void update(ChainType& chain, const CoefficientType& coefficients)
    {
        updateCoefficients(chain.template get<Index>().coefficients, coefficients[Index]);
        chain.template setBypassed<Index>(false);
    }
    
    
    // Update low cut filter based on slope
    template<typename ChainType, typename CoefficientType>
    void updateCutFilter(ChainType& chain, const CoefficientType& coefficients, const int& slope)
    {

        // Setup low cut channel
        chain.template setBypassed<0>(true);
        chain.template setBypassed<1>(true);
        chain.template setBypassed<2>(true);
        chain.template setBypassed<3>(true);
        
        switch (slope)
        {
            case Slope_48:
            {
                update<3>(chain, coefficients);
            }
                
            case Slope_36:
            {
                update<2>(chain, coefficients);
            }
            
            case Slope_24:
            {
                update<1>(chain, coefficients);
            }
                
            case Slope_12:
            {
                update<0>(chain, coefficients);
            }
        }
    }
    
    void updateLowCutFilters(const ChainSettings& chain_settings);
    void updateHighCutFilters(const ChainSettings& chain_settings);
    void updateFilters();
    
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParametricEQAudioProcessor)
};
