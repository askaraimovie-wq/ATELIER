#include "AtelierEditor.h"
#include <cstdint>
#include <cstdlib>
#include <cmath>

namespace
{
    struct AtelierThemePalette
    {
        const char* name;
        juce::Colour bg;
        juce::Colour ink;
        juce::Colour accent;
        juce::Colour panel;
    };

    const std::array<AtelierThemePalette, 10> kThemes {{
        { "Paper Sketch",    juce::Colour(static_cast<uint32_t>(0xFFF6F1E3u)), juce::Colour(static_cast<uint32_t>(0xFF1A1A1Au)), juce::Colour(static_cast<uint32_t>(0xFF2B4C7Eu)), juce::Colour(static_cast<uint32_t>(0xFFECE5D3u)) },
        { "Vintage Parch",   juce::Colour(static_cast<uint32_t>(0xFFEBDCC0u)), juce::Colour(static_cast<uint32_t>(0xFF2C1E12u)), juce::Colour(static_cast<uint32_t>(0xFF8B4513u)), juce::Colour(static_cast<uint32_t>(0xFFE0CCA9u)) },
        { "Blueprint",       juce::Colour(static_cast<uint32_t>(0xFF0E2A47u)), juce::Colour(static_cast<uint32_t>(0xFFEAF6FFu)), juce::Colour(static_cast<uint32_t>(0xFF56B4E9u)), juce::Colour(static_cast<uint32_t>(0xFF153B61u)) },
        { "Charcoal Studio", juce::Colour(static_cast<uint32_t>(0xFF191B1Eu)), juce::Colour(static_cast<uint32_t>(0xFFF0F2F5u)), juce::Colour(static_cast<uint32_t>(0xFFD9822Bu)), juce::Colour(static_cast<uint32_t>(0xFF23272Cu)) },
        { "Warm Ivory",      juce::Colour(static_cast<uint32_t>(0xFFFAF6EEu)), juce::Colour(static_cast<uint32_t>(0xFF222222u)), juce::Colour(static_cast<uint32_t>(0xFF4A7C59u)), juce::Colour(static_cast<uint32_t>(0xFFF0EAE0u)) },
        { "Nocturne Brass",  juce::Colour(static_cast<uint32_t>(0xFF16151Cu)), juce::Colour(static_cast<uint32_t>(0xFFF5EFE6u)), juce::Colour(static_cast<uint32_t>(0xFFC99E48u)), juce::Colour(static_cast<uint32_t>(0xFF22202Bu)) },
        { "Slate Acoustic",  juce::Colour(static_cast<uint32_t>(0xFF242B33u)), juce::Colour(static_cast<uint32_t>(0xFFE8EEF2u)), juce::Colour(static_cast<uint32_t>(0xFF6BA3BEu)), juce::Colour(static_cast<uint32_t>(0xFF2E3741u)) },
        { "Sepia Folio",     juce::Colour(static_cast<uint32_t>(0xFFE4D5BCu)), juce::Colour(static_cast<uint32_t>(0xFF2B2118u)), juce::Colour(static_cast<uint32_t>(0xFF7A3E2Eu)), juce::Colour(static_cast<uint32_t>(0xFFD7C5A8u)) },
        { "Nordic Frost",    juce::Colour(static_cast<uint32_t>(0xFFF1F5F8u)), juce::Colour(static_cast<uint32_t>(0xFF1C242Bu)), juce::Colour(static_cast<uint32_t>(0xFF2E6F95u)), juce::Colour(static_cast<uint32_t>(0xFFE3EBF1u)) },
        { "Modern Obsidian", juce::Colour(static_cast<uint32_t>(0xFF121214u)), juce::Colour(static_cast<uint32_t>(0xFFFAFAFCu)), juce::Colour(static_cast<uint32_t>(0xFF9B5DE5u)), juce::Colour(static_cast<uint32_t>(0xFF1C1C21u)) }
    }};
}

AtelierAudioProcessorEditor::AtelierAudioProcessorEditor(AtelierAudioProcessor& p)
    : AudioProcessorEditor(&p), proc(p)
{
    const float scaledBase = 640.0f * proc.uiScaleFactor;
    const int baseSize = static_cast<int>(scaledBase);
    const int width = proc.extendedCanvas ? static_cast<int>(scaledBase * 1.25f) : baseSize;
    const int height = proc.extendedCanvas ? static_cast<int>(scaledBase * 1.12f) : baseSize;
    setSize(width, height);
    startTimerHz(20);
}

void AtelierAudioProcessorEditor::timerCallback()
{
    repaint();
}

void AtelierAudioProcessorEditor::paint(juce::Graphics& g)
{
    const auto& theme = kThemes[static_cast<size_t>(juce::jlimit(0, 9, proc.activeThemeIndex))];
    auto bounds = getLocalBounds();

    g.fillAll(theme.bg);
    drawBackgroundSketch(g, bounds);

    auto headerArea = bounds.removeFromTop(48);
    auto footerArea = bounds.removeFromBottom(88);

    drawHeader(g, headerArea);
    drawKeyboardAndVersion(g, footerArea);

    switch (proc.currentPage)
    {
        case AtelierPage::Main:    drawMainPage(g, bounds.reduced(12)); break;
        case AtelierPage::Mix:     drawMixPage(g, bounds.reduced(12)); break;
        case AtelierPage::Source:  drawSourcePage(g, bounds.reduced(12)); break;
        case AtelierPage::Effects: drawEffectsPage(g, bounds.reduced(12)); break;
    }

    if (settingsDrawerOpen)
        drawN6SettingsDrawer(g, getLocalBounds().reduced(24));
}

void AtelierAudioProcessorEditor::drawBackgroundSketch(juce::Graphics& g, juce::Rectangle<int> area)
{
    const auto& theme = kThemes[static_cast<size_t>(proc.activeThemeIndex)];
    g.saveState();
    g.setColour(theme.ink.withAlpha(0.08f));

    const float cx = static_cast<float>(area.getCentreX());
    const float cy = static_cast<float>(area.getCentreY()) + 30.0f;

    for (int row = 1; row <= 3; ++row)
    {
        const float r = 90.0f * static_cast<float>(row);
        juce::Path arc;
        arc.addCentredArc(cx, cy, r, r * 0.72f, 0.0f,
                          juce::MathConstants<float>::pi * 1.15f,
                          juce::MathConstants<float>::pi * 1.85f, true);
        g.strokePath(arc, juce::PathStrokeType(1.2f));
    }

    for (int p = -4; p <= 4; ++p)
    {
        const float pf = static_cast<float>(p);
        const float absPf = static_cast<float>(std::abs(p));
        const float px = cx + pf * 44.0f;
        const float py = cy - 45.0f + absPf * 6.0f;
        g.drawEllipse(px - 6.0f, py - 18.0f, 12.0f, 12.0f, 1.1f);
        g.drawLine(px, py - 6.0f, px, py + 14.0f, 1.1f);
        g.drawLine(px - 10.0f, py + 2.0f, px + 14.0f, py - 4.0f, 1.0f);
    }

    g.restoreState();
}

void AtelierAudioProcessorEditor::drawHeader(juce::Graphics& g, juce::Rectangle<int> area)
{
    const auto& theme = kThemes[static_cast<size_t>(proc.activeThemeIndex)];
    g.setColour(theme.panel.withAlpha(0.88f));
    g.fillRect(area);
    g.setColour(theme.ink);
    g.drawLine(0.0f, static_cast<float>(area.getBottom()), static_cast<float>(getWidth()), static_cast<float>(area.getBottom()), 1.5f);

    n6LogoBounds = area.removeFromLeft(56).reduced(6);
    g.setColour(theme.panel);
    g.fillRect(n6LogoBounds);
    g.setColour(theme.ink);
    g.drawRect(n6LogoBounds, 2);
    g.drawLine(static_cast<float>(n6LogoBounds.getX()), static_cast<float>(n6LogoBounds.getY() + 6),
               static_cast<float>(n6LogoBounds.getCentreX()), static_cast<float>(n6LogoBounds.getY() - 2), 1.4f);
    g.drawLine(static_cast<float>(n6LogoBounds.getCentreX()), static_cast<float>(n6LogoBounds.getY() - 2),
               static_cast<float>(n6LogoBounds.getRight()), static_cast<float>(n6LogoBounds.getY() + 6), 1.4f);
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    g.drawText("N6", n6LogoBounds, juce::Justification::centred);

    auto tabsArea = area.removeFromRight(proc.sourceUnlocked ? 228 : 170).reduced(6);
    const int tabW = proc.sourceUnlocked ? (tabsArea.getWidth() - 15) / 4 : (tabsArea.getWidth() - 10) / 3;

    auto drawTab = [&](juce::Rectangle<int>& r, const juce::String& label, AtelierPage page)
    {
        r = tabsArea.removeFromLeft(tabW);
        tabsArea.removeFromLeft(5);
        const bool active = (proc.currentPage == page);
        g.setColour(active ? theme.accent : theme.panel);
        g.fillRect(r);
        g.setColour(active ? juce::Colours::white : theme.ink);
        g.drawRect(r, 1);
        g.setFont(juce::Font(11.0f, juce::Font::bold));
        g.drawText(label, r, juce::Justification::centred);
    };

    drawTab(mainTabBounds, "MAIN", AtelierPage::Main);
    drawTab(mixTabBounds, "MIX", AtelierPage::Mix);
    if (proc.sourceUnlocked)
        drawTab(srcTabBounds, "SRC", AtelierPage::Source);
    else
        srcTabBounds = {};
    drawTab(fxTabBounds, "FX", AtelierPage::Effects);

    g.setColour(theme.ink);
    g.setFont(juce::Font(22.0f, juce::Font::bold));
    g.drawText("ATELIER", area, juce::Justification::centred);
}

void AtelierAudioProcessorEditor::drawMainPage(juce::Graphics& g, juce::Rectangle<int> area)
{
    const auto& theme = kThemes[static_cast<size_t>(proc.activeThemeIndex)];
    visibleArticulationCount = 0;
    for (int i = 0; i < 12; ++i)
        if (!proc.groups[static_cast<size_t>(i)].purged)
            ++visibleArticulationCount;

    visibleArticulationCount = juce::jlimit(1, 12, visibleArticulationCount);

    for (int i = 0; i < 12; ++i)
        mainArticulationBoxes[static_cast<size_t>(i)] = {};

    if (visibleArticulationCount == 1)
    {
        const int w = juce::jmin(260, area.getWidth() / 2);
        const int h = juce::jmin(140, area.getHeight() / 2);
        mainArticulationBoxes[0] = juce::Rectangle<int>(area.getCentreX() - w / 2, area.getCentreY() - h / 2, w, h);
    }
    else if (visibleArticulationCount == 2)
    {
        const int gap = 16;
        const int w = (area.getWidth() - gap * 3) / 2;
        const int h = juce::jmin(150, area.getHeight() / 2);
        const int y = area.getCentreY() - h / 2;
        mainArticulationBoxes[0] = juce::Rectangle<int>(area.getX() + gap, y, w, h);
        mainArticulationBoxes[1] = juce::Rectangle<int>(area.getX() + gap * 2 + w, y, w, h);
    }
    else
    {
        const int cols = (visibleArticulationCount <= 4) ? 2 : (visibleArticulationCount <= 9 ? 3 : 4);
        const int rows = (visibleArticulationCount + cols - 1) / cols;
        const int gap = 10;
        const int cellW = (area.getWidth() - gap * (cols + 1)) / cols;
        const int cellH = (area.getHeight() - gap * (rows + 1)) / rows;

        int placed = 0;
        for (int i = 0; i < 12 && placed < visibleArticulationCount; ++i)
        {
            if (proc.groups[static_cast<size_t>(i)].purged)
                continue;
            const int r = placed / cols;
            const int c = placed % cols;
            mainArticulationBoxes[static_cast<size_t>(i)] = juce::Rectangle<int>(
                area.getX() + gap + c * (cellW + gap),
                area.getY() + gap + r * (cellH + gap),
                cellW, cellH);
            ++placed;
        }
    }

    for (int i = 0; i < 12; ++i)
    {
        const auto box = mainArticulationBoxes[static_cast<size_t>(i)];
        if (box.isEmpty())
            continue;

        const bool isActive = !proc.groups[static_cast<size_t>(i)].bypassed;
        g.setColour(isActive ? theme.accent.withAlpha(0.9f) : theme.panel.withAlpha(0.85f));
        g.fillRect(box);
        g.setColour(theme.ink);
        g.drawRect(box, isActive ? 2 : 1);
        g.setColour(isActive ? juce::Colours::white : theme.ink);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText(proc.groups[static_cast<size_t>(i)].name, box, juce::Justification::centred);
    }
}

void AtelierAudioProcessorEditor::drawMixPage(juce::Graphics& g, juce::Rectangle<int> area)
{
    const auto& theme = kThemes[static_cast<size_t>(proc.activeThemeIndex)];
    const int cols = 2;
    const int rows = 2;
    const int gap = 14;
    const int boxW = (area.getWidth() - gap * (cols + 1)) / cols;
    const int boxH = (area.getHeight() - gap * (rows + 1)) / rows;

    for (int i = 0; i < 4; ++i)
    {
        const int r = i / cols;
        const int c = i % cols;
        juce::Rectangle<int> box(area.getX() + gap + c * (boxW + gap),
                                 area.getY() + gap + r * (boxH + gap),
                                 boxW, boxH);

        g.setColour(theme.panel.withAlpha(0.9f));
        g.fillRect(box);
        g.setColour(theme.ink);
        g.drawRect(box, 2);

        auto inner = box.reduced(10);
        g.setFont(juce::Font(13.0f, juce::Font::bold));
        g.drawText("MIC / SPKR " + juce::String(i + 1), inner.removeFromTop(24), juce::Justification::centredLeft);

        auto faderTrack = inner.removeFromTop(28).reduced(4, 6);
        g.drawRect(faderTrack, 1);
        const float normVol = juce::jlimit(0.0f, 1.0f, (proc.mics[static_cast<size_t>(i)].volumeDb + 24.0f) / 36.0f);
        const int fillW = static_cast<int>(static_cast<float>(faderTrack.getWidth()) * normVol);
        auto fillR = faderTrack.withWidth(fillW);
        g.setColour(theme.accent);
        g.fillRect(fillR);

        g.setColour(theme.ink);
        g.setFont(juce::Font(11.0f));
        g.drawText("Vol: " + juce::String(proc.mics[static_cast<size_t>(i)].volumeDb, 1) + " dB",
                   inner.removeFromTop(20), juce::Justification::centredLeft);
        g.drawText("Pan: " + juce::String(proc.mics[static_cast<size_t>(i)].pan, 2)
                   + (proc.mics[static_cast<size_t>(i)].mute ? "  [MUTE]" : "")
                   + (proc.mics[static_cast<size_t>(i)].solo ? "  [SOLO]" : ""),
                   inner.removeFromTop(20), juce::Justification::centredLeft);
    }
}

void AtelierAudioProcessorEditor::drawSourcePage(juce::Graphics& g, juce::Rectangle<int> area)
{
    if (!proc.sourceUnlocked)
    {
        drawMainPage(g, area);
        return;
    }

    const auto& theme = kThemes[static_cast<size_t>(proc.activeThemeIndex)];
    g.setColour(theme.ink);
    g.setFont(juce::Font(11.0f, juce::Font::bold));

    auto topBar = area.removeFromTop(26);
    g.drawText("Preset: " + proc.activePresetName + " | Root: " + proc.libraryRootPath,
               topBar.removeFromLeft(topBar.getWidth() - 120), juce::Justification::centredLeft);
    linkGroupsButtonBounds = topBar.removeFromRight(110);
    g.setColour(proc.engineDsp.isLinkMode() ? theme.accent : theme.panel);
    g.fillRect(linkGroupsButtonBounds);
    g.setColour(proc.engineDsp.isLinkMode() ? juce::Colours::white : theme.ink);
    g.drawRect(linkGroupsButtonBounds, 1);
    g.drawText(proc.engineDsp.isLinkMode() ? "LINK: ON" : "LINK: OFF", linkGroupsButtonBounds, juce::Justification::centred);

    area.removeFromTop(6);

    auto groupStrip = area.removeFromTop(54);
    const int gW = (groupStrip.getWidth() - 11 * 4) / 12;
    for (int i = 0; i < 12; ++i)
    {
        sourceGroupBoxes[static_cast<size_t>(i)] = juce::Rectangle<int>(
            groupStrip.getX() + i * (gW + 4), groupStrip.getY(), gW, groupStrip.getHeight());
        const auto box = sourceGroupBoxes[static_cast<size_t>(i)];
        const bool isSel = (proc.engineDsp.getActiveGroup() == i);
        g.setColour(isSel ? theme.accent : theme.panel);
        g.fillRect(box);
        g.setColour(theme.ink);
        g.drawRect(box, 1);
        g.setColour(isSel ? juce::Colours::white : theme.ink);
        g.setFont(juce::Font(9.5f, juce::Font::bold));
        g.drawText("G" + juce::String(i + 1) + " " + proc.groups[static_cast<size_t>(i)].name,
                   box.reduced(2), juce::Justification::centred);
    }

    area.removeFromTop(8);

    auto midControls = area.removeFromTop(64);
    for (int v = 0; v < 3; ++v)
    {
        sourceVolKnobs[static_cast<size_t>(v)] = midControls.removeFromLeft(68).reduced(4);
        auto kb = sourceVolKnobs[static_cast<size_t>(v)];
        g.setColour(theme.ink);
        g.drawEllipse(kb.toFloat(), 1.5f);
        g.setFont(juce::Font(10.0f));
        g.drawText("Vol " + juce::String(v + 1), kb, juce::Justification::centred);
    }

    auto micToggleRow = midControls.reduced(6);
    const int mW = (micToggleRow.getWidth() - 18) / 4;
    for (int m = 0; m < 4; ++m)
    {
        micRealFakeBoxes[static_cast<size_t>(m)] = juce::Rectangle<int>(
            micToggleRow.getX() + m * (mW + 6), micToggleRow.getY() + 10, mW, 34);
        const auto mb = micRealFakeBoxes[static_cast<size_t>(m)];
        const bool isFake = proc.mics[static_cast<size_t>(m)].useFakeMic;
        g.setColour(isFake ? theme.accent : theme.panel);
        g.fillRect(mb);
        g.setColour(isFake ? juce::Colours::white : theme.ink);
        g.drawRect(mb, 1);
        g.setFont(juce::Font(9.5f, juce::Font::bold));
        g.drawText("M" + juce::String(m + 1) + (isFake ? " FAKE" : " REAL"), mb, juce::Justification::centred);
    }

    area.removeFromTop(8);

    auto waveAdsrRow = area.removeFromTop(105);
    auto waveBox = waveAdsrRow.removeFromLeft(waveAdsrRow.getWidth() / 2 - 6);
    waveAdsrRow.removeFromLeft(12);
    adsrAreaBounds = waveAdsrRow;

    g.setColour(theme.panel.withAlpha(0.85f));
    g.fillRect(waveBox);
    g.fillRect(adsrAreaBounds);
    g.setColour(theme.ink);
    g.drawRect(waveBox, 1);
    g.drawRect(adsrAreaBounds, 1);
    g.drawText("WAVE + LOOP XFADE", waveBox.reduced(6), juce::Justification::topLeft);

    const int ax = adsrAreaBounds.getX() + 12;
    const int ay = adsrAreaBounds.getBottom() - 16;
    const int aw = adsrAreaBounds.getWidth() - 24;
    const int ah = adsrAreaBounds.getHeight() - 28;
    const float awf = static_cast<float>(aw);
    const float ahf = static_cast<float>(ah);
    const float pAtkX = static_cast<float>(ax) + adsrAttackPt * awf * 0.4f;
    const float pAtkY = static_cast<float>(ay) - ahf;
    const float pRelX = static_cast<float>(ax) + awf * (0.5f + adsrReleasePt * 0.45f);
    const float pRelY = static_cast<float>(ay);

    juce::Path envPath;
    envPath.startNewSubPath(static_cast<float>(ax), static_cast<float>(ay));
    envPath.lineTo(pAtkX, pAtkY);
    envPath.lineTo(pRelX, static_cast<float>(ay) - ahf * 0.7f);
    envPath.lineTo(pRelX, pRelY);
    g.setColour(theme.accent);
    g.strokePath(envPath, juce::PathStrokeType(2.0f));
    g.fillEllipse(pAtkX - 5.0f, pAtkY - 5.0f, 10.0f, 10.0f);
    g.fillEllipse(pRelX - 5.0f, pRelY - 5.0f, 10.0f, 10.0f);

    area.removeFromTop(8);

    keymapAreaBounds = area;
    g.setColour(theme.panel.withAlpha(0.85f));
    g.fillRect(keymapAreaBounds);
    g.setColour(theme.ink);
    g.drawRect(keymapAreaBounds, 1);
    g.drawText("KEYMAP RANGE: Note " + juce::String(keymapLowNote) + " - " + juce::String(keymapHighNote) + " (Drag)",
               keymapAreaBounds.reduced(8), juce::Justification::centred);
}

void AtelierAudioProcessorEditor::drawEffectsPage(juce::Graphics& g, juce::Rectangle<int> area)
{
    const auto& theme = kThemes[static_cast<size_t>(proc.activeThemeIndex)];
    const int cols = 2;
    const int rows = 3;
    const int gap = 8;
    const int boxW = (area.getWidth() - gap * (cols + 1)) / cols;
    const int boxH = (area.getHeight() - gap * (rows + 1)) / rows;

    struct FxInfo
    {
        const char* title;
        juce::String readout;
    };

    const std::array<FxInfo, 6> fxList {{
        { "EQ",         juce::String(proc.effects.eqGainDb, 1) + " dB" },
        { "COMPRESSOR", juce::String(proc.effects.compReductionDb, 1) + " dB GR" },
        { "SATURATION", juce::String(proc.effects.saturationDb, 1) + " dB" },
        { "REVERB",     juce::String(proc.effects.reverbDb, 1) + " dB" },
        { "WIDTH / PAN",juce::String(proc.effects.widthDb, 1) + " dB" },
        { "FILTER",     juce::String(proc.effects.filterCutoffHz, 0) + " Hz" }
    }};

    for (int i = 0; i < 6; ++i)
    {
        const int r = i / cols;
        const int c = i % cols;
        juce::Rectangle<int> box(area.getX() + gap + c * (boxW + gap),
                                 area.getY() + gap + r * (boxH + gap),
                                 boxW, boxH);
        g.setColour(theme.panel.withAlpha(0.88f));
        g.fillRect(box);
        g.setColour(theme.ink);
        g.drawRect(box, 1);
        g.setFont(juce::Font(11.0f, juce::Font::bold));
        g.drawText(fxList[static_cast<size_t>(i)].title, box.reduced(8, 4), juce::Justification::topLeft);
        g.drawText(fxList[static_cast<size_t>(i)].readout, box.reduced(8, 4), juce::Justification::topRight);

        auto graph = box.reduced(10, 22);
        g.setColour(theme.accent);
        g.drawLine(static_cast<float>(graph.getX()), static_cast<float>(graph.getCentreY()),
                   static_cast<float>(graph.getRight()), static_cast<float>(graph.getCentreY()) - (i % 2 == 0 ? 4.0f : -4.0f), 1.8f);
    }
}

void AtelierAudioProcessorEditor::drawKeyboardAndVersion(juce::Graphics& g, juce::Rectangle<int> area)
{
    const auto& theme = kThemes[static_cast<size_t>(proc.activeThemeIndex)];
    auto verRow = area.removeFromBottom(18);
    auto kbArea = area.reduced(8, 2);

    const int numWhiteKeys = 21;
    const float wkW = static_cast<float>(kbArea.getWidth()) / static_cast<float>(numWhiteKeys);

    for (int k = 0; k < numWhiteKeys; ++k)
    {
        juce::Rectangle<float> wk(static_cast<float>(kbArea.getX()) + static_cast<float>(k) * wkW,
                                  static_cast<float>(kbArea.getY()),
                                  wkW, static_cast<float>(kbArea.getHeight()));
        g.setColour(juce::Colours::white);
        g.fillRect(wk);
        g.setColour(theme.ink);
        g.drawRect(wk, 1.0f);
    }

    g.setColour(theme.ink);
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText(proc.versionInfo.toString(), verRow.removeFromRight(70).reduced(4, 0), juce::Justification::centredRight);
}

void AtelierAudioProcessorEditor::drawN6SettingsDrawer(juce::Graphics& g, juce::Rectangle<int> area)
{
    const auto& theme = kThemes[static_cast<size_t>(proc.activeThemeIndex)];
    g.setColour(theme.panel);
    g.fillRect(area);
    g.setColour(theme.ink);
    g.drawRect(area, 2);

    auto inner = area.reduced(16);
    g.setFont(juce::Font(15.0f, juce::Font::bold));
    g.drawText("N6 SETTINGS — THEMES (10), SIZE & SOURCE ACCESS", inner.removeFromTop(28), juce::Justification::centredLeft);

    const auto& curTheme = kThemes[static_cast<size_t>(proc.activeThemeIndex)];
    g.setFont(juce::Font(12.0f));
    g.drawText("Active Theme (" + juce::String(proc.activeThemeIndex + 1) + "/10): " + curTheme.name,
               inner.removeFromTop(24), juce::Justification::centredLeft);
    const int scalePct = static_cast<int>(proc.uiScaleFactor * 100.0f);
    g.drawText("UI Scale: " + juce::String(scalePct) + "% | Extended Canvas: "
               + (proc.extendedCanvas ? "ON" : "OFF"),
               inner.removeFromTop(24), juce::Justification::centredLeft);
    g.drawText("SOURCE Access: " + juce::String(proc.sourceUnlocked ? "UNLOCKED" : "LOCKED")
               + " (Click inside drawer to cycle Theme / Scale)",
               inner.removeFromTop(24), juce::Justification::centredLeft);
}

void AtelierAudioProcessorEditor::resized()
{
}

void AtelierAudioProcessorEditor::mouseDown(const juce::MouseEvent& e)
{
    const auto pos = e.getPosition();

    if (n6LogoBounds.contains(pos))
    {
        settingsDrawerOpen = !settingsDrawerOpen;
        repaint();
        return;
    }

    if (settingsDrawerOpen)
    {
        proc.activeThemeIndex = (proc.activeThemeIndex + 1) % 10;
        if (e.mods.isRightButtonDown())
            proc.extendedCanvas = !proc.extendedCanvas;
        repaint();
        return;
    }

    if (mainTabBounds.contains(pos)) { proc.currentPage = AtelierPage::Main; repaint(); return; }
    if (mixTabBounds.contains(pos))  { proc.currentPage = AtelierPage::Mix; repaint(); return; }
    if (proc.sourceUnlocked && srcTabBounds.contains(pos)) { proc.currentPage = AtelierPage::Source; repaint(); return; }
    if (fxTabBounds.contains(pos))   { proc.currentPage = AtelierPage::Effects; repaint(); return; }

    if (proc.currentPage == AtelierPage::Main)
    {
        for (int i = 0; i < 12; ++i)
        {
            if (!mainArticulationBoxes[static_cast<size_t>(i)].isEmpty()
                && mainArticulationBoxes[static_cast<size_t>(i)].contains(pos))
            {
                proc.selectArticulationGroup(i);
                repaint();
                return;
            }
        }
    }
    else if (proc.currentPage == AtelierPage::Source && proc.sourceUnlocked)
    {
        if (linkGroupsButtonBounds.contains(pos))
        {
            proc.engineDsp.setActiveGroup(proc.engineDsp.getActiveGroup(), !proc.engineDsp.isLinkMode());
            repaint();
            return;
        }

        for (int i = 0; i < 12; ++i)
        {
            if (sourceGroupBoxes[static_cast<size_t>(i)].contains(pos))
            {
                proc.selectArticulationGroup(i);
                if (e.mods.isLeftButtonDown() && e.getNumberOfClicks() == 2)
                {
                    auto* dlg = new juce::AlertWindow("Rename Group " + juce::String(i + 1),
                                                        "Enter new articulation/group name:",
                                                        juce::MessageBoxIconType::NoIcon);
                    dlg->addTextEditor("name", proc.groups[static_cast<size_t>(i)].name);
                    dlg->addButton("Save", 1);
                    dlg->addButton("Cancel", 0);
                    dlg->enterModalState(true, juce::ModalCallbackFunction::create([this, i, dlg](int res)
                    {
                        if (res == 1)
                            proc.setGroupName(i, dlg->getTextEditorContents("name"));
                        delete dlg;
                        repaint();
                    }), true);
                }
                repaint();
                return;
            }
        }

        for (int m = 0; m < 4; ++m)
        {
            if (micRealFakeBoxes[static_cast<size_t>(m)].contains(pos))
            {
                proc.mics[static_cast<size_t>(m)].useFakeMic = !proc.mics[static_cast<size_t>(m)].useFakeMic;
                repaint();
                return;
            }
        }

        if (keymapAreaBounds.contains(pos))
        {
            const float ratio = juce::jlimit(0.0f, 1.0f,
                static_cast<float>(pos.x - keymapAreaBounds.getX()) / static_cast<float>(juce::jmax(1, keymapAreaBounds.getWidth())));
            keymapHighNote = juce::jlimit(keymapLowNote + 1, 108, 24 + static_cast<int>(ratio * 84.0f));
            draggingKeymapNote = keymapHighNote;
            repaint();
        }
    }
}

void AtelierAudioProcessorEditor::mouseDrag(const juce::MouseEvent& e)
{
    const auto pos = e.getPosition();

    if (proc.currentPage == AtelierPage::Source && proc.sourceUnlocked)
    {
        if (draggingKeymapNote >= 0 && keymapAreaBounds.getWidth() > 0)
        {
            const float ratio = juce::jlimit(0.0f, 1.0f,
                static_cast<float>(pos.x - keymapAreaBounds.getX()) / static_cast<float>(keymapAreaBounds.getWidth()));
            keymapHighNote = juce::jlimit(keymapLowNote + 1, 108, 24 + static_cast<int>(ratio * 84.0f));
            repaint();
        }

        if (adsrAreaBounds.contains(pos))
        {
            adsrAttackPt = juce::jlimit(0.02f, 1.0f,
                static_cast<float>(pos.x - adsrAreaBounds.getX()) / static_cast<float>(juce::jmax(1, adsrAreaBounds.getWidth())));
            adsrReleasePt = juce::jlimit(0.05f, 1.0f,
                1.0f - static_cast<float>(pos.y - adsrAreaBounds.getY()) / static_cast<float>(juce::jmax(1, adsrAreaBounds.getHeight())));
            repaint();
        }
    }
}

void AtelierAudioProcessorEditor::mouseUp(const juce::MouseEvent&)
{
    draggingKeymapNote = -1;
}
