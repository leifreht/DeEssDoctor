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
    void setDeEssingParameters(float newThreshold, float newReduction, float newFrequency);
    void processBlock(juce::AudioBuffer<float>& buffer);

private:
    std::function<void(juce::AudioBuffer<float>&)> deEssingAlgorithm;
    juce::dsp::LinkwitzRileyFilter<float> highPassFilter;
    float threshold { 0.2f };
    float reduction { 0.5f };
    float frequency { 4000.0f };

    void applyDeEssing(juce::AudioBuffer<float>& buffer);
};
 
