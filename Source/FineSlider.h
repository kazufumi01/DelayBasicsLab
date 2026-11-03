#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// =============================================================================
//  FineSlider：ドラッグ量を自前で積算するロータリーノブ
//   ・通常ドラッグ：250px で全域（JUCE 標準と同じ感覚）
//   ・Shift + ドラッグ：1/25 の細かさで微調整（途中で Shift を押しても値が跳ばない）
//   ・数値部分をダブルクリック → 直接入力
// =============================================================================
class FineSlider : public juce::Slider
{
public:
    FineSlider() : juce::Slider (juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow) {}

    void mouseDown (const juce::MouseEvent& e) override
    {
        juce::Slider::mouseDown (e);           // ジェスチャー開始（onDragStart もここで呼ばれる）
        lastPos = e.position;
        dragPos = valueToProportionOfLength (getValue());
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (! isEnabled() || e.mods.isPopupMenu())
            return;

        const float diff = (e.position.x - lastPos.x) + (lastPos.y - e.position.y);
        lastPos = e.position;

        const double pixelsForFullRange = e.mods.isShiftDown() ? 6250.0 : 250.0;
        dragPos = juce::jlimit (0.0, 1.0, dragPos + diff / pixelsForFullRange);
        setValue (proportionOfLengthToValue (dragPos), juce::sendNotificationSync);
    }

private:
    juce::Point<float> lastPos;
    double dragPos = 0.0;      // 0.1ms 刻みに丸められる前の位置を保持（細かい動きを失わない）
};
