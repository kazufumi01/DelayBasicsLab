#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// =============================================================================
//  JPK "Studio" デザイン（EQ Anatomy / Meter Bridge と共通）
//  グラファイトの背景・アンバーのアクセント・Menlo の数値表示
// =============================================================================
namespace theme
{
const juce::Colour bg       (0xff121417);
const juce::Colour header   (0xff191c21);
const juce::Colour panel    (0xff191c21);
const juce::Colour well     (0xff0d0f12);
const juce::Colour line     (0xff2a2e35);
const juce::Colour grid     (0xff1d2126);
const juce::Colour zero     (0xff353a42);
const juce::Colour text     (0xffe4e1da);
const juce::Colour muted    (0xff8d929a);
const juce::Colour faint    (0xff6c727b);
const juce::Colour accent   (0xfff0b34a);
const juce::Colour onAccent (0xff1a1306);
const juce::Colour green    (0xff7cc66f);
const juce::Colour lessonBg (0xff1b1a16);
const juce::Colour lessonLn (0xff5a4620);
const juce::Colour knobBody (0xff23272e);
const juce::Colour laneL    (0xff62b6ea);   // タイムライン：L（青）
const juce::Colour laneR    (0xffec7a92);   // タイムライン：R（赤）

inline juce::Font ui (float size, bool bold = false)
{
   #if JUCE_MAC
    return juce::Font (juce::FontOptions ("Hiragino Sans", size, bold ? juce::Font::bold : juce::Font::plain));
   #elif JUCE_WINDOWS
    return juce::Font (juce::FontOptions ("Yu Gothic UI", size, bold ? juce::Font::bold : juce::Font::plain));
   #else
    return juce::Font (juce::FontOptions (size, bold ? juce::Font::bold : juce::Font::plain));
   #endif
}
inline juce::Font mono (float size, bool bold = false)
{
   #if JUCE_MAC
    return juce::Font (juce::FontOptions ("Menlo", size, bold ? juce::Font::bold : juce::Font::plain));
   #elif JUCE_WINDOWS
    return juce::Font (juce::FontOptions ("Consolas", size, bold ? juce::Font::bold : juce::Font::plain));
   #else
    return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), size, bold ? juce::Font::bold : juce::Font::plain));
   #endif
}

class LookAndFeel : public juce::LookAndFeel_V4
{
public:
    LookAndFeel()
    {
        setColour (juce::TextButton::buttonColourId, panel);
        setColour (juce::TextButton::buttonOnColourId, accent);
        setColour (juce::TextButton::textColourOffId, text);
        setColour (juce::TextButton::textColourOnId, onAccent);
        setColour (juce::ComboBox::backgroundColourId, knobBody);
        setColour (juce::ComboBox::outlineColourId, line);
        setColour (juce::ComboBox::textColourId, text);
        setColour (juce::ComboBox::arrowColourId, muted);
        setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff1c2026));
        setColour (juce::PopupMenu::textColourId, text);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, accent);
        setColour (juce::PopupMenu::highlightedTextColourId, onAccent);
        setColour (juce::Label::textColourId, text);
        setColour (juce::Label::textWhenEditingColourId, text);
        setColour (juce::Label::backgroundWhenEditingColourId, well);
        setColour (juce::Label::outlineWhenEditingColourId, accent);
        setColour (juce::TextEditor::backgroundColourId, well);
        setColour (juce::TextEditor::textColourId, text);
        setColour (juce::TextEditor::highlightColourId, accent.withAlpha (0.4f));
        setColour (juce::CaretComponent::caretColourId, accent);
        setColour (juce::Slider::textBoxTextColourId, text);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxHighlightColourId, accent.withAlpha (0.4f));
        setColour (juce::ScrollBar::thumbColourId, zero);
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down) override
    {
        auto r = b.getLocalBounds().toFloat().reduced (0.5f);
        const bool on = b.getToggleState();
        auto col = b.findColour (on ? juce::TextButton::buttonOnColourId : juce::TextButton::buttonColourId);
        if (! on && (over || down)) col = col.brighter (0.12f);
        g.setColour (col); g.fillRoundedRectangle (r, 4.0f);
        if (! on) { g.setColour (b.findColour (juce::ComboBox::outlineColourId)); g.drawRoundedRectangle (r, 4.0f, 1.0f); }
    }

    // ロータリーノブ：細いトラック＋アンバーの値アーク＋グラファイトの胴＋白い指針
    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                           float a0, float a1, juce::Slider& s) override
    {
        auto r = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (3.0f);
        const float rad = juce::jmin (r.getWidth(), r.getHeight()) * 0.5f;
        if (rad < 10.0f) return;
        const auto c = r.getCentre();
        const float ang = a0 + pos * (a1 - a0);
        const float ar = rad - 2.0f;
        const juce::PathStrokeType stroke (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);

        juce::Path track; track.addCentredArc (c.x, c.y, ar, ar, 0.0f, a0, a1, true);
        g.setColour (line); g.strokePath (track, stroke);
        juce::Path val; val.addCentredArc (c.x, c.y, ar, ar, 0.0f, a0, ang, true);
        g.setColour (s.isEnabled() ? accent : muted); g.strokePath (val, stroke);

        const float br = rad - 7.0f;
        g.setColour (knobBody); g.fillEllipse (c.x - br, c.y - br, br * 2.0f, br * 2.0f);
        g.setColour (line); g.drawEllipse (c.x - br, c.y - br, br * 2.0f, br * 2.0f, 1.0f);

        g.setColour (text);
        g.drawLine ({ c.getPointOnCircumference (br * 0.3f, ang), c.getPointOnCircumference (br * 0.88f, ang) }, 2.0f);
    }

    juce::Font getTextButtonFont (juce::TextButton&, int h) override { return ui (juce::jmin (12.0f, h * 0.5f)); }
    juce::Font getComboBoxFont (juce::ComboBox&) override { return ui (11.5f); }
    juce::Font getPopupMenuFont() override { return ui (13.0f); }
    juce::Font getLabelFont (juce::Label& l) override
    {
        if (dynamic_cast<juce::Slider*> (l.getParentComponent()) != nullptr)
            return mono (11.0f);                 // ノブの数値は Menlo
        return l.getFont();
    }
};
}
