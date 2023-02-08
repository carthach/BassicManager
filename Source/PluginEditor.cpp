/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BassicManagerAudioProcessorEditor::BassicManagerAudioProcessorEditor (BassicManagerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    scopedUpdateEditor (audioProcessor.updateEditor, [this] { updateGUI(); })
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    updateGUI();
    setResizable (true, true);
}

BassicManagerAudioProcessorEditor::~BassicManagerAudioProcessorEditor()
{    
}

//==============================================================================
void BassicManagerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void BassicManagerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto r = getLocalBounds();
    doLayout (inputViewers, r.removeFromTop (proportionOfHeight (0.5f)));
    doLayout (outputViewers, r);
}
