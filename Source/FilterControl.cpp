/*
  ==============================================================================

    FilterControl.cpp
    Created: 18 Nov 2024 12:08:17pm
    Author:  Leif Rehtanz

  ==============================================================================
*/

#include "FilterControl.h"

FilterControl::FilterControl()
{
    addAndMakeVisible(frequencySlider);
    addAndMakeVisible(qSlider);
    addAndMakeVisible(gainSlider);

    frequencySlider.setRange(20.0, 20000.0);
    frequencySlider.setValue(1000.0);
    frequencySlider.setTextValueSuffix(" Hz");
    frequencySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    frequencySlider.setPopupDisplayEnabled(true, false, this);
    frequencySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);

    qSlider.setRange(0.1, 10.0);
    qSlider.setValue(1.0);
    qSlider.setTextValueSuffix(" Q");
    qSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    qSlider.setPopupDisplayEnabled(true, false, this);
    qSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);

    gainSlider.setRange(-24.0, 24.0);
    gainSlider.setValue(0.0);
    gainSlider.setTextValueSuffix(" dB");
    gainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    gainSlider.setPopupDisplayEnabled(true, false, this);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
}

float FilterControl::getFrequency() const { return frequencySlider.getValue(); }
float FilterControl::getQFactor() const { return qSlider.getValue(); }
float FilterControl::getGain() const { return gainSlider.getValue(); }

void FilterControl::resized()
{
    auto area = getLocalBounds().reduced(10);
    frequencySlider.setBounds(area.removeFromTop(30));
    qSlider.setBounds(area.removeFromTop(30));
    gainSlider.setBounds(area.removeFromTop(30));
}
