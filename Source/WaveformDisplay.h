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
    
    void setSibilantBuffer(const juce::AudioBuffer<float>& buffer);
    
    private:
    juce::AudioBuffer<float> sibilantBuffer;
    bool hasSibilantData = false;
    
    void paintIfNoFileLoaded(juce::Graphics& g);
    void paintIfFileLoaded(juce::Graphics& g);
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void waveformChanged();
    
    juce::AudioThumbnail waveform;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};
