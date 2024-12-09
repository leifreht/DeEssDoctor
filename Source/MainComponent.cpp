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

    setAudioChannels(1, 1);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
//    processorManager.prepare(sampleRate, samplesPerBlockExpected, getTotalNumInputChannels());
    processorManager.prepare(sampleRate, samplesPerBlockExpected, 1);
    
    waveformDisplay.setSampleRate(sampleRate);
    
    currentSampleRate = sampleRate;

}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
//    transportSource.getNextAudioBlock(bufferToFill);
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

    // Middle section layout: Waveform + Overlay
    auto middleSectionBounds = bounds.removeFromTop(middleSectionHeight);
    waveformDisplay.setBounds(middleSectionBounds);
    positionOverlay.setBounds(middleSectionBounds);

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

//void MainComponent::openButtonClicked()
//{
//    chooser = std::make_unique<juce::FileChooser> ("Select a Wave file to play...",
//                                                   juce::File{},
//                                                   "*.wav");
//    auto chooserFlags = juce::FileBrowserComponent::openMode
//                      | juce::FileBrowserComponent::canSelectFiles;
//
//    chooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& fc)
//    {
//        auto file = fc.getResult();
//
//        if (file != juce::File{})
//        {
//            auto* reader = formatManager.createReaderFor (file);
//            fileLabel.setText(file.getFileName(), juce::dontSendNotification);
//
//            if (reader != nullptr)
//            {
//                auto newSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);
//                transportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate);
//                playButton.setEnabled (true);
//                waveformDisplay.setFile (file);
//                readerSource.reset (newSource.release());
//            }
//        }
//    });
//}


//void MainComponent::openButtonClicked()
//{
//    chooser = std::make_unique<juce::FileChooser>("Select an Audio file...",
//                                                juce::File{},
//                                                "*.wav;*.mp3;*.aiff;*.flac");
//
//    auto chooserFlags = juce::FileBrowserComponent::openMode
//                      | juce::FileBrowserComponent::canSelectFiles;
//
//    chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
//    {
//        auto file = fc.getResult();
//
//        if (file != juce::File{})
//        {
//            auto* reader = formatManager.createReaderFor(file);
//            fileLabel.setText(file.getFileName(), juce::dontSendNotification);
//
//            if (reader != nullptr)
//            {
//                // If the audio is stereo, sum to mono
//                std::unique_ptr<juce::AudioFormatReaderSource> newSource;
//                if (reader->numChannels > 1)
//                {
//                    int lengthInSamples = juce::jlimit<int>(
//                        0,
//                        std::numeric_limits<int>::max(),
//                        static_cast<int>(reader->lengthInSamples)
//                    );
//                    
//                    // Create a temporary buffer to hold summed mono data
//                    juce::AudioBuffer<float> monoBuffer(1, lengthInSamples);
//                    
//                    // Read the stereo data
//                    reader->read(&monoBuffer, 0, reader->lengthInSamples, 0, true, true);
//                    
//                    // Sum the two channels into one
//                    monoBuffer.applyGain(0, 0, monoBuffer.getNumSamples(), 0.5f); // Optional: prevent clipping
//
//                    // Create a new buffer source for the mono data
//                    auto* monoReader = new juce::AudioFormatReader(&monoBuffer, reader->sampleRate, 1);
//                    newSource = std::make_unique<juce::AudioFormatReaderSource>(monoReader, true);
//                }
//                else
//                {
//                    // Mono audio, no conversion needed
//                    newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
//                }
//
//                transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
//                playButton.setEnabled(true);
//                waveformDisplay.setFile(file);
//                readerSource.reset(newSource.release());
//            }
//        }
//    });
//}

//void MainComponent::openButtonClicked()
//{
//    chooser = std::make_unique<juce::FileChooser>(
//        "Select an Audio file...", juce::File{}, "*.wav;*.mp3;*.aiff;*.flac");
//
//    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
//
//    chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
//    {
//        auto file = fc.getResult();
//
//        if (file != juce::File{})
//        {
//            // Use the format manager to create an AudioFormatReader
//            auto* reader = formatManager.createReaderFor(file);
//            fileLabel.setText(file.getFileName(), juce::dontSendNotification);
//
//            if (reader != nullptr)
//            {
//                // Create a buffer to hold audio data
//                juce::AudioBuffer<float> buffer(reader->numChannels, (int)reader->lengthInSamples);
//                
//                // Read audio into buffer
//                reader->read(&buffer, 0, (int)reader->lengthInSamples, 0, true, true);
//
//                // Convert to mono if necessary
//                if (reader->numChannels > 1)
//                {
//                    for (int i = 1; i < reader->numChannels; ++i)
//                        buffer.addFrom(0, 0, buffer, i, 0, buffer.getNumSamples());
//                    buffer.applyGain(1.0f / reader->numChannels);
//                }
//
//                // Create an AudioFormatReaderSource using the reader
//                auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
//                transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
//
//                playButton.setEnabled(true);
//                waveformDisplay.setFile(file);
//                readerSource.reset(newSource.release());
//            }
//        }
//    });
//}

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
            auto* reader = formatManager.createReaderFor(file);

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
//        int samplesRead = 0;
//        while (samplesRead < totalSamples)
//        {
//            int samplesToRead = std::min(totalSamples - samplesRead, (int)bufferToWrite.getNumSamples());
//            juce::AudioSourceChannelInfo info(&bufferToWrite, samplesRead, samplesToRead);
//            transportSource.getNextAudioBlock(info);
//            samplesRead += samplesToRead;
//        }
//
//        // Process the buffer
//        processorManager.processBlock(bufferToWrite);
//
//        // Write to file directly without manual conversion
//        writer->writeFromAudioSampleBuffer(bufferToWrite, 0, bufferToWrite.getNumSamples());
//    });
//}


