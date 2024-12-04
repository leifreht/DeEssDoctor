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

void WaveformDisplay::setSampleRate(double rate)
{
    sampleRate = rate;
}

void WaveformDisplay::setSibilantRegions(const std::vector<SibilantRegion>& regions)
{
    sibilantRegions = regions;
    repaint();
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

void WaveformDisplay::paintIfFileLoaded(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);

    // Draw the waveform
    g.setColour(juce::Colours::blue);
    waveform.drawChannels(g, getLocalBounds(), 0.0, waveform.getTotalLength(), 1.0f);

    // Overlay sibilant regions
    g.setColour(juce::Colours::red.withAlpha(0.5f)); // Semi-transparent red

    for (const auto& region : sibilantRegions)
    {
        // Convert sample indices to time
        double startTime = (sampleRate > 0.0) ? static_cast<double>(region.startSample) / sampleRate : 0.0;
        double endTime = (sampleRate > 0.0) ? static_cast<double>(region.endSample) / sampleRate : 0.0;

        // Prevent division by zero
        if (waveform.getTotalLength() <= 0.0)
            continue;

        // Convert time to x position
        float startX = static_cast<float>((startTime / waveform.getTotalLength()) * getWidth());
        float endX = static_cast<float>((endTime / waveform.getTotalLength()) * getWidth());

        // Ensure that startX is less than endX
        if (startX >= endX)
            continue;

        // Create a Rectangle<float> explicitly
        juce::Rectangle<float> rect(startX, 0.0f, endX - startX, static_cast<float>(getHeight()));

        // Draw a rectangle over the sibilant region
        g.fillRect(rect);
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

//WaveformDisplay::WaveformDisplay(int sourceSamplesPerWaveformSample,
//                                 juce::AudioFormatManager& formatManager,
//                                 juce::AudioThumbnailCache& cache)
//    : waveform(sourceSamplesPerWaveformSample, formatManager, cache)
//{
//    waveform.addChangeListener(this);
//}
//
//void WaveformDisplay::setFile(const juce::File& file)
//{
//    waveform.setSource(new juce::FileInputSource(file));
//}
//
//void WaveformDisplay::setSampleRate(double rate)
//{
//    sampleRate = rate;
//}
//
//void WaveformDisplay::setSibilantRegions(const std::vector<SibilantRegion>& regions)
//{
//    sibilantRegions = regions;
//    repaint();
//}
//
//void WaveformDisplay::paint(juce::Graphics& g)
//{
//    if (waveform.getNumChannels() == 0)
//        paintIfNoFileLoaded(g);
//    else
//        paintIfFileLoaded(g);
//}
//
//void WaveformDisplay::paintIfNoFileLoaded(juce::Graphics& g)
//{
//    g.fillAll(juce::Colours::white);
//    g.setColour(juce::Colours::darkgrey);
//    g.drawFittedText("No File Loaded", getLocalBounds(), juce::Justification::centred, 1);
//}
//
//
//void WaveformDisplay::paintIfFileLoaded(juce::Graphics& g)
//{
//    g.fillAll(juce::Colours::white);
//
//    // Draw the waveform
//    g.setColour(juce::Colours::blue);
//    waveform.drawChannels(g, getLocalBounds(), 0.0, waveform.getTotalLength(), 1.0f);
//
//    // Overlay sibilant regions
//    g.setColour(juce::Colours::red.withAlpha(0.5f)); // Semi-transparent red
//
//    for (const auto& region : sibilantRegions)
//    {
//        // Convert sample indices to time
//        double startTime = (sampleRate > 0.0) ? (double)region.startSample / sampleRate : 0.0;
//        double endTime = (sampleRate > 0.0) ? (double)region.endSample / sampleRate : 0.0;
//
//        // Convert time to x position
//        float startX = (float)(startTime / waveform.getTotalLength()) * (float)getWidth();
//        float endX = (float)(endTime / waveform.getTotalLength()) * (float)getWidth();
//
//        // Create a Rectangle<float> explicitly
//        juce::Rectangle<float> rect(startX, 0.0f, endX - startX, (float)getHeight());
//
//        // Draw a rectangle over the sibilant region
//        g.fillRect(rect);
//    }
//}
//
//void WaveformDisplay::changeListenerCallback(juce::ChangeBroadcaster* source)
//{
//    if (source == &waveform)
//        waveformChanged();
//}
//
//void WaveformDisplay::waveformChanged()
//{
//    repaint();
//}
