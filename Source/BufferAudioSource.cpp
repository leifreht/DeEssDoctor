/*
  ==============================================================================

    BufferAudioSource.cpp
    Created: 12 Dec 2024 12:35:40pm
    Author:  Leif Rehtanz

  ==============================================================================
*/

#include "BufferAudioSource.h"

BufferAudioSource::BufferAudioSource(const juce::AudioBuffer<float>& bufferToUse, double sampleRateToUse)
    : buffer(bufferToUse), sampleRate(sampleRateToUse)
{
}

void BufferAudioSource::setNextReadPosition(juce::int64 newPosition)
{
    readPosition = juce::jlimit(juce::int64(0), getTotalLength(), newPosition);
}

juce::int64 BufferAudioSource::getNextReadPosition() const
{
    return readPosition;
}

juce::int64 BufferAudioSource::getTotalLength() const
{
    return buffer.getNumSamples();
}

void BufferAudioSource::prepareToPlay(int /*samplesPerBlockExpected*/, double /*sampleRate*/)
{
}

void BufferAudioSource::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto numSamplesToCopy = juce::jmin(bufferToFill.numSamples, buffer.getNumSamples() - static_cast<int>(readPosition));

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        if (channel < bufferToFill.buffer->getNumChannels())
        {
            bufferToFill.buffer->copyFrom(channel, bufferToFill.startSample, buffer, channel, (int)readPosition, (int)numSamplesToCopy);
        }
    }

    readPosition += numSamplesToCopy;

    if (numSamplesToCopy < bufferToFill.numSamples)
    {
        bufferToFill.clearActiveBufferRegion();
    }
}
