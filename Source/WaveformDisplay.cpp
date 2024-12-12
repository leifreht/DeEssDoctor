/*
  ==============================================================================

    WaveformDisplay.cpp
    Created: 18 Nov 2024 12:08:29pm
    Author:  Leif Rehtanz

  ==============================================================================
*/

#include "WaveformDisplay.h"

WaveformDisplay::WaveformDisplay(int sourceSamplesPerWaveformSample,
                                                   juce::AudioFormatManager& formatManager,
                                                   juce::AudioThumbnailCache& cache)
    : waveform(sourceSamplesPerWaveformSample, formatManager, cache)
{
    waveform.addChangeListener(this);
}

void WaveformDisplay::setFile(const juce::File& file)
{
    waveform.setSource(new juce::FileInputSource(file));
}

void WaveformDisplay::setSibilantBuffer(const juce::AudioBuffer<float>& buffer)
{
    sibilantBuffer.makeCopyOf(buffer);
    hasSibilantData = true;

    // Schedule repaint on the message thread
    juce::MessageManager::callAsync([this]()
    {
        repaint();
    });
}

void WaveformDisplay::paint(juce::Graphics& g)
{
    if (waveform.getNumChannels() == 0)
        paintIfNoFileLoaded(g);
    else
        paintIfFileLoaded(g);
}

void WaveformDisplay::paintIfNoFileLoaded(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);
    g.setColour(juce::Colours::darkgrey);
    g.drawFittedText("No File Loaded", getLocalBounds(), juce::Justification::centred, 1);
}

//void WaveformDisplay::paintIfFileLoaded(juce::Graphics& g)
//{
//    g.fillAll(juce::Colours::white);
//    g.setColour(juce::Colours::blue);
//    waveform.drawChannels(g, getLocalBounds(), 0.0, waveform.getTotalLength(), 1.0f);
//}

void WaveformDisplay::paintIfFileLoaded(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);

    // Draw the original waveform as mono (just channel 0)
    g.setColour(juce::Colours::blue);
    // Draw only the first channel of the thumbnail
    if (waveform.getNumChannels() > 0)
    {
        waveform.drawChannel(g, getLocalBounds(), 0.0, waveform.getTotalLength(), 0, 1.0f);
    }

    // Draw sibilants as overlay (just take channel 0 for mono)
    if (hasSibilantData && sibilantBuffer.getNumChannels() > 0)
    {
        g.setColour(juce::Colours::red);
        auto bounds = getLocalBounds();
        auto numSamples = sibilantBuffer.getNumSamples();

        // Use only channel 0 for a mono representation of sibilants
        auto* data = sibilantBuffer.getReadPointer(0);
        juce::Path sibilantPath;

        // To speed up drawing, consider skipping samples if you have many.
        // For now, we just draw every sample.
        for (int i = 0; i < numSamples; ++i)
        {
            float x = (static_cast<float>(i) / numSamples) * bounds.getWidth();
            // Scale and center waveform: data[i] expected to be in [-1, 1]
            float y = bounds.getHeight() * (0.5f - 0.5f * data[i]);

            if (i == 0)
                sibilantPath.startNewSubPath(x, y);
            else
                sibilantPath.lineTo(x, y);
        }

        g.strokePath(sibilantPath, juce::PathStrokeType(1.0f));
    }
}

void WaveformDisplay::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &waveform)
        waveformChanged();
}

void WaveformDisplay::waveformChanged()
{
    repaint();
}


