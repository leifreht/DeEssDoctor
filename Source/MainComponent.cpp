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
    
    // In the constructor
    exportButton.setButtonText("Export...");
    addAndMakeVisible(exportButton);
    exportButton.onClick = [this]()
    {
        // Implement export functionality here
    };
    
//    addAndMakeVisible(waveformDisplay);
//    addAndMakeVisible(mixerControl);
//    addAndMakeVisible(loadButton);

//    loadButton.setButtonText("Load Audio File");
//    loadButton.onClick = [this]() { loadAudioFile(); };

    // Connect the algorithm selector to the processor manager
    algorithmSelector.algorithmChanged = [this]()
    {
        auto selectedAlgorithm = algorithmSelector.getSelectedAlgorithm();

        if (selectedAlgorithm == "Amplitude Threshold")
            processorManager.setAlgorithm(amplitudeThresholdAlgorithm);
        else if (selectedAlgorithm == "Spectral Analysis")
            processorManager.setAlgorithm(spectralAnalysisAlgorithm);
    };

    // Connect filter controls to processor manager
    filterControl.frequencySlider.onValueChange = [this]()
    {
        processorManager.setFilterParameters(filterControl.getFrequency(), filterControl.getQFactor(), filterControl.getGain());
    };

//    // Connect mixer control to processor manager
//    mixerControl.mixSlider.onValueChange = [this]()
//    {
//        processorManager.setMixLevel(mixerControl.getMixLevel());
//    };

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
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (readerSource.get() == nullptr)
        bufferToFill.clearActiveBufferRegion();
    else
        transportSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    transportSource.releaseResources();
}

//void MainComponent::resized()
//{
//    // set bounds is x, y, w and h
//    
////    auto area = getLocalBounds();
////    auto controlHeight = 50;
//    openButton.setBounds(getWidth() - (getWidth() / 4), 10, getWidth() / 4, 20);
//    playButton.setBounds(10, 40, getWidth() - 20, 20);
//    stopButton.setBounds(10, 70, getWidth() - 20, 20);
//
//    juce::Rectangle<int> displayBounds(10, 100, getWidth() - 20, getHeight() - 120);
//    waveformDisplay.setBounds(displayBounds);
//    positionOverlay.setBounds(displayBounds);
//    
//
////    algorithmSelector.setBounds(area.removeFromTop(controlHeight));
////    filterControl.setBounds(area.removeFromTop(controlHeight));
////    loadButton.setBounds(area.removeFromTop(controlHeight));
////    mixerControl.setBounds(area.removeFromTop(controlHeight));
////    waveformDisplay.setBounds(area);
//}

void MainComponent::resized()
{
    auto bounds = getLocalBounds();
    
    // Define height proportions
    const int topSectionHeight = bounds.getHeight() / 8;  // 1/8 for the top section
    const int middleSectionHeight = bounds.getHeight() / 2;  // 1/2 for the middle section
    const int transportSectionHeight = bounds.getHeight() / 8;  // 1/8 for transport controls
    const int bottomSectionHeight = bounds.getHeight() / 4;  // 1/4 for the bottom section

    // Top section: File name, Open button, Export button
    juce::FlexBox topSection;
    topSection.flexDirection = juce::FlexBox::Direction::row;
    topSection.items.add(juce::FlexItem(fileLabel).withFlex(1.0f));  // File name label
    topSection.items.add(juce::FlexItem(openButton).withFlex(0.5f));  // Open button
    topSection.items.add(juce::FlexItem(exportButton).withFlex(0.5f));  // Export button
    topSection.performLayout(bounds.removeFromTop(topSectionHeight));

    // Middle section: Waveform display
    juce::FlexBox middleSection;
    middleSection.flexDirection = juce::FlexBox::Direction::column;
    middleSection.items.add(juce::FlexItem(waveformDisplay).withFlex(1.0f));  // Waveform display
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



