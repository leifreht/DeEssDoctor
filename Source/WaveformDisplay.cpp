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

    // Draw original waveform
    g.setColour(juce::Colours::blue);
    waveform.drawChannels(g, getLocalBounds(), 0.0, waveform.getTotalLength(), 1.0f);

    // Draw sibilants as overlay
    if (hasSibilantData && sibilantBuffer.getNumChannels() > 0)
    {
        g.setColour(juce::Colours::red);
        auto bounds = getLocalBounds();
        auto totalLength = waveform.getTotalLength();
        auto numSamples = sibilantBuffer.getNumSamples();

        for (int channel = 0; channel < sibilantBuffer.getNumChannels(); ++channel)
        {
            auto* data = sibilantBuffer.getReadPointer(channel);
            juce::Path sibilantPath;

            for (int i = 0; i < numSamples; ++i)
            {
                float x = (static_cast<float>(i) / numSamples) * bounds.getWidth();
                float y = bounds.getHeight() * (0.5f - 0.5f * data[i]);

                if (i == 0)
                    sibilantPath.startNewSubPath(x, y);
                else
                    sibilantPath.lineTo(x, y);
            }

            g.strokePath(sibilantPath, juce::PathStrokeType(1.0f));
        }
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


