/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

using namespace juce::dsp;

//==============================================================================
/**
*/
class BassicManagerAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    BassicManagerAudioProcessor();
    ~BassicManagerAudioProcessor() override;

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
    
    void updateCrossoverFrequency(double sampleRate);
    
    enum CHANNELS { L, R, C, LFE, LS, RS};

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState parameters;
    
    juce::AudioBuffer<float> sumBuffer;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> crossoverFrequency, lfeLowPassFrequency;
    float lowPassBoost;
    
    juce::OwnedArray<juce::OwnedArray<IIR::Filter<float>>> filterArrays;
    LinkwitzRileyFilter<float> sumLowPassFilter, lfeLowPassFilter;
        
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BassicManagerAudioProcessor)
};
