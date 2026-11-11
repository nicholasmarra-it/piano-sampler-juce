#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cstdint>
#include <utility>

// LFO Implementation
void LFO::setSampleRate(double newSampleRate)
{
    sampleRate = newSampleRate;
    updatePhaseDelta();
}

void LFO::setFrequency(float newFrequency)
{
    frequency = newFrequency;
    updatePhaseDelta();
}

void LFO::setWaveform(LFOWaveform newWaveform)
{
    waveform = newWaveform;
}

float LFO::getNextValue()
{
    phase += phaseDelta;
    if (phase >= 1.0f)
    {
        phase -= 1.0f;
        if (waveform == LFOWaveform::Random)
        {
            lastRandomValue = random.nextFloat() * 2.0f - 1.0f;
        }
    }

    switch (waveform)
    {
    case LFOWaveform::Sine:
        return std::sin(phase * 2.0f * juce::MathConstants<float>::pi);

    case LFOWaveform::Square:
        return phase < 0.5f ? 1.0f : -1.0f;

    case LFOWaveform::Triangle:
        if (phase < 0.25f)
            return phase * 4.0f;
        else if (phase < 0.75f)
            return 1.0f - (phase - 0.25f) * 4.0f;
        else
            return (phase - 0.75f) * 4.0f - 1.0f;

    case LFOWaveform::Sawtooth:
        return 2.0f * phase - 1.0f;

    case LFOWaveform::Random:
        return lastRandomValue;

    default:
        return 0.0f;
    }
}

float LFO::getCurrentValue() const
{
    switch (waveform)
    {
    case LFOWaveform::Sine:
        return std::sin(phase * 2.0f * juce::MathConstants<float>::pi);

    case LFOWaveform::Square:
        return phase < 0.5f ? 1.0f : -1.0f;

    case LFOWaveform::Triangle:
        if (phase < 0.25f)
            return phase * 4.0f;
        else if (phase < 0.75f)
            return 1.0f - (phase - 0.25f) * 4.0f;
        else
            return (phase - 0.75f) * 4.0f - 1.0f;

    case LFOWaveform::Sawtooth:
        return 2.0f * phase - 1.0f;

    case LFOWaveform::Random:
        return lastRandomValue;

    default:
        return 0.0f;
    }
}

void LFO::reset()
{
    phase = 0.0f;
    lastRandomValue = 0.0f;
}

void LFO::updatePhaseDelta()
{
    phaseDelta = frequency / static_cast<float>(sampleRate);
}

// ProgettoPlugInAudioProcessor Implementation
ProgettoPlugInAudioProcessor::ProgettoPlugInAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    parameters(*this, nullptr, juce::Identifier("PianoSynth"),
        {
            std::make_unique<juce::AudioParameterFloat>("cutoff_frequency", "Cutoff Frequency", juce::NormalisableRange<float>(20.0f, 20000.0f, 0.1f, 0.2f), 1000.0f),
            std::make_unique<juce::AudioParameterFloat>("resonance", "Resonance", juce::NormalisableRange<float>(0.1f, 10.0f, 0.1f), 1.0f),
            std::make_unique<juce::AudioParameterChoice>("filter_mode", "Filter Mode", juce::StringArray{ "Low-Pass", "High-Pass", "Band-Pass", "Bypass" }, 0),
            std::make_unique<juce::AudioParameterBool>("hold", "Hold", false),
            std::make_unique<juce::AudioParameterFloat>("attack", "Attack", juce::NormalisableRange<float>(0.001f, 5.0f, 0.001f, 0.5f), 0.002f),
            std::make_unique<juce::AudioParameterFloat>("decay", "Decay", juce::NormalisableRange<float>(0.001f, 5.0f, 0.001f, 0.5f), 0.01f),
            std::make_unique<juce::AudioParameterFloat>("sustain", "Sustain", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f),
            std::make_unique<juce::AudioParameterFloat>("release", "Release", juce::NormalisableRange<float>(0.001f, 5.0f, 0.001f, 0.5f), 0.08f),
            std::make_unique<juce::AudioParameterFloat>("filter_attack", "Filter Attack", juce::NormalisableRange<float>(0.001f, 5.0f, 0.001f, 0.5f), 0.002f),
            std::make_unique<juce::AudioParameterFloat>("filter_decay", "Filter Decay", juce::NormalisableRange<float>(0.001f, 5.0f, 0.001f, 0.5f), 0.01f),
            std::make_unique<juce::AudioParameterFloat>("filter_sustain", "Filter Sustain", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f),
            std::make_unique<juce::AudioParameterFloat>("filter_release", "Filter Release", juce::NormalisableRange<float>(0.001f, 5.0f, 0.001f, 0.5f), 0.08f),
            std::make_unique<juce::AudioParameterFloat>("filter_envelope_amount", "Filter Envelope Amount", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f),
            std::make_unique<juce::AudioParameterFloat>("lfo_frequency", "LFO Frequency", juce::NormalisableRange<float>(0.1f, 20.0f, 0.1f), 1.0f),
            std::make_unique<juce::AudioParameterChoice>("lfo_waveform", "LFO Waveform", juce::StringArray{ "Sine", "Square", "Triangle", "Sawtooth", "Random" }, 0),
            std::make_unique<juce::AudioParameterFloat>("lfo_filter_amount", "LFO Filter Amount", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f),
            std::make_unique<juce::AudioParameterFloat>("lfo_volume_amount", "LFO Volume Amount", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f),
            std::make_unique<juce::AudioParameterFloat>("lfo_pitch_amount", "LFO Pitch Amount", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f),
            std::make_unique<juce::AudioParameterFloat>("saturation_drive", "Saturation Drive", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f),
            std::make_unique<juce::AudioParameterFloat>("saturation_mix", "Saturation Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f),
            std::make_unique<juce::AudioParameterBool>("reverse_playback", "Reverse Playback", false),
            std::make_unique<juce::AudioParameterBool>("loop_enabled", "Loop Enabled", false),
            std::make_unique<juce::AudioParameterFloat>("loop_start", "Loop Start", juce::NormalisableRange<float>(0.0f, 600.0f, 0.1f), 0.0f),
            std::make_unique<juce::AudioParameterFloat>("loop_end", "Loop End", juce::NormalisableRange<float>(0.0f, 600.0f, 0.1f), 0.0f),
            std::make_unique<juce::AudioParameterBool>("lofi_enabled", "Lo-Fi Enabled", false),
            std::make_unique<juce::AudioParameterFloat>("noise_mix", "Noise Mix", 0.0f, 1.0f, 0.0f),
            std::make_unique<juce::AudioParameterFloat>("bit_crush", "Bit Crush", 0.0f, 1.0f, 0.0f),
            std::make_unique<juce::AudioParameterFloat>("sample_rate", "Sample Rate", 0.1f, 1.0f, 1.0f),
            std::make_unique<juce::AudioParameterFloat>("random_playback", "Random Playback", 0.0f, 1.0f, 0.0f),
            std::make_unique<juce::AudioParameterFloat>("pitch_variation", "Pitch Variation", 0.0f, 0.5f, 0.0f),
            std::make_unique<juce::AudioParameterBool>("random_playback_enabled", "Random Playback Enabled", false),
            std::make_unique<juce::AudioParameterBool>("creative_enabled", "Creative Enabled", false),
            std::make_unique<juce::AudioParameterFloat>("stereo_width", "Stereo Width",juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f), 1.0f),
            std::make_unique<juce::AudioParameterFloat>("autopan_rate", "AutoPan Rate",juce::NormalisableRange<float>(0.05f, 10.0f, 0.01f), 1.0f),
            std::make_unique<juce::AudioParameterFloat>("autopan_depth", "AutoPan Depth",juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f),
            std::make_unique<juce::AudioParameterFloat>("stutter_rate", "Stutter Rate",juce::NormalisableRange<float>(0.25f, 32.0f, 0.01f), 4.0f),
            std::make_unique<juce::AudioParameterFloat>("stutter_depth", "Stutter Depth",juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f)
        })
{
    // Initialize parameter references
    cutoffFrequencyParameter = parameters.getRawParameterValue("cutoff_frequency");
    resonanceParameter = parameters.getRawParameterValue("resonance");
    filterModeParameter = parameters.getRawParameterValue("filter_mode");
    holdParameter = parameters.getRawParameterValue("hold");

    attackParameter = parameters.getRawParameterValue("attack");
    decayParameter = parameters.getRawParameterValue("decay");
    sustainParameter = parameters.getRawParameterValue("sustain");
    releaseParameter = parameters.getRawParameterValue("release");
    filterAttackParameter = parameters.getRawParameterValue("filter_attack");
    filterDecayParameter = parameters.getRawParameterValue("filter_decay");
    filterSustainParameter = parameters.getRawParameterValue("filter_sustain");
    filterReleaseParameter = parameters.getRawParameterValue("filter_release");
    filterEnvelopeAmountParameter = parameters.getRawParameterValue("filter_envelope_amount");

    lfoFrequencyParameter = parameters.getRawParameterValue("lfo_frequency");
    lfoWaveformParameter = parameters.getRawParameterValue("lfo_waveform");
    lfoFilterAmountParameter = parameters.getRawParameterValue("lfo_filter_amount");
    lfoVolumeAmountParameter = parameters.getRawParameterValue("lfo_volume_amount");
    lfoPitchAmountParameter = parameters.getRawParameterValue("lfo_pitch_amount");

    saturationDriveParameter = parameters.getRawParameterValue("saturation_drive");
    saturationMixParameter = parameters.getRawParameterValue("saturation_mix");

    reversePlaybackParameter = parameters.getRawParameterValue("reverse_playback");
    loopEnabledParameter = parameters.getRawParameterValue("loop_enabled");
    loopStartParameter = parameters.getRawParameterValue("loop_start");
    loopEndParameter = parameters.getRawParameterValue("loop_end");

    lofiEnabledParameter = parameters.getRawParameterValue("lofi_enabled");
    noiseMixParameter = parameters.getRawParameterValue("noise_mix");
    bitCrushParameter = parameters.getRawParameterValue("bit_crush");
    sampleRateReductionParameter = parameters.getRawParameterValue("sample_rate");
    randomPlaybackParameter = parameters.getRawParameterValue("random_playback");
    pitchVariationParameter = parameters.getRawParameterValue("pitch_variation");
    randomPlaybackEnabledParameter = parameters.getRawParameterValue("random_playback_enabled");

    // Creative FX refs
    creativeEnabledParameter = parameters.getRawParameterValue("creative_enabled");
    stereoWidthParameter = parameters.getRawParameterValue("stereo_width");
    autopanRateParameter = parameters.getRawParameterValue("autopan_rate");
    autopanDepthParameter = parameters.getRawParameterValue("autopan_depth");
    stutterRateParameter = parameters.getRawParameterValue("stutter_rate");
    stutterDepthParameter = parameters.getRawParameterValue("stutter_depth");

    rebuildVoices(16);
    formatManager.registerBasicFormats();

    // Inizializza buffer overdub
    overdubBuffer.setSize(2, 44100 * 10); // 10 secondi
}

ProgettoPlugInAudioProcessor::~ProgettoPlugInAudioProcessor() {
    transportSource.setSource(nullptr);
    readerSource.reset();
}

void ProgettoPlugInAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
    filter.setSamplingRate(static_cast<float>(sampleRate));
    setPlayMode(playMode);

    transportSource.prepareToPlay(samplesPerBlock, sampleRate);

    int bufferSize = static_cast<int>(sampleRate * 10.0);
    loopBuffer.setSize(2, bufferSize);
    loopBuffer.clear();
    overdubBuffer.setSize(2, bufferSize);
    overdubBuffer.clear();

    lfo.setSampleRate(sampleRate);

    autoPanPhase = 0.0;
    stutterPhase = 0.0;
}

void ProgettoPlugInAudioProcessor::releaseResources()
{
    transportSource.releaseResources();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ProgettoPlugInAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo()
        || layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}
#endif

// FUNZIONI EFFETTI

void ProgettoPlugInAudioProcessor::updateVoiceEffects()
{
    bool lofiEnabled = *lofiEnabledParameter > 0.5f;

    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<SamplerVoice*>(synth.getVoice(i)))
        {
            // Imposta lo-fi enabled SOLO se il toggle e' attivo
            voice->setLoFiEnabled(lofiEnabled);

            // Passa i parametri lo-fi SOLO se abilitato, altrimenti imposta a 0
            if (lofiEnabled)
            {
                voice->setNoiseMix(*noiseMixParameter);
                voice->setBitCrush(*bitCrushParameter);
                voice->setSampleRateReduction(*sampleRateReductionParameter);
            }
            else
            {
                // Disabilita tutti gli effetti lo-fi quando il toggle e' spento
                voice->setNoiseMix(0.0f);
                voice->setBitCrush(0.0f);
                voice->setSampleRateReduction(1.0f); // Sample rate normale
            }

            // parametri random playback 
            voice->setPitchVariation(*pitchVariationParameter);
            voice->setRandomPlaybackEnabled(*randomPlaybackEnabledParameter > 0.5f);
            voice->setRandomPlayback(*randomPlaybackParameter);
        }
    }
}

void ProgettoPlugInAudioProcessor::applyLoFiEffects(juce::AudioBuffer<float>& buffer)
{
    float noiseMix = *noiseMixParameter;
    float bitCrush = *bitCrushParameter;
    float sampleRateReduction = *sampleRateReductionParameter;

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // Aggiungi rumore
            if (noiseMix > 0.0f)
            {
                float noise = effectRandom.nextFloat() * 2.0f - 1.0f;
                channelData[sample] = (1.0f - noiseMix) * channelData[sample] + noiseMix * noise;
            }

            // Bit crushing
            if (bitCrush > 0.0f)
            {
                float steps = juce::jmap(bitCrush, 0.0f, 1.0f, 256.0f, 8.0f);
                channelData[sample] = std::round(channelData[sample] * steps) / steps;
            }

            // Sample rate reduction
            if (sampleRateReduction < 1.0f)
            {
                float reductionFactor = juce::jmap(sampleRateReduction, 0.0f, 1.0f, 32.0f, 1.0f);
                if (sampleCounter % static_cast<int>(reductionFactor) == 0)
                {
                    heldSample[channel] = channelData[sample];
                }
                channelData[sample] = heldSample[channel];
            }
        }
    }

    sampleCounter += buffer.getNumSamples();
}

void ProgettoPlugInAudioProcessor::applySaturationToBuffer(juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    float drive = *saturationDriveParameter;
    float mix = *saturationMixParameter;

    if (drive > 0.0f || mix > 0.0f)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            for (int sample = startSample; sample < startSample + numSamples; ++sample)
            {
                channelData[sample] = saturateSample(channelData[sample], drive, mix);
            }
        }
    }
}

void ProgettoPlugInAudioProcessor::applyCreativeEffects(juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    if (*creativeEnabledParameter > 0.5f)
    {
        const float width = juce::jlimit(0.0f, 2.0f, stereoWidthParameter->load());
        const float apRate = autopanRateParameter->load();
        const float apAmt = juce::jlimit(0.0f, 1.0f, autopanDepthParameter->load());
        const float stRate = stutterRateParameter->load();
        const float stAmt = juce::jlimit(0.0f, 1.0f, stutterDepthParameter->load());

        const double sr = getSampleRate();

        for (int sample = startSample; sample < startSample + numSamples; ++sample)
        {
            // --- AutoPan (sine LFO in controfase sui canali)
            const float pan = (apAmt > 0.0f) ? std::sin((float)autoPanPhase) * apAmt : 0.0f;
            // L = (1 - pan*0.5), R = (1 + pan*0.5) per lieve movimento
            const float gL = 1.0f - (pan * 0.5f);
            const float gR = 1.0f + (pan * 0.5f);

            // --- Stutter/Gate (square LFO)
            float gate = 1.0f;
            if (stAmt > 0.0f)
                gate = (std::sin((float)stutterPhase) >= 0.0f ? 1.0f : 0.0f) * stAmt + (1.0f - stAmt);

            // Leggi, applica AutoPan + Gate
            float L = buffer.getSample(0, sample) * gL * gate;
            float R = (buffer.getNumChannels() > 1 ? buffer.getSample(1, sample) : L) * gR * gate;

            // --- Stereo Width (Mid/Side)
            float M = 0.5f * (L + R);
            float S = 0.5f * (L - R);
            S *= width; // 0=mono, 1=normale, 2=larghezza raddoppiata

            L = M + S;
            R = M - S;

            buffer.setSample(0, sample, L);
            if (buffer.getNumChannels() > 1)
                buffer.setSample(1, sample, R);

            // Avanza le fasi
            advancePhase(autoPanPhase, apRate, sr);
            advancePhase(stutterPhase, stRate, sr);
        }
    }
}

void ProgettoPlugInAudioProcessor::applyRandomEffects(juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    if (*randomPlaybackEnabledParameter > 0.5f && *randomPlaybackParameter > 0.0f)
    {
        float probability = *randomPlaybackParameter * 0.5f;

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);

            for (int sample = startSample; sample < startSample + numSamples; ++sample)
            {
                if (effectRandom.nextFloat() < probability)
                {
                    // Random playback - salta a una posizione casuale nel buffer
                    int randomOffset = effectRandom.nextInt(numSamples);
                    int targetSample = juce::jmin(startSample + randomOffset, startSample + numSamples - 1);
                    channelData[sample] = channelData[targetSample];
                }

                // Pitch variation applicata come modulazione casuale del volume
                if (*pitchVariationParameter > 0.0f)
                {
                    float variation = 1.0f + (effectRandom.nextFloat() * 2.0f - 1.0f) * *pitchVariationParameter;
                    channelData[sample] *= variation;
                }
            }
        }
    }
}

// PROCESS BLOCK PRINCIPALE

void ProgettoPlugInAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    updateAdsrParameters();
    updateFilterAdsrParameters();
    updateVoiceEffects();

    lfo.setFrequency(*lfoFrequencyParameter);
    int waveformIndex = static_cast<int>(*lfoWaveformParameter);
    lfo.setWaveform(static_cast<LFOWaveform>(waveformIndex));

    // Ottieni il valore LFO per il primo campione per pitch e filtro
    float lfoValueForPitchAndFilter = lfo.getCurrentValue();

    //  modulazione del pitch 
    float pitchMod = *lfoPitchAmountParameter;
    if (pitchMod > 0.0f)
    {
        float pitchModulation = lfoValueForPitchAndFilter * pitchMod * 2.0f;
        for (int i = 0; i < synth.getNumVoices(); ++i)
        {
            if (auto* voice = dynamic_cast<SamplerVoice*>(synth.getVoice(i)))
            {
                voice->setPitchModulation(pitchModulation);
            }
        }
    }

    reversePlayback = *reversePlaybackParameter > 0.5f;
    loopEnabled = *loopEnabledParameter > 0.5f;
    loopStart = *loopStartParameter;
    loopEnd = *loopEndParameter;

    bool currentHoldState = *holdParameter > 0.5f;
    if (currentHoldState != holdEnabled) {
        holdEnabled = currentHoldState;
        if (!holdEnabled) {
            for (int note : heldNotes) {
                synth.noteOff(1, note, 0.0f, true);
            }
            heldNotes.clear();
        }
    }

    buffer.clear();

    keyboardState.processNextMidiBuffer(midi, 0, buffer.getNumSamples(), true);

    // Processa synth 
    if (holdEnabled) {
        juce::MidiBuffer heldMidi;

        for (const auto metadata : midi) {
            auto message = metadata.getMessage();

            if (message.isNoteOn()) {
                int note = message.getNoteNumber();
                if (!heldNotes.contains(note)) {
                    heldNotes.add(note);
                }
                heldMidi.addEvent(message, metadata.samplePosition);
            }
            else if (!message.isNoteOff()) {
                heldMidi.addEvent(message, metadata.samplePosition);
            }
        }

        synth.renderNextBlock(buffer, heldMidi, 0, buffer.getNumSamples());
    }
    else {
        synth.renderNextBlock(buffer, midi, 0, buffer.getNumSamples());
    }

    // Processa loop recording e playback
    processLoopRecording(buffer);
    processLoopPlayback(buffer);

    // Processa LFO volume modulation per campione
    float volumeMod = *lfoVolumeAmountParameter;
    if (volumeMod > 0.0f)
    {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float lfoValue = lfo.getNextValue(); // Ottiene il valore per questo campione
            float volumeFactor = 1.0f + (lfoValue * volumeMod * 0.5f);
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                float* channelData = buffer.getWritePointer(channel);
                channelData[sample] *= volumeFactor;
            }
        }
    }
    else
    {
        // Se non c'e' modulazione del volume, avanziamo comunque l'LFO
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            lfo.getNextValue();
    }

    // Processa audio player
    if (audioPlayerPlaying && readerSource != nullptr) {
        juce::AudioBuffer<float> tempBuffer(buffer.getNumChannels(), buffer.getNumSamples());
        tempBuffer.clear();

        if (reversePlayback) {
            for (int channel = 0; channel < tempBuffer.getNumChannels(); ++channel) {
                auto* channelData = tempBuffer.getWritePointer(channel);
                for (int sample = 0; sample < tempBuffer.getNumSamples(); ++sample) {
                    if (reversePosition >= 0 && reversePosition < reverseBuffer.getNumSamples()) {
                        channelData[sample] = reverseBuffer.getSample(channel, reversePosition);
                        reversePosition--;
                    }
                    else {
                        if (loopEnabled) {
                            reversePosition = reverseBuffer.getNumSamples() - 1;
                            if (reversePosition >= 0) {
                                channelData[sample] = reverseBuffer.getSample(channel, reversePosition);
                                reversePosition--;
                            }
                        }
                    }
                }
            }
        }
        else {
            juce::AudioSourceChannelInfo info(tempBuffer);

            if (loopEnabled && loopEnd > loopStart) {
                double currentTime = transportSource.getCurrentPosition();
                if (currentTime >= loopEnd) {
                    transportSource.setPosition(loopStart);
                }
            }

            transportSource.getNextAudioBlock(info);
        }

        // Applica effetti all'audio del loop
        if (*lofiEnabledParameter > 0.5f) {
            applyLoFiEffects(tempBuffer);
        }

        applySaturationToBuffer(tempBuffer, 0, tempBuffer.getNumSamples());
        applyRandomEffects(tempBuffer, 0, tempBuffer.getNumSamples());
        applyCreativeEffects(tempBuffer, 0, tempBuffer.getNumSamples());

        // Aggiungi l'audio processato al buffer principale
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            buffer.addFrom(channel, 0, tempBuffer, channel, 0, tempBuffer.getNumSamples());
        }
    }

    // Processa filtro sull'intero mix
    const auto cutoffFrequency = cutoffFrequencyParameter->load();
    const auto resonance = resonanceParameter->load();
    const auto filterMode = static_cast<int>(filterModeParameter->load());
    const auto filterEnvelopeAmount = filterEnvelopeAmountParameter->load();

    // True filter ADSR modulation
    float filterEnvModulation = 1.0f;
    if (*filterEnvelopeAmountParameter > 0.0f)
    {
        float maxEnvValue = 0.0f;
        for (int i = 0; i < synth.getNumVoices(); ++i)
        {
            if (auto* voice = dynamic_cast<SamplerVoice*>(synth.getVoice(i)))
            {
                if (voice->isActive())
                {
                    float envValue = voice->getFilterAdsrValue();
                    maxEnvValue = juce::jmax(maxEnvValue, envValue);
                }
            }
        }
        filterEnvModulation = 1.0f + (maxEnvValue * *filterEnvelopeAmountParameter * 10.0f);
    }

    // Usa lfoValueForPitchAndFilter per la modulazione del filtro
    float filterMod = *lfoFilterAmountParameter;
    float lfoFilterModulation = 1.0f + (lfoValueForPitchAndFilter * filterMod);

    filter.setResonance(*resonanceParameter);
    filter.setMode(static_cast<FilterMode>(static_cast<int>(*filterModeParameter)));

    // Calcola la modulazione combinata
    float baseCutoff = *cutoffFrequencyParameter;
    float modulatedCutoff = baseCutoff * filterEnvModulation;
    modulatedCutoff = juce::jlimit(20.0f, 20000.0f, modulatedCutoff);

    // Applica il cutoff modulato al filtro
    filter.setCutoffFrequency(modulatedCutoff);

    // Processa il blocco con il filtro
    filter.processBlock(buffer, midi);

    // Processa saturazione sul mix finale
    applySaturationToBuffer(buffer, 0, buffer.getNumSamples());

    // CREATIVE FX sul mix finale
    applyCreativeEffects(buffer, 0, buffer.getNumSamples());
}

// FUNZIONI AUSILIARIE

void ProgettoPlugInAudioProcessor::rebuildVoices(int numVoices)
{
    synth.clearVoices();

    for (int i = 0; i < numVoices; ++i)
        synth.addVoice(new SamplerVoice());

    const bool legato = (playMode == PlayMode::MonoLegato);
    const bool retrig = (playMode == PlayMode::MonoRetrigger);

    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* v = dynamic_cast<SamplerVoice*>(synth.getVoice(i)))
            v->setMode(legato, retrig);
    }
}

void ProgettoPlugInAudioProcessor::setPlayMode(PlayMode m)
{
    playMode = m;
    const int numVoices = (playMode == PlayMode::Poly ? 16 : 1);
    rebuildVoices(numVoices);
}

void ProgettoPlugInAudioProcessor::processLoopRecording(juce::AudioBuffer<float>& buffer)
{
    const juce::ScopedLock lock(loopLock);

    if (loopState == LoopState::Recording || loopState == LoopState::Overdubbing)
    {
        int numSamples = buffer.getNumSamples();
        int bufferSize = loopBuffer.getNumSamples();

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            const float* sourceData = buffer.getReadPointer(channel);
            float* destData = loopBuffer.getWritePointer(channel);
            float* overdubData = overdubBuffer.getWritePointer(channel);

            for (int i = 0; i < numSamples; ++i)
            {
                if (loopRecordingPosition < bufferSize)
                {
                    if (loopState == LoopState::Recording)
                    {
                        // Recording normale - sovrascrive
                        destData[loopRecordingPosition] = sourceData[i];
                        overdubData[loopRecordingPosition] = 0.0f; // Reset overdub buffer
                    }
                    else if (loopState == LoopState::Overdubbing)
                    {
                        // Overdub - aggiunge al contenuto esistente
                        overdubData[loopRecordingPosition] += sourceData[i];
                        destData[loopRecordingPosition] += sourceData[i];
                    }

                    loopRecordingPosition++;
                }
                else
                {
                    // Loop completo, torna all'inizio
                    loopRecordingPosition = 0;

                    if (loopState == LoopState::Recording)
                    {
                        loopState = LoopState::Playing;
                        loopLength = static_cast<double>(bufferSize) / getSampleRate();
                    }
                    break;
                }
            }
        }

        if (loopState == LoopState::Recording && isFirstRecording)
        {
            loopLength = static_cast<double>(loopRecordingPosition) / getSampleRate();
            isFirstRecording = false;
        }
    }
}

void ProgettoPlugInAudioProcessor::processLoopPlayback(juce::AudioBuffer<float>& buffer)
{
    const juce::ScopedLock lock(loopLock);

    if (loopState == LoopState::Playing || loopState == LoopState::Overdubbing)
    {
        int numSamples = buffer.getNumSamples();
        int bufferSize = loopBuffer.getNumSamples();

        if (loopLength > 0.0)
        {
            int totalLoopSamples = static_cast<int>(loopLength * getSampleRate());

            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                const float* sourceData = loopBuffer.getReadPointer(channel);
                float* destData = buffer.getWritePointer(channel);

                for (int i = 0; i < numSamples; ++i)
                {
                    if (loopPlaybackPosition < totalLoopSamples)
                    {
                        float sampleValue = sourceData[loopPlaybackPosition];
                        destData[i] += sampleValue;
                        loopPlaybackPosition++;
                    }
                    else
                    {
                        loopPlaybackPosition = 0;
                        if (loopPlaybackPosition < totalLoopSamples)
                        {
                            float sampleValue = sourceData[loopPlaybackPosition];
                            destData[i] += sampleValue;
                            loopPlaybackPosition++;
                        }
                    }
                }
            }

            loopPosition = static_cast<double>(loopPlaybackPosition) / getSampleRate();
        }
    }
}

// Loop recording functions
void ProgettoPlugInAudioProcessor::startLoopRecording()
{
    const juce::ScopedLock lock(loopLock);

    if (loopState == LoopState::Stopped)
    {
        loopState = LoopState::Recording;
        loopRecordingPosition = 0;
        loopPlaybackPosition = 0;
        isFirstRecording = true;
        loopBuffer.clear();
        overdubBuffer.clear();
    }
    else if (loopState == LoopState::Playing)
    {
        loopState = LoopState::Overdubbing;
        loopRecordingPosition = loopPlaybackPosition; // Sincronizza con la posizione di playback
        isFirstRecording = false;
    }
}

void ProgettoPlugInAudioProcessor::stopLoopRecording()
{
    const juce::ScopedLock lock(loopLock);

    if (loopState == LoopState::Recording)
    {
        loopState = LoopState::Playing;
        loopLength = static_cast<double>(loopRecordingPosition) / getSampleRate();
    }
}

void ProgettoPlugInAudioProcessor::startOverdub()
{
    const juce::ScopedLock lock(loopLock);

    if (loopState == LoopState::Playing && loopLength > 0.0)
    {
        loopState = LoopState::Overdubbing;
        loopRecordingPosition = loopPlaybackPosition; // Sincronizza con la posizione corrente
        isFirstRecording = false;
    }
}

void ProgettoPlugInAudioProcessor::stopOverdub()
{
    const juce::ScopedLock lock(loopLock);

    if (loopState == LoopState::Overdubbing)
    {
        loopState = LoopState::Playing;
    }
}

void ProgettoPlugInAudioProcessor::clearLoop()
{
    const juce::ScopedLock lock(loopLock);

    loopState = LoopState::Stopped;
    loopBuffer.clear();
    overdubBuffer.clear();
    loopRecordingPosition = 0;
    loopPlaybackPosition = 0;
    loopLength = 0.0;
    loopPosition = 0.0;
    isFirstRecording = true;
}

void ProgettoPlugInAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ProgettoPlugInAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr) {
        if (xmlState->hasTagName(parameters.state.getType())) {
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));

            holdEnabled = *holdParameter > 0.5f;
            if (!holdEnabled) {
                heldNotes.clear();
            }
        }
    }
}

bool ProgettoPlugInAudioProcessor::loadSampleFromFile(const juce::File& file,
    int rootNote,
    int minVelocity, int maxVelocity,
    int coarseSemitones, int fineCents,
    bool oneShot)
{
    if (!file.existsAsFile())
        return false;

    juce::AudioFormatManager fm;
    fm.registerBasicFormats();

    std::unique_ptr<juce::AudioFormatReader> reader(fm.createReaderFor(file));
    if (!reader)
        return false;

    const int64_t numSamples = (int64_t)reader->lengthInSamples;
    const int numCh = (int)reader->numChannels;
    if (numSamples <= 0)
        return false;

    juce::AudioBuffer<float> buffer(juce::jmax(1, numCh), (int)numSamples);
    reader->read(&buffer, 0, (int)numSamples, 0, true, true);

    juce::BigInteger midiNotes;
    midiNotes.setRange(0, 128, true);

    auto* sound = new SamplerSound(file.getFileNameWithoutExtension(),
        std::move(buffer),
        midiNotes,
        rootNote,
        reader->sampleRate);

    sound->minVelocity = juce::jlimit(1, 127, minVelocity);
    sound->maxVelocity = juce::jlimit(1, 127, maxVelocity);
    if (sound->minVelocity > sound->maxVelocity)
        std::swap(sound->minVelocity, sound->maxVelocity);

    sound->coarseSemitones = coarseSemitones;
    sound->fineCents = fineCents;
    sound->oneShot = oneShot;

    synth.addSound(sound);

    thumbnail.setSource(new juce::FileInputSource(file));
    loadedFileName = file.getFileName();

    return true;
}

bool ProgettoPlugInAudioProcessor::loadLoopSample(const juce::File& file)
{
    if (!file.existsAsFile())
        return false;

    setLoopPlaying(false);
    transportSource.setSource(nullptr);
    readerSource.reset();

    auto* reader = formatManager.createReaderFor(file);
    if (reader != nullptr)
    {
        thumbnail.setSource(new juce::FileInputSource(file));
        loadedFileName = file.getFileName();

        readerSource.reset(new juce::AudioFormatReaderSource(reader, true));

        transportSource.setSource(readerSource.get(),
            0,
            nullptr,
            reader->sampleRate);

        int numSamples = (int)reader->lengthInSamples;
        int numChannels = (int)reader->numChannels;
        reverseBuffer.setSize(numChannels, numSamples);
        reader->read(&reverseBuffer, 0, numSamples, 0, true, true);
        reversePosition = numSamples - 1;

        double duration = transportSource.getLengthInSeconds();
        loopStart = 0.0;
        loopEnd = duration;
        currentLoopPosition = 0.0;

        loadedFileName = file.getFileName();
        return true;
    }

    return false;
}

void ProgettoPlugInAudioProcessor::setLoopPlaying(bool playing)
{
    if (playing) {
        if (readerSource != nullptr && !audioPlayerPlaying) {
            transportSource.start();
            audioPlayerPlaying = true;
        }
    }
    else {
        if (audioPlayerPlaying) {
            transportSource.stop();
            audioPlayerPlaying = false;
        }
    }
}

void ProgettoPlugInAudioProcessor::setReversePlayback(bool reverse)
{
    reversePlayback = reverse;
    if (reversePlaybackParameter != nullptr) {
        parameters.getParameter("reverse_playback")->setValueNotifyingHost(reverse ? 1.0f : 0.0f);
    }
}

void ProgettoPlugInAudioProcessor::setLoopStart(double start)
{
    loopStart = juce::jlimit(0.0, transportSource.getLengthInSeconds(), start);
    if (loopStartParameter != nullptr) {
        parameters.getParameter("loop_start")->setValueNotifyingHost(
            (float)loopStart / (float)transportSource.getLengthInSeconds());
    }
}

void ProgettoPlugInAudioProcessor::setLoopEnd(double end)
{
    loopEnd = juce::jlimit(0.0, transportSource.getLengthInSeconds(), end);
    if (loopEndParameter != nullptr) {
        parameters.getParameter("loop_end")->setValueNotifyingHost(
            (float)loopEnd / (float)transportSource.getLengthInSeconds());
    }
}

void ProgettoPlugInAudioProcessor::setLoopEnabled(bool enabled)
{
    loopEnabled = enabled;
    if (loopEnabled) {
        transportSource.setPosition(loopStart);
    }
}

void ProgettoPlugInAudioProcessor::resetAllParameters()
{
    parameters.getParameter("cutoff_frequency")->setValueNotifyingHost(0.5f);
    parameters.getParameter("resonance")->setValueNotifyingHost(0.0f);
    parameters.getParameter("filter_mode")->setValueNotifyingHost(0.0f);
    parameters.getParameter("hold")->setValueNotifyingHost(0.0f);

    parameters.getParameter("attack")->setValueNotifyingHost(0.0f);
    parameters.getParameter("decay")->setValueNotifyingHost(0.0f);
    parameters.getParameter("sustain")->setValueNotifyingHost(1.0f);
    parameters.getParameter("release")->setValueNotifyingHost(0.0f);
    parameters.getParameter("filter_attack")->setValueNotifyingHost(0.0f);
    parameters.getParameter("filter_decay")->setValueNotifyingHost(0.0f);
    parameters.getParameter("filter_sustain")->setValueNotifyingHost(1.0f);
    parameters.getParameter("filter_release")->setValueNotifyingHost(0.0f);
    parameters.getParameter("filter_envelope_amount")->setValueNotifyingHost(0.0f);

    parameters.getParameter("lfo_frequency")->setValueNotifyingHost(0.1f);
    parameters.getParameter("lfo_waveform")->setValueNotifyingHost(0.0f);
    parameters.getParameter("lfo_filter_amount")->setValueNotifyingHost(0.0f);
    parameters.getParameter("lfo_volume_amount")->setValueNotifyingHost(0.0f);
    parameters.getParameter("lfo_pitch_amount")->setValueNotifyingHost(0.0f);

    parameters.getParameter("saturation_drive")->setValueNotifyingHost(0.0f);
    parameters.getParameter("saturation_mix")->setValueNotifyingHost(0.0f);

    parameters.getParameter("reverse_playback")->setValueNotifyingHost(0.0f);
    parameters.getParameter("loop_enabled")->setValueNotifyingHost(0.0f);
    parameters.getParameter("loop_start")->setValueNotifyingHost(0.0f);
    parameters.getParameter("loop_end")->setValueNotifyingHost(0.0f);

    parameters.getParameter("creative_enabled")->setValueNotifyingHost(0.0f);
    parameters.getParameter("stereo_width")->setValueNotifyingHost(1.0f);
    parameters.getParameter("autopan_rate")->setValueNotifyingHost(1.0f);
    parameters.getParameter("autopan_depth")->setValueNotifyingHost(0.0f);
    parameters.getParameter("stutter_rate")->setValueNotifyingHost(4.0f);
    parameters.getParameter("stutter_depth")->setValueNotifyingHost(0.0f);

    // Reset parametri effetti
    parameters.getParameter("lofi_enabled")->setValueNotifyingHost(0.0f);
    parameters.getParameter("noise_mix")->setValueNotifyingHost(0.0f);
    parameters.getParameter("bit_crush")->setValueNotifyingHost(0.0f);
    parameters.getParameter("sample_rate")->setValueNotifyingHost(1.0f);
    parameters.getParameter("random_playback")->setValueNotifyingHost(0.0f);
    parameters.getParameter("pitch_variation")->setValueNotifyingHost(0.0f);
    parameters.getParameter("random_playback_enabled")->setValueNotifyingHost(0.0f);

    setLoopPlaying(false);
    clearLoop();

    holdEnabled = false;
    heldNotes.clear();

    thumbnail.clear();
    loadedFileName.clear();

    setPlayMode(PlayMode::Poly);
}

void ProgettoPlugInAudioProcessor::setAdsrParameters(float attack, float decay, float sustain, float release)
{
    parameters.getParameter("attack")->setValueNotifyingHost(attack);
    parameters.getParameter("decay")->setValueNotifyingHost(decay);
    parameters.getParameter("sustain")->setValueNotifyingHost(sustain);
    parameters.getParameter("release")->setValueNotifyingHost(release);

    updateAdsrParameters();
}

void ProgettoPlugInAudioProcessor::setFilterAdsrParameters(float attack, float decay, float sustain, float release, float envelopeAmount)
{
    parameters.getParameter("filter_attack")->setValueNotifyingHost(attack);
    parameters.getParameter("filter_decay")->setValueNotifyingHost(decay);
    parameters.getParameter("filter_sustain")->setValueNotifyingHost(sustain);
    parameters.getParameter("filter_release")->setValueNotifyingHost(release);
    parameters.getParameter("filter_envelope_amount")->setValueNotifyingHost(envelopeAmount);
}

void ProgettoPlugInAudioProcessor::updateAdsrParameters()
{
    juce::ADSR::Parameters adsrParams;
    adsrParams.attack = *attackParameter;
    adsrParams.decay = *decayParameter;
    adsrParams.sustain = *sustainParameter;
    adsrParams.release = *releaseParameter;

    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto voice = dynamic_cast<SamplerVoice*>(synth.getVoice(i)))
        {
            voice->setAdsrParameters(adsrParams);
        }
    }
}

void ProgettoPlugInAudioProcessor::updateFilterAdsrParameters()
{
    juce::ADSR::Parameters filterAdsrParams;
    filterAdsrParams.attack = *filterAttackParameter;
    filterAdsrParams.decay = *filterDecayParameter;
    filterAdsrParams.sustain = *filterSustainParameter;
    filterAdsrParams.release = *filterReleaseParameter;

    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto voice = dynamic_cast<SamplerVoice*>(synth.getVoice(i)))
        {
            voice->setFilterAdsrParameters(filterAdsrParams);
        }
    }
}

float ProgettoPlugInAudioProcessor::saturateSample(float input, float drive, float mix)
{
    float saturated = std::tanh(input * (1.0f + drive * 10.0f));
    return (1.0f - mix) * input + mix * saturated;
}

juce::AudioProcessorEditor* ProgettoPlugInAudioProcessor::createEditor()
{
    return new ProgettoPlugInAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ProgettoPlugInAudioProcessor();
}

// Preset system implementation
void ProgettoPlugInAudioProcessor::savePreset(const juce::String& presetName)
{
    auto presetDir = getPresetDirectory();
    if (!presetDir.exists())
        presetDir.createDirectory();

    auto presetFile = presetDir.getChildFile(presetName + ".xml");
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    xml->writeTo(presetFile);
}

void ProgettoPlugInAudioProcessor::loadPreset(const juce::String& presetName)
{
    auto presetDir = getPresetDirectory();
    auto presetFile = presetDir.getChildFile(presetName + ".xml");

    if (presetFile.existsAsFile())
    {
        if (auto xml = juce::XmlDocument::parse(presetFile))
        {
            parameters.replaceState(juce::ValueTree::fromXml(*xml));
        }
    }
}

void ProgettoPlugInAudioProcessor::handleKeyPress(int noteNumber)
{
    if (!currentlyPressedNotes.contains(noteNumber))
    {
        currentlyPressedNotes.add(noteNumber);

        // Invia note MIDI al synth
        juce::MidiMessage noteOn = juce::MidiMessage::noteOn(1, noteNumber, 0.8f);
        keyboardState.processNextMidiEvent(noteOn);

        // Trigger sul synth
        synth.noteOn(1, noteNumber, 0.8f);
    }
}

void ProgettoPlugInAudioProcessor::handleKeyRelease(int noteNumber)
{
    currentlyPressedNotes.removeAllInstancesOf(noteNumber);

    // Invia note OFF MIDI
    juce::MidiMessage noteOff = juce::MidiMessage::noteOff(1, noteNumber, 0.0f);
    keyboardState.processNextMidiEvent(noteOff);

    // Trigger note off sul synth
    synth.noteOff(1, noteNumber, 0.0f, true);
}

juce::StringArray ProgettoPlugInAudioProcessor::getPresetList() const
{
    juce::StringArray presets;
    auto presetDir = getPresetDirectory();

    if (presetDir.exists())
    {
        auto presetFiles = presetDir.findChildFiles(juce::File::findFiles, false, "*.xml");
        for (auto& file : presetFiles)
        {
            presets.add(file.getFileNameWithoutExtension());
        }
    }

    return presets;
}

juce::File ProgettoPlugInAudioProcessor::getPresetDirectory() const
{
    return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("PianoSynthPresets");
}