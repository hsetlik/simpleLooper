#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // Make sure you set the size of the component after
    // you add any child components.
    addAndMakeVisible(&openButton);
    openButton.setButtonText("Open. . .");
    openButton.onClick = [this] { openButtonClicked();};
    
    addAndMakeVisible(&clearButton);
    clearButton.setButtonText("Clear");
    clearButton.onClick = [this] { clearButtonClicked();};
    
    setSize (800, 600);

    setAudioChannels(2, 2);
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    bufferToFill.clearActiveBufferRegion();
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}

void MainComponent::openButtonClicked()
{
    shutdownAudio();
    juce::FileChooser chooser ("Select a .wav file", {}, "*.wav");
    if(chooser.browseForFileToOpen())
    {
        auto chosenFile = chooser.getResult();
        std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor(chosenFile));
        if(reader.get() != nullptr) //evaluates true if the reader is created successfully
        {
            auto duration = (float)reader->lengthInSamples / reader->sampleRate;
            if(duration < 5) //ensures the sample is not more than 5 seconds long
            {
                fileBuffer.setSize((int) reader->numChannels, (int)reader-> lengthInSamples); //sets aside a buffer of the appropriate size for the sample
                reader->read(&fileBuffer, //'read' will load the samples into memory at the address of the buffer
                             0, //load samples into the buffer beginning at buffer index 0
                             (int) reader->lengthInSamples, //the number of samples to load
                             0, //start reading the file from its sample index 0
                             true, //read the left channel?
                             true); //read the right channel?
                position = 0;
                setAudioChannels(0, (int) reader->numChannels);
            }
        }
    }
}
