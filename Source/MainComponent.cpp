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
    
    addAndMakeVisible(&waveformDisplay);
    addAndMakeVisible(&positionOverlay);
    
    addAndMakeVisible(algorithmSelector);
    addAndMakeVisible(filterControl);
    
    fileLabel.setText("No File Loaded", juce::dontSendNotification);
    fileLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(fileLabel);
    
    addAndMakeVisible(exportButton);
    exportButton.setButtonText("Export...");
    exportButton.onClick = [this] { exportButtonClicked(); };
        
    algorithmSelector.algorithmChanged = [this]()
    {
        auto selectedAlgorithm = algorithmSelector.getSelectedAlgorithm();

        if (selectedAlgorithm == "Amplitude Threshold")
            processorManager.setAlgorithm(amplitudeThresholdAlgorithm);
        else if (selectedAlgorithm == "Spectral Analysis")
            processorManager.setAlgorithm(spectralAnalysisAlgorithm);
    };

    filterControl.frequencySlider.onValueChange = [this]()
    {
        processorManager.setFilterParameters(filterControl.getFrequency(), filterControl.getQFactor(), filterControl.getGain());
    };

    setSize(1200, 800);
    
    formatManager.registerBasicFormats();
    transportSource.addChangeListener(this);

    setAudioChannels(2, 2);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
//    processorManager.prepare(sampleRate, samplesPerBlockExpected, getTotalNumInputChannels());
    processorManager.prepare(sampleRate, samplesPerBlockExpected, 2);
    
    waveformDisplay.setSampleRate(sampleRate);
    
    currentSampleRate = sampleRate;

}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (readerSource.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    // Get audio from transport source
    juce::AudioBuffer<float> mainBuffer;
    mainBuffer.makeCopyOf(*bufferToFill.buffer);

    transportSource.getNextAudioBlock(bufferToFill);

    // Process the buffer
    processorManager.processBlock(mainBuffer);

    // Copy processed data back to bufferToFill
    for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
    {
        bufferToFill.buffer->copyFrom(channel, bufferToFill.startSample, mainBuffer, channel, 0, bufferToFill.numSamples);
    }

    // Retrieve detected sibilant regions for visualization
    detectedSibilantRegions = processorManager.getSibilantRegions();
    waveformDisplay.setSibilantRegions(detectedSibilantRegions);
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
    topSection.items.add(juce::FlexItem(openButton).withFlex(0.5f));
    topSection.items.add(juce::FlexItem(exportButton).withFlex(0.5f));
    topSection.performLayout(bounds.removeFromTop(topSectionHeight));

    // Middle section: Waveform display
    juce::FlexBox middleSection;
    middleSection.flexDirection = juce::FlexBox::Direction::column;
    middleSection.items.add(juce::FlexItem(waveformDisplay).withFlex(1.0f));
    middleSection.performLayout(bounds.removeFromTop(middleSectionHeight));

    // Transport controls
    juce::FlexBox transportSection;
    transportSection.flexDirection = juce::FlexBox::Direction::row;
    transportSection.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
    transportSection.items.add(juce::FlexItem(playButton).withFlex(1.0f));  // Play button
    transportSection.items.add(juce::FlexItem(stopButton).withFlex(1.0f));  // Stop button
    transportSection.performLayout(bounds.removeFromTop(transportSectionHeight));

    // Bottom section: Filter controls and algorithm selector
    juce::FlexBox bottomSection;
    bottomSection.flexDirection = juce::FlexBox::Direction::column;
    bottomSection.items.add(juce::FlexItem(filterControl).withFlex(1.0f));  // Filter controls
    bottomSection.items.add(juce::FlexItem(algorithmSelector).withFlex(1.0f));  // Algorithm selector
    bottomSection.performLayout(bounds);
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
                                                                                  2, // Number of channels
                                                                                  16, // Bits per sample
                                                                                  {}, // Metadata
                                                                                  0)); // Quality

        if (writer == nullptr)
            return; // Handle writer creation failure

        // Since the writer now owns the stream, release it from the unique_ptr
        outputStream.release();

        // Prepare a buffer to hold the entire audio
        int totalSamples = static_cast<int>(transportSource.getLengthInSeconds() * currentSampleRate);
        juce::AudioBuffer<float> bufferToWrite(2, totalSamples); // Assuming stereo

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

//void MainComponent::exportButtonClicked()
//{
//    juce::FileChooser exportChooser("Save Processed Audio...", juce::File{}, "*.wav");
//
//    exportChooser.launchAsync(juce::FileBrowserComponent::saveMode, [this](const juce::FileChooser& fc)
//    {
//        auto file = fc.getResult();
//        if (file == juce::File{})
//            return;
//
//        // Create an output stream
//        std::unique_ptr<juce::FileOutputStream> outputStream(file.createOutputStream());
//
//        if (outputStream == nullptr)
//            return; // Handle error appropriately
//
//        // Set up an audio format writer
//        juce::AudioFormat* format = formatManager.findFormatForFileExtension(file.getFileExtension());
//
//        if (format == nullptr)
//            return; // Handle unsupported format
//
//        std::unique_ptr<juce::AudioFormatWriter> writer(format->createWriterFor(outputStream.get(),
//                                                                                  currentSampleRate, // Use stored sample rate
//                                                                                  2, // Number of channels
//                                                                                  16, // Bits per sample
//                                                                                  {}, // Metadata
//                                                                                  0)); // Quality
//
//        if (writer == nullptr)
//            return; // Handle writer creation failure
//
//        // Since the writer now owns the stream, release it from the unique_ptr
//        outputStream.release();
//
//        // Prepare a buffer to hold the entire audio
//        int totalSamples = static_cast<int>(transportSource.getLengthInSeconds() * currentSampleRate);
//        juce::AudioBuffer<float> bufferToWrite(2, totalSamples); // Assuming stereo
//
//        // Reset transport source to the beginning
//        transportSource.setPosition(0.0);
//
//        // Read the entire audio into bufferToWrite
//        juce::AudioSourceChannelInfo info(&bufferToWrite, 0, bufferToWrite.getNumSamples());
//        transportSource.getNextAudioBlock(info);
//
//        // Process the buffer
//        processorManager.processBlock(bufferToWrite);
//
//        // Convert float samples to 16-bit integers for WAV
//        juce::AudioBuffer<int16_t> intBuffer;
//        intBuffer.setSize(bufferToWrite.getNumChannels(), bufferToWrite.getNumSamples());
//
//        for (int channel = 0; channel < bufferToWrite.getNumChannels(); ++channel)
//        {
//            bufferToWrite.copyFrom(channel, 0, bufferToWrite, channel, 0, bufferToWrite.getNumSamples());
//            bufferToWrite.applyGain(channel, 0, bufferToWrite.getNumSamples(), 32767.0f); // Scale to int16
//            int16_t* dest = intBuffer.getWritePointer(channel);
//            const float* src = bufferToWrite.getReadPointer(channel);
//            for (int i = 0; i < bufferToWrite.getNumSamples(); ++i)
//                dest[i] = static_cast<int16_t>(juce::jlimit<float>(-32768.0f, src[i], 32767.0f));
//        }
//
//        // Write to file
//        writer->writeFromAudioSampleBuffer(bufferToWrite, 0, bufferToWrite.getNumSamples());
//    });
//}


