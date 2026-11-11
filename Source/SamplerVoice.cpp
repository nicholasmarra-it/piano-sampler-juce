#include "SamplerVoice.h"
#include <cmath>

void SamplerVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* s, int)
{
    auto* snd = dynamic_cast<SamplerSound*> (s);
    if (snd == nullptr)
    {
        clearCurrentNote();
        return;
    }

    float pitchRandom = 0.0f;
    if (randomPlaybackEnabled && pitchVariation > 0.0f)
    {
        pitchRandom = random.nextFloat() * pitchVariation * 2.0f - pitchVariation;
    }

    const double semitones = (double)(midiNoteNumber - snd->midiRootNote + snd->coarseSemitones)
        + ((double)snd->fineCents / 100.0) + pitchRandom;
    const double pitch = std::pow(2.0, semitones / 12.0);
    const double rate = (snd->sourceSampleRate > 0.0) ? (snd->sourceSampleRate / getSampleRate()) : 1.0;
    basePitchRatio = pitch * rate;

    updatePitchRatio();

    const bool wasActive = adsr.isActive();

    if (randomPlaybackEnabled)
    {
        randomPlaybackCounter = 0;
        randomPlaybackInterval = random.nextInt(500) + 100;
    }

    if (legatoMode && wasActive)
    {
        if (retriggerOnLegato)
        {
            adsr.noteOn();
            filterAdsr.noteOn();
        }
        currentSound = snd;
        sourceSamplePosition = 0.0;
    }
    else
    {
        currentSound = snd;
        sourceSamplePosition = 0.0;
        adsr.setParameters(adsrParams);
        filterAdsr.setParameters(filterAdsrParams);
        adsr.noteOn();
        filterAdsr.noteOn();
        lastFilterAdsrValue = 0.0f;
    }
}

void SamplerVoice::stopNote(float velocity, bool allowTailOff)
{
    if (currentSound && currentSound->oneShot)
        return;

    if (allowTailOff)
    {
        adsr.noteOff();
        filterAdsr.noteOff();
    }
    else
    {
        clearCurrentNote();
        currentSound = nullptr;
        adsr.reset();
        filterAdsr.reset();
        lastFilterAdsrValue = 0.0f;
    }
}

void SamplerVoice::renderNextBlock(juce::AudioBuffer<float>& output, int startSample, int numSamples)
{
    if (currentSound == nullptr)
        return;

    auto& data = currentSound->audioBuffer;

    const float* inL = data.getReadPointer(0);
    const float* inR = data.getNumChannels() > 1 ? data.getReadPointer(1) : nullptr;

    float* outL = output.getWritePointer(0, startSample);
    float* outR = output.getNumChannels() > 1 ? output.getWritePointer(1, startSample) : nullptr;

    while (--numSamples >= 0)
    {
        const int pos = (int)sourceSamplePosition;

        if (pos < 0 || pos >= currentSound->length)
        {
            stopNote(0.0f, true);
            break;
        }

        const float env = adsr.getNextSample();

        lastFilterAdsrValue = filterAdsr.getNextSample();

        if (env <= 0.0f)
        {
            clearCurrentNote();
            currentSound = nullptr;
            break;
        }

        float dryL = inL[pos];
        float dryR = (inR != nullptr ? inR[pos] : dryL);

        if (loFiEnabled)
        {
            dryL = processLoFi(dryL);
            dryR = processLoFi(dryR);

            if (noiseMix > 0.0f)
            {
                float noise = generateNoise();
                dryL = (1.0f - noiseMix) * dryL + noiseMix * noise;
                dryR = (1.0f - noiseMix) * dryR + noiseMix * noise;
            }
        }

        if (randomPlaybackEnabled && randomPlaybackAmount > 0.0f)
        {
            applyRandomPlayback();
        }

        const float sL = dryL * env;
        const float sR = dryR * env;

        if (outL) *outL++ += sL;
        if (outR) *outR++ += sR;

        sourceSamplePosition += currentPitchRatio;
        randomPlaybackCounter++;
    }

    if (!adsr.isActive())
    {
        clearCurrentNote();
        currentSound = nullptr;
        lastFilterAdsrValue = 0.0f;
    }
}

void SamplerVoice::updatePitchRatio()
{
    double modulatedPitch = basePitchRatio * std::pow(2.0, pitchModulation / 12.0);
    currentPitchRatio = modulatedPitch;
}

void SamplerVoice::applyRandomPlayback()
{
    if (randomPlaybackCounter >= randomPlaybackInterval)
    {
        randomPlaybackCounter = 0;
        randomPlaybackInterval = random.nextInt(800) + 200;

        float probability = randomPlaybackAmount * 0.5f;
        if (random.nextFloat() < probability)
        {
            int newPosition = random.nextInt(currentSound->length);
            sourceSamplePosition = (double)newPosition;

            if (pitchVariation > 0.0f)
            {
                float tempPitchVariation = random.nextFloat() * pitchVariation * 0.5f;
                currentPitchRatio = basePitchRatio * std::pow(2.0, tempPitchVariation / 12.0);
            }
        }
    }
}

float SamplerVoice::processLoFi(float sample)
{
    if (bitCrushAmount > 0.0f)
    {
        float steps = juce::jmap(bitCrushAmount, 0.0f, 1.0f, 512.0f, 8.0f);
        sample = std::round(sample * steps) / steps;
    }

    if (sampleRateReduction < 1.0f)
    {
        static int counter = 0;
        static float holdSample = 0.0f;

        float reductionFactor = juce::jmap(sampleRateReduction, 0.0f, 1.0f, 32.0f, 1.0f);
        if (counter % static_cast<int>(reductionFactor) == 0)
            holdSample = sample;

        sample = holdSample;
        counter++;
    }

    return sample;
}

float SamplerVoice::generateNoise()
{
    return random.nextFloat() * 2.0f - 1.0f;
}