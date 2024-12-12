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
    processButton.onClick = [this]() {
        processFile();
    };
    
    addAndMakeVisible(&waveformDisplay);
    addAndMakeVisible(&positionOverlay);
    
    addAndMakeVisible(algorithmSelector);
    addAndMakeVisible(filterControl);
    
    fileLabel.setText("No File Loaded", juce::dontSendNotification);
    fileLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(fileLabel);
        
    filterControl.frequencySlider.onValueChange = [this]()
    {
        processorManager.setDeEssingParameters(
            filterControl.thresholdSlider.getValue(),
            filterControl.reductionSlider.getValue(),
            filterControl.frequencySlider.getValue(),
            filterControl.hysteresisSlider.getValue()
        );

        DBG("Frequency Slider Changed: " << filterControl.frequencySlider.getValue() << " Hz");
    };

    filterControl.thresholdSlider.onValueChange = [this]()
    {
        processorManager.setDeEssingParameters(
            filterControl.thresholdSlider.getValue(),
            filterControl.reductionSlider.getValue(),
            filterControl.frequencySlider.getValue(),
            filterControl.hysteresisSlider.getValue()
        );

        DBG("Threshold Slider Changed: " << filterControl.thresholdSlider.getValue() << " dB");
    };

    filterControl.reductionSlider.onValueChange = [this]()
    {
        processorManager.setDeEssingParameters(
            filterControl.thresholdSlider.getValue(),
            filterControl.reductionSlider.getValue(),
            filterControl.frequencySlider.getValue(),
            filterControl.hysteresisSlider.getValue()
        );

        DBG("Reduction Slider Changed: " << filterControl.reductionSlider.getValue() << " dB");
    };
    
    filterControl.hysteresisSlider.onValueChange = [this]()
    {
        processorManager.setDeEssingParameters(
            filterControl.thresholdSlider.getValue(),
            filterControl.reductionSlider.getValue(),
            filterControl.frequencySlider.getValue(),
            filterControl.hysteresisSlider.getValue()
        );
        DBG("Hysteresis Slider Changed: " << filterControl.hysteresisSlider.getValue() << " dB");
    };
    
    originalButton.onClick = [this]() {
        playbackMode = PlaybackMode::Original;
        updatePlaybackSource();
    };

    sibilantsButton.onClick = [this]() {
        playbackMode = PlaybackMode::SibilantsOnly;
        updatePlaybackSource();
    };

    deEssedButton.onClick = [this]() {
        playbackMode = PlaybackMode::DeEssed;
        updatePlaybackSource();
    };

    addAndMakeVisible(originalButton);
    addAndMakeVisible(sibilantsButton);
    addAndMakeVisible(deEssedButton);

    setSize(1200, 800);
    
    formatManager.registerBasicFormats();
    transportSource.addChangeListener(this);

    setAudioChannels(1, 1);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::processFile()
{
    if (!loadedFile.existsAsFile())
    {
        DBG("No file loaded to process!");
        return;
    }

    // Set parameters
    processorManager.setDeEssingParameters(
        filterControl.getThreshold(),
        filterControl.getReduction(),
        filterControl.getFrequency(),
        filterControl.getHysteresis()
    );

    // Process the file offline: this updates original, processed, and sibilant buffers
    processorManager.processFileForSibilants(loadedFile);

    // After processing, call updatePlaybackSource() to ensure the current mode's buffer is used
    updatePlaybackSource();

    DBG("Processing complete.");
}

void MainComponent::updatePlaybackSource()
{
    // Ensure we have processed buffers available. If not processed yet, do nothing or prompt the user.
    // For now, let's assume "Process" was already clicked at least once, or we have an original file.

    // Remove the previous source
    transportSource.setSource(nullptr);

    // Choose the buffer based on playbackMode
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

    // Create a BufferAudioSource for the chosen buffer
    // We need the sample rate of the file. You can store it from processFile() or reader creation.
    // For now, assume we stored 'lastSampleRate' in processFile().
    double sampleRate = processorManager.getSampleRate(); // (Add a getter in processorManager if needed.)
    bufferAudioSource = std::make_unique<BufferAudioSource>(*chosenBuffer, sampleRate);
    transportSource.setSource(bufferAudioSource.get(), 0, nullptr);

    // Update waveform display's sibilant overlay to always show sibilants if needed
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

//void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
//{
//    if (readerSource.get() == nullptr)
//    {
//        bufferToFill.clearActiveBufferRegion();
//    }
//    else
//    {
//        transportSource.getNextAudioBlock(bufferToFill);
//        processorManager.processBlock(*bufferToFill.buffer);
//        
////        juce::AudioBuffer<float> sibilantBuffer;
////        processorManager.getSibilantBuffer(sibilantBuffer);
////        waveformDisplay.setSibilantBuffer(sibilantBuffer);
//    }
//}

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

    // Top section: File name, Open button, Export button
    juce::FlexBox topSection;
    topSection.flexDirection = juce::FlexBox::Direction::row;
    topSection.items.add(juce::FlexItem(fileLabel).withFlex(1.0f));
    topSection.items.add(juce::FlexItem(openButton).withFlex(0.25f));
    topSection.items.add(juce::FlexItem(processButton).withFlex(0.25f));
    topSection.performLayout(bounds.removeFromTop(topSectionHeight));

    // Middle section layout: Waveform + Overlay
    auto middleSectionBounds = bounds.removeFromTop(middleSectionHeight);
    waveformDisplay.setBounds(middleSectionBounds);
    positionOverlay.setBounds(middleSectionBounds);

    // Transport controls
    juce::FlexBox transportSection;
    transportSection.flexDirection = juce::FlexBox::Direction::row;
    transportSection.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
    transportSection.items.add(juce::FlexItem(playButton).withFlex(1.0f));
    transportSection.items.add(juce::FlexItem(stopButton).withFlex(1.0f));
    transportSection.performLayout(bounds.removeFromTop(transportSectionHeight));

    // Bottom section: Filter controls and algorithm selector
    juce::FlexBox bottomSection;
    bottomSection.flexDirection = juce::FlexBox::Direction::column;
    bottomSection.items.add(juce::FlexItem(filterControl).withFlex(1.0f));  // Filter controls
    bottomSection.items.add(juce::FlexItem(algorithmSelector).withFlex(1.0f));  // Algorithm selector
    bottomSection.performLayout(bounds);
    
    originalButton.setBounds(10, 10, 100, 30);
    sibilantsButton.setBounds(120, 10, 100, 30);
    deEssedButton.setBounds(230, 10, 100, 30);
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
    chooser = std::make_unique<juce::FileChooser>(
        "Select an Audio file...", juce::File{}, "*.wav;*.mp3;*.aiff;*.flac");

    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();

        if (file != juce::File{})
        {
            loadedFile = file;
            auto* reader = formatManager.createReaderFor (file);
            fileLabel.setText(file.getFileName(), juce::dontSendNotification);

            if (reader != nullptr)
            {
                // Ensure UI updates happen on the message thread
                juce::MessageManager::callAsync([this, file]() {
                    fileLabel.setText(file.getFileName(), juce::dontSendNotification);
                    waveformDisplay.setFile(file);
                });

                auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
                transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);

                playButton.setEnabled(true);
                readerSource.reset(newSource.release());
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

void MainComponent::exportButtonClicked()
{
    juce::FileChooser exportChooser("Save Processed Audio...", juce::File{}, "*.wav");

    exportChooser.launchAsync(juce::FileBrowserComponent::saveMode, [this](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file == juce::File{})
            return;

        // Create an output stream
        std::unique_ptr<juce::FileOutputStream> outputStream(file.createOutputStream());

        if (outputStream == nullptr)
            return; // Handle error appropriately

        // Set up an audio format writer
        juce::AudioFormat* format = formatManager.findFormatForFileExtension(file.getFileExtension());

        if (format == nullptr)
            return; // Handle unsupported format

        std::unique_ptr<juce::AudioFormatWriter> writer(format->createWriterFor(outputStream.get(),
                                                                                  currentSampleRate, // Use stored sample rate
                                                                                  1, // Number of channels (mono)
                                                                                  16, // Bits per sample
                                                                                  {}, // Metadata
                                                                                  0)); // Quality

        if (writer == nullptr)
            return; // Handle writer creation failure

        // Since the writer now owns the stream, release it from the unique_ptr
        outputStream.release();

        // Prepare a buffer to hold the entire audio
        int totalSamples = static_cast<int>(transportSource.getLengthInSeconds() * currentSampleRate);
        juce::AudioBuffer<float> bufferToWrite(1, totalSamples); // Mono

        // Reset transport source to the beginning
        transportSource.setPosition(0.0);

        // Read the entire audio into bufferToWrite
        int samplesRead = 0;
        while (samplesRead < totalSamples)
        {
            int samplesToRead = std::min(totalSamples - samplesRead, (int)bufferToWrite.getNumSamples());
            juce::AudioSourceChannelInfo info(&bufferToWrite, samplesRead, samplesToRead);
            transportSource.getNextAudioBlock(info);
            samplesRead += samplesToRead;
        }

        // Process the buffer
        processorManager.processBlock(bufferToWrite);

        // Write to file directly without manual conversion
        writer->writeFromAudioSampleBuffer(bufferToWrite, 0, bufferToWrite.getNumSamples());
    });
}


