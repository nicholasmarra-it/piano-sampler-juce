#pragma once

#include <JuceHeader.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include "SamplerSound.h"
#include "WaveformDisplay.h"

class ProgettoPlugInAudioProcessorEditor final : public juce::AudioProcessorEditor,
    private juce::Button::Listener,
    private juce::ComboBox::Listener,
    private juce::Slider::Listener,
    private juce::Timer,
    public juce::FileDragAndDropTarget
{
public:
    explicit ProgettoPlugInAudioProcessorEditor(ProgettoPlugInAudioProcessor&);
    ~ProgettoPlugInAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;
   
    //PC keybord MIDI support
    bool keyPressed(const juce::KeyPress& key) override;
    bool keyStateChanged(bool isKeyDown) override;
    void focusGained(juce::Component::FocusChangeType cause) override;
    void focusLost(juce::Component::FocusChangeType cause) override;

private:
    void buttonClicked(juce::Button* b) override;
    void comboBoxChanged(juce::ComboBox* box) override;
    void sliderValueChanged(juce::Slider* s) override;
    void timerCallback() override;

    void applySampleMappingToAllSounds();
    static void setLabelFont(juce::Label& l, float size, bool bold = false);
    void setupLabel(juce::Label& label, const juce::String& text);

    // Helper functions
    void updatePresetList();

    ProgettoPlugInAudioProcessor& processor;

    //PC keybord MIDI mapping
    juce::Array<juce::KeyPress> keyMappings;
    juce::HashMap<int, bool> keyStateMap; 
    void initializeKeyMappings();
    int keyPressToMidiNote(const juce::KeyPress& key) const;
    void releaseAllKeys(); 
    juce::Label keyboardStatusLabel;

    // Waveform
    WaveformDisplay waveformDisplay;
    juce::Label     waveformLabel;
    void syncWaveformFromProcessor();   // <— nuovo helper
    juce::String lastShownFileName;

    bool isDragActive = false;

    // Sampler / Mapping
    juce::TextButton loadButton{ "Load WAV/AIFF" };
    juce::Slider rootNote, coarse, fine;
    juce::ToggleButton oneShot{ "One-shot" };
    juce::ComboBox modeBox;

    // Filter
    juce::Slider cutoffFrequencySlider, resonanceSlider;
    juce::ComboBox filterModeBox;
    juce::ToggleButton holdButton;
    juce::Label cutoffLabel, resonanceLabel, filterModeLabel, filterTitle;

    // Amp ADSR
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel, adsrTitle;

    // Filter ADSR
    juce::Slider filterAttackSlider, filterDecaySlider, filterSustainSlider, filterReleaseSlider, filterEnvelopeAmountSlider;
    juce::Label  filterAttackLabel, filterDecayLabel, filterSustainLabel, filterReleaseLabel, filterEnvelopeAmountLabel, filterAdsrTitle;

    // LFO
    juce::Slider lfoFrequencySlider;
    juce::ComboBox lfoWaveformBox;
    juce::Slider lfoFilterAmountSlider, lfoVolumeAmountSlider, lfoPitchAmountSlider;
    juce::Label  lfoFrequencyLabel, lfoWaveformLabel, lfoFilterAmountLabel, lfoVolumeAmountLabel, lfoPitchAmountLabel, lfoTitle;

    // Saturation
    juce::Slider saturationDriveSlider, saturationMixSlider;
    juce::Label  saturationDriveLabel, saturationMixLabel, saturationTitle;

    // Loop player/rec
    juce::TextButton loadLoopButton{ "Load Loop" };
    juce::TextButton playLoopButton{ "Play/Stop" };
    juce::ToggleButton reverseButton{ "Reverse" };
    juce::ToggleButton loopEnableButton{ "Loop Enable" };
    juce::Slider loopStartSlider, loopEndSlider;
    juce::Label  loopStartLabel, loopEndLabel, loopTitle;

    juce::TextButton recordLoopButton{ "Record Loop" };
    juce::TextButton overdubButton{ "Overdub" };
    juce::TextButton clearLoopButton{ "Clear Loop" };
    juce::Label  loopStateLabel, loopLengthLabel;
    juce::TextButton resetButton{ "RESET" };

    // Lo-Fi Effects Section
    juce::Label lofiSectionLabel;
    juce::ToggleButton lofiToggle;
    juce::Slider noiseMixSlider, bitCrushSlider, sampleRateSlider;
    juce::Label noiseLabel, crushLabel, rateLabel;

    // Random Effects Section
    juce::Label randomSectionLabel;
    juce::ToggleButton randomPlaybackToggle;
    juce::Slider randomPlaybackSlider, pitchVariationSlider;
    juce::Label randomPlaybackLabel, pitchVariationLabel;

    // Creative FX Section
    juce::Label creativeSectionLabel;
    juce::ToggleButton creativeToggle;
    juce::Slider stereoWidthSlider, autopanRateSlider, autopanDepthSlider, stutterRateSlider, stutterDepthSlider;
    juce::Label stereoWidthLabel, autopanRateLabel, autopanDepthLabel, stutterRateLabel, stutterDepthLabel;

    // Presets Section
    juce::Label presetSectionLabel;
    juce::Label presetLabel;
    juce::ComboBox presetComboBox;
    juce::TextButton savePresetButton, loadPresetButton;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAttachment, resonanceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filterModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>   holdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment, decayAttachment, sustainAttachment, releaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterAttackAttachment, filterDecayAttachment, filterSustainAttachment, filterReleaseAttachment, filterEnvelopeAmountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoFrequencyAttachment, lfoFilterAmountAttachment, lfoVolumeAmountAttachment, lfoPitchAmountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfoWaveformAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> saturationDriveAttachment, saturationMixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> reverseAttachment, loopEnableAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> loopStartAttachment, loopEndAttachment;

    // New parameters attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lofiAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> noiseMixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bitCrushAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sampleRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> randomPlaybackToggleAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> randomPlaybackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pitchVariationAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> creativeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> stereoWidthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> autopanRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> autopanDepthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> stutterRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> stutterDepthAttachment;

    juce::MidiKeyboardComponent keyboardComponent;
    std::unique_ptr<juce::FileChooser> fileChooser;

    // slider helpers
    static juce::Slider* setupAdsrSlider(juce::Slider& s, double min, double max, double def);
    static juce::Slider* setupLfoSlider(juce::Slider& s, double min, double max, double def);
    static juce::Slider* setupSaturationSlider(juce::Slider& s, double min, double max, double def);
    static juce::Slider* setupLoFiSlider(juce::Slider& s, double min, double max, double def);

    struct CleanKnobLNF : public juce::LookAndFeel_V4
    {
        juce::Colour track = juce::Colour(0xff8fb0ff); // progress
        juce::Colour trackBg = juce::Colour(0x334a4a4a); // sfondo pista
        juce::Colour thumb = juce::Colours::white;      // cursore
        juce::Colour tick = juce::Colour(0x55ffffff);  // tacche leggere

        void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h, float pos,
            float startAng, float endAng, juce::Slider& s) override
        {
            g.addTransform(juce::AffineTransform()); // AA default
            auto b = juce::Rectangle<float>(x, y, w, h).reduced(6.0f);
            auto r = std::min(b.getWidth(), b.getHeight()) * 0.5f;
            auto c = b.getCentre();
            float thickness = std::max(3.5f, r * 0.16f);

            // anello di fondo + lieve ombra interna
            g.setColour(trackBg);
            g.fillEllipse(b);
            g.setColour(trackBg.withMultipliedAlpha(1.2f));
            g.drawEllipse(b.reduced(2.0f), 2.0f);

            // arco di valore
            float ang = startAng + pos * (endAng - startAng);
            juce::Path p; p.addCentredArc(c.x, c.y, r - thickness * 0.75f, r - thickness * 0.75f,
                0.0f, startAng, ang, true);
            g.setColour(track);
            g.strokePath(p, juce::PathStrokeType(thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            // cap di estremità
            auto tip = c + juce::Point<float>(std::cos(ang), std::sin(ang)) * (r - thickness * 0.75f);
            g.fillEllipse(juce::Rectangle<float>(thickness, thickness).withCentre(tip));

            // thumb (pallino) con alone
            g.setColour(thumb.withAlpha(0.95f));
            g.fillEllipse(juce::Rectangle<float>(12.f, 12.f).withCentre(tip));
            g.setColour(thumb.withAlpha(s.isMouseOverOrDragging() ? 0.25f : 0.12f));
            g.fillEllipse(juce::Rectangle<float>(22.f, 22.f).withCentre(tip));
        }

        void drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h,
            float sliderPos, float, float,
            const juce::Slider::SliderStyle style, juce::Slider& s) override
        {
            auto r = juce::Rectangle<float>(x, y, w, h).reduced(5.0f);

            // pista
            auto trackArea = (style == juce::Slider::LinearVertical)
                ? r.withWidth(7.0f).withCentre(r.getCentre())
                : r.withHeight(7.0f).withCentre(r.getCentre());

            // fondo + leggero “glow” al rollover
            g.setColour(trackBg);
            g.fillRoundedRectangle(trackArea, 3.5f);
            if (s.isMouseOverOrDragging())
            {
                g.setColour(trackBg.withMultipliedAlpha(1.25f));
                g.drawRoundedRectangle(trackArea, 3.5f, 1.2f);
            }

            // parte riempita (valore)
            g.setColour(track);
            if (style == juce::Slider::LinearVertical)
                g.fillRoundedRectangle(trackArea.withY(sliderPos).withHeight(trackArea.getBottom() - sliderPos), 3.5f);
            else
                g.fillRoundedRectangle(trackArea.withRight(sliderPos), 3.5f);

            // cap terminale
            g.fillEllipse(style == juce::Slider::LinearVertical
                ? juce::Rectangle<float>(7.5f, 7.5f).withCentre({ trackArea.getCentreX(), sliderPos })
                : juce::Rectangle<float>(7.5f, 7.5f).withCentre({ sliderPos, trackArea.getCentreY() }));

            // thumb con alone
            g.setColour(thumb);
            if (style == juce::Slider::LinearVertical)
                g.fillEllipse(trackArea.getCentreX() - 7.0f, sliderPos - 7.0f, 14.0f, 14.0f);
            else
                g.fillEllipse(sliderPos - 7.0f, trackArea.getCentreY() - 7.0f, 14.0f, 14.0f);

            g.setColour(thumb.withAlpha(s.isMouseOverOrDragging() ? 0.25f : 0.12f));
            if (style == juce::Slider::LinearVertical)
                g.fillEllipse(trackArea.getCentreX() - 12.f, sliderPos - 12.f, 24.f, 24.f);
            else
                g.fillEllipse(sliderPos - 12.f, trackArea.getCentreY() - 12.f, 24.f, 24.f);
        }

        // TextBox sobrio
        void drawLabel(juce::Graphics& g, juce::Label& label) override
        {
            auto bounds = label.getLocalBounds().toFloat();
            const float radius = 6.0f;

            auto bg = label.findColour(juce::Label::backgroundColourId);
            auto txt = label.findColour(juce::Label::textColourId);
            auto out = label.findColour(juce::Label::outlineColourId);

            g.setColour(bg);
            g.fillRoundedRectangle(bounds, radius);

            if (!out.isTransparent())
            {
                g.setColour(out);
                g.drawRoundedRectangle(bounds.reduced(0.5f), radius, 1.0f);
            }

            g.setColour(txt);
            g.setFont(label.getFont());
            auto textArea = label.getBorderSize().subtractedFrom(label.getLocalBounds());
            g.drawFittedText(label.getText(), textArea, label.getJustificationType(), 1);
        }

        void drawButtonBackground(juce::Graphics& g, juce::Button& b,
            const juce::Colour& bg, bool over, bool down) override
        {
            auto r = b.getLocalBounds().toFloat();
            float radius = 10.0f;

            juce::Colour base = bg;
            if (over) base = base.brighter(0.06f);
            if (down) base = base.darker(0.08f);

            g.setColour(base);
            g.fillRoundedRectangle(r, radius);

            // bordo tenue
            g.setColour(base.contrasting(0.12f).withAlpha(0.25f));
            g.drawRoundedRectangle(r.reduced(0.5f), radius, 1.0f);
        }
    };

    CleanKnobLNF lnfClean;

    // Raccoglie tutti gli slider per applicare la skin in modo affidabile (copertura totale)
    std::vector<juce::Slider*> getAllSliders()
    {
        // INSERISCI qui TUTTI i tuoi slider, anche quelli creati in sezioni/aiuti
        return {
            // LFO
            &lfoFrequencySlider, &lfoFilterAmountSlider, &lfoVolumeAmountSlider, &lfoPitchAmountSlider,
            // Filtro base
            &cutoffFrequencySlider, &resonanceSlider,
            // ADSR amp (mancavano)
            &attackSlider, &decaySlider, &sustainSlider, &releaseSlider,
            // ADSR filtro (mancavano)
            &filterAttackSlider, &filterDecaySlider, &filterSustainSlider, &filterReleaseSlider, &filterEnvelopeAmountSlider,
            // Saturation
            &saturationDriveSlider, &saturationMixSlider,
            // Loop
            &loopStartSlider, &loopEndSlider,
            // Mapping
            &rootNote, &coarse, &fine,
            // Lo-Fi
            &noiseMixSlider, &bitCrushSlider, &sampleRateSlider,
            // Random
            &randomPlaybackSlider, &pitchVariationSlider,
            // Creative FX
            &stereoWidthSlider, &autopanRateSlider, &autopanDepthSlider, &stutterRateSlider, &stutterDepthSlider
        };
    }

    void applySkinToAllSliders()
    {
        for (auto* s : getAllSliders())
        {
            if (s == nullptr) continue;

            // Look&Feel locale (non strettamente necessario visto il default globale,
            // ma utile se in futuro vuoi escludere singoli slider)
            s->setLookAndFeel(&lnfClean);

            // Posizione TextBox: sotto per rotary e verticali, destra per orizzontali
            const auto st = s->getSliderStyle();
            if (st == juce::Slider::Rotary
                || st == juce::Slider::RotaryHorizontalVerticalDrag
                || st == juce::Slider::RotaryVerticalDrag
                || st == juce::Slider::LinearVertical)
            {
                s->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 20);
            }
            else
            {
                s->setTextBoxStyle(juce::Slider::TextBoxRight, false, 56, 20);
            }

            // Colori che la TextBox (Label interna) erediterà dal padre Slider
            s->setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
            s->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0x66000000)); // scuro
            s->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProgettoPlugInAudioProcessorEditor)
};