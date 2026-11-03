#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "Timeline.h"
#include "ChoiceButtons.h"
#include "ParamCompare.h"
#include "FineSlider.h"

class DelayBasicsLabEditor : public juce::AudioProcessorEditor,
                             private juce::Timer
{
public:
    explicit DelayBasicsLabEditor (DelayBasicsLabProcessor&);
    ~DelayBasicsLabEditor() override;

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
    juce::LookAndFeel_V4 lnf;

    juce::Label title, signature, bpmLabel;
    juce::Label presetTitle, sampleTitle, modeTitle, syncLTitle, syncRTitle;

    ChoiceButtons presetButtons, sampleButtons, modeButtons, syncLButtons, syncRButtons, langButtons;

    juce::TextEditor explain;
    ParamCompare compare;     // プリセット初期値 ⇔ 現在値
    Timeline timeline;

    Knob timeL, timeR, feedback, hp, lp, rate, depth, wet;
    juce::ToggleButton delayOff { "Bypass" }, wetOnly { "Wet Only" }, wetLock { "Wet Lock" }, monoCheck { "Mono" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> offAtt, wetOnlyAtt, wetLockAtt, monoAtt;

    int shownPreset = -1;
    juce::Rectangle<int> knobPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelayBasicsLabEditor)
};
