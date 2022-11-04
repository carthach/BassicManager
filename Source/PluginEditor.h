/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"

//==============================================================================
/**
*/
class BassicManagerAudioProcessorEditor  : public juce::AudioProcessorEditor,
    private BassicManagerAudioProcessor::MeterListener
{
public:
    BassicManagerAudioProcessorEditor (BassicManagerAudioProcessor&);
    ~BassicManagerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BassicManagerAudioProcessor& audioProcessor;
    std::array<SimpleMeter, 6> inputMeters, outputMeters;
    
    //==============================================================================
    // Called from the audio thread.
    void handleNewMeterValue (int type, int channel, float value) override
    {
        if(type)
            outputMeters[(size_t) channel].update (value);
        else
            inputMeters[(size_t) channel].update (value);
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BassicManagerAudioProcessorEditor)
};
