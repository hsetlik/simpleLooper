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
    
    setSize (300, 200);

    formatManager.registerBasicFormats();
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto numInputChannels = fileBuffer.getNumChannels();
    auto numOutputChannels = bufferToFill.buffer->getNumChannels();
    
    auto samplesRemaining = bufferToFill.numSamples; //this var allows the while loop to check whether the whole sample has been loaded
    auto sampleOffset = bufferToFill.startSample; //sample offset var starts at the buffer's starting sample
    while(samplesRemaining > 0)
    {
        auto bufferSamplesRemaining = fileBuffer.getNumSamples() - position; //decrements the remaining sample count by the number of samples loaded in the last loop
        auto samplesThisTime = juce::jmin(samplesRemaining, bufferSamplesRemaining); //tracks how many samples are loaded in each loop
        for(auto channel = 0; channel < numOutputChannels; ++channel) //just ensures that all channels are loaded in the same way
        {
            bufferToFill.buffer->copyFrom(channel,//copy data from the buffer to the channel in the 1st argument
                                          sampleOffset, //starts writing at the first unwritten position in the output buffer
                                          fileBuffer, //the source buffer
                                          channel % numInputChannels, //chooses the channel of the source buffer from which to copy
                                          position, //the source buffer is read starting here
                                          samplesThisTime); //the number of samples to be read on this loop
        }
        samplesRemaining -= samplesThisTime; //updates the data for the next iteration of the loop
        sampleOffset += samplesThisTime;
        position += samplesThisTime;
        
        if(position == fileBuffer.getNumSamples()){ //causes the position to reset (i.e. loop) such that the file is again read from the beginning
            position = 0;
        }
    }
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.
    fileBuffer.setSize(0, 0);
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
    openButton .setBounds (10, 10, getWidth() - 20, 20);
    clearButton.setBounds (10, 40, getWidth() - 20, 20);
}

void MainComponent::openButtonClicked()
{
    shutdownAudio();
    juce::FileChooser chooser ("Select a .wav file", {}, "*.wav");
    if(chooser.browseForFileToOpen())
    {
        auto chosenFile = chooser.getResult();
        std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor(chosenFile)); //this reader object is responsible for loading data from the sample file into the fileBuffer object
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
                setAudioChannels(0, (int) reader->numChannels); //turns audio back on after turning it off on line 88
            }
        }
    }
}

void MainComponent::clearButtonClicked(){
    shutdownAudio();
}
