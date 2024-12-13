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

    // Mode buttons as toggleable TextButtons with a radio group
    int modeRadioGroup = 2345;
    originalButton.setClickingTogglesState(true);
    originalButton.setRadioGroupId(modeRadioGroup);
    originalButton.setToggleState(true, juce::dontSendNotification); // default selected
    originalButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::darkblue);
    originalButton.onClick = [this]() {
        playbackMode = PlaybackMode::Original;
        updatePlaybackSource();
    };
    addAndMakeVisible(originalButton);

    sibilantsButton.setClickingTogglesState(true);
    sibilantsButton.setRadioGroupId(modeRadioGroup);
    sibilantsButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::darkblue);
    sibilantsButton.onClick = [this]() {
        playbackMode = PlaybackMode::SibilantsOnly;
        updatePlaybackSource();
    };
    addAndMakeVisible(sibilantsButton);

    deEssedButton.setClickingTogglesState(true);
    deEssedButton.setRadioGroupId(modeRadioGroup);
    deEssedButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::darkblue);
    deEssedButton.onClick = [this]() {
        playbackMode = PlaybackMode::DeEssed;
        updatePlaybackSource();
    };
    addAndMakeVisible(deEssedButton);

    // Algorithm buttons as toggleable TextButtons with a radio group
    int algoRadioGroup = 3456;
    sampleBasedButton.setClickingTogglesState(true);
    sampleBasedButton.setRadioGroupId(algoRadioGroup);
    sampleBasedButton.setToggleState(true, juce::dontSendNotification); // default selected
    sampleBasedButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::darkblue);
    sampleBasedButton.onClick = [this]() { algorithmChanged(AlgorithmType::SampleBased); };
    addAndMakeVisible(sampleBasedButton);

    rmsBasedButton.setClickingTogglesState(true);
    rmsBasedButton.setRadioGroupId(algoRadioGroup);
    rmsBasedButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::darkblue);
    rmsBasedButton.onClick = [this]() { algorithmChanged(AlgorithmType::RMSBased); };
    addAndMakeVisible(rmsBasedButton);

    fftBasedButton.setClickingTogglesState(true);
    fftBasedButton.setRadioGroupId(algoRadioGroup);
    fftBasedButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::darkblue);
    fftBasedButton.onClick = [this]() { algorithmChanged(AlgorithmType::FFTBased); };
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
                    processorManager.defaultDeEssingAlgorithm(buffer, thr, mix, freq, hyst);
                }
            );
            break;
        case AlgorithmType::RMSBased:
            processorManager.setDeEssingAlgorithm(
                [this](juce::AudioBuffer<float>& buffer, float thr, float mix, float freq, int hyst)
                {
                    processorManager.rmsDeEssingAlgorithm(buffer, thr, mix, freq, hyst);
                }
            );
            break;
        case AlgorithmType::FFTBased:
            processorManager.setDeEssingAlgorithm(
                [this](juce::AudioBuffer<float>& buffer, float thr, float mix, float freq, int hyst)
                {
                    // Placeholder: pass-through FFT
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

    // Heights for different sections
    const int topSectionHeight = bounds.getHeight() / 16;    // For file label, open, process
    const int waveformHeight   = bounds.getHeight() / 2;     // Waveform area
    const int buttonRowHeight  = bounds.getHeight() / 16;    // Each row of buttons
    const int bottomHeight     = bounds.getHeight() / 4;     // For all slider sets

    // 1) Top section: File name, Open button, Process button
    {
        juce::FlexBox topSection;
        topSection.flexDirection = juce::FlexBox::Direction::row;
        topSection.items.add(juce::FlexItem(fileLabel).withFlex(1.0f));
        topSection.items.add(juce::FlexItem(openButton).withFlex(0.25f));
        topSection.items.add(juce::FlexItem(processButton).withFlex(0.25f));
        topSection.performLayout(bounds.removeFromTop(topSectionHeight));
    }

    // 2) Waveform display
    auto waveformArea = bounds.removeFromTop(waveformHeight);
    waveformDisplay.setBounds(waveformArea);
    positionOverlay.setBounds(waveformArea);

    // 3) Original / Sibilants / De-essed buttons (replaces old transport row)
    {
        juce::FlexBox modeSection;
        modeSection.flexDirection = juce::FlexBox::Direction::row;
        modeSection.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
        modeSection.items.add(juce::FlexItem(originalButton).withFlex(1.0f));
        modeSection.items.add(juce::FlexItem(sibilantsButton).withFlex(1.0f));
        modeSection.items.add(juce::FlexItem(deEssedButton).withFlex(1.0f));
        modeSection.performLayout(bounds.removeFromTop(buttonRowHeight));
    }

    // 4) Play / Stop buttons below the mode buttons
    {
        juce::FlexBox transportSection;
        transportSection.flexDirection = juce::FlexBox::Direction::row;
        transportSection.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
        transportSection.items.add(juce::FlexItem(playButton).withFlex(1.0f));
        transportSection.items.add(juce::FlexItem(stopButton).withFlex(1.0f));
        transportSection.performLayout(bounds.removeFromTop(buttonRowHeight));
    }

    // 5) Radio buttons for algorithms (horizontally placed)
    {
        juce::FlexBox algoSection;
        algoSection.flexDirection = juce::FlexBox::Direction::row;
        algoSection.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
        algoSection.items.add(juce::FlexItem(sampleBasedButton).withFlex(1.0f));
        algoSection.items.add(juce::FlexItem(rmsBasedButton).withFlex(1.0f));
        algoSection.items.add(juce::FlexItem(fftBasedButton).withFlex(1.0f));
        algoSection.performLayout(bounds.removeFromTop(buttonRowHeight));
    }

    // 6) Three columns for the slider sets at the bottom:
    //    Left: Sample-based (FilterControl)
    //    Middle: RMS (RmsControl)
    //    Right: FFT (FftControl)

    // We have the remainder of the window for the slider sets
    auto bottomArea = bounds;

    int columnWidth = bottomArea.getWidth() / 3;

    auto sampleCol = bottomArea.removeFromLeft(columnWidth);
    filterControl.setBounds(sampleCol.reduced(10));

    auto rmsCol = bottomArea.removeFromLeft(columnWidth);
    rmsControl.setBounds(rmsCol.reduced(10));

    auto fftCol = bottomArea;
    fftControl.setBounds(fftCol.reduced(10));
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




