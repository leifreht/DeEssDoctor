/*
  ==============================================================================

    Algorithms.h
    Created: 18 Nov 2024 1:55:04pm
    Author:  Leif Rehtanz

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// Declare the available S-detection algorithms
void amplitudeThresholdAlgorithm(juce::AudioBuffer<float>& buffer);
void spectralAnalysisAlgorithm(juce::AudioBuffer<float>& buffer);
