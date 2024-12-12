/*
  ==============================================================================

    AudioProcessorManager.h
    Created: 18 Nov 2024 12:07:30pm
    Author:  Leif Rehtanz

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <functional>
#include <vector>
#include "SibilantRegion.h" // Ensure this is included

class AudioProcessorManager
{
public:
    AudioProcessorManager();
    ~AudioProcessorManager() = default;

    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void setDeEssingParameters(float newThreshold, float newReduction, float newFrequency, float newHysteresis);
    void processBlock(juce::AudioBuffer<float>& buffer);
    void processFileForSibilants(const juce::File& file);

    const juce::AudioBuffer<float>& getOriginalBuffer() const { return originalBuffer; }
    const juce::AudioBuffer<float>& getProcessedBuffer() const { return processedBuffer; }
    const juce::AudioBuffer<float>& getSibilantBuffer() const { return sibilantBuffer; }

    void setDeEssingAlgorithm(std::function<void(juce::AudioBuffer<float>&, float, float, float, int)> algorithm);
    
    double getSampleRate() const { return processedSampleRate; }

private:
    double processedSampleRate = 44100.0;
    
    juce::AudioBuffer<float> originalBuffer;
    juce::AudioBuffer<float> processedBuffer;
    juce::AudioBuffer<float> sibilantBuffer;

    // Filters for default de-essing algorithm
    juce::dsp::LinkwitzRileyFilter<float> highPassFilter;
    juce::dsp::LinkwitzRileyFilter<float> allPassFilter;

    // Parameters
    float threshold{ -20.0f };
    float mixLevel{ 0.0f };
    float frequency{ 6500.0f };
    int hysteresisSamples{ 100 };
//    std::vector<int> hysteresisCounters; 

    // Customizable de-essing algorithm
    std::function<void(juce::AudioBuffer<float>&, float, float, float, int)> deEssingAlgorithm;

    // Default de-essing logic
    void defaultDeEssingAlgorithm(juce::AudioBuffer<float>& buffer, float threshold, float mixLevel, float frequency, int hysteresisSamples);
};
