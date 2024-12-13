/*
  ==============================================================================

    FftControl.h
    Created: 13 Dec 2024 2:42:03pm
    Author:  Leif Rehtanz

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class FftControl : public juce::Component
{
public:
    FftControl()
    {
        addAndMakeVisible(windowSizeSlider);
        windowSizeSlider.setRange(128.0, 8192.0, 1.0);
        windowSizeSlider.setValue(1024.0);
        windowSizeSlider.setTextValueSuffix(" samples (FFT window)");

        addAndMakeVisible(overlapSlider);
        overlapSlider.setRange(0.0, 1.0, 0.01);
        overlapSlider.setValue(0.5);
        overlapSlider.setTextValueSuffix(" overlap (0 to 1)");

        addAndMakeVisible(freqThresholdSlider);
        freqThresholdSlider.setRange(-60.0, 0.0, 0.1);
        freqThresholdSlider.setValue(-20.0);
        freqThresholdSlider.setTextValueSuffix(" dB freq threshold");
    }

    float getWindowSize() const       { return (float)windowSizeSlider.getValue(); }
    float getOverlap() const          { return (float)overlapSlider.getValue(); }
    float getFreqThreshold() const    { return (float)freqThresholdSlider.getValue(); }

    juce::Slider windowSizeSlider;
    juce::Slider overlapSlider;
    juce::Slider freqThresholdSlider;

    void resized() override
    {
        auto area = getLocalBounds().reduced(10);
        windowSizeSlider.setBounds(area.removeFromTop(25));
        overlapSlider.setBounds(area.removeFromTop(25));
        freqThresholdSlider.setBounds(area.removeFromTop(25));
    }
};
