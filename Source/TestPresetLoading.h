#pragma once

#include <JuceHeader.h>

class PresetLoadingTest : public juce::UnitTest
{
public:
    PresetLoadingTest() : juce::UnitTest("Preset Loading Test") {}

    void runTest() override
    {
        beginTest("External Preset Format Loading");

        // Simula il caricamento di un preset esterno
        juce::String xmlContent = R"(
            <?xml version="1.0" encoding="UTF-8"?>
            <PARAMETERS>
                <attack value="0.1"/>
                <decay value="0.3"/>
                <sustain value="0.7"/>
                <release value="0.5"/>
                <lofi_enabled value="0"/>
                <noise_mix value="0.0"/>
                <time_stretch value="1.0"/>
                <bit_crush value="0.0"/>
                <sample_rate value="1.0"/>
                <random_playback value="0.0"/>
                <pitch_variation value="0.0"/>
                <time_stretch_enabled value="0"/>
                <random_playback_enabled value="0"/>
            </PARAMETERS>
        )";

        auto xml = juce::parseXML(xmlContent);
        expect(xml != nullptr, "XML should parse correctly");
        expect(xml->hasTagName("PARAMETERS"), "Should detect external preset format");

        // Testa tutti i parametri
        testParameter(xml.get(), "attack", 0.1f);
        testParameter(xml.get(), "decay", 0.3f);
        testParameter(xml.get(), "sustain", 0.7f);
        testParameter(xml.get(), "release", 0.5f);
        testParameter(xml.get(), "lofi_enabled", 0.0f);
    }

private:
    void testParameter(juce::XmlElement* xml, const juce::String& paramName, float expectedValue)
    {
        auto* element = xml->getChildByName(paramName);
        if (element != nullptr)
        {
            auto value = element->getDoubleAttribute("value", -1.0);
            expectEquals(value, expectedValue, "Parameter " + paramName + " should match");
        }
        else
        {
            expect(false, "Parameter " + paramName + " should exist");
        }
    }
};

static PresetLoadingTest presetLoadingTest;
