#include "AtelierFormat.h"

namespace
{
    void xorScramble(std::vector<uint8_t>& data)
    {
        for (size_t i = 0; i < data.size(); ++i)
            data[i] = static_cast<uint8_t>(data[i] ^ static_cast<uint8_t>(AtelierNmsArchive::kMaskByte + (i & 0x3F)));
    }
}

bool AtelierNmsArchive::saveToFile(const juce::File& targetFile,
                                   const juce::String& libraryCategory,
                                   const std::array<NmsGroupData, 12>& groups,
                                   juce::String& errorOut)
{
    juce::DynamicObject::Ptr root = new juce::DynamicObject();
    root->setProperty("format", "ATELIER_NMS");
    root->setProperty("category", libraryCategory);

    juce::Array<juce::var> groupArr;
    for (int g = 0; g < 12; ++g)
    {
        const auto& grp = groups[static_cast<size_t>(g)];
        juce::DynamicObject::Ptr gObj = new juce::DynamicObject();
        gObj->setProperty("index", g + 1);
        gObj->setProperty("name", grp.name);
        gObj->setProperty("volumeDb", grp.volumeDb);
        gObj->setProperty("bypassed", grp.bypassed);
        gObj->setProperty("purged", grp.purged);
        gObj->setProperty("linked", grp.linked);
        gObj->setProperty("fakeRR", grp.fakeRoundRobin);
        gObj->setProperty("fakeVel", grp.fakeVelocity);
        gObj->setProperty("attack", grp.attackSec);
        gObj->setProperty("decay", grp.decaySec);
        gObj->setProperty("sustain", grp.sustainLevel);
        gObj->setProperty("release", grp.releaseSec);

        juce::Array<juce::var> sampleArr;
        for (const auto& s : grp.samples)
        {
            juce::DynamicObject::Ptr sObj = new juce::DynamicObject();
            sObj->setProperty("name", s.name);
            sObj->setProperty("rootNote", s.rootNote);
            sObj->setProperty("lowNote", s.lowNote);
            sObj->setProperty("highNote", s.highNote);
            sObj->setProperty("loopStart", s.loopStartSec);
            sObj->setProperty("loopEnd", s.loopEndSec);
            sObj->setProperty("loopXfadeMs", s.loopCrossfadeMs);

            std::vector<uint8_t> protectedBytes = s.encodedAudioData;
            xorScramble(protectedBytes);
            juce::MemoryBlock mb(protectedBytes.data(), protectedBytes.size());
            sObj->setProperty("payloadB64", mb.toBase64Encoding());
            sampleArr.add(juce::var(sObj));
        }

        gObj->setProperty("samples", sampleArr);
        groupArr.add(juce::var(gObj));
    }

    root->setProperty("groups", groupArr);
    const juce::String jsonText = juce::JSON::toString(juce::var(root), true);
    const juce::MemoryBlock rawJson(jsonText.toUTF8(), jsonText.getNumBytesAsGNUStrLength());

    juce::MemoryOutputStream compressedStream;
    {
        juce::GZIPCompressorOutputStream gz(compressedStream, 9);
        gz.write(rawJson.getData(), rawJson.getSize());
        gz.flush();
    }

    std::vector<uint8_t> finalBytes(compressedStream.getSize());
    if (!finalBytes.empty())
        std::memcpy(finalBytes.data(), compressedStream.getData(), finalBytes.size());
    xorScramble(finalBytes);

    targetFile.deleteFile();
    juce::FileOutputStream out(targetFile);
    if (!out.openedOk())
    {
        errorOut = "Could not open .nms file for writing.";
        return false;
    }

    out.writeInt(static_cast<int>(kMagic));
    out.writeInt(static_cast<int>(finalBytes.size()));
    if (!finalBytes.empty())
        out.write(finalBytes.data(), finalBytes.size());
    out.flush();
    return true;
}

bool AtelierNmsArchive::loadFromFile(const juce::File& sourceFile,
                                     juce::String& libraryCategoryOut,
                                     std::array<NmsGroupData, 12>& groupsOut,
                                     juce::String& errorOut)
{
    if (!sourceFile.existsAsFile())
    {
        errorOut = ".nms file not found.";
        return false;
    }

    juce::FileInputStream in(sourceFile);
    if (!in.openedOk())
    {
        errorOut = "Unable to open .nms file.";
        return false;
    }

    const uint32_t magic = static_cast<uint32_t>(in.readInt());
    if (magic != kMagic)
    {
        errorOut = "Invalid .nms signature.";
        return false;
    }

    const int byteLen = in.readInt();
    if (byteLen <= 0)
    {
        errorOut = "Empty .nms payload.";
        return false;
    }

    std::vector<uint8_t> scrambled(static_cast<size_t>(byteLen));
    in.read(scrambled.data(), byteLen);
    xorScramble(scrambled);

    juce::MemoryInputStream gzIn(scrambled.data(), scrambled.size(), false);
    juce::GZIPDecompressorInputStream gzStream(gzIn);
    const juce::String jsonText = gzStream.readEntireStreamAsString();
    const juce::var parsed = juce::JSON::parse(jsonText);

    if (!parsed.isObject())
    {
        errorOut = "Corrupt .nms metadata.";
        return false;
    }

    libraryCategoryOut = parsed.getProperty("category", "General").toString();
    const juce::var groupsVar = parsed.getProperty("groups", juce::var());
    if (auto* arr = groupsVar.getArray())
    {
        for (int i = 0; i < juce::jmin(12, arr->size()); ++i)
        {
            const auto& gVar = arr->getReference(i);
            auto& gOut = groupsOut[static_cast<size_t>(i)];
            gOut.name = gVar.getProperty("name", "Group " + juce::String(i + 1)).toString();
            gOut.volumeDb = static_cast<float>(gVar.getProperty("volumeDb", 0.0));
            gOut.bypassed = static_cast<bool>(gVar.getProperty("bypassed", false));
            gOut.purged = static_cast<bool>(gVar.getProperty("purged", false));
            gOut.linked = static_cast<bool>(gVar.getProperty("linked", false));
            gOut.fakeRoundRobin = static_cast<float>(gVar.getProperty("fakeRR", 0.25));
            gOut.fakeVelocity = static_cast<float>(gVar.getProperty("fakeVel", 0.35));
            gOut.attackSec = static_cast<float>(gVar.getProperty("attack", 0.02));
            gOut.decaySec = static_cast<float>(gVar.getProperty("decay", 0.20));
            gOut.sustainLevel = static_cast<float>(gVar.getProperty("sustain", 0.85));
            gOut.releaseSec = static_cast<float>(gVar.getProperty("release", 0.35));
            gOut.samples.clear();

            if (auto* sArr = gVar.getProperty("samples", juce::var()).getArray())
            {
                for (int s = 0; s < sArr->size(); ++s)
                {
                    const auto& sVar = sArr->getReference(s);
                    NmsSampleEntry entry;
                    entry.name = sVar.getProperty("name", "Sample").toString();
                    entry.rootNote = static_cast<int>(sVar.getProperty("rootNote", 60));
                    entry.lowNote = static_cast<int>(sVar.getProperty("lowNote", 0));
                    entry.highNote = static_cast<int>(sVar.getProperty("highNote", 127));
                    entry.loopStartSec = static_cast<double>(sVar.getProperty("loopStart", 0.0));
                    entry.loopEndSec = static_cast<double>(sVar.getProperty("loopEnd", 0.0));
                    entry.loopCrossfadeMs = static_cast<float>(sVar.getProperty("loopXfadeMs", 15.0));

                    juce::MemoryBlock mb;
                    if (mb.fromBase64Encoding(sVar.getProperty("payloadB64", "").toString()))
                    {
                        entry.encodedAudioData.resize(mb.getSize());
                        if (!entry.encodedAudioData.empty())
                            std::memcpy(entry.encodedAudioData.data(), mb.getData(), mb.getSize());
                        xorScramble(entry.encodedAudioData);
                    }
                    gOut.samples.push_back(std::move(entry));
                }
            }
        }
    }

    return true;
}
