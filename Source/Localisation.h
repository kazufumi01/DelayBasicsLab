#pragma once
#include <juce_core/juce_core.h>
#include "Presets.h"

// =============================================================================
//  日本語 / English の切り替え用テキスト
//  ・プリセット解説の日本語は Presets.h、英語はここ（同じ並び順）
//  ・UI の短い文言は ui() で取得
// =============================================================================
enum class Lang { Japanese = 0, English };

struct PresetTextEN { const char* tag; const char* what; const char* use; const char* ear; };

inline const PresetTextEN& presetTextEN (int index)
{
    static const PresetTextEN texts[] =
    {
        { "All-round",
          "Quarter-note echoes return three or four times, getting slightly darker each time. Your everyday starting point for delay.",
          "Vocals, guitars, synths, anything. Put it on a send (Wet Only) and shape it from here.",
          "Feedback sets the number of repeats, Low Pass the depth, Wet the presence. Those three alone fit most songs. Track what you moved in the comparison table." },

        { "Presence",
          "A single reflection returns about 0.1 s after the source, with the top end slightly rolled off. Its origin is the 1950s tape slapback.",
          "Vocals, guitars, snare. Adds a forward, three-dimensional presence without clouding the space the way a reverb can.",
          "Toggle Bypass to hear the vocal outline thicken. Around 100 ms is the turning point: shorter feels tighter, longer feels more rockabilly." },

        { "Thickness",
          "Delays shorter than about 30 ms fuse with the source and are heard as thickness rather than a separate echo (the precedence effect). Different L/R times plus slow modulation sound like a double-tracked part.",
          "Thicker vocals, wider synths and guitars. A stand-in for a real double take.",
          "Turn on Mono check to hear the comb filtering typical of short delays. Set Depth to 0 to hear how unnatural a static double sounds." },

        { "Width",
          "The left side is the dry signal; only the right side is delayed by 12 ms. You localise toward the earlier left signal, but sound also arrives from the right, so the image spreads sideways.",
          "Spreading mono sources (guitar, pad, backing vocals) across the stereo field. Use it as an insert on the track.",
          "Always switch on Mono check. When L and R are summed, some frequencies cancel and the sound turns thin. That is the classic Haas pitfall." },

        { "Movement",
          "Echoes bounce left, right, left... Each side's feedback is routed into the opposite side (cross-feedback).",
          "Synth arpeggios, guitar riffs, vocal phrase endings. Creates movement at the sides without crowding the centre.",
          "Watch blue and red alternate on the timeline. A high-pass of 250 Hz or more keeps the echoes from fighting the bass and kick." },

        { "Groove",
          "Dotted eighth on the left, quarter note on the right. The two offset rhythms interlock and fill the gaps with sixteenth-note answers.",
          "The classic way to turn a simple eighth-note guitar or synth line into a busy rhythm. Also great behind long vocal notes.",
          "Watch the dotted-eighth echoes land off the beat lines. The lower the Low Pass, the further the echoes sit behind the source." },

        { "Texture / Build",
          "High feedback with HP/LP filters inside the loop narrows the band on every pass. Repeats get thinner, darker and wobblier as they fade.",
          "Throws on vocal endings or snare hits. Automate the send up for one beat to create transitions.",
          "Watch the colours darken on the timeline as the highs are shaved off. Around 90% feedback you are on the edge of self-oscillation (watch your level)." },

        { "Delay vs reverb",
          "Short 40-80 ms echoes with different L/R times and feedback build up densely enough to start sounding like a room.",
          "A close sense of air when a reverb pushes things too far back. Blends drums or acoustic guitar without muddying them.",
          "Sweep Feedback from 0 to 60% and find where an echo turns into ambience." },
    };
    return texts[juce::jlimit (0, 7, index)];
}

// UI の短い文言
enum class UiText { WhatHappens, WhereToUse, WhatToListen, Footer, ColParam, ColPreset, ColPresetCol, ColNow, Changed, SameAsPreset, Locked };

inline juce::String ui (UiText key, Lang lang)
{
    const bool ja = (lang == Lang::Japanese);
    switch (key)
    {
        case UiText::WhatHappens:  return ja ? jp ("■ 何が起きている？") : "> What is happening?";
        case UiText::WhereToUse:   return ja ? jp ("■ 使いどころ")       : "> Where to use it";
        case UiText::WhatToListen: return ja ? jp ("■ 聴くポイント")     : "> What to listen for";
        case UiText::Footer:
            return ja ? jp ("※入力が無い時は SAMPLE で Pluck / Snare+Kick / Vox を選ぶと、その音にディレイが掛かります。"
                            "選択中のプリセットボタンをもう一度押すと初期値に戻ります。\n"
                            "※ノブは Shift + ドラッグで微調整、数値のダブルクリックで直接入力。"
                            "SYNC 中に Time を触ると同期が外れ、その音価の ms から手動で調整できます。")
                      : "Tip: with no audio input, pick Pluck / Snare+Kick / Vox under SAMPLE to hear the delay. "
                        "Click the selected preset button again to return to its initial values.\n"
                        "Shift + drag a knob for fine control; double-click its value to type a number. "
                        "Touching Time while synced releases SYNC so you can fine-tune from that note value in ms.";
        case UiText::ColParam:     return ja ? jp ("パラメーター") : "Parameter";
        case UiText::ColPreset:    return ja ? jp ("初期値：")     : "Preset: ";
        case UiText::ColPresetCol: return ja ? jp ("初期値")       : "Preset";
        case UiText::ColNow:       return ja ? jp ("現在値")       : "Now";
        case UiText::Changed:      return ja ? jp ("個 変更")      : " changed";
        case UiText::SameAsPreset: return ja ? jp ("= 初期値")     : "= preset";
        case UiText::Locked:       return ja ? jp ("Wet Lock 中")  : "Wet Lock";
    }
    return {};
}

// macOS の言語設定が日本語なら日本語、それ以外は英語
inline Lang systemDefaultLang()
{
    return juce::SystemStats::getUserLanguage().startsWithIgnoreCase ("ja") ? Lang::Japanese : Lang::English;
}
