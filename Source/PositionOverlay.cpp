/*
  ==============================================================================

    PositionOverlay.cpp
    Created: 29 Nov 2024 8:11:02pm
    Author:  Leif Rehtanz

  ==============================================================================
*/

#include "PositionOverlay.h"


PositionOverlay::PositionOverlay(juce::AudioTransportSource& transportSourceToUse)
    : transportSource(transportSourceToUse)
{
    startTimer(40);
}

void PositionOverlay::paint(juce::Graphics& g)
{
    auto duration = (float)transportSource.getLengthInSeconds();

    if (duration > 0.0)
    {
        auto audioPosition = (float)transportSource.getCurrentPosition();
        auto drawPosition = (audioPosition / duration) * (float)getWidth();

        g.setColour(juce::Colours::green);
        g.drawLine(drawPosition, 0.0f, drawPosition, (float)getHeight(), 2.0f);
    }
}

void PositionOverlay::mouseDown(const juce::MouseEvent& event)
{
    auto duration = transportSource.getLengthInSeconds();

    if (duration > 0.0)
    {
        auto clickPosition = event.position.x;
        auto audioPosition = (clickPosition / (float)getWidth()) * duration;

        transportSource.setPosition(audioPosition);
    }
}

void PositionOverlay::timerCallback()
{
    repaint();
}
