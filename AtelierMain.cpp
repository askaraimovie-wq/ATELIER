#include "AtelierProcessor.h"

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AtelierAudioProcessor();
}
