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
    // Frequency Slider
    addAndMakeVisible(frequencySlider);
    frequencySlider.setRange(1000.0, 10000.0, 1.0); // Suitable range for high-shelf
    frequencySlider.setValue(5000.0);
    frequencySlider.setTextValueSuffix(" Hz");
    frequencySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    frequencySlider.setPopupDisplayEnabled(true, false, this);
    frequencySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);

    frequencyLabel.setText("Frequency", juce::dontSendNotification);
    addAndMakeVisible(frequencyLabel);
    frequencyLabel.attachToComponent(&frequencySlider, true);

    // Q Factor Slider
    addAndMakeVisible(qSlider);
    qSlider.setRange(0.1, 10.0, 0.1);
    qSlider.setValue(1.0);
    qSlider.setTextValueSuffix(" Q");
    qSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    qSlider.setPopupDisplayEnabled(true, false, this);
    qSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);

    qLabel.setText("Q Factor", juce::dontSendNotification);
    addAndMakeVisible(qLabel);
    qLabel.attachToComponent(&qSlider, true);

    // Gain Slider
    addAndMakeVisible(gainSlider);
    gainSlider.setRange(-12.0, 12.0, 0.1); // Gain in dB
    gainSlider.setValue(0.0);
    gainSlider.setTextValueSuffix(" dB");
    gainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    gainSlider.setPopupDisplayEnabled(true, false, this);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);

    gainLabel.setText("Gain", juce::dontSendNotification);
    addAndMakeVisible(gainLabel);
    gainLabel.attachToComponent(&gainSlider, true);
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
