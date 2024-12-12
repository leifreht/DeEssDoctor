/*
  ==============================================================================

    BufferAudioSource.h
    Created: 12 Dec 2024 12:35:40pm
    Author:  Leif Rehtanz

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class BufferAudioSource : public juce::PositionableAudioSource
{
public:
    BufferAudioSource(const juce::AudioBuffer<float>& bufferToUse, double sampleRate);

    void setNextReadPosition(juce::int64 newPosition) override;
    juce::int64 getNextReadPosition() const override;
    juce::int64 getTotalLength() const override;
    bool isLooping() const override { return false; }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override {}
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

private:
    const juce::AudioBuffer<float>& buffer;
    double sampleRate;
    juce::int64 readPosition{ 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BufferAudioSource)
};
