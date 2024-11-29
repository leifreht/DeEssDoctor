/*
  ==============================================================================

    AudioProcessorManager.cpp
    Created: 18 Nov 2024 12:07:09pm
    Author:  Leif Rehtanz

  ==============================================================================
*/

#include "AudioProcessorManager.h"
#include "Algorithms.h"

AudioProcessorManager::AudioProcessorManager()
    : mixLevel(0.5f) // Default to 50% mix
{
    // Set a default algorithm (e.g., amplitude threshold)
    setAlgorithm(amplitudeThresholdAlgorithm);

    // Default filter: low-pass at 20 kHz (effectively bypass)
    filterCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100.0f, 20000.0f);
    iirFilter.coefficients = filterCoefficients;

    // Note: No need to call prepare here; it will be called externally
}

void AudioProcessorManager::prepare(double sampleRate, int samplesPerBlock, int numChannels)
{
    // Prepare the filter with the given sample rate and number of channels
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = numChannels;

    iirFilter.prepare(spec);
}

void AudioProcessorManager::setAlgorithm(std::function<void(juce::AudioBuffer<float>&)> newAlgorithm)
{
    currentAlgorithm = newAlgorithm;
}

void AudioProcessorManager::setFilterParameters(float frequency, float q, float gain)
{
    // Create peak filter coefficients dynamically
    filterCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100.0f, frequency, q, juce::Decibels::decibelsToGain(gain));
    // Update the filter's coefficients
    *iirFilter.coefficients = *filterCoefficients;
}

void AudioProcessorManager::setMixLevel(float newMixLevel)
{
    mixLevel = juce::jlimit(0.0f, 1.0f, newMixLevel); // Clamp mixLevel to [0.0, 1.0]
}

void AudioProcessorManager::processBlock(juce::AudioBuffer<float>& buffer)
{
    if (currentAlgorithm)
    {
        // Create a copy of the original buffer for processing
        juce::AudioBuffer<float> processedBuffer;
        processedBuffer.makeCopyOf(buffer);

        // Apply the S-detection algorithm
        currentAlgorithm(processedBuffer);

        // Apply filtering to the detected S-regions
        applyFilter(processedBuffer);

        // Mix processed and original audio based on mixLevel
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            buffer.applyGain(channel, 0, buffer.getNumSamples(), 1.0f - mixLevel); // Scale original
            processedBuffer.applyGain(channel, 0, processedBuffer.getNumSamples(), mixLevel); // Scale processed
            buffer.addFrom(channel, 0, processedBuffer, channel, 0, buffer.getNumSamples()); // Add them
        }
    }
}

void AudioProcessorManager::applyFilter(juce::AudioBuffer<float>& buffer)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    iirFilter.process(context);
}
