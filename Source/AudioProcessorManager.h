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

class AudioProcessorManager
{
public:
    AudioProcessorManager();
    ~AudioProcessorManager() = default;

    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void setAlgorithm(std::function<void(juce::AudioBuffer<float>&)> newAlgorithm);
    void setFilterParameters(float frequency, float q, float gain);
    void setMixLevel(float mixLevel);

    void processBlock(juce::AudioBuffer<float>& buffer);

private:
    std::function<void(juce::AudioBuffer<float>&)> currentAlgorithm; // Active S-detection algorithm
    juce::dsp::IIR::Filter<float> iirFilter;                         // Filter for processing
    juce::dsp::IIR::Coefficients<float>::Ptr filterCoefficients;     // Filter coefficients

    float mixLevel; // 0.0 (original) to 1.0 (processed)

    void applyFilter(juce::AudioBuffer<float>& buffer);
};
 
