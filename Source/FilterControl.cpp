/*
  ==============================================================================

    FilterControl.cpp
    Created: 18 Nov 2024 12:08:17pm
    Author:  Leif Rehtanz

  ==============================================================================
*/

#include "FilterControl.h"

#include "FilterControl.h"

FilterControl::FilterControl()
{
    addAndMakeVisible(frequencySlider);
    frequencySlider.setRange(2000.0, 20000.0, 10.0);
    frequencySlider.setValue(4000.0);
    frequencySlider.setTextValueSuffix(" Hz");

    addAndMakeVisible(thresholdSlider);
    thresholdSlider.setRange(-60.0, 0.0, 0.1);
    thresholdSlider.setValue(-20.0);
    thresholdSlider.setTextValueSuffix(" dB");

    addAndMakeVisible(reductionSlider);
    reductionSlider.setRange(-60.0, 6.0, 0.1);
    reductionSlider.setValue(0.0);
    reductionSlider.setTextValueSuffix(" dB");
    
    addAndMakeVisible(hysteresisSlider);
    hysteresisSlider.setRange(1.0, 300.0, 1.0);
    hysteresisSlider.setValue(50.0);
    hysteresisSlider.setTextValueSuffix(" samples");
    
}

float FilterControl::getFrequency() const { return frequencySlider.getValue(); }
float FilterControl::getThreshold() const { return thresholdSlider.getValue(); }
float FilterControl::getReduction() const { return reductionSlider.getValue(); }
float FilterControl::getHysteresis() const { return hysteresisSlider.getValue(); }

void FilterControl::resized()
{
    auto area = getLocalBounds().reduced(10);
    frequencySlider.setBounds(area.removeFromTop(25));
    thresholdSlider.setBounds(area.removeFromTop(25));
    reductionSlider.setBounds(area.removeFromTop(25));
    hysteresisSlider.setBounds(area.removeFromTop(25));
}
