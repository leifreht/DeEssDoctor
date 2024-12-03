/*
  ==============================================================================

    FilterControl.h
    Created: 18 Nov 2024 12:08:17pm
    Author:  Leif Rehtanz

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class FilterControl : public juce::Component
{
public:
    FilterControl();
    ~FilterControl() override = default;

    float getFrequency() const;
    float getQFactor() const;
    float getGain() const;

    juce::Slider frequencySlider;
    juce::Slider qSlider;
    juce::Slider gainSlider;

private:
    juce::Label frequencyLabel;
    juce::Label qLabel;
    juce::Label gainLabel;

    void resized() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterControl)
};
