/*
  ==============================================================================

    PositionOverlay.h
    Created: 29 Nov 2024 8:11:02pm
    Author:  Leif Rehtanz

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class PositionOverlay : public juce::Component,
                              private juce::Timer
{
public:
    PositionOverlay(juce::AudioTransportSource& transportSourceToUse);

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;

private:
    void timerCallback() override;

    juce::AudioTransportSource& transportSource;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PositionOverlay)
};
