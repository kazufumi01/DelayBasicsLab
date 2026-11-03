#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

// =============================================================================
//  パラメーターID・テンポ同期の定義（Processor と Editor の両方から使う）
// =============================================================================
namespace ParamID
{
    inline constexpr const char* mode      = "mode";
    inline constexpr const char* timeL     = "timeL";
    inline constexpr const char* timeR     = "timeR";
    inline constexpr const char* syncL     = "syncL";
    inline constexpr const char* syncR     = "syncR";
    inline constexpr const char* feedback  = "feedback";
    inline constexpr const char* hp        = "hp";
    inline constexpr const char* lp        = "lp";
    inline constexpr const char* rate      = "rate";
    inline constexpr const char* depth     = "depth";
    inline constexpr const char* wet       = "wet";
    inline constexpr const char* delayOff  = "delayOff";
    inline constexpr const char* wetOnly   = "wetOnly";
    inline constexpr const char* monoCheck = "monoCheck";
    inline constexpr const char* demo      = "demo";
    inline constexpr const char* wetLock   = "wetLock";
}

enum class DelayMode { Mono = 0, Stereo, PingPong, Haas };

inline const juce::StringArray& modeNames()
{
    static const juce::StringArray names { "Mono", "Stereo (L/R)", "Ping-Pong", "Haas (L dry / R delayed)" };
    return names;
}

// 0 = 同期OFF（ms指定）。それ以外は「4分音符 = 1.0」とした長さ。
inline const juce::StringArray& syncNames()
{
    static const juce::StringArray names { "Off (ms)", "1/16", "1/8T", "1/8", "1/8D", "1/4T", "1/4", "1/4D", "1/2" };
    return names;
}

inline const juce::StringArray& demoNames()
{
    static const juce::StringArray names { "Off", "Pluck", "Snare+Kick", "Vox" };
    return names;
}

inline double syncBeats (int index)
{
    static const double beats[] = { 0.0, 0.25, 1.0 / 3.0, 0.5, 0.75, 2.0 / 3.0, 1.0, 1.5, 2.0 };
    return beats[juce::jlimit (0, 8, index)];
}

// 同期中ならテンポから、OFFならスライダーの ms をそのまま使う
inline double resolveTimeMs (int syncIndex, double manualMs, double bpm)
{
    if (syncIndex <= 0)
        return manualMs;
    return 60000.0 / juce::jmax (20.0, bpm) * syncBeats (syncIndex);
}
