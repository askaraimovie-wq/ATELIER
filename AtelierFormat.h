#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <array>
#include <vector>

struct NmsSampleEntry
{
    juce::String name;
    int rootNote = 60;
    int lowNote = 0;
    int highNote = 127;
    double loopStartSec = 0.0;
    double loopEndSec = 0.0;
    float loopCrossfadeMs = 15.0f;
    std::vector<uint8_t> encodedAudioData;
};

struct NmsGroupData
{
    juce::String name = "Group";
    float volumeDb = 0.0f;
    bool bypassed = false;
    bool purged = false;
    bool linked = false;
    float fakeRoundRobin = 0.25f;
    float fakeVelocity = 0.35f;
    float attackSec = 0.02f;
    float decaySec = 0.20f;
    float sustainLevel = 0.85f;
    float releaseSec = 0.35f;
    std::vector<NmsSampleEntry> samples;
};

class AtelierNmsArchive
{
public:
    static constexpr uint32_t kMagic = 0x4E4D5331; // "NMS1"
    static constexpr uint8_t kMaskByte = 0x5A;

    // Writes compressed/protected .nms container (prevents direct raw WAV extraction)
    static bool saveToFile(const juce::File& targetFile,
                           const juce::String& libraryCategory,
                           const std::array<NmsGroupData, 12>& groups,
                           juce::String& errorOut);

    // Reads metadata & protected sample blocks from .nms container
    static bool loadFromFile(const juce::File& sourceFile,
                           juce::String& libraryCategoryOut,
                           std::array<NmsGroupData, 12>& groupsOut,
                           juce::String& errorOut);
};
