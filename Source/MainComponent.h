#pragma once

#include <JuceHeader.h>
#include "FilterControl.h"
#include "WaveformDisplay.h"
#include "PositionOverlay.h"
#include "AudioProcessorManager.h"
#include "BufferAudioSource.h"
#include "RmsControl.h"
#include "FftControl.h"

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

private:
    enum class PlaybackMode { Original, SibilantsOnly, DeEssed };
    PlaybackMode playbackMode = PlaybackMode::Original;
    void updatePlaybackSource();

    enum class AlgorithmType { SampleBased, RMSBased, FFTBased };
    AlgorithmType currentAlgorithm = AlgorithmType::SampleBased;
    void algorithmChanged(AlgorithmType newAlgorithm);

    juce::File loadedFile;

    // Transport controls
    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::TextButton processButton{"Process"};

    juce::TextButton originalButton{"Original"};
    juce::TextButton sibilantsButton{"Sibilants"};
    juce::TextButton deEssedButton{"De-Essed"};

    // Algorithm Radio Buttons
    juce::ToggleButton sampleBasedButton{"Sample-based"};
    juce::ToggleButton rmsBasedButton{"RMS-based"};
    juce::ToggleButton fftBasedButton{"FFT-based"};

    std::unique_ptr<juce::FileChooser> chooser; 
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    std::unique_ptr<BufferAudioSource> bufferAudioSource;

    juce::AudioThumbnailCache waveformCache;
    WaveformDisplay waveformDisplay;
    PositionOverlay positionOverlay;

    juce::Label fileLabel;

    FilterControl filterControl; // For sample-based
    RmsControl rmsControl;       // For RMS-based
    FftControl fftControl;       // For FFT-based

    AudioProcessorManager processorManager;

    enum TransportState { Stopped, Starting, Playing, Stopping };
    TransportState state;

    void changeState(TransportState newState);
    void transportSourceChanged();
    void openButtonClicked();
    void playButtonClicked();
    void stopButtonClicked();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
