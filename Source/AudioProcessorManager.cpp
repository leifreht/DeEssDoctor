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
    
    allPassFilter.setType(juce::dsp::LinkwitzRileyFilterType::allpass);
}

void AudioProcessorManager::prepare(double sampleRate, int samplesPerBlock, int numChannels)
{
    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(samplesPerBlock), static_cast<juce::uint32>(numChannels) };
    highPassFilter.prepare(spec);
    highPassFilter.reset();
    
    allPassFilter.prepare(spec);
    allPassFilter.reset();
}

void AudioProcessorManager::setDeEssingParameters(float newThreshold, float newMixLevel, float newFrequency, float newHysteresis)
{
    threshold = newThreshold;
    mixLevel = newMixLevel;
    frequency = newFrequency;
    hysteresisSamples = (int) newHysteresis;
    highPassFilter.setCutoffFrequency(frequency);
}

void AudioProcessorManager::processBlock(juce::AudioBuffer<float>& buffer)
{
    applyDeEssing(buffer);
}

void AudioProcessorManager::applyDeEssing(juce::AudioBuffer<float>& buffer)
{
    // Create a buffer to store sibilants
    juce::AudioBuffer<float> sibilantBuffer;
    sibilantBuffer.makeCopyOf(buffer);
    
    juce::AudioBuffer<float> originalBuffer;
    originalBuffer.makeCopyOf(buffer);
    
    // Apply high-pass filter to isolate sibilants
    juce::dsp::AudioBlock<float> sibilantBlock(sibilantBuffer);
    juce::dsp::ProcessContextReplacing<float> sibilantContext(sibilantBlock);
    highPassFilter.process(sibilantContext);
    
    // Apply all-pass filter to the original buffer to align delays
    juce::dsp::AudioBlock<float> originalBlock(originalBuffer);
    juce::dsp::ProcessContextReplacing<float> originalContext(originalBlock);
    allPassFilter.process(originalContext);
    
    buffer.clear();
    
    if (hysteresisCounters.size() != static_cast<size_t>(buffer.getNumChannels()))
    {
        hysteresisCounters.resize(buffer.getNumChannels(), 0);
    }
    
    
    // Process each channel for sibilant detection and removal
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* originalData = originalBuffer.getWritePointer(channel);
        auto* sibilantData = sibilantBuffer.getWritePointer(channel);
        
        int& counter = hysteresisCounters[channel];
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            //            // Detect sibilants above the threshold
            //            if (std::abs(sibilantData[sample]) > juce::Decibels::decibelsToGain(threshold))
            //            {
            //                originalData[sample] -= sibilantData[sample]; // Subtract sibilant from original
            //            }
            //            else
            //            {
            //                sibilantData[sample] = 0.0f; // Zero out non-sibilant regions
            //            }
            // Check if the sample crosses the upper threshold
            if (std::abs(sibilantData[sample]) > juce::Decibels::decibelsToGain(threshold))
            {
                counter = hysteresisSamples; // Reset the counter
            }
            
            // If the counter is active, classify as sibilant
            if (counter > 0)
            {
                --counter;
                originalData[sample] -= sibilantData[sample]; // Subtract sibilant from original
            }
            else
            {
                sibilantData[sample] = 0.0f; // Zero out non-sibilant regions
            }
        }
    }
    
    // Mix adjusted sibilants back into the original signal
    float gainFactor = juce::Decibels::decibelsToGain(mixLevel);
    //    DBG("Gain factor: " << gainFactor << " Mix level: " << mixLevel);
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* originalData = originalBuffer.getWritePointer(channel);
        auto* sibilantData = sibilantBuffer.getWritePointer(channel);
        
        auto* finalData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            finalData[sample] = originalData[sample]; // Copy original data
            finalData[sample] += gainFactor * sibilantData[sample]; // Mix sibilants back
        }
    }
}

