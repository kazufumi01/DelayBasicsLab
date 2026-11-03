#pragma once
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <cmath>
#include "Params.h"

// =============================================================================
//  FractionalDelay
//  リングバッファ＋4点エルミート補間。ブラウザ版の DelayNode に相当。
//  「小数サンプル」の遅延が読めるので、時間を揺らす（Mod）・滑らかに変える
//  （テープのようなピッチの滑り）ことができる。
// =============================================================================
class FractionalDelay
{
public:
    void prepare (int maxDelaySamples)
    {
        int size = 1;
        while (size < maxDelaySamples + 8)
            size <<= 1;                        // 2のべき乗にして & でラップ
        buffer.assign ((size_t) size, 0.0f);
        mask = size - 1;
        writePos = 0;
    }

    void clear() { std::fill (buffer.begin(), buffer.end(), 0.0f); }

    // delaySamples 前に書いた値を読む（write より先に呼ぶ）
    float read (float delaySamples) const
    {
        const float d    = juce::jlimit (3.0f, (float) (mask - 4), delaySamples); // 補間の4点目が未書き込み領域に入らないよう3以上
        const float pos  = (float) writePos - d;
        const int   i    = (int) std::floor (pos);
        const float frac = pos - (float) i;

        const float xm1 = at (i - 1), x0 = at (i), x1 = at (i + 1), x2 = at (i + 2);
        // 4点エルミート（Catmull-Rom）補間
        const float c1 = 0.5f * (x1 - xm1);
        const float c2 = xm1 - 2.5f * x0 + 2.0f * x1 - 0.5f * x2;
        const float c3 = 0.5f * (x2 - xm1) + 1.5f * (x0 - x1);
        return ((c3 * frac + c2) * frac + c1) * frac + x0;
    }

    void write (float x)
    {
        buffer[(size_t) writePos] = x;
        writePos = (writePos + 1) & mask;
    }

private:
    float at (int i) const { return buffer[(size_t) (i & mask)]; }

    std::vector<float> buffer;
    int mask = 0, writePos = 0;
};

// =============================================================================
//  DelayEngine：ブラウザ版の build()/apply() を1サンプルずつの処理に移植したもの
//
//   入力 ─┬───────────────────────────── Dry ──┐
//         └→ [Delay L/R] → HP → LP ─┬→ Wet ──┤→ 出力（必要ならモノラル化）
//              ↑                     │
//              └──── × Feedback ─────┘   ← フィルターがループ内にあるので
//                                          リピートごとに効きが積み重なる
// =============================================================================
struct DelaySettings
{
    DelayMode mode = DelayMode::Mono;
    float timeLms = 110.0f, timeRms = 110.0f;
    float feedback = 0.05f;              // 0..0.95
    float hpHz = 200.0f, lpHz = 5000.0f;
    float rateHz = 0.5f, depthMs = 0.0f;
    float wetGain = 0.35f;               // リニア
    bool  delayOff = false, wetOnly = false, monoCheck = false;
};

class DelayEngine
{
public:
    void prepare (double sampleRate, int maxBlockSize)
    {
        fs = sampleRate;
        const int maxSamples = (int) std::ceil (fs * 4.0);      // 最大4秒

        for (auto& d : lines)
            d.prepare (maxSamples);

        juce::dsp::ProcessSpec spec { fs, (juce::uint32) maxBlockSize, 2 };
        hp.prepare (spec);  hp.setType (juce::dsp::StateVariableTPTFilterType::highpass);
        lp.prepare (spec);  lp.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        hp.setResonance (1.0f / std::sqrt (2.0f));               // バターワース（ピーク無し）
        lp.setResonance (1.0f / std::sqrt (2.0f));

        // 時間の変化は 50ms かけて滑らかに → テープのようにピッチが滑る
        timeL.reset (fs, 0.05);  timeR.reset (fs, 0.05);
        fbSm.reset  (fs, 0.02);  wetSm.reset (fs, 0.02);  drySm.reset (fs, 0.02);
        depthSm.reset (fs, 0.05);

        reset();
    }

    void reset()
    {
        for (auto& d : lines)
            d.clear();
        hp.reset();
        lp.reset();
        phase = 0.0;
        firstBlock = true;
    }

    void process (juce::AudioBuffer<float>& buffer, int numInputChannels, const DelaySettings& s)
    {
        // モード切替時はバッファを空にする（古いエコーが別の配線に流れて
        // 一瞬鳴るのを防ぐ。ブラウザ版で直したバグと同じ理由）
        if (s.mode != lastMode)
        {
            reset();
            lastMode = s.mode;
        }

        const float toSamples = (float) (fs / 1000.0);
        setTarget (timeL, s.timeLms * toSamples);
        setTarget (timeR, s.timeRms * toSamples);
        setTarget (depthSm, s.depthMs * toSamples);
        setTarget (fbSm, s.mode == DelayMode::Haas ? 0.0f : juce::jlimit (0.0f, 0.95f, s.feedback));
        setTarget (wetSm, s.delayOff ? 0.0f : s.wetGain);
        setTarget (drySm, s.wetOnly ? 0.0f : 1.0f);
        firstBlock = false;

        hp.setCutoffFrequency (juce::jlimit (20.0f, 20000.0f, s.hpHz));
        lp.setCutoffFrequency (juce::jlimit (20.0f, (float) (fs * 0.45), s.lpHz));

        const double phaseInc = juce::MathConstants<double>::twoPi * s.rateHz / fs;
        const int n = buffer.getNumSamples();
        auto* outL = buffer.getWritePointer (0);
        auto* outR = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;
        const float* inRptr = numInputChannels > 1 ? buffer.getReadPointer (1) : buffer.getReadPointer (0);

        for (int i = 0; i < n; ++i)
        {
            const float inL  = outL[i];
            const float inR  = inRptr[i];
            const float mono = 0.5f * (inL + inR);

            // --- 時間の揺らぎ（Mod）：L と R で逆相にして広がりを出す ---
            const float mod = depthSm.getNextValue() * (float) std::sin (phase);
            phase += phaseInc;
            if (phase > juce::MathConstants<double>::twoPi)
                phase -= juce::MathConstants<double>::twoPi;

            const float tL = timeL.getNextValue();
            const float tR = timeR.getNextValue();
            const float dL = tL + mod;
            const float dR = (s.mode == DelayMode::Mono ? tL + mod : tR - mod);

            // --- 読み出し → ループ内フィルター ---
            const float fL = lp.processSample (0, hp.processSample (0, lines[0].read (dL)));
            const float fR = lp.processSample (1, hp.processSample (1, lines[1].read (dR)));

            const float g = fbSm.getNextValue();
            float wetL = 0.0f, wetR = 0.0f, dryL = inL, dryR = inR;

            // --- 書き込み（モードごとの配線） ---
            switch (s.mode)
            {
                case DelayMode::Mono:      // モノラル入力 → 1本のディレイ → 左右同じ
                    lines[0].write (mono + g * fL);
                    lines[1].write (0.0f);
                    wetL = wetR = fL;
                    break;

                case DelayMode::Stereo:    // L→L、R→R の独立した2本
                    lines[0].write (inL + g * fL);
                    lines[1].write (inR + g * fR);
                    wetL = fL;  wetR = fR;
                    break;

                case DelayMode::PingPong:  // L の出力を R に、R の出力を L に戻す
                    lines[0].write (mono + g * fR);
                    lines[1].write (g * fL);
                    wetL = fL;  wetR = fR;
                    break;

                case DelayMode::Haas:      // 左は原音、右だけ遅らせる（フィードバック無し）
                    lines[0].write (0.0f);
                    lines[1].write (mono);
                    dryL = mono;  dryR = 0.0f;
                    wetR = fR;
                    break;
            }

            const float dg = drySm.getNextValue();
            const float wg = wetSm.getNextValue();
            float l = dryL * dg + wetL * wg;
            float r = dryR * dg + wetR * wg;

            if (s.monoCheck)                // L+R を足して聴く（コムフィルター確認用）
                l = r = 0.5f * (l + r);

            outL[i] = l;
            if (outR != nullptr)
                outR[i] = r;
        }
    }

private:
    void setTarget (juce::SmoothedValue<float>& sv, float v)
    {
        if (firstBlock) sv.setCurrentAndTargetValue (v);
        else            sv.setTargetValue (v);
    }

    double fs = 44100.0, phase = 0.0;
    bool firstBlock = true;
    DelayMode lastMode = DelayMode::Mono;
    FractionalDelay lines[2];
    juce::dsp::StateVariableTPTFilter<float> hp, lp;
    juce::SmoothedValue<float> timeL, timeR, fbSm, wetSm, drySm, depthSm;
};
