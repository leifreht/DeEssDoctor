#pragma once

#include <JuceHeader.h>
#include "AlgorithmSelector.h"
#include "FilterControl.h"
#include "WaveformDisplay.h"
#include "PositionOverlay.h"
#include "AudioProcessorManager.h"
#include "BufferAudioSource.h" // Include your custom BufferAudioSource class

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
    // Enum for playback modes
    enum class PlaybackMode
    {
        Original,       // Unprocessed audio
        SibilantsOnly,  // Isolated sibilants
        DeEssed         // Audio with sibilants removed
    };

    // Member variables
    PlaybackMode playbackMode = PlaybackMode::Original;
    void updatePlaybackSource();

    juce::File loadedFile; // Currently loaded file

    // Transport controls
    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::TextButton processButton{"Process"};

    juce::TextButton originalButton{"Original"};
    juce::TextButton sibilantsButton{"Sibilants"};
    juce::TextButton deEssedButton{"De-Essed"};

    // JUCE components
    std::unique_ptr<juce::FileChooser> chooser;
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;

    std::unique_ptr<BufferAudioSource> bufferAudioSource; // Custom audio source for playback

    juce::AudioThumbnailCache waveformCache;
    WaveformDisplay waveformDisplay;
    PositionOverlay positionOverlay;

    juce::Label fileLabel;

    AlgorithmSelector algorithmSelector;
    FilterControl filterControl;

    AudioProcessorManager processorManager;

    // Transport state management
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };

    TransportState state;

    void changeState(TransportState newState);
    void transportSourceChanged();
    void openButtonClicked();
    void playButtonClicked();
    void stopButtonClicked();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
