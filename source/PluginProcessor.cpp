/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BassicManagerAudioProcessor::BassicManagerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
: AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                  .withInput  ("Input",  juce::AudioChannelSet::create5point1(), true)
#endif
                  .withOutput ("Output", juce::AudioChannelSet::create5point1(), true)
#endif
                  ),
sumBuffer(5, getSampleRate()),
crossoverFrequency(60.0f),
lfeLowPassFrequency(120.0f),
lowPassBoost(10.0f),
parameters (*this, nullptr, juce::Identifier ("APVTSTutorial"),
            {
    std::make_unique<juce::AudioParameterFloat> (
                                                 juce::ParameterID{"crossoverFrequency", 1},
                                                 "Crossover Frequency",
                                                 20.0f,
                                                 250.0f,
                                                 60.0f),
    std::make_unique<juce::AudioParameterFloat> (
                                                 juce::ParameterID{"lfeLowPassFrequency", 1},
                                                 "LFE Low Pass Frequency",
                                                 20.0f,
                                                 250.0f,
                                                 120.0f),
    std::make_unique<juce::AudioParameterBool> (
                                                juce::ParameterID{"lfeBoost", 1},
                                                "LFE Boost",
                                                false)
})
#endif
{
    for(int i=0; i<5; i++)
    {
        filterArrays.add(new juce::OwnedArray<IIR::Filter<float>>);
        for(int j=0; j<8; j++)
            filterArrays[i]->add(new IIR::Filter<float>());
    }
    
    incomingLevels = outgoingLevels = incomingReadableLevels = outgoingReadableLevels = std::vector<float> (6, 0.0f);
    
    startTimerHz (60);
}

BassicManagerAudioProcessor::~BassicManagerAudioProcessor()
{
    stopTimer();
    cancelPendingUpdate();
}

//==============================================================================
const juce::String BassicManagerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BassicManagerAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool BassicManagerAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool BassicManagerAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double BassicManagerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BassicManagerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int BassicManagerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BassicManagerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BassicManagerAudioProcessor::getProgramName (int index)
{
    return {};
}

void BassicManagerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BassicManagerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    ProcessSpec highPassSpec { sampleRate, static_cast<juce::uint32> (samplesPerBlock), 1 };
    ProcessSpec lowPassSpec { sampleRate, static_cast<juce::uint32> (samplesPerBlock), 1 };
    
    for(int i=0; i<5; i++){
        auto filterArray = filterArrays.getUnchecked(i);
        for(int j=0; j<8; j++)
        {
            auto filter = filterArray->getUnchecked(j);
            filter->prepare(highPassSpec);
        }
    }
    
    sumLowPassFilter.prepare(lowPassSpec);
    
    lfeLowPassFilter.prepare(lowPassSpec);
    
    sumBuffer.setSize(1, samplesPerBlock);
    
    crossoverFrequency.reset(sampleRate, 0.001);
    lfeLowPassFrequency.reset(sampleRate, 0.001);
    
    updateCrossoverFrequency(sampleRate);
    lfeLowPassFilter.setCutoffFrequency(lfeLowPassFrequency.getNextValue());
    
    incomingLevels = outgoingLevels = incomingReadableLevels = outgoingReadableLevels = std::vector<float> (6, 0.0f);

    triggerAsyncUpdate();
}

void BassicManagerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool BassicManagerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet()  == juce::AudioChannelSet::disabled()
     || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::disabled())
        return false;
 
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::create5point1()
     || layouts.getMainOutputChannelSet() != juce::AudioChannelSet::create5point1())
        return false;
    
    return true;
}

void BassicManagerAudioProcessor::updateCrossoverFrequency(double sampleRate)
{
    auto coefficientsArray = FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(crossoverFrequency.getNextValue(), sampleRate, 1);
    
    for(int i=0; i<5; i++){
        auto filterArray = filterArrays.getUnchecked(i);
        for(int j=0; j<8; j++)
        {
            auto filter = filterArray->getUnchecked(j);
            filter->coefficients = *coefficientsArray[0];
        }
    }
    
    sumLowPassFilter.setCutoffFrequency(crossoverFrequency.getNextValue());
}

void BassicManagerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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
    
    SpinLock::ScopedTryLockType lock (levelMutex);

    if (lock.isLocked())
    {
        for (size_t i = 0; i < totalNumInputChannels; ++i)
        {
            const auto minMax = buffer.findMinMax ((int) i, 0, buffer.getNumSamples());
            const auto newMax = (float) std::max (std::abs (minMax.getStart()), std::abs (minMax.getEnd()));

            auto& toUpdate = incomingLevels[i];
            toUpdate = jmax (toUpdate, newMax);
        }
    }
    
    sumBuffer.copyFrom(0, 0, buffer, CHANNELS::L, 0, buffer.getNumSamples());
    sumBuffer.addFrom(0, 0, buffer, CHANNELS::R, 0, buffer.getNumSamples());
    sumBuffer.addFrom(0, 0, buffer, CHANNELS::C, 0, buffer.getNumSamples());
    sumBuffer.addFrom(0, 0, buffer, CHANNELS::LS, 0, buffer.getNumSamples());
    sumBuffer.addFrom(0, 0, buffer, CHANNELS::RS, 0, buffer.getNumSamples());
    
    AudioBlock<float> sumBufferBlock(sumBuffer);
    ProcessContextReplacing<float> sumBufferContext(sumBufferBlock);
    sumLowPassFilter.process(sumBufferContext);
    
    // Replace the full range output high-passed
    
    AudioBlock<float> block(buffer);
    
    int filterCounter = 0;
    
    for(int i : {L, R, C, LS, RS}){
        
        auto channelBlock = block.getSingleChannelBlock(i);
        
        ProcessContextReplacing<float> context(channelBlock);
        auto filterArray = filterArrays.getUnchecked(filterCounter);
        for(int j=0; j<8; j++)
        {
            auto filter = filterArray->getUnchecked(j);
            filter->process(context);
        }
        
        filterCounter++;
    }
    
    // Replace the LFE channel with its low-passed version
    // apply +10dB of gain
    // then add the summed low pass content
    
    auto channelBlock = block.getSingleChannelBlock(CHANNELS::LFE);
    ProcessContextReplacing<float> lfeLowPassContext(channelBlock);
    lfeLowPassFilter.process(lfeLowPassContext);
    
    buffer.applyGain(CHANNELS::LFE, 0, buffer.getNumSamples(), juce::Decibels::decibelsToGain(10));
    
    buffer.addFrom(CHANNELS::LFE, 0, sumBuffer, 0, 0, buffer.getNumSamples());
    
    lfeLowPassFrequency.setTargetValue(*parameters.getRawParameterValue("lfeLowPassFrequency"));
    lfeLowPassFrequency.getNextValue();
    
    if(lfeLowPassFrequency.isSmoothing())
        lfeLowPassFilter.setCutoffFrequency(lfeLowPassFrequency.getNextValue());
    
    crossoverFrequency.setTargetValue(*parameters.getRawParameterValue("crossoverFrequency"));
    crossoverFrequency.getNextValue();
    
    if(crossoverFrequency.isSmoothing())
        updateCrossoverFrequency(getSampleRate());
    
    if (lock.isLocked())
    {
        for (size_t i = 0; i < totalNumOutputChannels; ++i)
        {
            const auto minMax = buffer.findMinMax ((int) i, 0, buffer.getNumSamples());
            const auto newMax = (float) std::max (std::abs (minMax.getStart()), std::abs (minMax.getEnd()));

            auto& toUpdate = outgoingLevels[i];
            toUpdate = jmax (toUpdate, newMax);
        }
    }
}

//==============================================================================
bool BassicManagerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BassicManagerAudioProcessor::createEditor()
{
//    return new juce::GenericAudioProcessorEditor(*this);
    return new BassicManagerAudioProcessorEditor (*this);
}

//==============================================================================
void BassicManagerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void BassicManagerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BassicManagerAudioProcessor();
}
