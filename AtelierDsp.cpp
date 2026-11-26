#include "AtelierDsp.h"
#include <cmath>

void AtelierEngineDsp::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    const auto blockSize = static_cast<juce::uint32>(juce::jmax(1, samplesPerBlock));

    juce::dsp::ProcessSpec spec { currentSampleRate, blockSize, 2 };
    toneFilter.prepare(spec);
    reverb.prepare(spec);
}

void AtelierEngineDsp::setActiveGroup(int groupIndex, bool newLinkMode)
{
    activeGroup = juce::jlimit(0, 11, groupIndex);
    linkMode = newLinkMode;
}

void AtelierEngineDsp::setGroupLinked(int groupIndex, bool linked)
{
    groupLinked[static_cast<size_t>(juce::jlimit(0, 11, groupIndex))] = linked;
}

void AtelierEngineDsp::setSourceVol(int slotIndex, float gainLinear)
{
    sourceVols[static_cast<size_t>(juce::jlimit(0, 2, slotIndex))] = juce::jlimit(0.0f, 2.0f, gainLinear);
}

void AtelierEngineDsp::triggerNote(int midiNote, float velocity)
{
    noteHeld = true;
    std::uniform_real_distribution<float> dist(-0.04f, 0.04f);
    lastFakeRrDelta = dist(rng);
    lastFakeVelScale = juce::jlimit(0.1f, 1.5f, 0.25f + velocity * 1.15f);

    const float semitoneOffset = static_cast<float>(midiNote - 69) + lastFakeRrDelta;
    heldFreq = 440.0f * std::pow(2.0f, semitoneOffset / 12.0f);
    envLevel = juce::jmin(1.0f, envLevel + 0.65f * lastFakeVelScale);
}

void AtelierEngineDsp::releaseNote(int midiNote)
{
    juce::ignoreUnused(midiNote);
    noteHeld = false;
}

void AtelierEngineDsp::processBlock(juce::AudioBuffer<float>& buffer,
                                    const std::array<MicChannelState, 4>& mics,
                                    const EffectsState& fx)
{
    const int numSamples = buffer.getNumSamples();
    const int numCh = buffer.getNumChannels();
    if (numSamples <= 0 || numCh <= 0)
        return;

    buffer.clear();

    const float activeSourceMix = (sourceVols[0] + sourceVols[1] + sourceVols[2]) / 3.0f;
    const float phaseInc = static_cast<float>(juce::MathConstants<double>::twoPi * heldFreq / currentSampleRate);

    for (int i = 0; i < numSamples; ++i)
    {
        if (noteHeld)
            envLevel = juce::jmin(1.0f, envLevel + 0.002f);
        else
            envLevel = juce::jmax(0.0f, envLevel - 0.0015f);

        float sample = 0.0f;
        if (envLevel > 0.0001f)
        {
            sample = std::sin(heldPhase) * 0.12f * envLevel * lastFakeVelScale * activeSourceMix;
            heldPhase += phaseInc;
            if (heldPhase >= juce::MathConstants<float>::twoPi)
                heldPhase -= juce::MathConstants<float>::twoPi;
        }

        for (int ch = 0; ch < numCh; ++ch)
            buffer.setSample(ch, i, sample);
    }

    bool anySolo = false;
    for (const auto& m : mics)
        if (m.solo) anySolo = true;

    float micSumGain = 0.0f;
    for (const auto& m : mics)
    {
        if (m.mute || (anySolo && !m.solo))
            continue;
        float g = juce::Decibels::decibelsToGain(m.volumeDb);
        if (m.useFakeMic)
            g *= 1.05f;
        micSumGain += g;
    }
    micSumGain = juce::jlimit(0.0f, 4.0f, micSumGain * 0.25f);

    const float clampedCutoff = juce::jlimit(80.0f, static_cast<float>(currentSampleRate * 0.45), fx.filterCutoffHz);
    *toneFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(currentSampleRate, clampedCutoff);

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    toneFilter.process(ctx);

    const float satGain = juce::Decibels::decibelsToGain(fx.saturationDb);
    const float eqGain = juce::Decibels::decibelsToGain(fx.eqGainDb);
    const float totalGain = micSumGain * satGain * eqGain;

    for (int ch = 0; ch < numCh; ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            float v = data[i] * totalGain;
            if (fx.saturationDb > 0.1f)
                v = std::tanh(v * 1.4f);
            data[i] = v;
        }
    }

    juce::dsp::Reverb::Parameters revParams;
    revParams.wetLevel = juce::jlimit(0.0f, 1.0f, juce::Decibels::decibelsToGain(fx.reverbDb));
    revParams.dryLevel = 1.0f;
    revParams.roomSize = 0.55f;
    reverb.setParameters(revParams);
    reverb.process(ctx);
}
