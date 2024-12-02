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

    setSize(800, 600);
    
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
//    processorManager.prepare(sampleRate, samplesPerBlockExpected, 2);
//    processorManager.prepare(sampleRate, samplesPerBlockExpected, getMainBusNumInputChannels());
//    processorManager.prepare(sampleRate, samplesPerBlockExpected, getTotalNumInputChannels());
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (readerSource.get() == nullptr)
        bufferToFill.clearActiveBufferRegion();
    else
        transportSource.getNextAudioBlock(bufferToFill);
}

//void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
//{
//    // Clear the buffer initially
//    bufferToFill.clearActiveBufferRegion();
//
//    // Forward the audio buffer to processorManager
//    juce::AudioBuffer<float> buffer(bufferToFill.buffer->getArrayOfWritePointers(), bufferToFill.buffer->getNumChannels(), bufferToFill.startSample, bufferToFill.numSamples);
//    processorManager.processBlock(buffer);
//
//    // Copy processed data back to the bufferToFill
//    bufferToFill.buffer->copyFrom(0, bufferToFill.startSample, buffer, 0, 0, buffer.getNumSamples());
//}

void MainComponent::releaseResources()
{
    transportSource.releaseResources();
}

//void MainComponent::paint(juce::Graphics& g)
//{
//    g.fillAll(juce::Colours::darkgrey); // Background color
//}

void MainComponent::resized()
{
//    auto area = getLocalBounds();
//    auto controlHeight = 50;
    openButton.setBounds(10, 10, getWidth() - 20, 20);
    playButton.setBounds(10, 40, getWidth() - 20, 20);
    stopButton.setBounds(10, 70, getWidth() - 20, 20);

    juce::Rectangle<int> displayBounds(10, 100, getWidth() - 20, getHeight() - 120);
    waveformDisplay.setBounds(displayBounds);
    positionOverlay.setBounds(displayBounds);
    

//    algorithmSelector.setBounds(area.removeFromTop(controlHeight));
//    filterControl.setBounds(area.removeFromTop(controlHeight));
//    loadButton.setBounds(area.removeFromTop(controlHeight));
//    mixerControl.setBounds(area.removeFromTop(controlHeight));
//    waveformDisplay.setBounds(area);
}

//void MainComponent::loadAudioFile()
//{
//    auto chooser = std::make_unique<juce::FileChooser>(
//        "Select an audio file...", juce::File(), "*.wav;*.mp3;*.aiff");
//
//    chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
//        [this](const juce::FileChooser& fc)
//        {
//            auto result = fc.getResult();
//
//            if (result.existsAsFile())
//            {
//                audioFile = result;
//                waveformDisplay.loadFile(audioFile);
//            }
//        });
//}

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



