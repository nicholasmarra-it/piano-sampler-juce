#pragma once

#include <JuceHeader.h>

class SamplerSound : public juce::SynthesiserSound
{
public:
    SamplerSound(juce::String nameIn,
        juce::AudioBuffer<float>&& bufferIn,
        const juce::BigInteger& notes,
        int midiRootNoteIn,
        double sourceSampleRateIn)
        : name(std::move(nameIn)),
        audioBuffer(std::move(bufferIn)),
        midiNotes(notes),
        midiRootNote(midiRootNoteIn),
        length(audioBuffer.getNumSamples()),
        sourceSampleRate(sourceSampleRateIn)
    {
    }

    bool appliesToNote(int midiNoteNumber) override { return midiNotes[midiNoteNumber]; }
    bool appliesToChannel(int) override { return true; }

    juce::String              name;
    juce::AudioBuffer<float>  audioBuffer;
    juce::BigInteger          midiNotes;
    int    midiRootNote = 60;
    int    length = 0;
    double sourceSampleRate = 44100.0;

    int  minVelocity = 1;
    int  maxVelocity = 127;
    int  coarseSemitones = 0;
    int  fineCents = 0;
    bool oneShot = false;
};