using namespace juce;

//==============================================================================
// A very simple decaying meter.
class SimpleMeter  : public Component,
                     private Timer
{
public:
    SimpleMeter()
    {
        startTimerHz (30);
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (Colours::transparentBlack);

        auto area = g.getClipBounds();
        g.setColour (getLookAndFeel().findColour (Slider::thumbColourId));
//        g.fillRoundedRectangle (area.toFloat(), 6.0);
        g.fillRect(area.toFloat());

        auto unfilledHeight = area.getHeight() * (1.0 - level);
        g.reduceClipRegion (area.getX(), area.getY(),
                            area.getWidth(), (int) unfilledHeight);
        g.setColour (getLookAndFeel().findColour (Slider::trackColourId));
//        g.fillRoundedRectangle (area.toFloat(), 6.0);
        g.fillRect(area.toFloat());
    }

    //==============================================================================
    // Called from the audio thread.
    void update (float newLevel)
    {
        // We don't care if maxLevel gets set to zero (in timerCallback) between the
        // load and the assignment.
        maxLevel = jmax (maxLevel.load(), newLevel);
    }

private:
    //==============================================================================
    void timerCallback() override
    {
        auto callbackLevel = maxLevel.exchange (0.0);

        float decayFactor = 0.95f;

        if (callbackLevel > level)
            level = callbackLevel;
        else if (level > 0.001)
            level *= decayFactor;
        else
            level = 0;

        repaint();
    }

    std::atomic<float> maxLevel {0.0};
    float level = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleMeter)
};
