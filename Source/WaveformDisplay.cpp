/*
  ==============================================================================

    WaveformDisplay.cpp
    Created: 18 Nov 2024 12:08:29pm
    Author:  Leif Rehtanz

  ==============================================================================
*/

#include "WaveformDisplay.h"

WaveformDisplay::WaveformDisplay()
    : thumbnailCache(5), // Cache size for up to 5 thumbnails
      thumbnail(512, formatManager, thumbnailCache)
{
    formatManager.registerBasicFormats(); // Register WAV, MP3, etc.
    thumbnail.addChangeListener(this);    // Listen for thumbnail changes
}

void WaveformDisplay::loadFile(const juce::File& audioFile)
{
    thumbnail.setSource(new juce::FileInputSource(audioFile));
    repaint(); // Repaint to show the new waveform
}

void WaveformDisplay::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);

    if (thumbnail.getNumChannels() == 0)
    {
        g.setColour(juce::Colours::grey);
        g.drawFittedText("No File Loaded", getLocalBounds(), juce::Justification::centred, 1);
    }
    else
    {
        g.setColour(juce::Colours::blue);
        thumbnail.drawChannels(g, getLocalBounds(), 0.0, thumbnail.getTotalLength(), 1.0f);
    }
}

void WaveformDisplay::resized()
{
    // No additional components to resize
}
