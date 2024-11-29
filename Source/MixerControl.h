/*
  ==============================================================================

    MixerControl.h
    Created: 18 Nov 2024 12:08:38pm
    Author:  Leif Rehtanz

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class MixerControl : public juce::Component
{
public:
    MixerControl();
    ~MixerControl() override = default;

    float getMixLevel() const;

    juce::Slider mixSlider;

private:
    void resized() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixerControl)
};
