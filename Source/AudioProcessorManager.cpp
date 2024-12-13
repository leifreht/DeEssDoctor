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
    processedSampleRate = reader->sampleRate;

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

void AudioProcessorManager::defaultDeEssingAlgorithm(juce::AudioBuffer<float>& buffer,
                                                     float threshold, float mixLevel,
                                                     float frequency, int hysteresisSamples)
{
    // Make copies of the input buffer for processing
    juce::AudioBuffer<float> originalBuffer;
    juce::AudioBuffer<float> sibilantBufferTemp;

    originalBuffer.makeCopyOf(buffer);
    sibilantBufferTemp.makeCopyOf(buffer);

    // High-pass filter to isolate sibilants
    {
        juce::dsp::AudioBlock<float> sibilantBlock(sibilantBufferTemp);
        juce::dsp::ProcessContextReplacing<float> sibilantContext(sibilantBlock);
        highPassFilter.setCutoffFrequency(frequency);
        highPassFilter.process(sibilantContext);
    }

    // All-pass filter to align phase/time with the high-passed signal
    {
        juce::dsp::AudioBlock<float> originalBlock(originalBuffer);
        juce::dsp::ProcessContextReplacing<float> originalContext(originalBlock);
        allPassFilter.process(originalContext);
    }

    // Clear the final buffer to build from scratch
    buffer.clear();

    int numChannels = buffer.getNumChannels();
    int numSamples = buffer.getNumSamples();

    // Hysteresis-based sibilant detection and subtraction
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* originalData = originalBuffer.getWritePointer(channel);
        auto* sibilantData = sibilantBufferTemp.getWritePointer(channel);

        int counter = 0; // local hysteresis counter

        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Check threshold
            if (std::abs(sibilantData[sample]) > juce::Decibels::decibelsToGain(threshold))
            {
                counter = hysteresisSamples;
            }

            if (counter > 0)
            {
                // Still in sibilant region
                --counter;
                originalData[sample] -= sibilantData[sample];
            }
            else
            {
                // Outside sibilant region
                sibilantData[sample] = 0.0f;
            }
        }
    }

    // Mix adjusted sibilants back into the original signal
    float gainFactor = juce::Decibels::decibelsToGain(mixLevel);
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* originalData = originalBuffer.getWritePointer(channel);
        auto* sibilantData = sibilantBufferTemp.getWritePointer(channel);
        auto* finalData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            finalData[sample] = originalData[sample] + gainFactor * sibilantData[sample];
        }
    }

    // Update the class member sibilantBuffer for visualization
    sibilantBuffer.makeCopyOf(sibilantBufferTemp);
}


void AudioProcessorManager::rmsDeEssingAlgorithm(juce::AudioBuffer<float>& buffer,
                                                 float threshold,    // main threshold (dB)
                                                 float mixLevel,
                                                 float frequency,
                                                 int hysteresisDb)  // hysteresis in dB
{
    // Hardcoded parameters for demonstration
    double sampleRate = processedSampleRate;
    float detectionWindowMs = 10.0f; // short window for RMS
    float crossfadeMs = 5.0f;        // crossfade time

    int detectionWindowSamples = juce::jmax(1, (int)(detectionWindowMs * sampleRate / 1000.0));
    int crossfadeSamples = (int)(crossfadeMs * sampleRate / 1000.0);

    float thresholdHigh = threshold;               // e.g. -20 dB
    float thresholdLow = threshold - hysteresisDb; // e.g. threshold = -20, hysteresisDb=10 => thresholdLow=-30 dB

    juce::AudioBuffer<float> originalBuffer;
    juce::AudioBuffer<float> sibilantBufferTemp;
    originalBuffer.makeCopyOf(buffer);
    sibilantBufferTemp.makeCopyOf(buffer);

    // High-pass filter for sibilants
    {
        juce::dsp::AudioBlock<float> sibilantBlock(sibilantBufferTemp);
        juce::dsp::ProcessContextReplacing<float> sibilantContext(sibilantBlock);
        highPassFilter.setCutoffFrequency(frequency);
        highPassFilter.process(sibilantContext);
    }

    // All-pass filter for alignment
    {
        juce::dsp::AudioBlock<float> originalBlock(originalBuffer);
        juce::dsp::ProcessContextReplacing<float> originalContext(originalBlock);
        allPassFilter.process(originalContext);
    }

    buffer.clear();

    int numChannels = buffer.getNumChannels();
    int numSamples = buffer.getNumSamples();
    float gainFactor = juce::Decibels::decibelsToGain(mixLevel);

    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* originalData = originalBuffer.getWritePointer(channel);
        auto* sibilantData = sibilantBufferTemp.getWritePointer(channel);

        std::deque<float> windowSamples;
        double sumOfSquares = 0.0;

        bool inSibilant = false;
        int segmentStart = -1;
        std::vector<std::pair<int,int>> sibilantSegments;

        for (int sample = 0; sample < numSamples; ++sample)
        {
            float s = sibilantData[sample];
            float sq = s * s;
            windowSamples.push_back(sq);
            sumOfSquares += sq;

            if ((int)windowSamples.size() > detectionWindowSamples)
            {
                sumOfSquares -= windowSamples.front();
                windowSamples.pop_front();
            }

            float rmsDb = -100.0f;
            if (!windowSamples.empty())
            {
                double meanSq = sumOfSquares / windowSamples.size();
                float rms = (float)std::sqrt(meanSq);
                rmsDb = juce::Decibels::gainToDecibels(rms);
            }

            // Hysteresis logic:
            if (!inSibilant && rmsDb > thresholdHigh)
            {
                // Enter sibilant mode
                inSibilant = true;
                segmentStart = sample;
            }
            else if (inSibilant && rmsDb < thresholdLow)
            {
                // Exit sibilant mode
                inSibilant = false;
                sibilantSegments.push_back({segmentStart, sample - 1});
            }
        }

        // If ended in sibilant mode
        if (inSibilant)
            sibilantSegments.push_back({segmentStart, numSamples - 1});

        // Zero out all samples that are not in sibilant segments
        std::vector<bool> isSibilantSample(numSamples, false);
        for (auto& seg : sibilantSegments)
        {
            for (int i = seg.first; i <= seg.second; ++i)
                isSibilantSample[i] = true;
        }

        for (int i = 0; i < numSamples; ++i)
        {
            if (!isSibilantSample[i])
                sibilantData[i] = 0.0f;
        }

        // Apply crossfades at the start and end of each segment
        for (auto& seg : sibilantSegments)
        {
            int segLen = seg.second - seg.first + 1;
            int fadeInLen = juce::jmin(crossfadeSamples, segLen / 2);
            int fadeOutLen = juce::jmin(crossfadeSamples, segLen / 2);

            // Fade in
            for (int i = 0; i < fadeInLen; ++i)
            {
                float alpha = (float)i / fadeInLen;
                sibilantData[seg.first + i] *= alpha;
            }

            // Fade out
            for (int i = 0; i < fadeOutLen; ++i)
            {
                float alpha = 1.0f - (float)i / fadeOutLen;
                sibilantData[seg.second - i] *= alpha;
            }
        }

        // Mix the processed signal back
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float reduced = originalData[sample] - sibilantData[sample];
            buffer.setSample(channel, sample, reduced + gainFactor * sibilantData[sample]);
        }
    }

    // Store for visualization
    sibilantBuffer.makeCopyOf(sibilantBufferTemp);
}
