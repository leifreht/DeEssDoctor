/*
  ==============================================================================

    RmsControl.h
    Created: 13 Dec 2024 2:41:53pm
    Author:  Leif Rehtanz

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class RmsControl : public juce::Component
{
public:
    RmsControl()
    {
        addAndMakeVisible(thresholdSlider);
        thresholdSlider.setRange(-60.0, 0.0, 0.1);
        thresholdSlider.setValue(-20.0);
        thresholdSlider.setTextValueSuffix(" dB (RMS)");

        addAndMakeVisible(reductionSlider);
        reductionSlider.setRange(-60.0, 6.0, 0.1);
        reductionSlider.setValue(0.0);
        reductionSlider.setTextValueSuffix(" dB reduction (RMS)");

        addAndMakeVisible(frequencySlider);
        frequencySlider.setRange(2000.0, 20000.0, 10.0);
        frequencySlider.setValue(4000.0);
        frequencySlider.setTextValueSuffix(" Hz (RMS)");

        addAndMakeVisible(hysteresisSlider);
        hysteresisSlider.setRange(1.0, 300.0, 1.0);
        hysteresisSlider.setValue(50.0);
        hysteresisSlider.setTextValueSuffix(" samples (RMS)");
    }

    float getThreshold() const   { return thresholdSlider.getValue(); }
    float getReduction() const   { return reductionSlider.getValue(); }
    float getFrequency() const   { return frequencySlider.getValue(); }
    float getHysteresis() const  { return hysteresisSlider.getValue(); }

    juce::Slider thresholdSlider;
    juce::Slider reductionSlider;
    juce::Slider frequencySlider;
    juce::Slider hysteresisSlider;

    void resized() override
    {
        auto area = getLocalBounds().reduced(10);
        thresholdSlider.setBounds(area.removeFromTop(25));
        reductionSlider.setBounds(area.removeFromTop(25));
        frequencySlider.setBounds(area.removeFromTop(25));
        hysteresisSlider.setBounds(area.removeFromTop(25));
    }
};
