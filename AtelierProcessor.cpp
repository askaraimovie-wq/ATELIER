#include "AtelierProcessor.h"
#include "AtelierEditor.h"

AtelierAudioProcessor::AtelierAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    const char* defaultNames[12] = {
        "Sustain", "Spiccato", "Staccato", "Tremolo",
        "Trill", "Pizzicato", "Marcato", "Legato",
        "Col Legno", "Harmonics", "Sul Pont", "Sul Tasto"
    };

    for (int i = 0; i < 12; ++i)
    {
        groups[static_cast<size_t>(i)].name = defaultNames[i];
        groups[static_cast<size_t>(i)].bypassed = (i != 0);
    }

    auto docs = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
    libraryRootPath = docs.getChildFile("My Library").getFullPathName();
}

bool AtelierAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto mainOut = layouts.getMainOutputChannelSet();
    return mainOut == juce::AudioChannelSet::mono() || mainOut == juce::AudioChannelSet::stereo();
}

void AtelierAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    engineDsp.prepare(sampleRate, samplesPerBlock);
}

void AtelierAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn() && msg.getVelocity() > 0)
            engineDsp.triggerNote(msg.getNoteNumber(), msg.getFloatVelocity());
        else if (msg.isNoteOff() || (msg.isNoteOn() && msg.getVelocity() == 0))
            engineDsp.releaseNote(msg.getNoteNumber());
    }

    engineDsp.processBlock(buffer, mics, effects);
}

void AtelierAudioProcessor::setGroupName(int index, const juce::String& newName)
{
    if (index < 0 || index >= 12)
        return;
    const juce::String trimmed = newName.trim();
    if (trimmed.isNotEmpty())
        groups[static_cast<size_t>(index)].name = trimmed;
}

void AtelierAudioProcessor::selectArticulationGroup(int index)
{
    if (index < 0 || index >= 12)
        return;

    const bool linkOn = engineDsp.isLinkMode();
    if (!linkOn)
    {
        for (int i = 0; i < 12; ++i)
            groups[static_cast<size_t>(i)].bypassed = (i != index);
    }
    else
    {
        groups[static_cast<size_t>(index)].bypassed = !groups[static_cast<size_t>(index)].bypassed;
    }

    engineDsp.setActiveGroup(index, linkOn);
}

bool AtelierAudioProcessor::tryUnlockSourceWithCode(const juce::String& code)
{
    if (code.trim() == AtelierVersion::kUnlockPasscode)
    {
        sourceUnlocked = true;
        return true;
    }
    return false;
}

void AtelierAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::DynamicObject::Ptr state = new juce::DynamicObject();
    state->setProperty("themeIndex", activeThemeIndex);
    state->setProperty("uiScale", uiScaleFactor);
    state->setProperty("extendedCanvas", extendedCanvas);
    state->setProperty("sourceUnlocked", sourceUnlocked);
    state->setProperty("libraryRootPath", libraryRootPath);
    state->setProperty("activePresetName", activePresetName);
    state->setProperty("linkMode", engineDsp.isLinkMode());
    state->setProperty("activeGroup", engineDsp.getActiveGroup());

    juce::Array<juce::var> gNames;
    for (const auto& g : groups)
        gNames.add(g.name);
    state->setProperty("groupNames", gNames);

    const juce::String text = juce::JSON::toString(juce::var(state));
    const auto utf8 = text.toUTF8();
    destData.replaceAll(utf8.getAddress(), text.getNumBytesAsUTF8());
}

void AtelierAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (data == nullptr || sizeInBytes <= 0)
        return;

    const juce::String text = juce::String::fromUTF8(static_cast<const char*>(data), sizeInBytes);
    const juce::var parsed = juce::JSON::parse(text);
    if (!parsed.isObject())
        return;

    activeThemeIndex = juce::jlimit(0, 9, static_cast<int>(parsed.getProperty("themeIndex", 0)));
    uiScaleFactor = juce::jlimit(0.5f, 1.5f, static_cast<float>(parsed.getProperty("uiScale", 1.0)));
    extendedCanvas = static_cast<bool>(parsed.getProperty("extendedCanvas", false));
    sourceUnlocked = static_cast<bool>(parsed.getProperty("sourceUnlocked", true));
    libraryRootPath = parsed.getProperty("libraryRootPath", libraryRootPath).toString();
    activePresetName = parsed.getProperty("activePresetName", activePresetName).toString();

    const bool linkOn = static_cast<bool>(parsed.getProperty("linkMode", false));
    const int activeGrp = juce::jlimit(0, 11, static_cast<int>(parsed.getProperty("activeGroup", 0)));
    engineDsp.setActiveGroup(activeGrp, linkOn);

    if (auto* arr = parsed.getProperty("groupNames", juce::var()).getArray())
    {
        for (int i = 0; i < juce::jmin(12, arr->size()); ++i)
            setGroupName(i, arr->getReference(i).toString());
    }
}

juce::AudioProcessorEditor* AtelierAudioProcessor::createEditor()
{
    return new AtelierAudioProcessorEditor(*this);
}
