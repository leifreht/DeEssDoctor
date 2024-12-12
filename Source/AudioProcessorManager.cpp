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

    // Assign default de-essing algorithm
    deEssingAlgorithm = [this](juce::AudioBuffer<float>& buffer, float threshold, float mixLevel, float frequency, int hysteresisSamples)
    {
        defaultDeEssingAlgorithm(buffer, threshold, mixLevel, frequency, hysteresisSamples);
    };
}

void AudioProcessorManager::prepare(double sampleRate, int samplesPerBlock, int numChannels)
{
    juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32>(samplesPerBlock), static_cast<juce::uint32>(numChannels) };
    highPassFilter.prepare(spec);
    highPassFilter.reset();

    allPassFilter.prepare(spec);
    allPassFilter.reset();
}

void AudioProcessorManager::setDeEssingAlgorithm(std::function<void(juce::AudioBuffer<float>&, float, float, float, int)> algorithm)
{
    deEssingAlgorithm = std::move(algorithm);
}

void AudioProcessorManager::setDeEssingParameters(float newThreshold, float newMixLevel, float newFrequency, float newHysteresis)
{
    threshold = newThreshold;
    mixLevel = newMixLevel;
    frequency = newFrequency;
    hysteresisSamples = (int)newHysteresis;

    highPassFilter.setCutoffFrequency(frequency);
}

void AudioProcessorManager::processFileForSibilants(const juce::File& file)
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    auto reader = std::unique_ptr<juce::AudioFormatReader>(formatManager.createReaderFor(file));
    if (reader == nullptr)
    {
        DBG("Failed to create AudioFormatReader for the file.");
        return;
    }

    // Resize buffers
    originalBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
    processedBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
    sibilantBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);

    // Read the audio data into originalBuffer
    reader->read(&originalBuffer, 0, (int)reader->lengthInSamples, 0, true, true);

    // Copy the original audio to processedBuffer for modification
    processedBuffer.makeCopyOf(originalBuffer);

    // Apply the selected algorithm to generate sibilants and de-essed audio
    deEssingAlgorithm(processedBuffer, threshold, mixLevel, frequency, hysteresisSamples);
//    deEssingAlgorithm(processedBuffer, sibilantBuffer, threshold, mixLevel, frequency, hysteresisSamples);
}

void AudioProcessorManager::defaultDeEssingAlgorithm(juce::AudioBuffer<float>& buffer, float threshold, float mixLevel, float frequency, int hysteresisSamples)
{
    // 'buffer' here is our processedBuffer (already a copy of the original)
    // We'll create a temporary sibilantBuffer copy for processing the sibilant regions
    juce::AudioBuffer<float> sibilantBufferTemp;
    sibilantBufferTemp.makeCopyOf(buffer);

    juce::dsp::AudioBlock<float> sibilantBlock(sibilantBufferTemp);
    juce::dsp::ProcessContextReplacing<float> sibilantContext(sibilantBlock);
    highPassFilter.process(sibilantContext);

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* sibilantData = sibilantBufferTemp.getWritePointer(channel);
        auto* processedData = buffer.getWritePointer(channel);

        int counter = 0;
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // Sibilant detection: if above threshold, reset hysteresis counter
            if (std::abs(sibilantData[sample]) > juce::Decibels::decibelsToGain(threshold))
            {
                counter = hysteresisSamples;
            }

            if (counter > 0)
            {
                // Mark sibilant region
                counter--;
                // Reduce sibilant in the processed buffer
                processedData[sample] -= sibilantData[sample] * juce::Decibels::decibelsToGain(mixLevel);
            }
            else
            {
                // Outside sibilant region, zero out in sibilant buffer
                sibilantData[sample] = 0.0f;
            }
        }
    }

    // Now sibilantBufferTemp contains only sibilant sounds where detected.
    // Update the class member sibilantBuffer with the isolated sibilants.
    sibilantBuffer.makeCopyOf(sibilantBufferTemp);
}

//void AudioProcessorManager::applyDeEssing(juce::AudioBuffer<float>& buffer)
//{
//    
//    juce::AudioBuffer<float> sibilantBuffer;
//    sibilantBuffer.makeCopyOf(buffer);
//    
//    juce::AudioBuffer<float> originalBuffer;
//    originalBuffer.makeCopyOf(buffer);
//    
//    // Apply high-pass filter to isolate sibilants
//    juce::dsp::AudioBlock<float> sibilantBlock(sibilantBuffer);
//    juce::dsp::ProcessContextReplacing<float> sibilantContext(sibilantBlock);
//    highPassFilter.process(sibilantContext);
//    
//    // Apply all-pass filter to the original buffer to align delays
//    juce::dsp::AudioBlock<float> originalBlock(originalBuffer);
//    juce::dsp::ProcessContextReplacing<float> originalContext(originalBlock);
//    allPassFilter.process(originalContext);
//    
//    buffer.clear();
//    
//    if (hysteresisCounters.size() != static_cast<size_t>(buffer.getNumChannels()))
//    {
//        hysteresisCounters.resize(buffer.getNumChannels(), 0);
//    }
//    
//    
//    // Process each channel for sibilant detection and removal
//    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
//    {
//        auto* originalData = originalBuffer.getWritePointer(channel);
//        auto* sibilantData = sibilantBuffer.getWritePointer(channel);
//        
//        int& counter = hysteresisCounters[channel];
//        
//        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
//        {
//            // Check if the sample crosses the upper threshold
//            if (std::abs(sibilantData[sample]) > juce::Decibels::decibelsToGain(threshold))
//            {
//                counter = hysteresisSamples; // Reset the counter
//            }
//            
//            // If the counter is active, classify as sibilant
//            if (counter > 0)
//            {
//                --counter;
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
//    //    DBG("Gain factor: " << gainFactor << " Mix level: " << mixLevel);
//    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
//    {
//        auto* originalData = originalBuffer.getWritePointer(channel);
//        auto* sibilantData = sibilantBuffer.getWritePointer(channel);
//        
//        auto* finalData = buffer.getWritePointer(channel);
//        
//        lastSibilantBuffer.makeCopyOf(sibilantBuffer);
//        
//        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
//        {
//            finalData[sample] = originalData[sample]; // Copy original data
//            finalData[sample] += gainFactor * sibilantData[sample]; // Mix sibilants back
//        }
//    }
//}

//void AudioProcessorManager::getSibilantBuffer(juce::AudioBuffer<float>& buffer) const
//{
//    buffer.makeCopyOf(lastSibilantBuffer); 
//}
//
//void AudioProcessorManager::processFileForSibilants(const juce::File& file)
//{
//    juce::AudioFormatManager formatManager;
//    formatManager.registerBasicFormats();
//
//    auto reader = std::unique_ptr<juce::AudioFormatReader>(formatManager.createReaderFor(file));
//    if (reader == nullptr)
//    {
//        DBG("Failed to create AudioFormatReader for the file.");
//        return;
//    }
//
//    processedBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
//    reader->read(&processedBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
//
//    applyDeEssing(processedBuffer);
//
//    // Save sibilant buffer for visualization
//    lastSibilantBuffer.makeCopyOf(processedBuffer);
//}

