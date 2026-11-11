#pragma once

#include <JuceHeader.h>
#include "SamplerSound.h"

class SamplerVoice : public juce::SynthesiserVoice
{
public:
    SamplerVoice() = default;

    bool canPlaySound(juce::SynthesiserSound* s) override
    {
        return dynamic_cast<SamplerSound*> (s) != nullptr;
    }

    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* s, int) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}

    void renderNextBlock(juce::AudioBuffer<float>&, int startSample, int numSamples) override;

    void setAdsrParameters(const juce::ADSR::Parameters& params) { adsr.setParameters(params); adsrParams = params; }
    void setFilterAdsrParameters(const juce::ADSR::Parameters& params) { filterAdsr.setParameters(params); filterAdsrParams = params; }
    void setPitchModulation(float semitones) { pitchModulation = semitones; updatePitchRatio(); }

    void setLoFiEnabled(bool enabled) { loFiEnabled = enabled; }
    void setNoiseMix(float mix) { noiseMix = mix; }
    void setBitCrush(float amount) { bitCrushAmount = amount; }
    void setSampleRateReduction(float reduction) { sampleRateReduction = reduction; }
    void setRandomPlayback(float amount) { randomPlaybackAmount = amount; }
    void setPitchVariation(float variation) { pitchVariation = variation; }
    void setRandomPlaybackEnabled(bool enabled) { randomPlaybackEnabled = enabled; }

    float getFilterAdsrValue() const { return lastFilterAdsrValue; }
    bool isActive() const { return adsr.isActive(); }

private:
    void updatePitchRatio();
    float processLoFi(float sample);
    float generateNoise();
    void applyRandomPlayback();

    juce::ADSR adsr;
    juce::ADSR filterAdsr;
    juce::ADSR::Parameters adsrParams{ 0.002f, 0.01f, 1.0f, 0.08f };
    juce::ADSR::Parameters filterAdsrParams{ 0.002f, 0.01f, 1.0f, 0.08f };

    double basePitchRatio = 1.0;
    double currentPitchRatio = 1.0;
    double sourceSamplePosition = 0.0;
    float pitchModulation = 0.0f;

    bool legatoMode = false;
    bool retriggerOnLegato = false;

    SamplerSound* currentSound = nullptr;

    bool loFiEnabled = false;
    float noiseMix = 0.0f;
    float bitCrushAmount = 0.0f;
    float sampleRateReduction = 1.0f;

    float randomPlaybackAmount = 0.0f;
    float pitchVariation = 0.0f;
    bool randomPlaybackEnabled = false;
    int randomPlaybackCounter = 0;
    int randomPlaybackInterval = 1000;

    juce::Random random;

    float lastFilterAdsrValue = 0.0f;

    friend class ProgettoPlugInAudioProcessor;
    void setMode(bool legato, bool retrigger) { legatoMode = legato; retriggerOnLegato = retrigger; }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SamplerVoice)
};