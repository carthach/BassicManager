/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"

const Colour textColour = Colours::white.withAlpha (0.8f);

inline void drawBackground (Component& comp, Graphics& g)
{
    g.setColour (comp.getLookAndFeel().findColour (ResizableWindow::backgroundColourId).darker (0.8f));
    g.fillRoundedRectangle (comp.getLocalBounds().toFloat(), 4.0f);
}

inline void configureLabel (Label& label, const AudioProcessor::Bus* layout)
{
    const auto text = layout != nullptr
                          ? (layout->getName() + ": " + layout->getCurrentLayout().getDescription())
                          : "";
    label.setText (text, dontSendNotification);
    label.setJustificationType (Justification::centred);
    label.setColour (Label::textColourId, textColour);
}

class InputBusViewer : public Component,
                       private Timer
{
public:
    InputBusViewer (BassicManagerAudioProcessor& proc, bool isInputBus, int busNumber)
        : processor (proc),
          isInput(isInputBus),
          bus (busNumber)
          
    {
        configureLabel (layoutName, processor.getBus (isInput, bus));
        addAndMakeVisible (layoutName);

        startTimerHz (60);
    }

    ~InputBusViewer() override
    {
        stopTimer();
    }

    void paint (Graphics& g) override
    {
        drawBackground (*this, g);

        auto* layout = processor.getBus (isInput, bus);

        if (layout == nullptr)
            return;

        const auto channelSet = layout->getCurrentLayout();
        const auto numChannels = channelSet.size();

        Grid grid;

        grid.autoFlow = Grid::AutoFlow::column;
        grid.autoColumns = grid.autoRows = Grid::TrackInfo (Grid::Fr (1));
        grid.items.insertMultiple (0, GridItem(), numChannels);
        grid.performLayout (getLocalBounds());

        const auto minDb = -50.0f;
        const auto maxDb = 6.0f;

        for (auto i = 0; i < numChannels; ++i)
        {
            g.setColour (Colours::orange.darker());

            const auto levelInDb = Decibels::gainToDecibels (processor.getLevel (isInput, bus, i), minDb);
                                    
            const auto fractionOfHeight = jmap (levelInDb, minDb, maxDb, 0.0f, 1.0f);
            const auto bounds = grid.items[i].currentBounds;
            const auto trackBounds = bounds.withSizeKeepingCentre (16, bounds.getHeight() - 10).toFloat();
            g.fillRect (trackBounds.withHeight (trackBounds.proportionOfHeight (fractionOfHeight)).withBottomY (trackBounds.getBottom()));

            g.setColour (textColour);

            g.drawText (channelSet.getAbbreviatedChannelTypeName (channelSet.getTypeOfChannel (i)),
                        bounds,
                        Justification::centredBottom);
        }
    }

    void resized() override
    {
        layoutName.setBounds (getLocalBounds().removeFromTop (20));
    }

    int getNumChannels() const
    {
        if (auto* b = processor.getBus (isInput, bus))
            return b->getCurrentLayout().size();

        return 0;
    }

private:
    void timerCallback() override { repaint(); }

    BassicManagerAudioProcessor& processor;
    int bus = 0;
    bool isInput = true;
    Label layoutName;
};

//==============================================================================
/**
*/
class BassicManagerAudioProcessorEditor  : public juce::AudioProcessorEditor    
{
public:
    BassicManagerAudioProcessorEditor (BassicManagerAudioProcessor&);
    ~BassicManagerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    template <typename Range>
    void doLayout (Range& range, Rectangle<int> bounds) const
    {
        FlexBox fb;

        for (auto& viewer : range)
        {
            if (viewer.getNumChannels() != 0)
            {
                fb.items.add (FlexItem (viewer)
                                  .withFlex ((float) viewer.getNumChannels())
                                  .withMargin (4.0f));
            }
        }

        fb.performLayout (bounds);
    }
    
    void updateGUI()
    {
        inputViewers.clear();

        const auto inputBuses = getAudioProcessor()->getBusCount (true);

        for (auto i = 0; i < inputBuses; ++i)
        {
            inputViewers.emplace_back (audioProcessor, true, i);
            addAndMakeVisible (inputViewers.back());
        }
        
        outputViewers.clear();

        const auto outputBuses = getAudioProcessor()->getBusCount (false);

        for (auto i = 0; i < outputBuses; ++i)
        {
            outputViewers.emplace_back (audioProcessor, false, i);
            addAndMakeVisible (outputViewers.back());
        }

        const auto channels = jmax (processor.getTotalNumInputChannels(),
                                    processor.getTotalNumOutputChannels());
        setSize (jmax (150, channels * 40), 200);

        resized();
    }


private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BassicManagerAudioProcessor& audioProcessor;
    
    ScopedValueSetter<std::function<void()>> scopedUpdateEditor;
    std::list<InputBusViewer> inputViewers;
    std::list<InputBusViewer> outputViewers;
    
    //==============================================================================
    // Called from the audio thread.
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BassicManagerAudioProcessorEditor)
};
