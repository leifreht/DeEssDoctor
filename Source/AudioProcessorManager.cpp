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

void AudioProcessorManager::setDeEssingParameters(float newThreshold, float newMixLevel, float newFrequency)
{
    threshold = newThreshold;
    mixLevel = newMixLevel;
    frequency = newFrequency;
    highPassFilter.setCutoffFrequency(frequency);
}

void AudioProcessorManager::processBlock(juce::AudioBuffer<float>& buffer)
{
    applyDeEssing(buffer);
}

//void AudioProcessorManager::applyDeEssing(juce::AudioBuffer<float>& buffer)
//{
//    juce::AudioBuffer<float> highPassedBuffer;
//    highPassedBuffer.makeCopyOf(buffer);
//
//    juce::dsp::AudioBlock<float> block(highPassedBuffer);
//    juce::dsp::ProcessContextReplacing<float> context(block);
//    highPassFilter.process(context);
//    
//    juce::dsp::AudioBlock<float> originalBlock(buffer);
//    juce::dsp::ProcessContextReplacing<float> originalContext(originalBlock);
//    allPassFilter.process(originalContext);
//    
//    buffer.clear();
//
//    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
//    {
////        auto* originalData = buffer.getWritePointer(channel);
//        auto* highPassedData = highPassedBuffer.getWritePointer(channel);
//        auto* outputData = buffer.getWritePointer(channel);
//
//        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
//        {
//            if (std::abs(highPassedData[sample]) > juce::Decibels::decibelsToGain(threshold))
//            {
////                originalData[sample] -= reduction * highPassedData[sample];
//                outputData[sample] = highPassedData[sample];
//            }
//        }
//    }
//}

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
        
    // Process each channel for sibilant detection and removal
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* originalData = originalBuffer.getWritePointer(channel);
        auto* sibilantData = sibilantBuffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // Detect sibilants above the threshold
            if (std::abs(sibilantData[sample]) > juce::Decibels::decibelsToGain(threshold))
            {
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
    DBG("Gain factor: " << gainFactor << " Mix level: " << mixLevel);
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

//void AudioProcessorManager::applyDeEssing(juce::AudioBuffer<float>& buffer)
//{
//    juce::AudioBuffer<float> sibilantBuffer;
//    sibilantBuffer.makeCopyOf(buffer); // Copy the input buffer for sibilants
//
//    // Apply high-pass filter to isolate sibilants
//    juce::dsp::AudioBlock<float> block(sibilantBuffer);
//    juce::dsp::ProcessContextReplacing<float> context(block);
//    highPassFilter.process(context);
//
//    // Process each channel for sibilant detection and removal
//    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
//    {
//        auto* originalData = buffer.getWritePointer(channel);
//        auto* sibilantData = sibilantBuffer.getWritePointer(channel);
//
//        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
//        {
//            // Detect sibilants above the threshold
//            if (std::abs(sibilantData[sample]) > juce::Decibels::decibelsToGain(threshold))
//            {
//                originalData[sample] -= sibilantData[sample]; // Subtract sibilant from original
//            }
//            else
//            {
//                sibilantData[sample] = 0.0f; // Zero out non-sibilant regions
//            }
//        }
//    }
//
//    // Mix adjusted sibilants back into the original signal
//    float gainFactor = juce::Decibels::decibelsToGain(mixLevel);
//    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
//    {
//        auto* originalData = buffer.getWritePointer(channel);
//        auto* sibilantData = sibilantBuffer.getWritePointer(channel);
//
//        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
//        {
//            originalData[sample] += gainFactor * sibilantData[sample]; // Mix sibilants back
//        }
//    }
//}
