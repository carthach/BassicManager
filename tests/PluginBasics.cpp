#include "helpers/test_helpers.h"
#include <PluginProcessor.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

class TestOscillator {
    float sampleRate;
    float frequency;
    float angle, angleIncrement;
    
public:
    TestOscillator() : sampleRate(44100.0f), frequency(441.0f), angle(0.0f)
    {
        prepareToPlay(frequency);
    }
    
    void prepareToPlay(float frequencyIn)
    {
        frequency = frequencyIn;
        angleIncrement = frequency *  juce::MathConstants<float>::twoPi / sampleRate;
    }
    
    void process(juce::AudioBuffer<float> & block, int channel)
    {
        auto samples = block.getWritePointer(channel);
        
        for(int i=0; i<block.getNumSamples(); ++i)
        {
            angle += angleIncrement;
            
            if(angle >= juce::MathConstants<float>::twoPi)
                angle = 0;
            
            *samples++ = std::sin(angle);
        }
    }
};


// https://github.com/McMartin/FRUT/issues/490#issuecomment-663544272

TEST_CASE("Test high frequency", "[highFrequency]") {
    auto gui = juce::ScopedJuceInitialiser_GUI {};
    juce::MidiBuffer midiBuffer;
    BassicManagerAudioProcessor testPlugin;
    
    float sampleRate = 44100.0;
    int blockSize = 512;
    auto dbThreshold = 0.5f;
    
    int lengthInSamples = (int)sampleRate * 1;
    
    auto testFrequency = 1000.0f;
    
    // Create our plugin
    testPlugin.prepareToPlay(sampleRate, blockSize);
    
    // Create a main buffer
    juce::AudioBuffer<float> buffer(6, lengthInSamples);
    buffer.clear();
    
    // Create 6 identical test tones and validate
    for(int i=0; i<6; i++)
    {
        TestOscillator testOscillator;
        testOscillator.prepareToPlay(testFrequency);
        testOscillator.process(buffer, i);
                
        auto channelRMSDecibel = juce::Decibels::gainToDecibels(buffer.getRMSLevel(i, 0, lengthInSamples));
        
        channelRMSDecibel *= -1.0f;
        CHECK_THAT(channelRMSDecibel, Catch::Matchers::WithinRel(3.0f, dbThreshold));
    }
    
    // Run through the plugin
    juce::AudioBuffer<float> blockBuffer;
    
    for(int i=0; i<lengthInSamples;i+=blockSize)
    {
        auto subBlockSize = std::min(blockSize, lengthInSamples-i);
        blockBuffer.setDataToReferTo(buffer.getArrayOfWritePointers(), 6, i, subBlockSize);
    
        testPlugin.processBlock(blockBuffer, midiBuffer);
    }
    
    // Check the channel outputs
    for(int i=0; i<6; i++)
    {
        auto channelRMSDecibel = juce::Decibels::gainToDecibels(buffer.getRMSLevel(i, 0, lengthInSamples));
        channelRMSDecibel *= -1.0f;

        // LFE should filter out the high frquency, otherwise there shouldn't be much change
        if(i==BassicManagerAudioProcessor::CHANNELS::LFE)
            CHECK(channelRMSDecibel >= 40.0f);
        else
            CHECK_THAT(channelRMSDecibel, Catch::Matchers::WithinRel(3.0f, 0.1f));
    }
}

TEST_CASE("Test low frequency", "[lowFrequency]") {
    auto gui = juce::ScopedJuceInitialiser_GUI {};
    juce::MidiBuffer midiBuffer;
    BassicManagerAudioProcessor testPlugin;
    
    float sampleRate = 44100.0;
    int blockSize = 512;
    auto dbThreshold = 0.5f;
    
    int lengthInSamples = (int)sampleRate * 1;
    
    auto testFrequency = 30.0f;
    
    // Create our plugin
    testPlugin.prepareToPlay(sampleRate, blockSize);
    
    // Create a main buffer
    juce::AudioBuffer<float> buffer(6, lengthInSamples);
    buffer.clear();
    
    // Create 6 identical test tones and validate
    for(int i=0; i<6; i++)
    {
        TestOscillator testOscillator;
        testOscillator.prepareToPlay(testFrequency);
        testOscillator.process(buffer, i);
                
        auto channelRMSDecibel = juce::Decibels::gainToDecibels(buffer.getRMSLevel(i, 0, lengthInSamples));
        
        channelRMSDecibel *= -1.0f;
        CHECK_THAT(channelRMSDecibel, Catch::Matchers::WithinRel(3.0f, dbThreshold));
    }
    
    // Run through the plugin
    juce::AudioBuffer<float> blockBuffer;
    
    for(int i=0; i<lengthInSamples;i+=blockSize)
    {
        auto subBlockSize = std::min(blockSize, lengthInSamples-i);
        blockBuffer.setDataToReferTo(buffer.getArrayOfWritePointers(), 6, i, subBlockSize);
    
        testPlugin.processBlock(blockBuffer, midiBuffer);
    }
    
    // Check the channel outputs
    for(int i=0; i<6; i++)
    {
        auto channelRMSDecibel = juce::Decibels::gainToDecibels(buffer.getRMSLevel(i, 0, lengthInSamples));
                    
        // LFE should have the low frequency, and the tops should filter it out
        if(i==BassicManagerAudioProcessor::CHANNELS::LFE)
            CHECK(channelRMSDecibel >= -3.0f);
        else
            CHECK(channelRMSDecibel <= -40.0f);
    }
}