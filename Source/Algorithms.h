/*
  ==============================================================================

    Algorithms.h
    Created: 18 Nov 2024 1:55:04pm
    Author:  Leif Rehtanz

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include "SibilantRegion.h" 

//// Struct to hold detected sibilant regions
//struct SibilantRegion
//{
//    int startSample;
//    int endSample;
//};

// Declare the available S-detection algorithms
void amplitudeThresholdAlgorithm(juce::AudioBuffer<float>& buffer, std::vector<SibilantRegion>& detectedRegions);
void spectralAnalysisAlgorithm(juce::AudioBuffer<float>& buffer, std::vector<SibilantRegion>& detectedRegions);
