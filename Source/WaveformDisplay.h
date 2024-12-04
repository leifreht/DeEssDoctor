/*
  ==============================================================================

    WaveformDisplay.h
    Created: 18 Nov 2024 12:08:29pm
    Author:  Leif Rehtanz

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Algorithms.h"
#include "SibilantRegion.h"

class WaveformDisplay : public juce::Component,
                        private juce::ChangeListener
{
public:
    WaveformDisplay(int sourceSamplesPerWaveformSample,
                   juce::AudioFormatManager& formatManager,
                   juce::AudioThumbnailCache& cache);

    void setFile(const juce::File& file);
    void setSibilantRegions(const std::vector<SibilantRegion>& regions); 
    void setSampleRate(double rate);
    void paint(juce::Graphics& g) override;

private:
    void paintIfNoFileLoaded(juce::Graphics& g);
    void paintIfFileLoaded(juce::Graphics& g);
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void waveformChanged();

    juce::AudioThumbnail waveform;
    std::vector<SibilantRegion> sibilantRegions; // Store regions
    double sampleRate = 44100.0; 
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};
