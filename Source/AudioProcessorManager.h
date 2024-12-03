/*
  ==============================================================================

    AudioProcessorManager.h
    Created: 18 Nov 2024 12:07:30pm
    Author:  Leif Rehtanz

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <memory>
#include <functional>
#include <vector>
#include "SibilantRegion.h"

//// Struct to hold detected sibilant regions
//struct SibilantRegion
//{
//    int startSample;
//    int endSample;
//};

class AudioProcessorManager
{
public:
    AudioProcessorManager();
    ~AudioProcessorManager() = default;

    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void setAlgorithm(std::function<void(juce::AudioBuffer<float>&, std::vector<SibilantRegion>&)> newAlgorithm);
    void setFilterParameters(float frequency, float q, float gain);
    void setMixLevel(float mixLevel);

    void processBlock(juce::AudioBuffer<float>& buffer);

    // Get detected sibilant regions for visualization
    const std::vector<SibilantRegion>& getSibilantRegions() const { return sibilantRegions; }

private:
    std::function<void(juce::AudioBuffer<float>&, std::vector<SibilantRegion>&)> currentAlgorithm; // S-detection algorithm
    juce::dsp::IIR::Filter<float> highPassFilter;                         // High-pass filter for sibilant isolation
    juce::dsp::IIR::Coefficients<float>::Ptr highPassCoefficients;       // High-pass filter coefficients

    // Processing chain for sibilants
    juce::dsp::IIR::Filter<float> highShelfFilter;
    juce::dsp::IIR::Coefficients<float>::Ptr highShelfCoefficients;

    float mixLevel; // 0.0 (original) to 1.0 (processed)

    std::vector<SibilantRegion> sibilantRegions; // Detected sibilant regions

    void applyHighPassFilter(juce::AudioBuffer<float>& buffer);
    void applyHighShelfFilter(juce::AudioBuffer<float>& buffer);
};
