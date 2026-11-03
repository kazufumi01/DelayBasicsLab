#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <cmath>

// =============================================================================
//  DemoSynth：オーディオ入力が無くても設定を試せるサンプル音源
//  ブラウザ版の pluck() / snare() / kick() / vox() と同じ音・同じ2小節パターン。
//  ホストのテンポに同期して鳴る（Standalone では 120 BPM）。
// =============================================================================
class DemoSynth
{
public:
    enum Source { Off = 0, Pluck, Snare, Vox };

    void prepare (double sampleRate) { fs = sampleRate; reset(); }

    void reset()
    {
        for (auto& v : voices) v.active = false;
        samplesToNextStep = 0.0;
        step = 0;
    }

    // buffer の全チャンネルをサンプル音で上書きする（入力は使わない）
    void render (juce::AudioBuffer<float>& buffer, int source, double bpm)
    {
        if (source != lastSource) { reset(); lastSource = source; }

        const double sixteenth = fs * 60.0 / juce::jmax (20.0, bpm) / 4.0;
        const int n = buffer.getNumSamples();
        auto* out = buffer.getWritePointer (0);

        for (int i = 0; i < n; ++i)
        {
            if (samplesToNextStep <= 0.0)
            {
                triggerStep (source, step, sixteenth);
                step = (step + 1) % 32;
                samplesToNextStep += sixteenth;
            }
            samplesToNextStep -= 1.0;

            float s = 0.0f;
            for (auto& v : voices)
                if (v.active)
                    s += renderVoice (v);

            out[i] = std::tanh (s * 0.9f);          // 念のためのソフトクリップ
        }

        for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
            buffer.copyFrom (ch, 0, buffer, 0, 0, n);
    }

private:
    // --- 2極TPTステートバリアブルフィルター（LP/BP/HPを同時に出力） ---
    struct Svf
    {
        float ic1 = 0, ic2 = 0, a1 = 0, a2 = 0, a3 = 0, k = 1;
        void set (float freq, float q, double sr)
        {
            const float g = std::tan (juce::MathConstants<float>::pi * juce::jlimit (20.0f, (float) (sr * 0.45), freq) / (float) sr);
            k = 1.0f / q;
            a1 = 1.0f / (1.0f + g * (g + k));
            a2 = g * a1;
            a3 = g * a2;
        }
        // mode: 0=LP, 1=BP(ピーク0dBに正規化), 2=HP
        float process (float x, int mode)
        {
            const float v3 = x - ic2;
            const float v1 = a1 * ic1 + a2 * v3;
            const float v2 = ic2 + a2 * ic1 + a3 * v3;
            ic1 = 2.0f * v1 - ic1;
            ic2 = 2.0f * v2 - ic2;
            return mode == 0 ? v2 : mode == 1 ? k * v1 : x - k * v1 - v2;
        }
    };

    enum Type { TPluck, TSnare, TKick, TVox };

    struct Voice
    {
        bool  active = false;
        Type  type = TPluck;
        double t = 0.0;              // 発音からの経過秒
        float freq = 440.0f, dur = 0.5f;
        double ph1 = 0, ph2 = 0, vib = 0;
        Svf f[4];
    };

    static float mtof (int m) { return 440.0f * std::pow (2.0f, (float) (m - 69) / 12.0f); }
    // peak から 0.0005 まで dec 秒で下がる指数減衰（Web Audio の exponentialRamp と同じ形）
    static float expEnv (double t, float attack, float peak, float dec)
    {
        if (t < attack) return peak * (float) (t / attack);
        const float tau = dec / std::log (peak / 0.0005f);
        return peak * std::exp (-(float) (t - attack) / tau);
    }
    static float expGlide (double t, float from, float to, float time)
    {
        const float x = (float) juce::jmin (1.0, t / time);
        return from * std::pow (to / from, x);
    }
    static float saw (double ph) { return (float) (2.0 * ph - 1.0); }
    static float tri (double ph) { return (float) (ph < 0.5 ? 4.0 * ph - 1.0 : 3.0 - 4.0 * ph); }

    void start (Type type, float freq, float dur)
    {
        Voice* target = &voices[0];
        for (auto& v : voices)                     // 空きボイス、無ければ一番古いもの
        {
            if (! v.active) { target = &v; break; }
            if (v.t > target->t) target = &v;
        }
        *target = Voice();
        target->active = true;
        target->type = type;
        target->freq = freq;
        target->dur = dur;

        if (type == TSnare)
        {
            target->f[0].set (700.0f, 0.707f, fs);     // HP
            target->f[1].set (2200.0f, 0.7f, fs);      // BP
        }
        else if (type == TVox)
        {
            const float formants[4][2] = { { 730, 9 }, { 1090, 11 }, { 2440, 14 }, { 3400, 16 } };
            for (int i = 0; i < 4; ++i)
                target->f[i].set (formants[i][0], formants[i][1], fs);
        }
    }

    void triggerStep (int source, int s, double sixteenthSamples)
    {
        static const int pluckNotes[32] = { 76,0,0,79,0,0,81,0,0,0,74,0,0,0,0,0, 72,0,0,76,0,0,79,0,0,0,67,0,0,0,0,0 };
        const double beatSec = sixteenthSamples * 4.0 / fs;

        switch (source)
        {
            case Pluck:
                if (pluckNotes[s] != 0) start (TPluck, mtof (pluckNotes[s]), 0.5f);
                break;
            case Snare:
                if (s % 16 == 4 || s % 16 == 12) start (TSnare, 0, 0.25f);
                if (s == 0 || s == 16 || s == 26) start (TKick, 0, 0.4f);
                break;
            case Vox:
                if (s == 0)  start (TVox, mtof (69), (float) (0.9  * beatSec));
                if (s == 8)  start (TVox, mtof (72), (float) (0.45 * beatSec));
                if (s == 12) start (TVox, mtof (71), (float) (0.3  * beatSec));
                if (s == 16) start (TVox, mtof (67), (float) (1.3  * beatSec));
                break;
            default: break;
        }
    }

    float renderVoice (Voice& v)
    {
        const double dt = 1.0 / fs;
        const double t = v.t;
        float out = 0.0f;

        switch (v.type)
        {
            case TPluck:   // ノコギリ波＋三角波 → 開いて閉じるローパス
            {
                v.f[0].set (expGlide (t, 4200.0f, 700.0f, 0.22f), 1.2f, fs);
                const float osc = saw (v.ph1) + tri (v.ph2);
                out = v.f[0].process (osc, 0) * expEnv (t, 0.003f, 0.32f, 0.38f);
                v.ph1 = std::fmod (v.ph1 + v.freq * dt, 1.0);
                v.ph2 = std::fmod (v.ph2 + v.freq * 2.003 * dt, 1.0);
                if (t > 0.5) v.active = false;
                break;
            }
            case TSnare:   // ノイズ（HP→BP）＋ピッチが下がる三角波
            {
                const float noise = rng.nextFloat() * 2.0f - 1.0f;
                const float body = v.f[1].process (v.f[0].process (noise, 2), 1) * expEnv (t, 0.001f, 0.55f, 0.17f);
                const float f = expGlide (t, 230.0f, 170.0f, 0.06f);
                const float tone = tri (v.ph1) * expEnv (t, 0.001f, 0.5f, 0.09f);
                v.ph1 = std::fmod (v.ph1 + f * dt, 1.0);
                out = body + tone;
                if (t > 0.25) v.active = false;
                break;
            }
            case TKick:    // 120Hz→48Hz に落ちるサイン波
            {
                const float f = expGlide (t, 120.0f, 48.0f, 0.12f);
                out = (float) std::sin (juce::MathConstants<double>::twoPi * v.ph1) * expEnv (t, 0.002f, 0.8f, 0.32f);
                v.ph1 = std::fmod (v.ph1 + f * dt, 1.0);
                if (t > 0.4) v.active = false;
                break;
            }
            case TVox:     // ビブラート付きノコギリ波 → 4つのフォルマント（「あ」）
            {
                const float vibDepth = 0.012f * (float) juce::jmin (1.0, t / juce::jmin ((double) v.dur, 0.35));
                const float f = v.freq * (1.0f + vibDepth * (float) std::sin (juce::MathConstants<double>::twoPi * v.vib));
                v.vib = std::fmod (v.vib + 5.4 * dt, 1.0);
                const float src = saw (v.ph1);
                v.ph1 = std::fmod (v.ph1 + f * dt, 1.0);
                const float gains[4] = { 1.0f, 0.55f, 0.28f, 0.12f };
                float s = 0.0f;
                for (int i = 0; i < 4; ++i)
                    s += v.f[i].process (src, 1) * gains[i] * 3.0f;

                float env;
                if (t < 0.05)        env = 0.5f * (float) (t / 0.05);
                else if (t < v.dur)  env = 0.5f;
                else                 env = 0.5f * std::exp (-(float) (t - v.dur) / (0.14f / std::log (1000.0f)));
                out = s * env;
                if (t > v.dur + 0.2) v.active = false;
                break;
            }
        }

        v.t += dt;
        return out;
    }

    double fs = 44100.0, samplesToNextStep = 0.0;
    int step = 0, lastSource = 0;
    std::array<Voice, 8> voices;
    juce::Random rng;
};
