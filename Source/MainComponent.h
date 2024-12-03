#pragma once

#include <JuceHeader.h>
#include "AlgorithmSelector.h"
#include "FilterControl.h"
#include "WaveformDisplay.h"
#include "PositionOverlay.h"
#include "AudioProcessorManager.h"
#include "Algorithms.h"

class MainComponent : public juce::AudioAppComponent, public juce::ChangeListener
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    void resized() override;
    
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
    double currentSampleRate = 44100.0;
    
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };
    void changeState(TransportState newState);
    void transportSourceChanged();
    void openButtonClicked();
    void playButtonClicked();
    void stopButtonClicked();
    void exportButtonClicked();
    
    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::TextButton exportButton;
    
    std::unique_ptr<juce::FileChooser> chooser;

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    TransportState state;
    juce::AudioThumbnailCache waveformCache;
    WaveformDisplay waveformDisplay;
    PositionOverlay positionOverlay;
    
    juce::Label fileLabel;
        
    AlgorithmSelector algorithmSelector;
    FilterControl filterControl;
    
    AudioProcessorManager processorManager;
    
    std::vector<SibilantRegion> detectedSibilantRegions;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
