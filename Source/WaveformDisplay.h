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
    WaveformDisplay(int sourceSamplesPerWaveformSample,
                             juce::AudioFormatManager& formatManager,
                             juce::AudioThumbnailCache& cache);

    void setFile(const juce::File& file);
    void paint(juce::Graphics& g) override;

private:
    void paintIfNoFileLoaded(juce::Graphics& g);
    void paintIfFileLoaded(juce::Graphics& g);
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void waveformChanged();

    juce::AudioThumbnail waveform;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
//public:
//    WaveformDisplay();
//    ~WaveformDisplay() override = default;
//
//    void loadFile(const juce::File& audioFile);
//    void paint(juce::Graphics& g) override;
//    void resized() override;
//
//private:
//    juce::AudioFormatManager formatManager;
//    juce::AudioThumbnailCache thumbnailCache;
//    juce::AudioThumbnail thumbnail;
//
//    void changeListenerCallback(juce::ChangeBroadcaster* source) override
//    {
//        repaint();
//    }
//
//    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformDisplay)
};
