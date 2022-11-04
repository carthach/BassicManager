/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BassicManagerAudioProcessorEditor::BassicManagerAudioProcessorEditor (BassicManagerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    audioProcessor.addMeterListener(*this);
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    for(auto& meter : inputMeters)
        addAndMakeVisible(meter);
    
    for(auto& meter : outputMeters)
        addAndMakeVisible(meter);
    
    setSize (400, 300);
}

BassicManagerAudioProcessorEditor::~BassicManagerAudioProcessorEditor()
{
    audioProcessor.removeMeterListener(*this);
}

//==============================================================================
void BassicManagerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void BassicManagerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto area = getBounds().reduced (20);

    area.removeFromLeft (60);
    
    auto outputArea = area.removeFromBottom(area.getHeight()/2.0);
    
    auto width = 10;
    
    area.removeFromTop(20);
    
    for (auto& meter : inputMeters)
    {
        area.removeFromLeft (10);
        meter.setBounds (area.removeFromLeft (width));
    }
    
    outputArea.removeFromTop(20);
    
    for (auto& meter : outputMeters)
    {
        outputArea.removeFromLeft (10);
        meter.setBounds (outputArea.removeFromLeft (width));
    }
}
