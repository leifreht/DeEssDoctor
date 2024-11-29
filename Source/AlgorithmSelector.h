/*
  ==============================================================================

    AlgorithmSelector.h
    Created: 18 Nov 2024 12:08:07pm
    Author:  Leif Rehtanz

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <functional>

class AlgorithmSelector : public juce::Component
{
public:
    AlgorithmSelector();
    ~AlgorithmSelector() override = default;

    juce::String getSelectedAlgorithm() const;

    std::function<void()> algorithmChanged; // Callback for when algorithm changes
    
    void resized() override;

private:
    juce::ComboBox algorithmDropdown;

    void selectionChanged();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AlgorithmSelector)
};
