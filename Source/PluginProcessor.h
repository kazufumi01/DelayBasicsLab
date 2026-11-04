#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "DelayEngine.h"
#include "DemoSynth.h"
#include "Localisation.h"

class DelayBasicsLabProcessor : public juce::AudioProcessor,
                                private juce::AudioProcessorValueTreeState::Listener,
                                private juce::AsyncUpdater
{
public:
    DelayBasicsLabProcessor();
    ~DelayBasicsLabProcessor() override;

    // --- オーディオ処理 ---
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;   // double版を隠さない（警告対策）

    // --- エディタ ---
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    // --- 基本情報 ---
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    // --- プリセット（ホストの「プログラム」としても見える） ---
    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    // --- 状態の保存／復元（DAWのプロジェクトに保存される） ---
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Editor から参照する
    juce::AudioProcessorValueTreeState apvts;
    double getHostBpm() const { return hostBpm.load(); }
    DelaySettings currentSettings() const;   // 今のパラメーター値をまとめて取得

    // 表示言語（オートメーション不要なのでパラメーターにせず、状態の中に保存）
    Lang getLanguage() const
    {
        const auto v = apvts.state.getProperty ("lang", (int) systemDefaultLang());
        return (int) v == 0 ? Lang::Japanese : Lang::English;
    }
    void setLanguage (Lang l) { apvts.state.setProperty ("lang", (int) l, nullptr); }

    // 画面の大きさなど（ScalingEditor が "zoom" を読み書きする。セッションに保存）
    juce::ValueTree uiState { "UI" };

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

private:
    // Bypass が ON になったら Wet Only / Wet Lock を OFF にする（メッセージスレッドで実行）
    void parameterChanged (const juce::String& id, float newValue) override;
    void handleAsyncUpdate() override;

    DelayEngine engine;
    juce::AudioBuffer<float> dryCopy;            // Bypass 用：処理前の音を保存
    juce::SmoothedValue<float> bypassMix;        // 0=エフェクト音, 1=入力音（クリック防止で20msクロスフェード）
    DemoSynth demo;
    std::atomic<double> hostBpm { 120.0 };
    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelayBasicsLabProcessor)
};
