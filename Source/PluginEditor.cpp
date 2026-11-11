#include "PluginEditor.h"
#include "PluginProcessor.h"

namespace
{
    constexpr int kTitleBarH = 35;
    constexpr int kWaveformH = 120;
    constexpr int kWaveformLabelH = 22;
    constexpr int kKeyboardH = 140;
    constexpr int kCorner = 8;
    constexpr int kPad = 10;

    static const juce::Colour kBg(0xff1e2228);
    static const juce::Colour kTitleBg(0xff191d22);
    static const juce::Colour kPanel(0xff2a2f36);
    static const juce::Colour kPanelBorder = juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.18f);
    static const juce::Colour kFrameBorder = juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.12f);

    inline juce::Colour kAccentReset() { return juce::Colour::fromRGB(138, 58, 58); }   // rosso spento
    inline juce::Colour kAccentSave() { return juce::Colour::fromRGB(62, 92, 128); }   // blu/grigio
    inline juce::Colour kAccentLoad() { return juce::Colour::fromRGB(47, 125, 87); }   // verde/grigio
    inline juce::Colour kBtnText() { return juce::Colours::white; }

    inline int kInnerPad() { return 10; }   // padding interno dei pannelli
    inline int kRowGap() { return 8; }    // spazio verticale tra righe
    inline int kColGap() { return 10; }   // spazio orizzontale tra colonne

    static inline juce::Font makeFont(float size, bool bold = false)
    {
        juce::Font f{ juce::FontOptions(size) };
        if (bold) f.setBold(true);
        return f;
    }

    inline void fitRotaryInBounds(juce::Slider& s, int margin = 6, int maxDiameter = 120, int minDiameter = 40, int textBoxH = -1)
    {
        // Assicura TextBox sotto (coerente con la tua skin)
        if (s.getTextBoxPosition() != juce::Slider::TextBoxBelow)
            s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, (textBoxH > 0 ? textBoxH : 20));

        auto r = s.getBounds();                 // bounds già assegnati dal tuo layout
        const int tbH = s.getTextBoxHeight();     // altezza reale della textbox

        const int availW = juce::jmax(0, r.getWidth() - 2 * margin);
        const int availH = juce::jmax(0, r.getHeight() - tbH - margin);

        int diam = juce::jmin(availW, availH);
        diam = juce::jmin(diam, maxDiameter);
        diam = juce::jmax(diam, minDiameter);

        const int cx = r.getCentreX();
        const int y = r.getY() + margin;         // piccolo margine in alto

        s.setBounds(cx - diam / 2, y, diam, diam + tbH);
    }
}

void ProgettoPlugInAudioProcessorEditor::setLabelFont(juce::Label& l, float size, bool bold)
{
    l.setFont(makeFont(size, bold));
}

void ProgettoPlugInAudioProcessorEditor::setupLabel(juce::Label& label, const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    setLabelFont(label, 14.0f);
    label.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(label);
}

//PC keybord MIDI
void ProgettoPlugInAudioProcessorEditor::initializeKeyMappings()
{
    keyMappings.add(juce::KeyPress('a', juce::ModifierKeys::noModifiers, 0)); // C3
    keyMappings.add(juce::KeyPress('s', juce::ModifierKeys::noModifiers, 0)); // D3
    keyMappings.add(juce::KeyPress('d', juce::ModifierKeys::noModifiers, 0)); // E3
    keyMappings.add(juce::KeyPress('f', juce::ModifierKeys::noModifiers, 0)); // F3
    keyMappings.add(juce::KeyPress('g', juce::ModifierKeys::noModifiers, 0)); // G3
    keyMappings.add(juce::KeyPress('h', juce::ModifierKeys::noModifiers, 0)); // A3
    keyMappings.add(juce::KeyPress('j', juce::ModifierKeys::noModifiers, 0)); // B3

    keyMappings.add(juce::KeyPress('w', juce::ModifierKeys::noModifiers, 0)); // C#3
    keyMappings.add(juce::KeyPress('e', juce::ModifierKeys::noModifiers, 0)); // D#3
    keyMappings.add(juce::KeyPress('t', juce::ModifierKeys::noModifiers, 0)); // F#3
    keyMappings.add(juce::KeyPress('y', juce::ModifierKeys::noModifiers, 0)); // G#3
    keyMappings.add(juce::KeyPress('u', juce::ModifierKeys::noModifiers, 0)); // A#3

    keyMappings.add(juce::KeyPress('z', juce::ModifierKeys::noModifiers, 0)); // C4
    keyMappings.add(juce::KeyPress('x', juce::ModifierKeys::noModifiers, 0)); // D4
    keyMappings.add(juce::KeyPress('c', juce::ModifierKeys::noModifiers, 0)); // E4
    keyMappings.add(juce::KeyPress('v', juce::ModifierKeys::noModifiers, 0)); // F4
    keyMappings.add(juce::KeyPress('b', juce::ModifierKeys::noModifiers, 0)); // G4
    keyMappings.add(juce::KeyPress('n', juce::ModifierKeys::noModifiers, 0)); // A4
    keyMappings.add(juce::KeyPress('m', juce::ModifierKeys::noModifiers, 0)); // B4
}

int ProgettoPlugInAudioProcessorEditor::keyPressToMidiNote(const juce::KeyPress& key) const
{
    static const juce::Array<int> noteMapping = {
        60, 62, 64, 65, 67, 69, 71,  
        61, 63, 66, 68, 70,          
        72, 74, 76, 77, 79, 81, 83   
    };

    for (int i = 0; i < keyMappings.size(); ++i)
    {
        if (keyMappings[i] == key && i < noteMapping.size())
            return noteMapping[i];
    }

    return -1; 
}

//PC keybord MIDI
bool ProgettoPlugInAudioProcessorEditor::keyPressed(const juce::KeyPress& key)
{
    const int noteNumber = keyPressToMidiNote(key);

    if (noteNumber != -1)
    {
        
        bool isCurrentlyPressed = keyStateMap.contains(noteNumber) ? keyStateMap[noteNumber] : false;

        if (!isCurrentlyPressed)
        {
            processor.handleKeyPress(noteNumber);
            keyStateMap.set(noteNumber, true);
            return true;
        }
    }

    // Gestione tasti speciali dell'interfaccia
    if (key.getKeyCode() == juce::KeyPress::spaceKey)
    {
        // Space per play/stop loop
        playLoopButton.triggerClick();
        return true;
    }
    else if (key.getKeyCode() == juce::KeyPress::escapeKey)
    {
        // ESC per rilasciare tutte le note
        releaseAllKeys();
        return true;
    }

    return false;
}

bool ProgettoPlugInAudioProcessorEditor::keyStateChanged(bool isKeyDown)
{
    if (!isKeyDown)
    {
        juce::Array<int> notesToRelease;

        for (auto& key : keyMappings)
        {
            int noteNumber = keyPressToMidiNote(key);
            if (noteNumber != -1)
            {
                bool wasPressed = keyStateMap.contains(noteNumber) ? keyStateMap[noteNumber] : false;
                if (wasPressed && !key.isCurrentlyDown())
                {
                    notesToRelease.add(noteNumber);
                }
            }
        }

        for (int note : notesToRelease)
        {
            processor.handleKeyRelease(note);
            keyStateMap.set(note, false);
        }
    }

    return false;
}

void ProgettoPlugInAudioProcessorEditor::focusGained(juce::Component::FocusChangeType cause)
{
    releaseAllKeys();
    repaint();
}

void ProgettoPlugInAudioProcessorEditor::focusLost(juce::Component::FocusChangeType cause)
{
    releaseAllKeys();
    repaint();
}

void ProgettoPlugInAudioProcessorEditor::releaseAllKeys()
{
    juce::Array<int> notesToRelease;

    for (auto it = keyStateMap.begin(); it != keyStateMap.end(); ++it)
    {
        if (it.getValue()) 
        {
            notesToRelease.add(it.getKey());
        }
    }

    for (int note : notesToRelease)
    {
        processor.handleKeyRelease(note);
        keyStateMap.set(note, false);
    }

    auto pressedNotes = processor.getCurrentlyPressedNotes();
    for (int note : pressedNotes)
    {
        processor.handleKeyRelease(note);
    }
}

// Slider helpers
juce::Slider* ProgettoPlugInAudioProcessorEditor::setupAdsrSlider(juce::Slider& s, double min, double max, double def)
{
    s.setRange(min, max, 0.001);
    s.setValue(def);
    s.setSliderStyle(juce::Slider::LinearVertical);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    return &s;
}

juce::Slider* ProgettoPlugInAudioProcessorEditor::setupLfoSlider(juce::Slider& s, double min, double max, double def)
{
    s.setRange(min, max, 0.01);
    s.setValue(def);
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);

    // Enhanced slider appearance
    s.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::lightblue);
    s.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    s.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    s.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::black.withAlpha(0.5f));
    s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentWhite);

    return &s;
}

juce::Slider* ProgettoPlugInAudioProcessorEditor::setupSaturationSlider(juce::Slider& s, double min, double max, double def)
{
    s.setRange(min, max, 0.01);
    s.setValue(def);
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);

    // Enhanced slider appearance
    s.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::orange);
    s.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    s.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    s.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::black.withAlpha(0.5f));
    s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentWhite);

    return &s;
}

juce::Slider* ProgettoPlugInAudioProcessorEditor::setupLoFiSlider(juce::Slider& s, double min, double max, double def)
{
    s.setRange(min, max, 0.01);
    s.setValue(def);
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);

    // Lo-Fi specific colors
    s.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::cyan);
    s.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    s.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    s.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::black.withAlpha(0.5f));
    s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentWhite);

    return &s;
}
// =============================================================================

ProgettoPlugInAudioProcessorEditor::ProgettoPlugInAudioProcessorEditor(ProgettoPlugInAudioProcessor& p)
    : AudioProcessorEditor(&p),
    processor(p),
    keyboardComponent(processor.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setSize(1400, 800);

    // Waveform area
    addAndMakeVisible(waveformDisplay);
    addAndMakeVisible(waveformLabel);
    waveformLabel.setText("No file loaded", juce::dontSendNotification);
    waveformLabel.setJustificationType(juce::Justification::centred);
    setLabelFont(waveformLabel, 14.0f);
    waveformLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.85f));

    waveformDisplay.onFileDropped = [this](const juce::File& file)
        {
            if (processor.loadLoopSample(file))
            {
                playLoopButton.setToggleState(true, juce::sendNotification);
                const double duration = processor.getLoopEnd();
                loopStartSlider.setRange(0.0, duration, 0.01);
                loopEndSlider.setRange(0.0, duration, 0.01);
                loopEndSlider.setValue(duration);
                syncWaveformFromProcessor();
                repaint();
            }
        };

    // Thumbanil processor
    waveformDisplay.setAudioThumbnail(&processor.getThumbnail());
    lastShownFileName = processor.getLoadedFileName();

    // Sampler/Mapping
    addAndMakeVisible(loadButton);
    loadButton.addListener(this);

    auto setupLinear = [](juce::Slider& s, double min, double max, double def, double step, const juce::String& suffix)
        {
            s.setRange(min, max, step);
            s.setValue(def);
            s.setSliderStyle(juce::Slider::LinearHorizontal);
            s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 64, 20);
            s.setTextValueSuffix(suffix);
            return &s;
        };

    addAndMakeVisible(*setupLinear(rootNote, 0, 127, 69, 1, " note"));
    addAndMakeVisible(*setupLinear(coarse, -48, 48, 0, 1, " st"));
    addAndMakeVisible(*setupLinear(fine, -100, 100, 0, 1, " c"));
    addAndMakeVisible(oneShot);

    for (auto* s : { &rootNote, &coarse, &fine })
        s->addListener(this);
    oneShot.addListener(this);

    modeBox.addItem("Poly", (int)PlayMode::Poly + 1);
    modeBox.addItem("Mono Legato", (int)PlayMode::MonoLegato + 1);
    modeBox.addItem("Mono Retrig", (int)PlayMode::MonoRetrigger + 1);
    modeBox.setSelectedId((int)PlayMode::Poly + 1);
    modeBox.addListener(this);
    addAndMakeVisible(modeBox);

    // Filter
    filterTitle.setText("FILTER", juce::dontSendNotification);
    setLabelFont(filterTitle, 16.0f, true);
    filterTitle.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(filterTitle);

    cutoffFrequencySlider.setSliderStyle(juce::Slider::LinearVertical);
    cutoffFrequencySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    cutoffFrequencySlider.setRange(20.0, 20000.0, 1.0);
    cutoffFrequencySlider.setValue(1000.0);
    cutoffFrequencySlider.setSkewFactorFromMidPoint(1000.0);
    cutoffFrequencySlider.setNumDecimalPlacesToDisplay(0);
    addAndMakeVisible(cutoffFrequencySlider);

    cutoffLabel.setText("Cutoff", juce::dontSendNotification);
    setLabelFont(cutoffLabel, 14.0f);
    cutoffLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(cutoffLabel);

    resonanceSlider.setSliderStyle(juce::Slider::LinearVertical);
    resonanceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    resonanceSlider.setRange(0.1, 10.0, 0.1);
    resonanceSlider.setValue(1.0);
    resonanceSlider.setNumDecimalPlacesToDisplay(1);
    addAndMakeVisible(resonanceSlider);

    resonanceLabel.setText("Resonance", juce::dontSendNotification);
    setLabelFont(resonanceLabel, 14.0f);
    resonanceLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(resonanceLabel);

    filterModeBox.addItem("Low-Pass", 1);
    filterModeBox.addItem("High-Pass", 2);
    filterModeBox.addItem("Band-Pass", 3);
    filterModeBox.addItem("Bypass", 4);
    filterModeBox.setSelectedId(1);
    filterModeBox.addListener(this);
    addAndMakeVisible(filterModeBox);

    filterModeLabel.setText("Mode", juce::dontSendNotification);
    setLabelFont(filterModeLabel, 14.0f);
    filterModeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(filterModeLabel);

    holdButton.setButtonText("HOLD");
    holdButton.setClickingTogglesState(true);
    holdButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::green);
    holdButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    addAndMakeVisible(holdButton);

    // Amp ADSR
    adsrTitle.setText("AMPLITUDE ADSR", juce::dontSendNotification);
    setLabelFont(adsrTitle, 16.0f, true);
    adsrTitle.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(adsrTitle);

    addAndMakeVisible(*setupAdsrSlider(attackSlider, 0.001, 5.0, 0.002));
    addAndMakeVisible(*setupAdsrSlider(decaySlider, 0.001, 5.0, 0.01));
    addAndMakeVisible(*setupAdsrSlider(sustainSlider, 0.0, 1.0, 1.0));
    addAndMakeVisible(*setupAdsrSlider(releaseSlider, 0.001, 5.0, 0.08));

    attackLabel.setText("Attack", juce::dontSendNotification);
    decayLabel.setText("Decay", juce::dontSendNotification);
    sustainLabel.setText("Sustain", juce::dontSendNotification);
    releaseLabel.setText("Release", juce::dontSendNotification);
    for (auto* l : { &attackLabel, &decayLabel, &sustainLabel, &releaseLabel })
    {
        setLabelFont(*l, 12.0f);
        l->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*l);
    }

    // Filter ADSR
    filterAdsrTitle.setText("FILTER ADSR", juce::dontSendNotification);
    setLabelFont(filterAdsrTitle, 16.0f, true);
    filterAdsrTitle.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(filterAdsrTitle);

    addAndMakeVisible(*setupAdsrSlider(filterAttackSlider, 0.001, 5.0, 0.002));
    addAndMakeVisible(*setupAdsrSlider(filterDecaySlider, 0.001, 5.0, 0.01));
    addAndMakeVisible(*setupAdsrSlider(filterSustainSlider, 0.0, 1.0, 1.0));
    addAndMakeVisible(*setupAdsrSlider(filterReleaseSlider, 0.001, 5.0, 0.08));
    addAndMakeVisible(*setupAdsrSlider(filterEnvelopeAmountSlider, 0.0, 1.0, 0.0));

    filterAttackLabel.setText("Attack", juce::dontSendNotification);
    filterDecayLabel.setText("Decay", juce::dontSendNotification);
    filterSustainLabel.setText("Sustain", juce::dontSendNotification);
    filterReleaseLabel.setText("Release", juce::dontSendNotification);
    filterEnvelopeAmountLabel.setText("Env Amt", juce::dontSendNotification);
    for (auto* l : { &filterAttackLabel, &filterDecayLabel, &filterSustainLabel, &filterReleaseLabel, &filterEnvelopeAmountLabel })
    {
        setLabelFont(*l, 12.0f);
        l->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*l);
    }

    // LFO
    lfoTitle.setText("LFO", juce::dontSendNotification);
    setLabelFont(lfoTitle, 16.0f, true);
    lfoTitle.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(lfoTitle);

    addAndMakeVisible(*setupLfoSlider(lfoFrequencySlider, 0.1, 20.0, 1.0));
    lfoFrequencyLabel.setText("Frequency", juce::dontSendNotification);
    setLabelFont(lfoFrequencyLabel, 12.0f);
    lfoFrequencyLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(lfoFrequencyLabel);

    lfoWaveformBox.addItem("Sine", 1);
    lfoWaveformBox.addItem("Square", 2);
    lfoWaveformBox.addItem("Triangle", 3);
    lfoWaveformBox.addItem("Sawtooth", 4);
    lfoWaveformBox.addItem("Random", 5);
    lfoWaveformBox.setSelectedId(1);
    addAndMakeVisible(lfoWaveformBox);

    lfoWaveformLabel.setText("Waveform", juce::dontSendNotification);
    setLabelFont(lfoWaveformLabel, 12.0f);
    lfoWaveformLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(lfoWaveformLabel);

    addAndMakeVisible(*setupLfoSlider(lfoFilterAmountSlider, 0.0, 1.0, 0.0));
    lfoFilterAmountLabel.setText("Filter Amt", juce::dontSendNotification);
    setLabelFont(lfoFilterAmountLabel, 12.0f);
    lfoFilterAmountLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(lfoFilterAmountLabel);

    addAndMakeVisible(*setupLfoSlider(lfoVolumeAmountSlider, 0.0, 1.0, 0.0));
    lfoVolumeAmountLabel.setText("Volume Amt", juce::dontSendNotification);
    setLabelFont(lfoVolumeAmountLabel, 12.0f);
    lfoVolumeAmountLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(lfoVolumeAmountLabel);

    addAndMakeVisible(*setupLfoSlider(lfoPitchAmountSlider, 0.0, 1.0, 0.0));
    lfoPitchAmountLabel.setText("Pitch Amt", juce::dontSendNotification);
    setLabelFont(lfoPitchAmountLabel, 12.0f);
    lfoPitchAmountLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(lfoPitchAmountLabel);

    // Saturation
    saturationTitle.setText("SATURATION", juce::dontSendNotification);
    setLabelFont(saturationTitle, 16.0f, true);
    saturationTitle.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(saturationTitle);

    addAndMakeVisible(*setupSaturationSlider(saturationDriveSlider, 0.0, 1.0, 0.0));
    saturationDriveLabel.setText("Drive", juce::dontSendNotification);
    setLabelFont(saturationDriveLabel, 12.0f);
    saturationDriveLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(saturationDriveLabel);

    addAndMakeVisible(*setupSaturationSlider(saturationMixSlider, 0.0, 1.0, 0.0));
    saturationMixLabel.setText("Mix", juce::dontSendNotification);
    setLabelFont(saturationMixLabel, 12.0f);
    saturationMixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(saturationMixLabel);

    // Loop
    loadLoopButton.setButtonText("Load Loop");
    loadLoopButton.addListener(this);
    addAndMakeVisible(loadLoopButton);

    playLoopButton.setButtonText("Play/Stop");
    playLoopButton.setClickingTogglesState(true);
    playLoopButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::orange);
    playLoopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    playLoopButton.addListener(this);
    addAndMakeVisible(playLoopButton);

    reverseButton.setButtonText("Reverse");
    reverseButton.setClickingTogglesState(true);
    reverseButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::white);
    reverseButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::grey);
    addAndMakeVisible(reverseButton);

    loopEnableButton.setButtonText("Loop Enable");
    loopEnableButton.setClickingTogglesState(true);
    loopEnableButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::white);
    loopEnableButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::grey);
    addAndMakeVisible(loopEnableButton);

    loopStartSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    loopStartSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 60, 20);
    loopStartSlider.setRange(0.0, 600.0, 0.1);
    loopStartSlider.setValue(0.0);
    addAndMakeVisible(loopStartSlider);

    loopEndSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    loopEndSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 60, 20);
    loopEndSlider.setRange(0.0, 600.0, 0.1);
    loopEndSlider.setValue(0.0);
    addAndMakeVisible(loopEndSlider);

    loopStartLabel.setText("Loop Start (s)", juce::dontSendNotification);
    setLabelFont(loopStartLabel, 12.0f);
    loopStartLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(loopStartLabel);

    loopEndLabel.setText("Loop End (s)", juce::dontSendNotification);
    setLabelFont(loopEndLabel, 12.0f);
    loopEndLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(loopEndLabel);

    loopTitle.setText("LOOP PLAYER", juce::dontSendNotification);
    setLabelFont(loopTitle, 16.0f, true);
    loopTitle.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(loopTitle);

    recordLoopButton.setButtonText("Record Loop");
    recordLoopButton.addListener(this);
    addAndMakeVisible(recordLoopButton);

    overdubButton.setButtonText("Overdub");
    overdubButton.addListener(this);
    addAndMakeVisible(overdubButton);

    clearLoopButton.setButtonText("Clear Loop");
    clearLoopButton.addListener(this);
    addAndMakeVisible(clearLoopButton);

    loopStateLabel.setText("State: Stopped", juce::dontSendNotification);
    setLabelFont(loopStateLabel, 12.0f);
    loopStateLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(loopStateLabel);

    loopLengthLabel.setText("Length: 0.00s", juce::dontSendNotification);
    setLabelFont(loopLengthLabel, 12.0f);
    loopLengthLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(loopLengthLabel);

    // Lo-Fi Effects Section
    lofiSectionLabel.setText("LO-FI EFFECTS", juce::dontSendNotification);
    setLabelFont(lofiSectionLabel, 16.0f, true);
    lofiSectionLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(lofiSectionLabel);

    lofiToggle.setButtonText("Enable Lo-Fi Effects");
    lofiToggle.setClickingTogglesState(true);
    lofiToggle.setColour(juce::ToggleButton::tickColourId, juce::Colours::lightblue);
    lofiToggle.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    addAndMakeVisible(lofiToggle);

    addAndMakeVisible(*setupLoFiSlider(noiseMixSlider, 0.0, 1.0, 0.0));
    noiseLabel.setText("Noise Mix", juce::dontSendNotification);
    setLabelFont(noiseLabel, 12.0f);
    noiseLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(noiseLabel);

    addAndMakeVisible(*setupLoFiSlider(bitCrushSlider, 0.0, 1.0, 0.0));
    crushLabel.setText("Bit Crush", juce::dontSendNotification);
    setLabelFont(crushLabel, 12.0f);
    crushLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(crushLabel);

    addAndMakeVisible(*setupLoFiSlider(sampleRateSlider, 0.1, 1.0, 1.0));
    rateLabel.setText("Sample Rate", juce::dontSendNotification);
    setLabelFont(rateLabel, 12.0f);
    rateLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(rateLabel);

    // Random Effects Section
    randomSectionLabel.setText("RANDOM EFFECTS", juce::dontSendNotification);
    setLabelFont(randomSectionLabel, 16.0f, true);
    randomSectionLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(randomSectionLabel);

    randomPlaybackToggle.setButtonText("Enable Random Effects");
    randomPlaybackToggle.setClickingTogglesState(true);
    randomPlaybackToggle.setColour(juce::ToggleButton::tickColourId, juce::Colours::green);
    randomPlaybackToggle.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    addAndMakeVisible(randomPlaybackToggle);

    addAndMakeVisible(*setupLoFiSlider(randomPlaybackSlider, 0.0, 1.0, 0.0));
    randomPlaybackLabel.setText("Random Amount", juce::dontSendNotification);
    setLabelFont(randomPlaybackLabel, 12.0f);
    randomPlaybackLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(randomPlaybackLabel);

    addAndMakeVisible(*setupLoFiSlider(pitchVariationSlider, 0.0, 0.1, 0.0));
    pitchVariationLabel.setText("Pitch Variation", juce::dontSendNotification);
    setLabelFont(pitchVariationLabel, 12.0f);
    pitchVariationLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(pitchVariationLabel);

    // Creative FX Section
    creativeSectionLabel.setText("CREATIVE FX", juce::dontSendNotification);
    setLabelFont(creativeSectionLabel, 16.0f, true);
    creativeSectionLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(creativeSectionLabel);

    creativeToggle.setButtonText("Enable Creative FX");
    creativeToggle.setClickingTogglesState(true);
    addAndMakeVisible(creativeToggle);

    addAndMakeVisible(*setupLoFiSlider(stereoWidthSlider, 0.0, 2.0, 1.0));
    stereoWidthLabel.setText("Stereo Width", juce::dontSendNotification);
    setLabelFont(stereoWidthLabel, 12.0f); stereoWidthLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(stereoWidthLabel);

    addAndMakeVisible(*setupLoFiSlider(autopanRateSlider, 0.05, 10.0, 1.0));
    autopanRateLabel.setText("Pan Rate", juce::dontSendNotification);
    setLabelFont(autopanRateLabel, 12.0f); autopanRateLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(autopanRateLabel);

    addAndMakeVisible(*setupLoFiSlider(autopanDepthSlider, 0.0, 1.0, 0.0));
    autopanDepthLabel.setText("Pan Depth", juce::dontSendNotification);
    setLabelFont(autopanDepthLabel, 12.0f); autopanDepthLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(autopanDepthLabel);

    addAndMakeVisible(*setupLoFiSlider(stutterRateSlider, 0.25, 32.0, 4.0));
    stutterRateLabel.setText("Stutter Rate", juce::dontSendNotification);
    setLabelFont(stutterRateLabel, 12.0f); stutterRateLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(stutterRateLabel);

    addAndMakeVisible(*setupLoFiSlider(stutterDepthSlider, 0.0, 1.0, 0.0));
    stutterDepthLabel.setText("Stutter Depth", juce::dontSendNotification);
    setLabelFont(stutterDepthLabel, 12.0f); stutterDepthLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(stutterDepthLabel);

    // Presets Section
    presetSectionLabel.setText("PRESETS", juce::dontSendNotification);
    setLabelFont(presetSectionLabel, 16.0f, true);
    presetSectionLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(presetSectionLabel);

    setupLabel(presetLabel, "Preset Selection:");
    presetComboBox.setTextWhenNothingSelected("Select a Preset");
    addAndMakeVisible(presetComboBox);

    savePresetButton.setButtonText("Save Preset");
    savePresetButton.setTooltip("Save current settings as a new preset");
    savePresetButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkblue);
    savePresetButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    savePresetButton.addListener(this);
    addAndMakeVisible(savePresetButton);

    loadPresetButton.setButtonText("Load Preset");
    loadPresetButton.setTooltip("Load selected preset");
    loadPresetButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgreen);
    loadPresetButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    loadPresetButton.addListener(this);
    addAndMakeVisible(loadPresetButton);

    resetButton.setButtonText("RESET");
    resetButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    resetButton.addListener(this);
    addAndMakeVisible(resetButton);

    // RESET (rosso più spento)
    resetButton.setColour(juce::TextButton::buttonColourId, kAccentReset());
    resetButton.setColour(juce::TextButton::buttonOnColourId, kAccentReset().brighter(0.10f));
    resetButton.setColour(juce::TextButton::textColourOffId, kBtnText());
    resetButton.setColour(juce::TextButton::textColourOnId, kBtnText());

    // SAVE PRESET (blu/grigio)
    savePresetButton.setColour(juce::TextButton::buttonColourId, kAccentSave());
    savePresetButton.setColour(juce::TextButton::buttonOnColourId, kAccentSave().brighter(0.12f));
    savePresetButton.setColour(juce::TextButton::textColourOffId, kBtnText());
    savePresetButton.setColour(juce::TextButton::textColourOnId, kBtnText());

    // LOAD PRESET (verde/grigio)
    loadPresetButton.setColour(juce::TextButton::buttonColourId, kAccentLoad());
    loadPresetButton.setColour(juce::TextButton::buttonOnColourId, kAccentLoad().brighter(0.12f));
    loadPresetButton.setColour(juce::TextButton::textColourOffId, kBtnText());
    loadPresetButton.setColour(juce::TextButton::textColourOnId, kBtnText());

    // Attachments for original parameters
    cutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "cutoff_frequency", cutoffFrequencySlider);
    resonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "resonance", resonanceSlider);
    filterModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.getValueTreeState(), "filter_mode", filterModeBox);
    holdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.getValueTreeState(), "hold", holdButton);

    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "attack", attackSlider);
    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "decay", decaySlider);
    sustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "sustain", sustainSlider);
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "release", releaseSlider);

    filterAttackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "filter_attack", filterAttackSlider);
    filterDecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "filter_decay", filterDecaySlider);
    filterSustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "filter_sustain", filterSustainSlider);
    filterReleaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "filter_release", filterReleaseSlider);
    filterEnvelopeAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "filter_envelope_amount", filterEnvelopeAmountSlider);

    lfoFrequencyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "lfo_frequency", lfoFrequencySlider);
    lfoWaveformAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.getValueTreeState(), "lfo_waveform", lfoWaveformBox);
    lfoFilterAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "lfo_filter_amount", lfoFilterAmountSlider);
    lfoVolumeAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "lfo_volume_amount", lfoVolumeAmountSlider);
    lfoPitchAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "lfo_pitch_amount", lfoPitchAmountSlider);

    saturationDriveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "saturation_drive", saturationDriveSlider);
    saturationMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "saturation_mix", saturationMixSlider);

    reverseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.getValueTreeState(), "reverse_playback", reverseButton);
    loopEnableAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.getValueTreeState(), "loop_enabled", loopEnableButton);
    loopStartAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "loop_start", loopStartSlider);
    loopEndAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "loop_end", loopEndSlider);

    // New parameters attachments
    lofiAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.getValueTreeState(), "lofi_enabled", lofiToggle);
    noiseMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "noise_mix", noiseMixSlider);
    bitCrushAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "bit_crush", bitCrushSlider);
    sampleRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "sample_rate", sampleRateSlider);
    randomPlaybackToggleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.getValueTreeState(), "random_playback_enabled", randomPlaybackToggle);
    randomPlaybackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "random_playback", randomPlaybackSlider);
    pitchVariationAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "pitch_variation", pitchVariationSlider);

    creativeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.getValueTreeState(), "creative_enabled", creativeToggle);
    stereoWidthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "stereo_width", stereoWidthSlider);
    autopanRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "autopan_rate", autopanRateSlider);
    autopanDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "autopan_depth", autopanDepthSlider);
    stutterRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "stutter_rate", stutterRateSlider);
    stutterDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "stutter_depth", stutterDepthSlider);

    addAndMakeVisible(keyboardComponent);
    keyboardComponent.setWantsKeyboardFocus(false);
    keyboardComponent.setMouseClickGrabsKeyboardFocus(false);

    //PC keybord MIDI implementation
    setWantsKeyboardFocus(true);
    setFocusContainerType(juce::Component::FocusContainerType::keyboardFocusContainer);
    initializeKeyMappings();

    // Update preset list
    updatePresetList();

    // Save preset callback
    savePresetButton.onClick = [this]()
        {
            auto* alertWindow = new juce::AlertWindow("Save Preset", "Enter preset name:",
                juce::AlertWindow::NoIcon, this);

            alertWindow->addTextEditor("preset", "", "Preset name:");
            alertWindow->addButton("OK", 1);
            alertWindow->addButton("Cancel", 0);

            alertWindow->enterModalState(true,
                juce::ModalCallbackFunction::create([this, alertWindow](int result)
                    {
                        if (result == 1)
                        {
                            auto name = alertWindow->getTextEditorContents("preset");
                            if (name.isNotEmpty())
                            {
                                processor.savePreset(name);
                                updatePresetList();
                                for (int i = 0; i < presetComboBox.getNumItems(); ++i)
                                {
                                    if (presetComboBox.getItemText(i) == name)
                                    {
                                        presetComboBox.setSelectedId(i + 1);
                                        break;
                                    }
                                }
                            }
                        }
                        delete alertWindow;
                    }),
                true
            );
        };

    // Load preset callback
    loadPresetButton.onClick = [this]()
        {
            auto selectedId = presetComboBox.getSelectedId();
            if (selectedId > 0)
            {
                auto presetName = presetComboBox.getItemText(selectedId - 1);
                processor.loadPreset(presetName);
            }
            else
            {
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                    "Load Preset",
                    "Please select a preset from the list.");
            }
        };

    applySkinToAllSliders();

    juce::LookAndFeel::setDefaultLookAndFeel(&lnfClean);

    startTimer(50);
}

void ProgettoPlugInAudioProcessorEditor::paint(juce::Graphics& g)
{
    // SFONDO UNICO
    g.fillAll(kBg);

    // TITLE BAR uniforme (niente gradiente)
    auto titleArea = getLocalBounds().removeFromTop(kTitleBarH);
    g.setColour(kTitleBg);
    g.fillRect(titleArea);

    g.setColour(juce::Colours::white);
    g.setFont(makeFont(18.0f, true));
    g.drawFittedText("ADVANCED PIANO SYNTH - FILTER, ADSR, LFO, SATURATION & LO-FI EFFECTS",
        titleArea, juce::Justification::centred, 1);

    auto area = getLocalBounds().withTrimmedTop(kTitleBarH).withTrimmedBottom(kKeyboardH).reduced(kPad);

    // Bordo della cornice waveform
    auto wfFrame = area.removeFromTop(kWaveformH + kWaveformLabelH + 8);
    g.setColour(kFrameBorder);
    g.drawRoundedRectangle(wfFrame.toFloat(), (float)kCorner, 1.2f);

    // ===== pannelli =====
    auto topRow = area.removeFromTop(area.getHeight() * 0.55f);
    const int cols = 6;
    int sectionW = topRow.getWidth() / cols;

    auto samplerArea = topRow.removeFromLeft(sectionW).reduced(5);
    auto filterArea = topRow.removeFromLeft(sectionW).reduced(5);
    auto ampAdsrArea = topRow.removeFromLeft(sectionW).reduced(5);
    auto filtAdsrArea = topRow.removeFromLeft(sectionW).reduced(5);
    auto lfoSatArea = topRow.removeFromLeft(sectionW).reduced(5);
    auto loopArea = topRow.reduced(5);

    auto bottomRow = area;
    int bottomW = bottomRow.getWidth() / 4;
    auto lofiArea = bottomRow.removeFromLeft(bottomW).reduced(5);
    auto randomArea = bottomRow.removeFromLeft(bottomW).reduced(5);
    auto creativeArea = bottomRow.removeFromLeft(bottomW).reduced(5);
    auto presetArea = bottomRow.reduced(5);

    auto fillPanel = [&](juce::Rectangle<int> r)
        {
            g.setColour(kPanel.withAlpha(0.90f));
            g.fillRoundedRectangle(r.toFloat(), (float)kCorner);
            g.setColour(kPanelBorder);
            g.drawRoundedRectangle(r.toFloat(), (float)kCorner, 1.5f);
        };

    // Tutte le sezioni con LO STESSO COLORE
    fillPanel(samplerArea);
    fillPanel(filterArea);
    fillPanel(ampAdsrArea);
    fillPanel(filtAdsrArea);
    fillPanel(lfoSatArea);
    fillPanel(loopArea);
    fillPanel(lofiArea);
    fillPanel(randomArea);
    fillPanel(creativeArea);
    fillPanel(presetArea);

    if (isDragActive)
    {
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.fillRect(getLocalBounds());
        g.setColour(juce::Colours::cyan.withAlpha(0.6f));
        g.drawRect(waveformDisplay.getBounds().expanded(4), 2);
    }
}

void ProgettoPlugInAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(kPad);
    area.removeFromTop(kTitleBarH);

    // Waveform
    auto wfBounds = area.removeFromTop(kWaveformH).reduced(6);
    waveformDisplay.setBounds(wfBounds);
    auto lab = area.removeFromTop(kWaveformLabelH);
    waveformLabel.setBounds(lab);

    // Area pannelli (sopra la tastiera)
    auto panels = area.withTrimmedBottom(kKeyboardH + 10);

    // Riga superiore (6 colonne)
    auto topRow = panels.removeFromTop(panels.getHeight() * 0.58f);
    const int cols = 6;
    const int sectionW = topRow.getWidth() / cols;

    // --- Sampler (Load WAV/AIFF) — centrato
    {
        auto samplerArea = topRow.removeFromLeft(sectionW).reduced(12);

        // Riga 1: bottone Load (centrato)
        {
            auto row = samplerArea.removeFromTop(34);
            loadButton.setBounds(row.withSizeKeepingCentre(180, 30));
        }

        samplerArea.removeFromTop(6); // gap

        // Riga 2: Mode + HOLD (ben separati)
        {
            auto row = samplerArea.removeFromTop(28).reduced(2);
            auto left = row.removeFromLeft(row.getWidth() * 0.65f);
            modeBox.setBounds(left.withTrimmedRight(10));
            row.removeFromLeft(10);
            holdButton.setBounds(row.removeFromRight(80));
        }

        samplerArea.removeFromTop(8); // gap

        // Riga 3: One-shot (centrato)
        {
            auto row = samplerArea.removeFromTop(24);
            oneShot.setBounds(row.withSizeKeepingCentre(110, 22));
        }

        samplerArea.removeFromTop(10); // gap

        // Righe 4–6: slider mapping (spazio per la TextBox a destra)
        const int h = 26;
        {
            auto r = samplerArea.removeFromTop(h);
            rootNote.setBounds(r.withTrimmedRight(70)); // lascia 70px virtuali per la TextBox
        }
        samplerArea.removeFromTop(6);
        {
            auto r = samplerArea.removeFromTop(h);
            coarse.setBounds(r.withTrimmedRight(70));
        }
        samplerArea.removeFromTop(6);
        {
            auto r = samplerArea.removeFromTop(h);
            fine.setBounds(r.withTrimmedRight(70));
        }
    }

    // --- Filter
    {
        auto filterArea = topRow.removeFromLeft(sectionW).reduced(12);
        filterTitle.setBounds(filterArea.removeFromTop(26));

        auto body = filterArea.removeFromTop(filterArea.getHeight() - 40);
        auto leftCol = body.removeFromLeft(body.getWidth() / 2).reduced(4);
        auto rightCol = body.reduced(4);

        cutoffLabel.setBounds(leftCol.removeFromTop(20));
        cutoffFrequencySlider.setBounds(leftCol);

        resonanceLabel.setBounds(rightCol.removeFromTop(20));
        resonanceSlider.setBounds(rightCol);

        auto modeRow = filterArea;
        filterModeLabel.setBounds(modeRow.removeFromLeft(80));
        filterModeBox.setBounds(modeRow);
    }

    // --- Amplitude ADSR (4 colonne)
    {
        auto ampAdsrArea = topRow.removeFromLeft(sectionW).reduced(12);
        adsrTitle.setBounds(ampAdsrArea.removeFromTop(26));

        auto as = ampAdsrArea.reduced(6);
        int w4 = as.getWidth() / 4;

        auto aA = as.removeFromLeft(w4); attackLabel.setBounds(aA.removeFromTop(18));  attackSlider.setBounds(aA);
        auto dA = as.removeFromLeft(w4); decayLabel.setBounds(dA.removeFromTop(18));   decaySlider.setBounds(dA);
        auto sA = as.removeFromLeft(w4); sustainLabel.setBounds(sA.removeFromTop(18)); sustainSlider.setBounds(sA);
        auto rA = as;                    releaseLabel.setBounds(rA.removeFromTop(18)); releaseSlider.setBounds(rA);
    }

    // --- Filter ADSR (5 colonne)
    {
        auto fAdsrArea = topRow.removeFromLeft(sectionW).reduced(12);
        filterAdsrTitle.setBounds(fAdsrArea.removeFromTop(26));

        auto fs = fAdsrArea.reduced(6);
        int w5 = fs.getWidth() / 5;

        auto fA = fs.removeFromLeft(w5); filterAttackLabel.setBounds(fA.removeFromTop(18));   filterAttackSlider.setBounds(fA);
        auto fD = fs.removeFromLeft(w5); filterDecayLabel.setBounds(fD.removeFromTop(18));    filterDecaySlider.setBounds(fD);
        auto fS = fs.removeFromLeft(w5); filterSustainLabel.setBounds(fS.removeFromTop(18));  filterSustainSlider.setBounds(fS);
        auto fR = fs.removeFromLeft(w5); filterReleaseLabel.setBounds(fR.removeFromTop(18));  filterReleaseSlider.setBounds(fR);
        auto fE = fs;                    filterEnvelopeAmountLabel.setBounds(fE.removeFromTop(18)); filterEnvelopeAmountSlider.setBounds(fE);
    }

    // --- LFO + SATURATION (Saturation in 2 colonne affiancate)
    {
        auto panel = topRow.removeFromLeft(sectionW).reduced(12);

        // ===== LFO =====
        lfoTitle.setBounds(panel.removeFromTop(24));
        {
            auto lfoRow = panel.removeFromTop(140); // fascia dedicata ai knob
            int colW = lfoRow.getWidth() / 4;

            auto c1 = lfoRow.removeFromLeft(colW).reduced(6);
            auto c2 = lfoRow.removeFromLeft(colW).reduced(6);
            auto c3 = lfoRow.removeFromLeft(colW).reduced(6);
            auto c4 = lfoRow.reduced(6);

            lfoFrequencyLabel.setBounds(c1.removeFromTop(18).withSizeKeepingCentre(c1.getWidth(), 18));
            lfoFrequencySlider.setBounds(c1);

            lfoFilterAmountLabel.setBounds(c2.removeFromTop(18).withSizeKeepingCentre(c2.getWidth(), 18));
            lfoFilterAmountSlider.setBounds(c2);

            lfoVolumeAmountLabel.setBounds(c3.removeFromTop(18).withSizeKeepingCentre(c3.getWidth(), 18));
            lfoVolumeAmountSlider.setBounds(c3);

            lfoPitchAmountLabel.setBounds(c4.removeFromTop(18).withSizeKeepingCentre(c4.getWidth(), 18));
            lfoPitchAmountSlider.setBounds(c4);
        }
        // Waveform subito sotto
        {
            const int rowH = 24;
            auto wfRow = panel.removeFromTop(rowH);
            const int labW = 72;
            auto labR = wfRow.removeFromLeft(labW);
            lfoWaveformLabel.setBounds(labR.reduced(0, 2));
            lfoWaveformBox.setBounds(wfRow.withSizeKeepingCentre(wfRow.getWidth(), 22));
        }
        panel.removeFromTop(6); // gap
        saturationTitle.setBounds(panel.removeFromTop(22));
        {
            auto satRow = panel.removeFromTop(96).reduced(6); // Drive | Mix affiancati
            auto left = satRow.removeFromLeft(satRow.getWidth() / 2).reduced(4);
            auto right = satRow.reduced(4);

            saturationDriveLabel.setBounds(left.removeFromTop(18).withSizeKeepingCentre(left.getWidth(), 18));
            saturationDriveSlider.setBounds(left);

            saturationMixLabel.setBounds(right.removeFromTop(18).withSizeKeepingCentre(right.getWidth(), 18));
            saturationMixSlider.setBounds(right);
        }

        
    }

    // --- LOOP (spazi più ampi e righe ben separate)
    {
        auto loopArea = topRow.reduced(12);

        // Riga 1: Load (sx) | Play/Stop (dx)
        {
            auto buttonsRow = loopArea.removeFromTop(32).reduced(2);
            auto left = buttonsRow.removeFromLeft(buttonsRow.getWidth() * 0.48f);
            loadLoopButton.setBounds(left.reduced(0, 2));
            buttonsRow.removeFromLeft(10);
            playLoopButton.setBounds(buttonsRow.reduced(0, 2));
        }

        loopArea.removeFromTop(8); // gap

        // Riga 2: Reverse | Loop Enable
        {
            auto toggles = loopArea.removeFromTop(22);
            reverseButton.setBounds(toggles.removeFromLeft(110));
            toggles.removeFromLeft(12);
            loopEnableButton.setBounds(toggles.removeFromLeft(140));
        }

        loopArea.removeFromTop(8); // gap

        // Riga 3: Loop Start (label + slider)
        loopStartLabel.setBounds(loopArea.removeFromTop(18));
        {
            auto r = loopArea.removeFromTop(24);
            loopStartSlider.setBounds(r.withTrimmedRight(70)); // spazio per TextBox
        }

        loopArea.removeFromTop(6); // gap

        // Riga 4: Loop End (label + slider)
        loopEndLabel.setBounds(loopArea.removeFromTop(18));
        {
            auto r = loopArea.removeFromTop(24);
            loopEndSlider.setBounds(r.withTrimmedRight(70));
        }

        loopArea.removeFromTop(10); // gap

        // Riga 5: Record | Overdub | Clear (tre colonne con gap)
        {
            auto r = loopArea.removeFromTop(30);
            int gap = 10;
            int third = (r.getWidth() - 2 * gap) / 3;
            recordLoopButton.setBounds(r.removeFromLeft(third));
            r.removeFromLeft(gap);
            overdubButton.setBounds(r.removeFromLeft(third));
            r.removeFromLeft(gap);
            clearLoopButton.setBounds(r.removeFromLeft(third));
        }

        loopArea.removeFromTop(6); // gap

        // Riga 6: Stato
        {
            auto status = loopArea.removeFromTop(18);
            loopStateLabel.setBounds(status.removeFromLeft(status.getWidth() / 2 - 4));
            loopLengthLabel.setBounds(status);
        }
    }

    // --------- Riga inferiore (3 pannelli): Lo-Fi | Random | Presets
    auto bottomRow = panels;
    const int bottomW = bottomRow.getWidth() / 4;

    // Lo-Fi — 3 colonne affiancate
    {
        auto lofiArea = bottomRow.removeFromLeft(bottomW).reduced(12);
        lofiSectionLabel.setBounds(lofiArea.removeFromTop(24));
        lofiToggle.setBounds(lofiArea.removeFromTop(22).withSizeKeepingCentre(180, 22));

        auto lofiGrid = lofiArea.reduced(6);
        const int colW = lofiGrid.getWidth() / 3;

        auto col1 = lofiGrid.removeFromLeft(colW).reduced(4);
        auto col2 = lofiGrid.removeFromLeft(colW).reduced(4);
        auto col3 = lofiGrid.reduced(4);

        noiseLabel.setBounds(col1.removeFromTop(18).withSizeKeepingCentre(120, 18));
        noiseMixSlider.setBounds(col1);

        crushLabel.setBounds(col2.removeFromTop(18).withSizeKeepingCentre(120, 18));
        bitCrushSlider.setBounds(col2);

        rateLabel.setBounds(col3.removeFromTop(18).withSizeKeepingCentre(120, 18));
        sampleRateSlider.setBounds(col3);
    }

    // Random — 2 colonne affiancate
    {
        auto randomArea = bottomRow.removeFromLeft(bottomW).reduced(12);
        randomSectionLabel.setBounds(randomArea.removeFromTop(24));
        randomPlaybackToggle.setBounds(randomArea.removeFromTop(22).withSizeKeepingCentre(200, 22));

        auto rGrid = randomArea.reduced(6);
        auto leftR = rGrid.removeFromLeft(rGrid.getWidth() / 2).reduced(4);
        auto rightR = rGrid.reduced(4);

        randomPlaybackLabel.setBounds(leftR.removeFromTop(18).withSizeKeepingCentre(130, 18));
        randomPlaybackSlider.setBounds(leftR);

        pitchVariationLabel.setBounds(rightR.removeFromTop(18).withSizeKeepingCentre(130, 18));
        pitchVariationSlider.setBounds(rightR);
    }

    // Creative — 3 colonne (Width | AutoPan | Stutter)
    {
        auto creativeArea = bottomRow.removeFromLeft(bottomW).reduced(12);
        creativeSectionLabel.setBounds(creativeArea.removeFromTop(24));
        creativeToggle.setBounds(creativeArea.removeFromTop(22).withSizeKeepingCentre(200, 22));

        auto grid = creativeArea.reduced(6);
        int colW = grid.getWidth() / 3;

        auto col1 = grid.removeFromLeft(colW).reduced(4);
        auto col2 = grid.removeFromLeft(colW).reduced(4);
        auto col3 = grid.reduced(4);

        // Col1: Stereo Width
        stereoWidthLabel.setBounds(col1.removeFromTop(18).withSizeKeepingCentre(120, 18));
        stereoWidthSlider.setBounds(col1);

        // Col2: AutoPan
        autopanRateLabel.setBounds(col2.removeFromTop(18).withSizeKeepingCentre(120, 18));
        autopanRateSlider.setBounds(col2.removeFromTop(col2.getHeight() / 2 - 8));
        autopanDepthLabel.setBounds(col2.removeFromTop(18).withSizeKeepingCentre(120, 18));
        autopanDepthSlider.setBounds(col2);

        // Col3: Stutter
        stutterRateLabel.setBounds(col3.removeFromTop(18).withSizeKeepingCentre(120, 18));
        stutterRateSlider.setBounds(col3.removeFromTop(col3.getHeight() / 2 - 8));
        stutterDepthLabel.setBounds(col3.removeFromTop(18).withSizeKeepingCentre(120, 18));
        stutterDepthSlider.setBounds(col3);
    }

    // Presets (padding + distanze morbide)
    {
        auto presetArea = bottomRow.reduced(12);
        presetSectionLabel.setBounds(presetArea.removeFromTop(24));

        presetLabel.setBounds(presetArea.removeFromTop(18));
        presetArea.removeFromTop(6); // gap

        presetComboBox.setBounds(presetArea.removeFromTop(28));
        presetArea.removeFromTop(10); // gap

        savePresetButton.setBounds(presetArea.removeFromTop(34));
        presetArea.removeFromTop(8); // gap
        loadPresetButton.setBounds(presetArea.removeFromTop(34));
    }

    // Tastiera + Reset
    auto kbArea = getLocalBounds().removeFromBottom(kKeyboardH).reduced(kPad);
    resetButton.setBounds(kbArea.removeFromLeft(110).reduced(6));
    keyboardComponent.setBounds(kbArea.reduced(4));

    // LFO
    fitRotaryInBounds(lfoFrequencySlider,        /*margin*/ 6, /*max*/ 86); // colonna stretta
    // Saturation
    fitRotaryInBounds(saturationDriveSlider, 6, 72);
    fitRotaryInBounds(saturationMixSlider, 6, 72);

    // Random Effects
    fitRotaryInBounds(randomPlaybackSlider, 8, 104);
    fitRotaryInBounds(pitchVariationSlider, 8, 104);

    // Creative FX
    fitRotaryInBounds(stereoWidthSlider, 8, 112, 44); // knob principale più grande
    fitRotaryInBounds(autopanRateSlider, 6, 68);
    fitRotaryInBounds(stutterRateSlider, 6, 68);
    fitRotaryInBounds(autopanDepthSlider, 6, 64);
    fitRotaryInBounds(stutterDepthSlider, 6, 64);

    // Lo-Fi (se vuoi rifinirli anche qui)
    fitRotaryInBounds(noiseMixSlider, 8, 96);
    fitRotaryInBounds(bitCrushSlider, 8, 96);
    fitRotaryInBounds(sampleRateSlider, 8, 96);
}

void ProgettoPlugInAudioProcessorEditor::buttonClicked(juce::Button* b)
{
    if (b == &loadButton)
    {
        fileChooser = std::make_unique<juce::FileChooser>("Select a sample", juce::File{}, "*.wav;*.aif;*.aiff");
        fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file.existsAsFile())
                    processor.loadSampleFromFile(file,
                        (int)rootNote.getValue(),
                        1,
                        127,
                        (int)coarse.getValue(),
                        (int)fine.getValue(),
                        oneShot.getToggleState());
                fileChooser.reset();
            });
    }
    else if (b == &oneShot)
    {
        applySampleMappingToAllSounds();
    }
    else if (b == &loadLoopButton)
    {
        fileChooser = std::make_unique<juce::FileChooser>("Select a sample for loop", juce::File{}, "*.wav;*.aif;*.aiff");
        fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file.existsAsFile())
                {
                    if (processor.loadLoopSample(file))
                    {
                        playLoopButton.setToggleState(true, juce::sendNotification);
                        double duration = processor.getLoopEnd();
                        loopStartSlider.setRange(0.0, duration, 0.01);
                        loopEndSlider.setRange(0.0, duration, 0.01);
                        loopEndSlider.setValue(duration);
                    }
                }
                fileChooser.reset();
            });
    }
    else if (b == &playLoopButton) { processor.setLoopPlaying(playLoopButton.getToggleState()); }
    else if (b == &recordLoopButton) { processor.startLoopRecording(); }
    else if (b == &overdubButton) { processor.startOverdub(); }
    else if (b == &clearLoopButton) { processor.clearLoop(); }
    else if (b == &resetButton)
    {
        processor.getSynth().clearSounds();
        processor.resetAllParameters();

        modeBox.setSelectedId((int)PlayMode::Poly + 1, juce::sendNotification);
        oneShot.setToggleState(false, juce::sendNotification);
        rootNote.setValue(69, juce::sendNotification);
        coarse.setValue(0, juce::sendNotification);
        fine.setValue(0, juce::sendNotification);

        applySampleMappingToAllSounds();
    }
}

void ProgettoPlugInAudioProcessorEditor::comboBoxChanged(juce::ComboBox* box)
{
    if (box == &filterModeBox)
        return;

    if (box == &modeBox)
    {
        PlayMode m = PlayMode::Poly;
        if (modeBox.getSelectedId() == (int)PlayMode::MonoLegato + 1)    m = PlayMode::MonoLegato;
        if (modeBox.getSelectedId() == (int)PlayMode::MonoRetrigger + 1) m = PlayMode::MonoRetrigger;
        processor.setPlayMode(m);
        return;
    }
}

void ProgettoPlugInAudioProcessorEditor::sliderValueChanged(juce::Slider* s)
{
    if (s == &rootNote || s == &coarse || s == &fine)
        applySampleMappingToAllSounds();
}

void ProgettoPlugInAudioProcessorEditor::applySampleMappingToAllSounds()
{
    auto& synth = processor.getSynth();

    const int rn = (int)rootNote.getValue();
    const int vmin = 1;
    const int vmax = 127;
    const int crs = (int)coarse.getValue();
    const int fcts = (int)fine.getValue();
    const bool os = oneShot.getToggleState();

    for (int i = 0; i < synth.getNumSounds(); ++i)
    {
        auto sndPtr = synth.getSound(i);
        if (auto* s = dynamic_cast<SamplerSound*>(sndPtr.get()))
        {
            s->midiRootNote = rn;
            s->minVelocity = juce::jlimit(1, 127, vmin);
            s->maxVelocity = juce::jlimit(1, 127, vmax);
            s->coarseSemitones = crs;
            s->fineCents = fcts;
            s->oneShot = os;
        }
    }
}

void ProgettoPlugInAudioProcessorEditor::timerCallback()
{
    // aggiornamento stato loop
    if (processor.getLoopState() == LoopState::Recording)        loopStateLabel.setText("State: Recording", juce::dontSendNotification);
    else if (processor.getLoopState() == LoopState::Overdubbing) loopStateLabel.setText("State: Overdub", juce::dontSendNotification);
    else if (processor.getLoopState() == LoopState::Playing)     loopStateLabel.setText("State: Playing", juce::dontSendNotification);
    else                                                         loopStateLabel.setText("State: Stopped", juce::dontSendNotification);

    loopLengthLabel.setText(juce::String("Length: ") + juce::String(processor.getLoopLength(), 2) + "s", juce::dontSendNotification);

    syncWaveformFromProcessor();

    //PC keybord MIDI
    int activeKeys = 0;
    for (auto it = keyStateMap.begin(); it != keyStateMap.end(); ++it)
    {
        if (it.getValue()) activeKeys++;
    }

    if (activeKeys > 0)
    {
        keyboardStatusLabel.setText("PC Keyboard: Playing - " + juce::String(activeKeys) + " keys active",
            juce::dontSendNotification);
        keyboardStatusLabel.setColour(juce::Label::textColourId, juce::Colours::yellow);
    }
    else
    {
        keyboardStatusLabel.setText("PC Keyboard: Ready - Use ASDFGHJ for notes",
            juce::dontSendNotification);
        keyboardStatusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
    }

    // Forza repaint della tastiera MIDI per aggiornare i tasti premuti
    keyboardComponent.repaint();
}

void ProgettoPlugInAudioProcessorEditor::updatePresetList()
{
    presetComboBox.clear();
    auto presets = processor.getPresetList();
    presetComboBox.addItemList(presets, 1);

    if (presets.size() > 0)
    {
        presetComboBox.setSelectedItemIndex(0);
    }
    else
    {
        presetComboBox.setText("No presets found", juce::dontSendNotification);
    }
}

void ProgettoPlugInAudioProcessorEditor::syncWaveformFromProcessor()
{
    const auto current = processor.getLoadedFileName();

    // Se il nome è cambiato (o se prima era vuoto), riallinea tutto
    if (current != lastShownFileName)
    {
        // collega/ri-collega il thumbnail del processor
        waveformDisplay.setAudioThumbnail(&processor.getThumbnail());

        // se stai visualizzando un loop, mostra l’area; altrimenti pulisci
        if (processor.isLoopEnabled() && processor.getLoopEnd() > processor.getLoopStart())
            waveformDisplay.setLoopRange(processor.getLoopStart(), processor.getLoopEnd());
        else
            waveformDisplay.setLoopRange(0.0, 0.0); // niente overlay

        // aggiorna la label
        waveformLabel.setText(current.isNotEmpty() ? current : "No file loaded",
            juce::dontSendNotification);

        lastShownFileName = current;
        repaint();
    }
}

bool ProgettoPlugInAudioProcessorEditor::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (auto& f : files)
        if (f.endsWithIgnoreCase(".wav") || f.endsWithIgnoreCase(".aif") || f.endsWithIgnoreCase(".aiff"))
            return true;
    return false;
}

void ProgettoPlugInAudioProcessorEditor::fileDragEnter(const juce::StringArray&, int, int)
{
    isDragActive = true;
    repaint();
}

void ProgettoPlugInAudioProcessorEditor::fileDragExit(const juce::StringArray&)
{
    isDragActive = false;
    repaint();
}

void ProgettoPlugInAudioProcessorEditor::filesDropped(const juce::StringArray& files, int x, int y)
{
    isDragActive = false;
    if (files.isEmpty()) return;

    juce::File file(files[0]);
    if (!file.existsAsFile()) return;

    // Dentro la waveform? => Loop
    const auto wfBounds = waveformDisplay.getBounds();
    const bool dropOnWaveform = wfBounds.contains(juce::Point<int>(x, y));

    if (dropOnWaveform)
    {
        if (processor.loadLoopSample(file))
        {
            playLoopButton.setToggleState(true, juce::sendNotification);
            const double duration = processor.getLoopEnd();
            loopStartSlider.setRange(0.0, duration, 0.01);
            loopEndSlider.setRange(0.0, duration, 0.01);
            loopEndSlider.setValue(duration);
            syncWaveformFromProcessor();
            repaint();
        }
        return;
    }

    // Altrove: Sampler (come il bottone "Load WAV/AIFF")
    processor.loadSampleFromFile(file,
        (int)rootNote.getValue(),
        1, 127,
        (int)coarse.getValue(),
        (int)fine.getValue(),
        oneShot.getToggleState());

    applySampleMappingToAllSounds();
    syncWaveformFromProcessor();
    repaint();
}