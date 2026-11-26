#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <random>

struct MicChannelState
{
    float volumeDb = 0.0f;
    float pan = 0.0f;
    bool mute = false;
    bool solo = false;
    bool useFakeMic = false;
};

struct EffectsState
{
    float eqGainDb = 0.0f;
    float compReductionDb = 0.0f;
    float saturationDb = 0.0f;
    float reverbDb = -18.0f;
    float widthDb = 0.0f;
    float filterCutoffHz = 12000.0f;
};

class AtelierEngineDsp
{
public:
    void prepare(double sampleRate, int samplesPerBlock);
    void setActiveGroup(int groupIndex, bool linkMode);
    void setGroupLinked(int groupIndex, bool linked);
    void setSourceVol(int slotIndex, float gainLinear);
    void triggerNote(int midiNote, float velocity);
    void releaseNote(int midiNote);
    void processBlock(juce::AudioBuffer<float>& buffer,
                      const std::array<MicChannelState, 4>& mics,
                      const EffectsState& fx);

    int getActiveGroup() const { return activeGroup; }
    bool isLinkMode() const { return linkMode; }
    bool isGroupLinked(int idx) const { return groupLinked[static_cast<size_t>(juce::jlimit(0, 11, idx))]; }
    float getSourceVol(int idx) const { return sourceVols[static_cast<size_t>(juce::jlimit(0, 2, idx))]; }
    float getLastFakeRrDelta() const { return lastFakeRrDelta; }
    float getLastFakeVelScale() const { return lastFakeVelScale; }

private:
    double currentSampleRate = 44100.0;
    int activeGroup = 0;
    bool linkMode = false;
    std::array<bool, 12> groupLinked {};
    std::array<float, 3> sourceVols { 1.0f, 1.0f, 1.0f };

    float heldPhase = 0.0f;
    float heldFreq = 220.0f;
    float envLevel = 0.0f;
    bool noteHeld = false;
    float lastFakeRrDelta = 0.0f;
    float lastFakeVelScale = 1.0f;

    std::mt19937 rng { 42u };
    juce::dsp::IIR::Filter<float> toneFilter;
    juce::dsp::Reverb reverb;
};
