/*
  ==============================================================================

    AudioProcessorManager.cpp
    Created: 18 Nov 2024 12:07:09pm
    Author:  Leif Rehtanz

  ==============================================================================
*/

#include "AudioProcessorManager.h"

AudioProcessorManager::AudioProcessorManager()
{
    highPassFilter.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    highPassFilter.setCutoffFrequency(frequency);
}

void AudioProcessorManager::prepare(double sampleRate, int samplesPerBlock, int numChannels)
{
    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(samplesPerBlock), static_cast<juce::uint32>(numChannels) };
    highPassFilter.prepare(spec);
    highPassFilter.reset();
}

void AudioProcessorManager::setDeEssingParameters(float newThreshold, float newReduction, float newFrequency)
{
    threshold = newThreshold;
    reduction = newReduction;
    frequency = newFrequency;
    highPassFilter.setCutoffFrequency(frequency);
}

void AudioProcessorManager::processBlock(juce::AudioBuffer<float>& buffer)
{
    applyDeEssing(buffer);
}

void AudioProcessorManager::applyDeEssing(juce::AudioBuffer<float>& buffer)
{
    juce::AudioBuffer<float> highPassedBuffer;
    highPassedBuffer.makeCopyOf(buffer);

    juce::dsp::AudioBlock<float> block(highPassedBuffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    highPassFilter.process(context);

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* originalData = buffer.getWritePointer(channel);
        auto* highPassedData = highPassedBuffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            if (std::abs(highPassedData[sample]) > threshold)
            {
                originalData[sample] -= reduction * highPassedData[sample];
            }
        }
    }
}
