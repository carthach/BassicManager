/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "SimpleMeter.h"

using namespace juce::dsp;

//==============================================================================
/**
*/
class BassicManagerAudioProcessor  : public juce::AudioProcessor,
                                    private AsyncUpdater,
                                    private Timer
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
    
    void timerCallback() override
        {
            const SpinLock::ScopedLockType lock (levelMutex);

            for (size_t i = 0; i < incomingReadableLevels.size(); ++i)
                incomingReadableLevels[i] = std::max (incomingReadableLevels[i] * 0.95f, std::exchange (incomingLevels[i], 0.0f));
            
            for (size_t i = 0; i < outgoingReadableLevels.size(); ++i)
                outgoingReadableLevels[i] = std::max (outgoingReadableLevels[i] * 0.95f, std::exchange (outgoingLevels[i], 0.0f));
        }
    
    float getLevel (bool isInput, int bus, int channel) const
    {
        if(isInput)
            return incomingReadableLevels[(size_t) getChannelIndexInProcessBlockBuffer (true, bus, channel)];
        else
            return outgoingReadableLevels[(size_t) getChannelIndexInProcessBlockBuffer (false, bus, channel)];
            
    }
    
    std::function<void()> updateEditor;
        
private:
    //==============================================================================
    juce::AudioProcessorValueTreeState parameters;
    
    juce::AudioBuffer<float> sumBuffer;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> crossoverFrequency, lfeLowPassFrequency;
    float lowPassBoost;
    
    juce::OwnedArray<juce::OwnedArray<IIR::Filter<float>>> filterArrays;
    LinkwitzRileyFilter<float> sumLowPassFilter, lfeLowPassFilter;
    
    SpinLock levelMutex;
    std::vector<float> incomingLevels, outgoingLevels;
    std::vector<float> incomingReadableLevels, outgoingReadableLevels;
    
    void handleAsyncUpdate() override
    {
        NullCheckedInvocation::invoke (updateEditor);
    }
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BassicManagerAudioProcessor)
};
