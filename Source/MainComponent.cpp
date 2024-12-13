#include "MainComponent.h"

MainComponent::MainComponent()
: state(Stopped),
  waveformCache(5),
  waveformDisplay(512, formatManager, waveformCache),
  positionOverlay(transportSource)
{
    addAndMakeVisible(&openButton);
    openButton.setButtonText("Open...");
    openButton.onClick = [this] { openButtonClicked(); };

    addAndMakeVisible(&playButton);
    playButton.setButtonText("Play");
    playButton.onClick = [this] { playButtonClicked(); };
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    playButton.setEnabled(false);

    addAndMakeVisible(&stopButton);
    stopButton.setButtonText("Stop");
    stopButton.onClick = [this] { stopButtonClicked(); };
    stopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    stopButton.setEnabled(false);

    addAndMakeVisible(processButton);
    processButton.onClick = [this]() { processFile(); };

    addAndMakeVisible(&waveformDisplay);
    addAndMakeVisible(&positionOverlay);

    addAndMakeVisible(filterControl);
    addAndMakeVisible(rmsControl);
    addAndMakeVisible(fftControl);

    fileLabel.setText("No File Loaded", juce::dontSendNotification);
    fileLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(fileLabel);

    // Playback mode buttons
    originalButton.onClick = [this]() { playbackMode = PlaybackMode::Original; updatePlaybackSource(); };
    sibilantsButton.onClick = [this]() { playbackMode = PlaybackMode::SibilantsOnly; updatePlaybackSource(); };
    deEssedButton.onClick = [this]() { playbackMode = PlaybackMode::DeEssed; updatePlaybackSource(); };

    addAndMakeVisible(originalButton);
    addAndMakeVisible(sibilantsButton);
    addAndMakeVisible(deEssedButton);

    // Algorithm Radio Buttons
    sampleBasedButton.setRadioGroupId(1234);
    rmsBasedButton.setRadioGroupId(1234);
    fftBasedButton.setRadioGroupId(1234);

    sampleBasedButton.setToggleState(true, juce::dontSendNotification);
    sampleBasedButton.onClick = [this] { algorithmChanged(AlgorithmType::SampleBased); };
    rmsBasedButton.onClick = [this] { algorithmChanged(AlgorithmType::RMSBased); };
    fftBasedButton.onClick = [this] { algorithmChanged(AlgorithmType::FFTBased); };

    addAndMakeVisible(sampleBasedButton);
    addAndMakeVisible(rmsBasedButton);
    addAndMakeVisible(fftBasedButton);

    setSize(1200, 800);

    formatManager.registerBasicFormats();
    transportSource.addChangeListener(this);

    setAudioChannels(2, 2);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::algorithmChanged(AlgorithmType newAlgorithm)
{
    currentAlgorithm = newAlgorithm;

    // Set the algorithm in processorManager
    switch (currentAlgorithm)
    {
        case AlgorithmType::SampleBased:
            processorManager.setDeEssingAlgorithm(
                [this](juce::AudioBuffer<float>& buffer, float thr, float mix, float freq, int hyst)
                {
                    // Use default sample-based implementation
                    processorManager.defaultDeEssingAlgorithm(buffer, thr, mix, freq, hyst);
                }
            );
            break;
        case AlgorithmType::RMSBased:
            processorManager.setDeEssingAlgorithm(
                [this](juce::AudioBuffer<float>& buffer, float thr, float mix, float freq, int hyst)
                {
                    // Placeholder: pass-through RMS (no processing yet)
                    // Just return buffer as-is for now
                }
            );
            break;
        case AlgorithmType::FFTBased:
            processorManager.setDeEssingAlgorithm(
                [this](juce::AudioBuffer<float>& buffer, float thr, float mix, float freq, int hyst)
                {
                    // Placeholder: pass-through FFT (no processing yet)
                    // Just return buffer as-is for now
                }
            );
            break;
    }

    if (loadedFile.existsAsFile())
        processFile();
}


void MainComponent::processFile()
{
    if (!loadedFile.existsAsFile())
    {
        DBG("No file loaded to process!");
        return;
    }

    // Set parameters based on currently selected algorithm
    switch (currentAlgorithm)
    {
        case AlgorithmType::SampleBased:
            processorManager.setDeEssingParameters(
                filterControl.getThreshold(),
                filterControl.getReduction(),
                filterControl.getFrequency(),
                filterControl.getHysteresis()
            );
            break;
        case AlgorithmType::RMSBased:
            processorManager.setDeEssingParameters(
                rmsControl.getThreshold(),
                rmsControl.getReduction(),
                rmsControl.getFrequency(),
                rmsControl.getHysteresis()
            );
            break;
        case AlgorithmType::FFTBased:
            // For FFT-based, we don't have a direct setDeEssingParameters for FFT yet.
            // Reuse setDeEssingParameters for now or add a new method if needed.
            // Just pass some defaults or reuse threshold/hysteresis as placeholders:
            processorManager.setDeEssingParameters(
                fftControl.getFreqThreshold(), // treat freqThreshold as threshold
                0.0f,                          // no reduction param currently used
                fftControl.getWindowSize(),    // treat windowSize as frequency param
                (int)(fftControl.getOverlap() * 100) // treat overlap as hysteresis for now, just placeholder
            );
            break;
    }

    processorManager.processFileForSibilants(loadedFile);
    updatePlaybackSource();

    DBG("Processing complete.");
}

void MainComponent::updatePlaybackSource()
{
    transportSource.setSource(nullptr);

    const juce::AudioBuffer<float>* chosenBuffer = nullptr;
    switch (playbackMode)
    {
        case PlaybackMode::Original:
            chosenBuffer = &processorManager.getOriginalBuffer();
            break;
        case PlaybackMode::SibilantsOnly:
            chosenBuffer = &processorManager.getSibilantBuffer();
            break;
        case PlaybackMode::DeEssed:
            chosenBuffer = &processorManager.getProcessedBuffer();
            break;
    }

    if (chosenBuffer->getNumSamples() == 0)
    {
        DBG("No processed data available. Maybe the user hasn't processed yet?");
        return;
    }

    double sampleRate = processorManager.getSampleRate();
    bufferAudioSource = std::make_unique<BufferAudioSource>(*chosenBuffer, sampleRate);
    transportSource.setSource(bufferAudioSource.get(), 0, nullptr);
    transportSource.setPosition(0.0);
    transportSource.stop();
    changeState(Stopped);

    waveformDisplay.setSibilantBuffer(processorManager.getSibilantBuffer());
    repaint();
}


void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    int numChannels = deviceManager.getCurrentAudioDevice()->getActiveInputChannels().countNumberOfSetBits();

    processorManager.prepare(sampleRate, samplesPerBlockExpected, numChannels);
    DBG("ProcessorManager prepared with sample rate: " << sampleRate
            << ", block size: " << samplesPerBlockExpected
            << ", num channels: " << numChannels);
}


void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (readerSource.get() == nullptr)
        {
            bufferToFill.clearActiveBufferRegion();
        }
        else
        {
            transportSource.getNextAudioBlock(bufferToFill);
        }
    
}

void MainComponent::releaseResources()
{
    transportSource.releaseResources();
}


void MainComponent::resized()
{
    auto bounds = getLocalBounds();

    const int topSectionHeight = bounds.getHeight() / 16;
    const int middleSectionHeight = bounds.getHeight() / 2;
    const int transportSectionHeight = bounds.getHeight() / 16;

    // Top: File, open, process
    {
        juce::FlexBox topSection;
        topSection.flexDirection = juce::FlexBox::Direction::row;
        topSection.items.add(juce::FlexItem(fileLabel).withFlex(1.0f));
        topSection.items.add(juce::FlexItem(openButton).withFlex(0.25f));
        topSection.items.add(juce::FlexItem(processButton).withFlex(0.25f));
        topSection.performLayout(bounds.removeFromTop(topSectionHeight));
    }

    // Middle: waveform
    auto middleSectionBounds = bounds.removeFromTop(middleSectionHeight);
    waveformDisplay.setBounds(middleSectionBounds);
    positionOverlay.setBounds(middleSectionBounds);

    // Transport
    {
        juce::FlexBox transportSection;
        transportSection.flexDirection = juce::FlexBox::Direction::row;
        transportSection.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
        transportSection.items.add(juce::FlexItem(playButton).withFlex(1.0f));
        transportSection.items.add(juce::FlexItem(stopButton).withFlex(1.0f));
        transportSection.performLayout(bounds.removeFromTop(transportSectionHeight));
    }

    // Bottom: place all controls
    // We'll arrange them in columns for clarity
    auto bottomArea = bounds;
    int columnWidth = bottomArea.getWidth() / 4;

    // Left column: algorithm radio buttons and playback mode buttons
    auto leftCol = bottomArea.removeFromLeft(columnWidth);
    originalButton.setBounds(leftCol.removeFromTop(30));
    sibilantsButton.setBounds(leftCol.removeFromTop(30));
    deEssedButton.setBounds(leftCol.removeFromTop(30));

    sampleBasedButton.setBounds(leftCol.removeFromTop(30));
    rmsBasedButton.setBounds(leftCol.removeFromTop(30));
    fftBasedButton.setBounds(leftCol.removeFromTop(30));

    // Next column: Sample-based controls
    auto sampleCol = bottomArea.removeFromLeft(columnWidth);
    filterControl.setBounds(sampleCol);

    // Next column: RMS-based controls
    auto rmsCol = bottomArea.removeFromLeft(columnWidth);
    rmsControl.setBounds(rmsCol);

    // Last column: FFT-based controls
    fftControl.setBounds(bottomArea);
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &transportSource)
        transportSourceChanged();
}

void MainComponent::changeState(TransportState newState)
{
    if (state != newState)
    {
        state = newState;

        switch (state)
        {
            case Stopped:
                stopButton.setEnabled(false);
                playButton.setEnabled(true);
                transportSource.setPosition(0.0);
                break;

            case Starting:
                playButton.setEnabled(false);
                transportSource.start();
                break;

            case Playing:
                stopButton.setEnabled(true);
                break;

            case Stopping:
                transportSource.stop();
                break;

            default:
                jassertfalse;
                break;
        }
    }
}

void MainComponent::transportSourceChanged()
{
    if (transportSource.isPlaying())
        changeState(Playing);
    else
        changeState(Stopped);
}

void MainComponent::openButtonClicked()
{
    chooser = std::make_unique<juce::FileChooser> ("Select a Wave file to play...",
                                                   juce::File{},
                                                   "*.wav");
    auto chooserFlags = juce::FileBrowserComponent::openMode
                      | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& fc)
    {
        auto file = fc.getResult();

        if (file != juce::File{})
        {
            loadedFile = file;
            auto* reader = formatManager.createReaderFor (file);
            fileLabel.setText(file.getFileName(), juce::dontSendNotification);

            if (reader != nullptr)
            {
                auto newSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);
                transportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate);
                playButton.setEnabled (true);
                waveformDisplay.setFile (file);
                readerSource.reset (newSource.release());
            }
        }
    });
}

void MainComponent::playButtonClicked()
{
    changeState(Starting);
}

void MainComponent::stopButtonClicked()
{
    changeState(Stopping);
}




