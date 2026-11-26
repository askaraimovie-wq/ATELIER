#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "AtelierProcessor.h"
#include <array>

class AtelierAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    private juce::Timer
{
public:
    explicit AtelierAudioProcessorEditor(AtelierAudioProcessor&);
    ~AtelierAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    void drawHeader(juce::Graphics&, juce::Rectangle<int> area);
    void drawBackgroundSketch(juce::Graphics&, juce::Rectangle<int> area);
    void drawMainPage(juce::Graphics&, juce::Rectangle<int> area);
    void drawMixPage(juce::Graphics&, juce::Rectangle<int> area);
    void drawSourcePage(juce::Graphics&, juce::Rectangle<int> area);
    void drawEffectsPage(juce::Graphics&, juce::Rectangle<int> area);
    void drawKeyboardAndVersion(juce::Graphics&, juce::Rectangle<int> area);
    void drawN6SettingsDrawer(juce::Graphics&, juce::Rectangle<int> area);

    AtelierAudioProcessor& proc;
    bool settingsDrawerOpen = false;
    int visibleArticulationCount = 1; // Auto-adjusts 1..12
    int draggingKeymapNote = -1;
    int keymapLowNote = 48;  // C3
    int keymapHighNote = 53; // F3
    float adsrAttackPt = 0.15f;
    float adsrReleasePt = 0.35f;

    juce::Rectangle<int> n6LogoBounds;
    juce::Rectangle<int> mainTabBounds;
    juce::Rectangle<int> mixTabBounds;
    juce::Rectangle<int> srcTabBounds;
    juce::Rectangle<int> fxTabBounds;
    juce::Rectangle<int> keymapAreaBounds;
    juce::Rectangle<int> adsrAreaBounds;
    std::array<juce::Rectangle<int>, 12> mainArticulationBoxes;
    std::array<juce::Rectangle<int>, 12> sourceGroupBoxes;
    std::array<juce::Rectangle<int>, 4> micRealFakeBoxes;
    std::array<juce::Rectangle<int>, 3> sourceVolKnobs;
    juce::Rectangle<int> linkGroupsButtonBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AtelierAudioProcessorEditor)
};
