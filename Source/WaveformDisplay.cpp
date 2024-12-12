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
        auto* data = sibilantBuffer.getReadPointer(0);

        int totalWidth = bounds.getWidth();
        if (totalWidth <= 0 || numSamples == 0)
            return;

        // Determine how many samples correspond to one vertical line
        int step = std::max(1, numSamples / totalWidth);

        juce::Path sibilantPath;
        // We'll draw a vertical line per pixel column representing the min/max in that segment
        for (int x = 0; x < totalWidth; ++x)
        {
            int start = x * step;
            int end = std::min(start + step, numSamples);

            float minVal = std::numeric_limits<float>::max();
            float maxVal = std::numeric_limits<float>::lowest();

            // Find min and max in this chunk
            for (int i = start; i < end; ++i)
            {
                float sampleVal = data[i];
                if (sampleVal < minVal) minVal = sampleVal;
                if (sampleVal > maxVal) maxVal = sampleVal;
            }

            // Convert min/max to vertical coordinates
            float yMin = bounds.getHeight() * (0.5f - 0.5f * maxVal);
            float yMax = bounds.getHeight() * (0.5f - 0.5f * minVal);

            // Draw a vertical line from min to max
            sibilantPath.startNewSubPath((float)x, yMin);
            sibilantPath.lineTo((float)x, yMax);
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


