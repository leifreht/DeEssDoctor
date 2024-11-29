/*
  ==============================================================================

    WaveformDisplay.h
    Created: 18 Nov 2024 12:08:29pm
    Author:  Leif Rehtanz

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class WaveformDisplay : public juce::Component,
                        private juce::ChangeListener
{
public:
    WaveformDisplay();
    ~WaveformDisplay() override = default;

    void loadFile(const juce::File& audioFile);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override
    {
        repaint();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformDisplay)
};
