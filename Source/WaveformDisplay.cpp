#include "WaveformDisplay.h"
#include <juce_audio_formats/juce_audio_formats.h>

WaveformDisplay::WaveformDisplay()
{
}

WaveformDisplay::~WaveformDisplay()
{
    clearThumbnail(); 
}

void WaveformDisplay::clearThumbnail() 
{
    if (thumbnail != nullptr)
    {
        thumbnail->removeChangeListener(this);
        thumbnail = nullptr;
    }
}

void WaveformDisplay::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1e2228)); // stesso kBg dell'editor

    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 1);   

    if (thumbnail != nullptr && thumbnail->getTotalLength() > 0.0)
    {
        auto audioLength = thumbnail->getTotalLength();
        auto area = getLocalBounds();
        auto startTime = 0.0;
        auto endTime = audioLength;

        // Draw the waveform
        g.setColour(juce::Colours::cyan);
        thumbnail->drawChannel(g, area, startTime, endTime, 0, 1.0f);

        // Draw loop range if enabled
        if (showLoopRange && loopEnd > loopStart)
        {
            auto loopStartX = area.getX() + (loopStart / audioLength) * area.getWidth();
            auto loopEndX = area.getX() + (loopEnd / audioLength) * area.getWidth();
            auto loopArea = juce::Rectangle<int>(loopStartX, area.getY(), loopEndX - loopStartX, area.getHeight());

            g.setColour(juce::Colours::orange.withAlpha(0.3f));
            g.fillRect(loopArea);
            g.setColour(juce::Colours::orange);
            g.drawRect(loopArea, 2);
        }

        // Draw playback position
        if (playbackPosition > 0.0)
        {
            auto posX = area.getX() + (playbackPosition / audioLength) * area.getWidth();
            g.setColour(juce::Colours::red);
            g.drawLine(posX, area.getY(), posX, area.getBottom(), 2.0f);
        }
    }
    else
    {
        g.setColour(juce::Colours::white);
        g.setFont(14.0f);
        g.drawFittedText("Drop an audio file to load", getLocalBounds(), juce::Justification::centred, 1);
    }
}

void WaveformDisplay::resized()
{
    // This method is where you should set the bounds of any child
}

void WaveformDisplay::setAudioThumbnail(juce::AudioThumbnail* newThumbnail)
{
    clearThumbnail();

    thumbnail = newThumbnail;

    if (thumbnail != nullptr)
        thumbnail->addChangeListener(this);

    repaint();
}

void WaveformDisplay::setPlaybackPosition(double position)
{
    playbackPosition = position;
    repaint();
}

void WaveformDisplay::setLoopRange(double start, double end)
{
    loopStart = start;
    loopEnd = end;
    showLoopRange = true;
    repaint();
}

void WaveformDisplay::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == thumbnail)
        repaint();
}