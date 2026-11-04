#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Params.h"
#include "Presets.h"
#include "Localisation.h"
#include "Theme.h"

// =============================================================================
//  ParamCompare：プリセットの初期値と、いま触っている値を並べて比較する表
//  変えたパラメーターはオレンジで強調し、差分（＋/−）も表示する。
// =============================================================================
class ParamCompare : public juce::Component
{
public:
    void setLanguage (Lang l)
    {
        if (l != lang) { lang = l; repaint(); }
    }

    void update (int presetIndex, const juce::AudioProcessorValueTreeState& apvts, double bpm)
    {
        if (! juce::isPositiveAndBelow (presetIndex, (int) factoryPresets().size()))
            return;

        const auto& pr = factoryPresets()[(size_t) presetIndex];
        auto cur = [&apvts] (const char* id) { return apvts.getRawParameterValue (id)->load(); };

        std::vector<Row> next;
        auto addChoice = [&] (const char* name, const juce::StringArray& names, int a, int b)
        {
            next.push_back ({ name, names[a], names[b], {}, a != b });
        };
        auto addNum = [&] (const char* name, float a, float b, int decimals, const juce::String& unit)
        {
            const auto sa = juce::String (a, decimals) + unit;
            const auto sb = juce::String (b, decimals) + unit;
            const bool changed = sa != sb;
            juce::String diff;
            if (changed)
            {
                const float d = b - a;
                diff = (d > 0 ? "+" : "") + juce::String (d, decimals) + unit;
            }
            next.push_back ({ name, sa, sb, diff, changed });
        };

        addChoice ("Mode",      modeNames(), (int) pr.mode, (int) cur (ParamID::mode));
        addChoice ("Sync L",    syncNames(), pr.syncL,      (int) cur (ParamID::syncL));
        // Time は「実際に鳴っている ms」で比べる（SYNC 中はホストのテンポから計算）
        auto effMs = [bpm] (int sync, float ms) { return (float) resolveTimeMs (sync, ms, bpm); };
        addNum    ("Time L",    effMs (pr.syncL, pr.timeL), effMs ((int) cur (ParamID::syncL), cur (ParamID::timeL)), 1, " ms");
        addChoice ("Sync R",    syncNames(), pr.syncR,      (int) cur (ParamID::syncR));
        addNum    ("Time R",    effMs (pr.syncR, pr.timeR), effMs ((int) cur (ParamID::syncR), cur (ParamID::timeR)), 1, " ms");
        addNum    ("Feedback",  pr.feedbackPct, cur (ParamID::feedback), 1, " %");
        addNum    ("High Pass", pr.hp,       cur (ParamID::hp), 0, " Hz");
        addNum    ("Low Pass",  pr.lp,       cur (ParamID::lp), 0, " Hz");
        addNum    ("Mod Rate",  pr.rate,     cur (ParamID::rate), 2, " Hz");
        addNum    ("Mod Depth", pr.depth,    cur (ParamID::depth), 2, " ms");
        if (cur (ParamID::wetLock) > 0.5f)     // ロック中はプリセット値と比べない
            next.push_back ({ "Wet", ui (UiText::Locked, lang), juce::String (cur (ParamID::wet), 1) + " dB", {}, false });
        else
            addNum ("Wet", pr.wetDb, cur (ParamID::wet), 1, " dB");

        presetName = pr.name;
        if (next != rows)
        {
            rows = std::move (next);
            repaint();
        }
    }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        g.setColour (theme::panel);
        g.fillRoundedRectangle (r, 6.0f);
        g.setColour (theme::line);
        g.drawRoundedRectangle (r.reduced (0.5f), 6.0f, 1.0f);

        auto area = getLocalBounds().reduced (10, 6);
        const int n = (int) rows.size() + 2;              // タイトル行 ＋ 列見出し ＋ 各行
        const int rowH = juce::jmax (11, area.getHeight() / n);
        const float fs = juce::jmin (12.0f, (float) rowH * 0.74f);
        const int c0 = area.getWidth() * 30 / 100, c1 = area.getWidth() * 24 / 100,
                  c2 = area.getWidth() * 24 / 100;

        int changedCount = 0;
        for (auto& row : rows) if (row.changed) ++changedCount;

        // タイトル行：初期値：<プリセット名> …… n個 変更 / = 初期値
        auto title = area.removeFromTop (rowH);
        g.setFont (theme::ui (fs, true));
        g.setColour (changedCount > 0 ? theme::accent : theme::green);
        const auto status = changedCount > 0 ? juce::String (changedCount) + ui (UiText::Changed, lang)
                                             : ui (UiText::SameAsPreset, lang);
        g.drawText (status, title, juce::Justification::centredRight);
        g.setColour (theme::text);
        g.drawText (ui (UiText::ColPreset, lang) + presetName,
                    title.withTrimmedRight (juce::roundToInt (fs * 6.0f)), juce::Justification::centredLeft);

        // 列見出し
        auto head = area.removeFromTop (rowH);
        g.setFont (theme::ui (fs * 0.92f));
        g.setColour (theme::faint);
        g.drawText (ui (UiText::ColParam, lang),     head.removeFromLeft (c0), juce::Justification::centredLeft);
        g.drawText (ui (UiText::ColPresetCol, lang), head.removeFromLeft (c1), juce::Justification::centredLeft);
        g.drawText (ui (UiText::ColNow, lang),       head.removeFromLeft (c2), juce::Justification::centredLeft);
        g.setColour (theme::line);
        g.drawHorizontalLine (head.getBottom(), (float) area.getX(), (float) area.getRight());

        for (auto& row : rows)
        {
            auto line = area.removeFromTop (rowH);
            if (row.changed)
            {
                g.setColour (theme::accent.withAlpha (0.14f));
                g.fillRect (line.toFloat().expanded (4.0f, 0.0f));
            }
            g.setFont (theme::ui (fs));
            g.setColour (theme::muted);
            g.drawText (row.name, line.removeFromLeft (c0), juce::Justification::centredLeft);
            g.setFont (theme::mono (fs * 0.95f));
            g.drawFittedText (row.preset, line.removeFromLeft (c1), juce::Justification::centredLeft, 1, 0.8f);
            g.setColour (row.changed ? theme::accent : theme::text);
            g.drawFittedText (row.now,    line.removeFromLeft (c2), juce::Justification::centredLeft, 1, 0.8f);
            g.drawFittedText (row.diff,   line, juce::Justification::centredRight, 1, 0.8f);
        }
    }

private:
    struct Row
    {
        juce::String name, preset, now, diff;
        bool changed = false;
        bool operator== (const Row& o) const { return name == o.name && preset == o.preset && now == o.now && changed == o.changed; }
        bool operator!= (const Row& o) const { return ! (*this == o); }
    };

    std::vector<Row> rows;
    juce::String presetName;
    Lang lang = Lang::Japanese;
};
