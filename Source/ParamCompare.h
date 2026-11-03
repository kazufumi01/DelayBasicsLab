#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Params.h"
#include "Presets.h"
#include "Localisation.h"

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
        g.setColour (juce::Colour (0xff1e2128));
        g.fillRoundedRectangle (r, 10.0f);
        g.setColour (juce::Colour (0xff323844));
        g.drawRoundedRectangle (r.reduced (0.5f), 10.0f, 1.0f);

        auto area = getLocalBounds().reduced (8, 4);
        const int n = (int) rows.size() + 1;
        const int rowH = juce::jmax (11, area.getHeight() / n);
        const int c0 = area.getWidth() * 26 / 100, c1 = area.getWidth() * 26 / 100,
                  c2 = area.getWidth() * 26 / 100;

        int changedCount = 0;
        for (auto& row : rows) if (row.changed) ++changedCount;

        // 見出し
        auto head = area.removeFromTop (rowH);
        g.setFont (font ((float) rowH * 0.72f, true));
        g.setColour (juce::Colour (0xff9ba2ad));
        g.drawText (ui (UiText::ColParam, lang), head.removeFromLeft (c0), juce::Justification::centredLeft);
        g.drawText (ui (UiText::ColPreset, lang) + presetName, head.removeFromLeft (c1), juce::Justification::centredLeft);
        g.setColour (juce::Colour (0xffece9e3));
        g.drawText (ui (UiText::ColNow, lang), head.removeFromLeft (c2), juce::Justification::centredLeft);
        g.setColour (changedCount > 0 ? juce::Colour (0xfff0a63a) : juce::Colour (0xff7fd3a0));
        g.drawText (changedCount > 0 ? juce::String (changedCount) + ui (UiText::Changed, lang) : ui (UiText::SameAsPreset, lang),
                    head, juce::Justification::centredRight);

        g.setFont (font ((float) rowH * 0.72f, false));
        for (auto& row : rows)
        {
            auto line = area.removeFromTop (rowH);
            if (row.changed)
            {
                g.setColour (juce::Colour (0x33f0a63a));
                g.fillRect (line.toFloat().expanded (4.0f, 0.0f));
            }
            g.setColour (juce::Colour (0xff9ba2ad));
            g.drawText (row.name,   line.removeFromLeft (c0), juce::Justification::centredLeft);
            g.drawText (row.preset, line.removeFromLeft (c1), juce::Justification::centredLeft);
            g.setColour (row.changed ? juce::Colour (0xfff0a63a) : juce::Colour (0xffece9e3));
            g.drawText (row.now,    line.removeFromLeft (c2), juce::Justification::centredLeft);
            g.drawText (row.diff,   line, juce::Justification::centredRight);
        }
    }

private:
    static juce::Font font (float size, bool bold)
    {
       #if JUCE_MAC
        return juce::Font (juce::FontOptions ("Hiragino Sans", size, bold ? juce::Font::bold : juce::Font::plain));
       #elif JUCE_WINDOWS
        // Windows 標準の日本語UIフォント（英語版Windowsにも入っている）
        return juce::Font (juce::FontOptions ("Yu Gothic UI", size, bold ? juce::Font::bold : juce::Font::plain));
       #else
        return juce::Font (juce::FontOptions (size, bold ? juce::Font::bold : juce::Font::plain));
       #endif
    }

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
