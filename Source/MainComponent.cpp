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
    if (!readerSource || processorManager.getProcessedBuffer().getNumSamples() == 0)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    auto& processedBuffer = processorManager.getProcessedBuffer();
    auto numSamples = bufferToFill.buffer->getNumSamples();
    auto startSample = bufferToFill.startSample;

    for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
    {
        auto* dest = bufferToFill.buffer->getWritePointer(channel, startSample);
        auto* src = processedBuffer.getReadPointer(channel);

        for (int i = 0; i < numSamples; ++i)
            dest[i] = src[i];
    }
}

void MainComponent::releaseResources()
{
    transportSource.releaseResources();
}


void MainComponent::resized()
{
    auto bounds = getLocalBounds();
    
    // Define height proportions
    const int topSectionHeight = bounds.getHeight() / 16;
    const int middleSectionHeight = bounds.getHeight() / 2;
    const int transportSectionHeight = bounds.getHeight() / 16;
//    const int bottomSectionHeight = bounds.getHeight() / 4;

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

void MainComponent::processFile()
{
    if (!loadedFile.existsAsFile())
    {
        DBG("No file loaded to process!");
        return;
    }

    processorManager.setDeEssingParameters(
        filterControl.getThreshold(),
        filterControl.getReduction(),
        filterControl.getFrequency(),
        filterControl.getHysteresis()
    );

    // Process the file for sibilants
    processorManager.processFileForSibilants(loadedFile);

    // Pass the processed sibilant data to the waveform display
    juce::AudioBuffer<float> sibilantBuffer;
    processorManager.getSibilantBuffer(sibilantBuffer);
    waveformDisplay.setSibilantBuffer(sibilantBuffer);

    repaint();
}



