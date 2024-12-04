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

    // High-pass filter to isolate sibilants (e.g., cutoff at 3000 Hz)
    highPassCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(44100.0f, 3000.0f, 1.0f);
    highPassFilter.coefficients = highPassCoefficients;

    // High-shelf filter for processing sibilants
    highShelfCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(44100.0f, 5000.0f, 0.707f, juce::Decibels::decibelsToGain(1.0f)); // Example values
    highShelfFilter.coefficients = highShelfCoefficients;
}

void AudioProcessorManager::prepare(double sampleRate, int samplesPerBlock, int numChannels)
{
    // Prepare the high-pass filter
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1; // Mono

    highPassFilter.prepare(spec);
    highPassFilter.reset();

    // Prepare the high-shelf filter
    highShelfFilter.prepare(spec);
    highShelfFilter.reset();
}

void AudioProcessorManager::setAlgorithm(std::function<void(juce::AudioBuffer<float>&, std::vector<SibilantRegion>&)> newAlgorithm)
{
    currentAlgorithm = newAlgorithm;
}

void AudioProcessorManager::setFilterParameters(float frequency, float q, float gain)
{
    // Update high-shelf filter coefficients
    highShelfCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(44100.0f, frequency, q, juce::Decibels::decibelsToGain(gain));
    *highShelfFilter.coefficients = *highShelfCoefficients;
}

void AudioProcessorManager::setMixLevel(float newMixLevel)
{
    mixLevel = juce::jlimit(0.0f, 1.0f, newMixLevel); // Clamp mixLevel to [0.0, 1.0]
}

void AudioProcessorManager::processBlock(juce::AudioBuffer<float>& buffer)
{
    if (currentAlgorithm)
    {
        // Step 1: Apply high-pass filter to isolate sibilants
        applyHighPassFilter(buffer);

        // Step 2: Detect sibilant regions
        sibilantRegions.clear();
        currentAlgorithm(buffer, sibilantRegions);

        // Step 3: Create a copy of the original buffer for processing
        juce::AudioBuffer<float> processedBuffer;
        processedBuffer.makeCopyOf(buffer);

        // Wrap processedBuffer in an AudioBlock
        juce::dsp::AudioBlock<float> processedBlock(processedBuffer);

        // Step 4: Extract and process sibilant regions
        for (const auto& region : sibilantRegions)
        {
            // Ensure region boundaries are within buffer
            int start = juce::jmax(0, region.startSample);
            int end = juce::jmin(buffer.getNumSamples(), region.endSample);

            // Process each channel separately
            for (size_t channel = 0; channel < processedBlock.getNumChannels(); ++channel)
            {
                // Get the sub-block for the sibilant region
                auto subBlock = processedBlock.getSingleChannelBlock(channel).getSubBlock((size_t)start, (size_t)(end - start));

                // Create a ProcessContextReplacing with the subblock
                juce::dsp::ProcessContextReplacing<float> context(subBlock);

                // Apply the high-shelf filter to the subblock
                highShelfFilter.process(context);
            }
        }

        // Step 5: Mix processed sibilants back with the original signal
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            buffer.applyGain(channel, 0, buffer.getNumSamples(), 1.0f - mixLevel); // Scale original
            processedBuffer.applyGain(channel, 0, processedBuffer.getNumSamples(), mixLevel); // Scale processed
            buffer.addFrom(channel, 0, processedBuffer, channel, 0, buffer.getNumSamples()); // Add them
        }
    }
}

void AudioProcessorManager::applyHighPassFilter(juce::AudioBuffer<float>& buffer)
{
    juce::dsp::AudioBlock<float> block(buffer);
    for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
    {
        auto channelBlock = block.getSingleChannelBlock(channel);
        juce::dsp::ProcessContextReplacing<float> context(channelBlock);
        highPassFilter.process(context);
    }
}

void AudioProcessorManager::applyHighShelfFilter(juce::AudioBuffer<float>& buffer)
{
    juce::dsp::AudioBlock<float> block(buffer);
    for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
    {
        auto channelBlock = block.getSingleChannelBlock(channel);
        juce::dsp::ProcessContextReplacing<float> context(channelBlock);
        highShelfFilter.process(context);
    }
}



//AudioProcessorManager::AudioProcessorManager()
//    : mixLevel(0.5f) // Default to 50% mix
//{
//    // Set a default algorithm (e.g., amplitude threshold)
//    setAlgorithm(amplitudeThresholdAlgorithm);
//
//    // High-pass filter to isolate sibilants (e.g., cutoff at 3000 Hz)
//    highPassCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(44100.0f, 3000.0f, 1.0f);
//    highPassFilter.coefficients = highPassCoefficients;
//
//    // High-shelf filter for processing sibilants
//    highShelfCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(44100.0f, 5000.0f, 0.707f, 1.0f); // Example values
//    highShelfFilter.coefficients = highShelfCoefficients;
//}
//
//void AudioProcessorManager::prepare(double sampleRate, int samplesPerBlock, int numChannels)
//{
//    // Prepare the high-pass filter
//    juce::dsp::ProcessSpec spec;
//    spec.sampleRate = sampleRate;
//    spec.maximumBlockSize = samplesPerBlock;
//    spec.numChannels = numChannels;
//
//    highPassFilter.prepare(spec);
//    highPassFilter.reset();
//
//    // Prepare the high-shelf filter
//    highShelfFilter.prepare(spec);
//    highShelfFilter.reset();
//}
//
//void AudioProcessorManager::setAlgorithm(std::function<void(juce::AudioBuffer<float>&, std::vector<SibilantRegion>&)> newAlgorithm)
//{
//    currentAlgorithm = newAlgorithm;
//}
//
//void AudioProcessorManager::setFilterParameters(float frequency, float q, float gain)
//{
//    // Update high-shelf filter parameters
//    highShelfCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(44100.0f, frequency, q, juce::Decibels::decibelsToGain(gain));
//    *highShelfFilter.coefficients = *highShelfCoefficients;
//}
//
//void AudioProcessorManager::setMixLevel(float newMixLevel)
//{
//    mixLevel = juce::jlimit(0.0f, 1.0f, newMixLevel); // Clamp mixLevel to [0.0, 1.0]
//}
//
//
//void AudioProcessorManager::processBlock(juce::AudioBuffer<float>& buffer)
//{
//    if (currentAlgorithm)
//    {
//        // Step 1: Apply high-pass filter to isolate sibilants
//        applyHighPassFilter(buffer);
//
//        // Step 2: Detect sibilant regions
//        sibilantRegions.clear();
//        currentAlgorithm(buffer, sibilantRegions);
//
//        // Step 3: Create a copy of the original buffer for processing
//        juce::AudioBuffer<float> processedBuffer;
//        processedBuffer.makeCopyOf(buffer);
//
//        // Step 4: Extract and process sibilant regions
//        for (const auto& region : sibilantRegions)
//        {
//            // Ensure region boundaries are within buffer
//            int start = juce::jmax(0, region.startSample);
//            int end = juce::jmin(buffer.getNumSamples(), region.endSample);
//
//            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
//            {
//                // Create an AudioBlock for the buffer
//                juce::dsp::AudioBlock<float> block(processedBuffer);
//
//                // Get a single-channel block
//                auto channelBlock = block.getSingleChannelBlock(channel);
//
//                // Get the subblock for the sibilant region
//                auto subBlock = channelBlock.getSubBlock(start, end - start);
//
//                // Create a ProcessContextReplacing with the subblock
//                juce::dsp::ProcessContextReplacing<float> context(subBlock);
//
//                // Apply the high-shelf filter to the subblock
//                highShelfFilter.process(context);
//            }
//        }
//
//        // Step 5: Mix processed sibilants back with the original signal
//        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
//        {
//            buffer.applyGain(channel, 0, buffer.getNumSamples(), 1.0f - mixLevel); // Scale original
//            processedBuffer.applyGain(channel, 0, processedBuffer.getNumSamples(), mixLevel); // Scale processed
//            buffer.addFrom(channel, 0, processedBuffer, channel, 0, buffer.getNumSamples()); // Add them
//        }
//    }
//}
//
//void AudioProcessorManager::applyHighPassFilter(juce::AudioBuffer<float>& buffer)
//{
//    juce::dsp::AudioBlock<float> block(buffer);
//    juce::dsp::ProcessContextReplacing<float> context(block);
//    highPassFilter.process(context);
//}
//
//void AudioProcessorManager::applyHighShelfFilter(juce::AudioBuffer<float>& buffer)
//{
//    juce::dsp::AudioBlock<float> block(buffer);
//    juce::dsp::ProcessContextReplacing<float> context(block);
//    highShelfFilter.process(context);
//}
