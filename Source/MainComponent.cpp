#include "MainComponent.h"

MainComponent::MainComponent()
{
    // Add GUI components
    addAndMakeVisible(algorithmSelector);
    addAndMakeVisible(filterControl);
    addAndMakeVisible(waveformDisplay);
    addAndMakeVisible(mixerControl);
    addAndMakeVisible(loadButton);

    loadButton.setButtonText("Load Audio File");
    loadButton.onClick = [this]() { loadAudioFile(); };

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

    // Connect mixer control to processor manager
    mixerControl.mixSlider.onValueChange = [this]()
    {
        processorManager.setMixLevel(mixerControl.getMixLevel());
    };

    setSize(800, 600);

    // Initialize audio channels (2 input, 2 output)
    setAudioChannels(2, 2);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    processorManager.prepare(sampleRate, samplesPerBlockExpected, getMainBusNumInputChannels());
    
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Clear the buffer initially
    bufferToFill.clearActiveBufferRegion();

    // Forward the audio buffer to processorManager
    juce::AudioBuffer<float> buffer(bufferToFill.buffer->getArrayOfWritePointers(), bufferToFill.buffer->getNumChannels(), bufferToFill.startSample, bufferToFill.numSamples);
    processorManager.processBlock(buffer);

    // Copy processed data back to the bufferToFill
    bufferToFill.buffer->copyFrom(bufferToFill.startSample, 0, buffer, 0, buffer.getNumSamples());
}

void MainComponent::releaseResources()
{
    // Clean up if necessary
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey); // Background color
}

void MainComponent::resized()
{
    auto area = getLocalBounds();
    auto controlHeight = 50;

    algorithmSelector.setBounds(area.removeFromTop(controlHeight));
    filterControl.setBounds(area.removeFromTop(controlHeight));
    loadButton.setBounds(area.removeFromTop(controlHeight));
    mixerControl.setBounds(area.removeFromTop(controlHeight));
    waveformDisplay.setBounds(area);
}

void MainComponent::loadAudioFile()
{
    juce::FileChooser chooser("Select an audio file...",
                              juce::File(),
                              "*.wav;*.mp3;*.aiff");

    if (chooser.browseForFileToOpen())
    {
        audioFile = chooser.getResult();
        waveformDisplay.loadFile(audioFile);
    }
}
