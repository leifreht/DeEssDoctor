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
    
    void getSibilantBuffer(juce::AudioBuffer<float>& buffer) const;
    
    void processFileForSibilants(const juce::File& file);
    const juce::AudioBuffer<float>& getProcessedBuffer() const { return processedBuffer; }

private:
    juce::AudioBuffer<float> processedBuffer; 
    
    juce::dsp::LinkwitzRileyFilter<float> highPassFilter;
    juce::dsp::LinkwitzRileyFilter<float> allPassFilter;
    
    float threshold { -20.0f };
    float mixLevel { 0.0f };
    float frequency { 6500.0f };
    int hysteresisSamples = 100;
    std::vector<int> hysteresisCounters;

    void applyDeEssing(juce::AudioBuffer<float>& buffer);
    
    juce::AudioBuffer<float> lastSibilantBuffer;
};
 
