#pragma once
#include <vector>
#include <cmath>
#include "JuceHeader.h"

enum class FilterMode {
    LowPass,
    HighPass,
    BandPass,
    Bypass
};

class LowpassHighpassFilter {
public:
    // setters
    void setMode(FilterMode newMode) {
        mode = newMode;
        updateCoefficients();
    }

    void setCutoffFrequency(float newCutoffFrequency) {
        cutoffFrequency = newCutoffFrequency;
        updateCoefficients();
    }

    void setResonance(float newResonance) {
        resonance = newResonance;
        updateCoefficients();
    }

    void setSamplingRate(float newSamplingRate) {
        samplingRate = newSamplingRate;
        updateCoefficients();
    }

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
        int numChannels = buffer.getNumChannels();
        int numSamples = buffer.getNumSamples();

        // Resize state vectors if necessary
        if (x1.size() != numChannels) {
            x1.resize(numChannels, 0.0f);
            x2.resize(numChannels, 0.0f);
            y1.resize(numChannels, 0.0f);
            y2.resize(numChannels, 0.0f);
        }

        for (int channel = 0; channel < numChannels; ++channel) {
            auto* channelSamples = buffer.getWritePointer(channel);

            for (int i = 0; i < numSamples; ++i) {
                float input = channelSamples[i];
                float output = b0 * input + b1 * x1[channel] + b2 * x2[channel] - a1 * y1[channel] - a2 * y2[channel];

                x2[channel] = x1[channel];
                x1[channel] = input;
                y2[channel] = y1[channel];
                y1[channel] = output;

                channelSamples[i] = output;
            }
        }
    }

private:
    void updateCoefficients() {

        if (mode == FilterMode::Bypass) {
            b0 = 1.0f;
            b1 = 0.0f;
            b2 = 0.0f;
            a0 = 1.0f;
            a1 = 0.0f;
            a2 = 0.0f;
            return;
        }

        constexpr auto PI = 3.14159265359f;
        float omega = 2.0f * PI * cutoffFrequency / samplingRate;
        float sn = std::sin(omega);
        float cs = std::cos(omega);
        float alpha = sn / (2.0f * resonance);

        // Usa il calcolo corretto per i coefficienti del filtro
        switch (mode) {
        case FilterMode::LowPass:
            b0 = (1.0f - cs) / 2.0f;
            b1 = 1.0f - cs;
            b2 = (1.0f - cs) / 2.0f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cs;
            a2 = 1.0f - alpha;
            break;
        case FilterMode::HighPass:
            b0 = (1.0f + cs) / 2.0f;
            b1 = -(1.0f + cs);
            b2 = (1.0f + cs) / 2.0f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cs;
            a2 = 1.0f - alpha;
            break;
        case FilterMode::BandPass:
            b0 = alpha;
            b1 = 0.0f;
            b2 = -alpha;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cs;
            a2 = 1.0f - alpha;
            break;
        case FilterMode::Bypass: // Già gestito sopra, ma incluso per completezza
            b0 = 1.0f;
            b1 = 0.0f;
            b2 = 0.0f;
            a0 = 1.0f;
            a1 = 0.0f;
            a2 = 0.0f;
            break;
        }

        // normalizza per a0
        b0 /= a0;
        b1 /= a0;
        b2 /= a0;
        a1 /= a0;
        a2 /= a0;
    }

    FilterMode mode = FilterMode::LowPass;
    float cutoffFrequency = 1000.0f;
    float resonance = 1.0f;
    float samplingRate = 44100.0f;

    // coefficienti
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f, a0 = 1.0f, a1 = 0.0f, a2 = 0.0f;

    // State variables per channel
    std::vector<float> x1, x2, y1, y2;
};