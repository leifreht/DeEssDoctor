#pragma once

#include <JuceHeader.h>
#include "AlgorithmSelector.h"
#include "FilterControl.h"
#include "WaveformDisplay.h"
#include "MixerControl.h"
#include "AudioProcessorManager.h"
#include "Algorithms.h"

class MainComponent : public juce::AudioAppComponent
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    AlgorithmSelector algorithmSelector;
    FilterControl filterControl;
    WaveformDisplay waveformDisplay;
    MixerControl mixerControl;

    AudioProcessorManager processorManager;

    juce::TextButton loadButton;
    juce::File audioFile;

    void loadAudioFile();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
