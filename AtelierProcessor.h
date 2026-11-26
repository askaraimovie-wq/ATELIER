#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "AtelierVersion.h"
#include "AtelierFormat.h"
#include "AtelierDsp.h"
#include <array>

enum class AtelierPage
{
    Main = 0,
    Mix,
    Source,
    Effects
};

class AtelierAudioProcessor : public juce::AudioProcessor
{
public:
    AtelierAudioProcessor();
    ~AtelierAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "ATELIER"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 1.5; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    AtelierVersion versionInfo;
    AtelierPage currentPage = AtelierPage::Main;
    bool sourceUnlocked = true;
    int activeThemeIndex = 0;
    float uiScaleFactor = 1.0f;
    bool extendedCanvas = false;

    juce::String libraryRootPath;
    juce::String activePresetName = "Init Atelier";
    std::array<NmsGroupData, 12> groups;
    std::array<MicChannelState, 4> mics;
    EffectsState effects;
    AtelierEngineDsp engineDsp;

    void setGroupName(int index, const juce::String& newName);
    void selectArticulationGroup(int index);
    bool tryUnlockSourceWithCode(const juce::String& code);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AtelierAudioProcessor)
};
