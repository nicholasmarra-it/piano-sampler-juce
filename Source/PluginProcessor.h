#pragma once

#include <JuceHeader.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "SamplerSound.h"
#include "SamplerVoice.h"
#include "LowpassHighpassFilter.h"

// Enums
enum class PlayMode { Poly = 0, MonoLegato = 1, MonoRetrigger = 2 };
enum class LFOWaveform { Sine = 0, Square = 1, Triangle = 2, Sawtooth = 3, Random = 4 };
enum class LoopState { Stopped, Recording, Playing, Overdubbing };

// LFO class
class LFO
{
public:
    LFO() = default;

    void setSampleRate(double newSampleRate);
    void setFrequency(float newFrequency);
    void setWaveform(LFOWaveform newWaveform);
    float getNextValue();
    float getCurrentValue() const;
    void reset();

private:
    void updatePhaseDelta();

    double sampleRate = 44100.0;
    float frequency = 1.0f;
    float phase = 0.0f;
    float phaseDelta = 0.0f;
    LFOWaveform waveform = LFOWaveform::Sine;
    juce::Random random;
    float lastRandomValue = 0.0f;
};

// Main Audio Processor
class ProgettoPlugInAudioProcessor final : public juce::AudioProcessor
{
public:
    ProgettoPlugInAudioProcessor();
    ~ProgettoPlugInAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Sample loading functions
    bool loadSampleFromFile(const juce::File& file,
        int rootNote,
        int minVelocity = 1, int maxVelocity = 127,
        int coarseSemitones = 0, int fineCents = 0,
        bool oneShot = false);

    bool loadLoopSample(const juce::File& file);
    void setLoopPlaying(bool playing);
    void setReversePlayback(bool reverse);
    void setLoopStart(double start);
    void setLoopEnd(double end);
    void setLoopEnabled(bool enabled);

    // Loop recording functions
    void startLoopRecording();
    void stopLoopRecording();
    void startOverdub();
    void stopOverdub();
    void clearLoop();

    // Parameter management
    void resetAllParameters();
    void setAdsrParameters(float attack, float decay, float sustain, float release);
    void setFilterAdsrParameters(float attack, float decay, float sustain, float release, float envelopeAmount);

    // Play mode management
    void setPlayMode(PlayMode m);

    // Preset system
    void savePreset(const juce::String& presetName);
    void loadPreset(const juce::String& presetName);
    juce::StringArray getPresetList() const;

    // Getters
    juce::Synthesiser& getSynth() noexcept { return synth; }
    juce::MidiKeyboardState& getKeyboardState() noexcept { return keyboardState; }
    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters; }
    LowpassHighpassFilter& getFilter() { return filter; }

    bool isHoldEnabled() const { return holdEnabled; }
    bool isLoopPlaying() const { return audioPlayerPlaying; }
    juce::String getLoadedFileName() const { return loadedFileName; }
    bool isReversePlayback() const { return reversePlayback; }
    double getLoopStart() const { return loopStart; }
    double getLoopEnd() const { return loopEnd; }
    bool isLoopEnabled() const { return loopEnabled; }
    LoopState getLoopState() const { return loopState; }
    double getLoopLength() const { return loopLength; }
    double getLoopPosition() const { return loopPosition; }

    // Waveform support
    juce::AudioThumbnail& getThumbnail() { return thumbnail; }

    //PC keybord MIDI support
    void handleKeyPress(int noteNumber);
    void handleKeyRelease(int noteNumber);
    juce::Array<int> getCurrentlyPressedNotes() const { return currentlyPressedNotes; }

private:
    void rebuildVoices(int numVoices);
    void updateAdsrParameters();
    void updateFilterAdsrParameters();
    void updateVoiceEffects();
    float saturateSample(float input, float drive, float mix);
    void processLoopRecording(juce::AudioBuffer<float>& buffer);
    void processLoopPlayback(juce::AudioBuffer<float>& buffer);
    void updateParameters();
    juce::File getPresetDirectory() const;

    // Effects functions
    void applyLoFiEffects(juce::AudioBuffer<float>& buffer);
    void applyCreativeEffects(juce::AudioBuffer<float>& buffer, int startSample, int numSamples);
    void applySaturationToBuffer(juce::AudioBuffer<float>& buffer, int startSample, int numSamples);
    void applyRandomEffects(juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

    juce::Synthesiser synth;
    juce::MidiKeyboardState keyboardState;
    PlayMode playMode{ PlayMode::Poly };

    LowpassHighpassFilter filter;
    LFO lfo;

    juce::AudioProcessorValueTreeState parameters;

    // Parameter references
    std::atomic<float>* cutoffFrequencyParameter = nullptr;
    std::atomic<float>* resonanceParameter = nullptr;
    std::atomic<float>* filterModeParameter = nullptr;
    std::atomic<float>* holdParameter = nullptr;

    std::atomic<float>* attackParameter = nullptr;
    std::atomic<float>* decayParameter = nullptr;
    std::atomic<float>* sustainParameter = nullptr;
    std::atomic<float>* releaseParameter = nullptr;
    std::atomic<float>* filterAttackParameter = nullptr;
    std::atomic<float>* filterDecayParameter = nullptr;
    std::atomic<float>* filterSustainParameter = nullptr;
    std::atomic<float>* filterReleaseParameter = nullptr;
    std::atomic<float>* filterEnvelopeAmountParameter = nullptr;

    std::atomic<float>* lfoFrequencyParameter = nullptr;
    std::atomic<float>* lfoWaveformParameter = nullptr;
    std::atomic<float>* lfoFilterAmountParameter = nullptr;
    std::atomic<float>* lfoVolumeAmountParameter = nullptr;
    std::atomic<float>* lfoPitchAmountParameter = nullptr;

    std::atomic<float>* saturationDriveParameter = nullptr;
    std::atomic<float>* saturationMixParameter = nullptr;

    std::atomic<float>* reversePlaybackParameter = nullptr;
    std::atomic<float>* loopEnabledParameter = nullptr;
    std::atomic<float>* loopStartParameter = nullptr;
    std::atomic<float>* loopEndParameter = nullptr;

    // Advanced effect parameters
    std::atomic<float>* lofiEnabledParameter = nullptr;
    std::atomic<float>* noiseMixParameter = nullptr;
    std::atomic<float>* bitCrushParameter = nullptr;
    std::atomic<float>* sampleRateReductionParameter = nullptr;
    std::atomic<float>* randomPlaybackParameter = nullptr;
    std::atomic<float>* pitchVariationParameter = nullptr;
    std::atomic<float>* randomPlaybackEnabledParameter = nullptr;

    // Creative FX parameters
    std::atomic<float>* creativeEnabledParameter = nullptr;
    std::atomic<float>* stereoWidthParameter = nullptr;
    std::atomic<float>* autopanRateParameter = nullptr;
    std::atomic<float>* autopanDepthParameter = nullptr;
    std::atomic<float>* stutterRateParameter = nullptr;
    std::atomic<float>* stutterDepthParameter = nullptr;

    // State variables
    bool holdEnabled = false;
    juce::Array<int> heldNotes;

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    bool audioPlayerPlaying = false;
    juce::String loadedFileName;

    bool reversePlayback = false;
    juce::AudioBuffer<float> reverseBuffer;
    int reversePosition = 0;

    double loopStart = 0.0;
    double loopEnd = 0.0;
    bool loopEnabled = false;
    double currentLoopPosition = 0.0;

    // Loop recording variables
    LoopState loopState = LoopState::Stopped;
    juce::AudioBuffer<float> loopBuffer;
    juce::AudioBuffer<float> overdubBuffer;
    int loopRecordingPosition = 0;
    int loopPlaybackPosition = 0;
    double loopLength = 0.0;
    double loopPosition = 0.0;
    bool isFirstRecording = true;
    juce::CriticalSection loopLock;

    // Variables for effects
    juce::Random effectRandom;
    int sampleCounter = 0;
    float heldSample[2] = { 0.0f, 0.0f };

    // Waveform componets
    juce::AudioThumbnailCache thumbnailCache{ 5 };
    juce::AudioThumbnail thumbnail{ 512, formatManager, thumbnailCache };

    // CreativeFx state
    double autoPanPhase = 0.0;
    double stutterPhase = 0.0;

    //PC keybord MIDI state
    juce::Array<int> currentlyPressedNotes;

    inline void advancePhase(double& phase, double freq, double sampleRate)
    {
        const double inc = (2.0 * juce::MathConstants<double>::pi) * (freq / sampleRate);
        phase += inc;
        if (phase >= 2.0 * juce::MathConstants<double>::pi)
            phase -= 2.0 * juce::MathConstants<double>::pi;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProgettoPlugInAudioProcessor)
};