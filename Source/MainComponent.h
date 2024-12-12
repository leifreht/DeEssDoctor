#pragma once

#include <JuceHeader.h>
#include "AlgorithmSelector.h"
#include "FilterControl.h"
#include "WaveformDisplay.h"
#include "PositionOverlay.h"
//#include "MixerControl.h"
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
    
    void processFile();
    
//    void paint(juce::Graphics& g) override;


private:
    juce::File loadedFile; 
    
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
    
    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::TextButton processButton{"Process"}; 
    
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


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
