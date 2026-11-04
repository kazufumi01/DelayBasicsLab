#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "Theme.h"
#include "ScalingEditor.h"
#include "Timeline.h"
#include "ChoiceButtons.h"
#include "ParamCompare.h"
#include "FineSlider.h"

// =============================================================================
//  画面（1000 × 700 を基準に、ウィンドウ全体を拡大・縮小）
//    header 44 · PRESET · SAMPLE / MODE · 解説（広い）｜初期値の比較（狭い）
//    · タイムライン · ノブ＋スイッチ · SYNC L / R
// =============================================================================
class DelayBasicsLabView : public juce::Component,
                           private juce::Timer
{
public:
    ZoomButton zoom;                       // ScalingEditor が操作する「100%」ボタン

    explicit DelayBasicsLabView (DelayBasicsLabProcessor&);
    ~DelayBasicsLabView() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void showPresetText (int index);
    void applyLanguage (Lang l);
    void followSync (juce::Slider& knob, const char* timeId, const char* syncId, double bpm);
    void releaseSync (const char* syncId);

    // ノブ1つ分（スライダー＋見出し＋Attachment）
    struct Knob
    {
        FineSlider   slider;     // Shift + ドラッグで微調整
        juce::Label  label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    DelayBasicsLabProcessor& proc;
    theme::LookAndFeel lnf;

    juce::Label presetTitle, sampleTitle, modeTitle, syncLTitle, syncRTitle;
    ChoiceButtons presetButtons, sampleButtons, modeButtons, syncLButtons, syncRButtons;
    juce::TextButton langBtn;

    juce::TextEditor explain;
    ParamCompare compare;     // プリセット初期値 ⇔ 現在値
    Timeline timeline;

    Knob timeL, timeR, feedback, hp, lp, rate, depth, wet;
    juce::TextButton delayOff { "Bypass" }, wetOnly { "Wet Only" }, wetLock { "Wet Lock" }, monoCheck { "Mono" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> offAtt, wetOnlyAtt, wetLockAtt, monoAtt;

    int shownPreset = -1;
    juce::String readout;
    juce::Rectangle<int> knobPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelayBasicsLabView)
};

class DelayBasicsLabEditor : public ScalingEditor<DelayBasicsLabView, DelayBasicsLabProcessor>
{
public:
    explicit DelayBasicsLabEditor (DelayBasicsLabProcessor& p)
        : ScalingEditor (p, 1000, 700, "delaybasicslab.zoom") {}
};
