#pragma once
#include <juce_core/juce_core.h>

struct AtelierVersion
{
    int major = 1;
    int minor = 0;
    int patch = 0;

    static constexpr const char* kUnlockPasscode = "N6ATELIER";

    void incrementRollover()
    {
        ++patch;
        if (patch > 9)
        {
            patch = 0;
            ++minor;
            if (minor > 9)
            {
                minor = 0;
                ++major;
            }
        }
    }

    juce::String toString() const
    {
        return "v" + juce::String(major) + "." + juce::String(minor) + "." + juce::String(patch);
    }

    juce::String getVst3FileName() const
    {
        return "ATELIER " + toString() + ".vst3";
    }
};
