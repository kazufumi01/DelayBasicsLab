#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "DelayEngine.h"

// =============================================================================
//  エコーのタイムライン（ブラウザ版の draw() の移植）
//  音を解析しているのではなく、パラメーターから「理論上のエコーの並び」を描く。
// =============================================================================
class Timeline : public juce::Component
{
public:
    void update (const DelaySettings& newSettings, double newBpm)
    {
        s = newSettings;
        bpm = newBpm;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto area = getLocalBounds().toFloat();
        g.setColour (juce::Colour (0xff101215));
        g.fillRoundedRectangle (area, 10.0f);

        struct Tap { float t, a; int lane; int n; };   // lane: 0=L 1=R 2=両方
        std::vector<Tap> taps;
        const float wet = s.wetGain, fb = s.feedback;
        const float lim = 0.0025f;

        auto addSeries = [&] (float step, int lane)
        {
            float a = wet, t = step;
            for (int n = 1; n < 40 && t < 6000.0f; ++n)
            {
                taps.push_back ({ t, a, lane, n });
                a *= fb;  t += step;
                if (a < lim) break;
            }
        };

        switch (s.mode)
        {
            case DelayMode::Haas:   taps.push_back ({ s.timeRms, wet, 1, 1 }); break;
            case DelayMode::Mono:   addSeries (s.timeLms, 2); break;
            case DelayMode::Stereo: addSeries (s.timeLms, 0); addSeries (s.timeRms, 1); break;
            case DelayMode::PingPong:
            {
                float a = wet, t = s.timeLms;
                for (int n = 1; n < 40 && t < 6000.0f; ++n)
                {
                    taps.push_back ({ t, a, (n % 2) ? 0 : 1, n });
                    a *= fb;  t += (n % 2) ? s.timeRms : s.timeLms;
                    if (a < lim) break;
                }
                break;
            }
        }

        // 表示範囲：−36dB 以上のエコーが収まる長さに自動調整
        float maxT = 0.0f;
        for (auto& tp : taps)
            if (tp.a > juce::Decibels::decibelsToGain (-36.0f))
                maxT = juce::jmax (maxT, tp.t);
        if (maxT <= 0.0f && ! taps.empty()) maxT = taps.front().t;
        const float win = juce::jlimit (60.0f, 4000.0f, maxT * 1.18f);

        const float padL = 30.0f, padR = 12.0f, top = 18.0f, bottom = 24.0f;
        const float laneH = (area.getHeight() - top - bottom) / 2.0f;
        auto x = [&] (float t) { return padL + (area.getWidth() - padL - padR) * t / win; };

        g.setFont (juce::Font (juce::FontOptions (11.0f)));
        g.setColour (juce::Colour (0xff6d7480));
        g.drawText ("L", 6, (int) (top + laneH / 2 - 7), 14, 14, juce::Justification::centred);
        g.drawText ("R", 6, (int) (top + laneH * 1.5f - 7), 14, 14, juce::Justification::centred);
        g.setColour (juce::Colour (0xff2a2f38));
        g.drawHorizontalLine ((int) (top + laneH + 2), padL, area.getWidth() - padR);

        // 拍の線（オレンジ）
        const float beat = (float) (60000.0 / juce::jmax (20.0, bpm));
        if (win > beat * 0.6f)
        {
            for (int k = 1; beat / 4 * k < win; ++k)
            {
                const bool isBeat = (k % 4 == 0);
                if (! isBeat && win > beat * 3) continue;
                g.setColour (juce::Colour (0xfff0a63a).withAlpha (isBeat ? 0.4f : 0.18f));
                g.drawVerticalLine ((int) x (beat / 4 * k), top, area.getHeight() - bottom);
            }
        }

        // 時間軸の目盛り
        const float steps[] = { 1, 2, 5, 10, 20, 50, 100, 200, 250, 500, 1000 };
        float step = 1000.0f;
        for (auto st : steps) if (win / st <= 8.0f) { step = st; break; }
        g.setColour (juce::Colour (0xff6d7480));
        for (float t = 0.0f; t <= win; t += step)
            g.drawText (juce::String ((int) t) + "ms",
                        juce::jlimit (0, (int) area.getWidth() - 54, (int) x (t) - 25), (int) (area.getHeight() - 18), 50, 14,
                        juce::Justification::centred);   // 右端の目盛りが切れないよう内側に収める

        auto bar = [&] (float t, float a, int lane, juce::Colour c, int n)
        {
            const float db = juce::Decibels::gainToDecibels (juce::jmax (a, 1.0e-4f));
            const float h  = juce::jmax (2.0f, laneH * juce::jmax (0.0f, (db + 42.0f) / 42.0f));
            const float y0 = top + (lane == 1 ? laneH + 4.0f : 0.0f) + laneH - h;
            g.setColour (c);
            g.fillRect (x (t) - 4.0f, y0, 8.0f, h);
            if (n > 0 && n < 10 && h > 14.0f)
            {
                g.setColour (juce::Colour (0xff0b0c0e));
                g.drawText (juce::String (n), (int) (x (t) - 4), (int) y0 + 1, 8, 11, juce::Justification::centred);
            }
        };

        // 原音
        if (! s.wetOnly)
        {
            bar (0.0f, 1.0f, 0, juce::Colour (0xff8b929c), 0);
            if (s.mode != DelayMode::Haas)
                bar (0.0f, 1.0f, 1, juce::Colour (0xff8b929c), 0);
        }

        // エコー：ループ内フィルターの累積で色を暗くする
        if (! s.delayOff)
        {
            auto shade = [&] (int n)
            {
                const float h  = 1.0f / std::sqrt (1.0f + std::pow (4000.0f / s.lpHz, 4.0f));
                const float hh = 1.0f / std::sqrt (1.0f + std::pow (s.hpHz / 250.0f, 4.0f));
                return 0.25f + 0.75f * std::pow (h * hh, (float) n);
            };
            const auto dark = juce::Colour (0xff282c34);
            for (auto& tp : taps)
            {
                if (tp.t > win) continue;
                const float k = shade (tp.n);
                if (tp.lane == 0 || tp.lane == 2)
                    bar (tp.t, tp.a, 0, dark.interpolatedWith (juce::Colour (0xff62b6ea), k), tp.n);
                if (tp.lane == 1 || tp.lane == 2)
                    bar (tp.t, tp.a, 1, dark.interpolatedWith (juce::Colour (0xffec7a92), k), tp.n);
            }
        }
    }

private:
    DelaySettings s;
    double bpm = 120.0;
};
