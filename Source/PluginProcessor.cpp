#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

using APVTS = juce::AudioProcessorValueTreeState;

// =============================================================================
//  パラメーター定義
//  NormalisableRange の setSkewForCentre は「ノブの真ん中を何にするか」。
//  ms や Hz は対数的に聴こえるので、中央を小さめの値に寄せると扱いやすい。
// =============================================================================
APVTS::ParameterLayout DelayBasicsLabProcessor::createLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    auto pid = [] (const char* id) { return juce::ParameterID { id, 1 }; };

    auto msRange = juce::NormalisableRange<float> (1.0f, 2000.0f, 0.1f);   // 1/2音符 @60BPM まで表示できる範囲
    msRange.setSkewForCentre (150.0f);
    auto hpRange = juce::NormalisableRange<float> (20.0f, 2000.0f, 1.0f);
    hpRange.setSkewForCentre (200.0f);
    auto lpRange = juce::NormalisableRange<float> (500.0f, 20000.0f, 1.0f);
    lpRange.setSkewForCentre (3500.0f);
    auto rateRange = juce::NormalisableRange<float> (0.05f, 6.0f, 0.01f);
    rateRange.setSkewForCentre (0.8f);

    // 新規に立ち上げた時の値＝プリセット「Default」の値（比較表が「= 初期値」から始まるように）
    // ※ Time の初期値はテンポ同期（1/4）中の 120BPM 換算。同期中は実際のテンポに追従する。
    const auto& d = factoryPresets().front();

    p.push_back (std::make_unique<juce::AudioParameterChoice> (pid (ParamID::mode), "Mode", modeNames(), (int) d.mode));
    p.push_back (std::make_unique<juce::AudioParameterFloat>  (pid (ParamID::timeL), "Time L", msRange,
                    (float) resolveTimeMs (d.syncL, d.timeL, 120.0),
                    juce::AudioParameterFloatAttributes().withLabel ("ms")));
    p.push_back (std::make_unique<juce::AudioParameterFloat>  (pid (ParamID::timeR), "Time R", msRange,
                    (float) resolveTimeMs (d.syncR, d.timeR, 120.0),
                    juce::AudioParameterFloatAttributes().withLabel ("ms")));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (pid (ParamID::syncL), "Sync L", syncNames(), d.syncL));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (pid (ParamID::syncR), "Sync R", syncNames(), d.syncR));
    p.push_back (std::make_unique<juce::AudioParameterFloat>  (pid (ParamID::feedback), "Feedback",
                    juce::NormalisableRange<float> (0.0f, 95.0f, 0.1f), d.feedbackPct,
                    juce::AudioParameterFloatAttributes().withLabel ("%")));
    p.push_back (std::make_unique<juce::AudioParameterFloat>  (pid (ParamID::hp), "High Pass", hpRange, d.hp,
                    juce::AudioParameterFloatAttributes().withLabel ("Hz")));
    p.push_back (std::make_unique<juce::AudioParameterFloat>  (pid (ParamID::lp), "Low Pass", lpRange, d.lp,
                    juce::AudioParameterFloatAttributes().withLabel ("Hz")));
    p.push_back (std::make_unique<juce::AudioParameterFloat>  (pid (ParamID::rate), "Mod Rate", rateRange, d.rate,
                    juce::AudioParameterFloatAttributes().withLabel ("Hz")));
    p.push_back (std::make_unique<juce::AudioParameterFloat>  (pid (ParamID::depth), "Mod Depth",
                    juce::NormalisableRange<float> (0.0f, 5.0f, 0.01f), d.depth,
                    juce::AudioParameterFloatAttributes().withLabel ("ms")));
    p.push_back (std::make_unique<juce::AudioParameterFloat>  (pid (ParamID::wet), "Wet",
                    juce::NormalisableRange<float> (-30.0f, 0.0f, 0.1f), d.wetDb,
                    juce::AudioParameterFloatAttributes().withLabel ("dB")));
    p.push_back (std::make_unique<juce::AudioParameterBool>   (pid (ParamID::delayOff),  "Bypass",     false));
    p.push_back (std::make_unique<juce::AudioParameterBool>   (pid (ParamID::wetOnly),   "Wet Only",   false));
    p.push_back (std::make_unique<juce::AudioParameterBool>   (pid (ParamID::wetLock),   "Wet Lock",   false));
    p.push_back (std::make_unique<juce::AudioParameterBool>   (pid (ParamID::monoCheck), "Mono Check", false));
    // 入力の代わりに鳴らすサンプル音源（Off ならトラックの音をそのまま処理）
    p.push_back (std::make_unique<juce::AudioParameterChoice> (pid (ParamID::demo), "Sample Source", demoNames(), 0));

    return { p.begin(), p.end() };
}

DelayBasicsLabProcessor::DelayBasicsLabProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
    apvts.addParameterListener (ParamID::delayOff, this);
}

DelayBasicsLabProcessor::~DelayBasicsLabProcessor()
{
    apvts.removeParameterListener (ParamID::delayOff, this);
    cancelPendingUpdate();
}

void DelayBasicsLabProcessor::parameterChanged (const juce::String&, float newValue)
{
    if (newValue > 0.5f)          // オートメーションならオーディオスレッドから来るので非同期で処理
        triggerAsyncUpdate();
}

void DelayBasicsLabProcessor::handleAsyncUpdate()
{
    for (auto* id : { ParamID::wetOnly, ParamID::wetLock })
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (0.0f);
}

// 入力はモノ／ステレオ、出力はステレオのみ（Haasやピンポンは2chが前提）
bool DelayBasicsLabProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    const auto in  = layouts.getMainInputChannelSet();
    return out == juce::AudioChannelSet::stereo()
        && (in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo());
}

void DelayBasicsLabProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock);
    demo.prepare (sampleRate);
    dryCopy.setSize (2, samplesPerBlock);
    bypassMix.reset (sampleRate, 0.02);
    bypassMix.setCurrentAndTargetValue (apvts.getRawParameterValue (ParamID::delayOff)->load() > 0.5f ? 1.0f : 0.0f);
}

DelaySettings DelayBasicsLabProcessor::currentSettings() const
{
    auto v = [this] (const char* id) { return apvts.getRawParameterValue (id)->load(); };
    const double bpm = hostBpm.load();

    DelaySettings s;
    s.mode      = (DelayMode) (int) v (ParamID::mode);
    s.timeLms   = (float) resolveTimeMs ((int) v (ParamID::syncL), v (ParamID::timeL), bpm);
    s.timeRms   = (float) resolveTimeMs ((int) v (ParamID::syncR), v (ParamID::timeR), bpm);
    if (s.mode == DelayMode::Mono)
        s.timeRms = s.timeLms;                       // Mono は L の時間だけを使う
    s.feedback  = v (ParamID::feedback) / 100.0f;
    s.hpHz      = v (ParamID::hp);
    s.lpHz      = v (ParamID::lp);
    s.rateHz    = v (ParamID::rate);
    s.depthMs   = v (ParamID::depth);
    s.wetGain   = juce::Decibels::decibelsToGain (v (ParamID::wet));
    s.delayOff  = v (ParamID::delayOff)  > 0.5f;
    s.wetOnly   = v (ParamID::wetOnly)   > 0.5f;
    s.monoCheck = v (ParamID::monoCheck) > 0.5f;
    return s;
}

void DelayBasicsLabProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;   // 減衰していく尾で CPU が跳ねるのを防ぐ

    const int numIn  = getTotalNumInputChannels();
    const int numOut = getTotalNumOutputChannels();
    for (int ch = numIn; ch < numOut; ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    // ホストのテンポを取得（取れない時は前回値のまま）
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto bpm = pos->getBpm())
                hostBpm.store (*bpm);

    // サンプル音源がONなら、入力をサンプル音で置き換えてからディレイへ
    const int demoSource = (int) apvts.getRawParameterValue (ParamID::demo)->load();
    if (demoSource != DemoSynth::Off)
        demo.render (buffer, demoSource, hostBpm.load());

    // Bypass 用に「処理前の音」を保存（モノ入力なら L を R にも複製）
    const int n = buffer.getNumSamples();
    if (dryCopy.getNumSamples() < n)
        dryCopy.setSize (2, n, false, false, true);
    const bool monoIn = (demoSource == DemoSynth::Off && numIn < 2);
    for (int ch = 0; ch < 2; ++ch)
        dryCopy.copyFrom (ch, 0, buffer, monoIn ? 0 : juce::jmin (ch, buffer.getNumChannels() - 1), 0, n);

    engine.process (buffer, demoSource != DemoSynth::Off ? 2 : numIn, currentSettings());

    // Bypass：Wet Only / Mono / Haas の配線に関係なく、入力音をそのまま返す
    bypassMix.setTargetValue (apvts.getRawParameterValue (ParamID::delayOff)->load() > 0.5f ? 1.0f : 0.0f);
    if (bypassMix.isSmoothing() || bypassMix.getTargetValue() > 0.5f)
    {
        for (int i = 0; i < n; ++i)
        {
            const float b = bypassMix.getNextValue();
            for (int ch = 0; ch < juce::jmin (2, buffer.getNumChannels()); ++ch)
            {
                auto* d = buffer.getWritePointer (ch);
                d[i] = d[i] * (1.0f - b) + dryCopy.getSample (ch, i) * b;
            }
        }
    }
}

// =============================================================================
//  プリセット
// =============================================================================
int DelayBasicsLabProcessor::getNumPrograms()
{
    return (int) factoryPresets().size();
}

const juce::String DelayBasicsLabProcessor::getProgramName (int index)
{
    if (juce::isPositiveAndBelow (index, getNumPrograms()))
        return factoryPresets()[(size_t) index].name;
    return {};
}

void DelayBasicsLabProcessor::setCurrentProgram (int index)
{
    if (! juce::isPositiveAndBelow (index, getNumPrograms()))
        return;

    currentProgram = index;
    const auto& pr = factoryPresets()[(size_t) index];

    auto set = [this] (const char* id, float value)
    {
        if (auto* param = apvts.getParameter (id))
            param->setValueNotifyingHost (param->convertTo0to1 (value));
    };

    set (ParamID::mode,     (float) (int) pr.mode);
    // SYNC 付きのプリセットは、ノブにも今のテンポでの ms を入れておく
    const double bpm = hostBpm.load();
    set (ParamID::timeL,    (float) juce::jlimit (1.0, 2000.0, resolveTimeMs (pr.syncL, pr.timeL, bpm)));
    set (ParamID::timeR,    (float) juce::jlimit (1.0, 2000.0, resolveTimeMs (pr.syncR, pr.timeR, bpm)));
    set (ParamID::syncL,    (float) pr.syncL);
    set (ParamID::syncR,    (float) pr.syncR);
    set (ParamID::feedback, pr.feedbackPct);
    set (ParamID::hp,       pr.hp);
    set (ParamID::lp,       pr.lp);
    set (ParamID::rate,     pr.rate);
    set (ParamID::depth,    pr.depth);
    set (ParamID::delayOff, 0.0f);

    // Wet Lock が ON なら、Wet の量と Wet Only はプリセットを切り替えても今の設定のまま
    if (apvts.getRawParameterValue (ParamID::wetLock)->load() < 0.5f)
    {
        set (ParamID::wet,     pr.wetDb);
        set (ParamID::wetOnly, 0.0f);
    }
    if (pr.mode != DelayMode::Haas)
        set (ParamID::monoCheck, 0.0f);
}

// =============================================================================
//  状態の保存／復元
// =============================================================================
void DelayBasicsLabProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("program", currentProgram, nullptr);
    state.removeChild (state.getChildWithName (uiState.getType()), nullptr);
    state.addChild (uiState.createCopy(), -1, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void DelayBasicsLabProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto tree = juce::ValueTree::fromXml (*xml);
            currentProgram = (int) tree.getProperty ("program", 0);
            const auto savedUi = tree.getChildWithName (uiState.getType());
            if (savedUi.isValid())
            {
                uiState.copyPropertiesFrom (savedUi, nullptr);
                tree.removeChild (savedUi, nullptr);
            }
            apvts.replaceState (tree);
        }
}

juce::AudioProcessorEditor* DelayBasicsLabProcessor::createEditor()
{
    return new DelayBasicsLabEditor (*this);
}

// JUCE がプラグインを生成するときに呼ぶ関数
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DelayBasicsLabProcessor();
}
