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

class AudioProcessorManager
{
public:
    AudioProcessorManager();
    ~AudioProcessorManager() = default;

    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void setDeEssingParameters(float newThreshold, float newReduction, float newFrequency, float newHysteresis);

    void processBlock(juce::AudioBuffer<float>& buffer);
    void processFileForSibilants(const juce::File& file);

    const juce::AudioBuffer<float>& getProcessedBuffer() const { return processedBuffer; }

    void setDeEssingAlgorithm(std::function<void(juce::AudioBuffer<float>&, float, float, float, int)> algorithm);

private:
    juce::AudioBuffer<float> processedBuffer;

    // Filters for default de-essing algorithm
    juce::dsp::LinkwitzRileyFilter<float> highPassFilter;
    juce::dsp::LinkwitzRileyFilter<float> allPassFilter;

    // Parameters
    float threshold{ -20.0f };
    float mixLevel{ 0.0f };
    float frequency{ 6500.0f };
    int hysteresisSamples{ 100 };

    // Customizable de-essing algorithm
    std::function<void(juce::AudioBuffer<float>&, float, float, float, int)> deEssingAlgorithm;

    // Default de-essing logic
    void defaultDeEssingAlgorithm(juce::AudioBuffer<float>& buffer, float threshold, float mixLevel, float frequency, int hysteresisSamples);
};
