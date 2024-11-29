/*
  ==============================================================================

    AlgorithmSelector.cpp
    Created: 18 Nov 2024 12:07:58pm
    Author:  Leif Rehtanz

  ==============================================================================
*/

#include "AlgorithmSelector.h"

AlgorithmSelector::AlgorithmSelector()
{
    addAndMakeVisible(algorithmDropdown);

    algorithmDropdown.addItem("Amplitude Threshold", 1);
    algorithmDropdown.addItem("Spectral Analysis", 2);
    algorithmDropdown.setSelectedId(1); // Default to first algorithm

    algorithmDropdown.onChange = [this]() { selectionChanged(); };
}

void AlgorithmSelector::selectionChanged()
{
    if (algorithmChanged) // Trigger the callback
        algorithmChanged();
}

juce::String AlgorithmSelector::getSelectedAlgorithm() const
{
    return algorithmDropdown.getText();
}

void AlgorithmSelector::resized()
{
    auto area = getLocalBounds().reduced(10);
    algorithmDropdown.setBounds(area);
}
