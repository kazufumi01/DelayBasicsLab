#include "PluginEditor.h"
#include "Presets.h"

namespace
{
    juce::StringArray presetLabels()
    {
        juce::StringArray s;
        for (auto& p : factoryPresets())
            s.add (p.name);
        return s;
    }

    // ボタンに収まるよう短い表記にする
    juce::StringArray shortSyncLabels()  { return { "ms", "1/16", "1/8T", "1/8", "1/8D", "1/4T", "1/4", "1/4D", "1/2" }; }
    juce::StringArray shortModeLabels()  { return { "Mono", "Stereo", "Ping-Pong", "Haas" }; }

    constexpr int headerH = 44;
}

DelayBasicsLabView::DelayBasicsLabView (DelayBasicsLabProcessor& p)
    : proc (p),
      presetButtons (presetLabels(), 1001),
      sampleButtons (demoNames(), 1002),
      modeButtons   (shortModeLabels(), 1003),
      syncLButtons  (shortSyncLabels(), 1004),
      syncRButtons  (shortSyncLabels(), 1005)
{
    using APVTS = juce::AudioProcessorValueTreeState;
    setLookAndFeel (&lnf);

    // --- セクション見出し（Menlo・グレー：EQ Anatomy / Meter Bridge と同じ） ---
    auto setupTitle = [this] (juce::Label& l, const juce::String& text)
    {
        l.setText (text, juce::dontSendNotification);
        l.setFont (theme::mono (10.0f, true));
        l.setColour (juce::Label::textColourId, theme::muted);
        addAndMakeVisible (l);
    };
    setupTitle (presetTitle, "PRESET");
    setupTitle (sampleTitle, "SAMPLE");
    setupTitle (modeTitle,   "MODE");
    setupTitle (syncLTitle,  "SYNC L");
    setupTitle (syncRTitle,  "SYNC R");

    // --- ボタン列 ---
    for (auto* cb : { &presetButtons, &sampleButtons, &modeButtons, &syncLButtons, &syncRButtons })
        addAndMakeVisible (*cb);

    presetButtons.onSelect = [this] (int index)          // プリセットはパラメーターではないので直接呼ぶ
    {
        proc.setCurrentProgram (index);
        showPresetText (index);
    };
    presetButtons.setSelected (proc.getCurrentProgram());

    sampleButtons.attachTo (*proc.apvts.getParameter (ParamID::demo));
    modeButtons.attachTo   (*proc.apvts.getParameter (ParamID::mode));
    syncLButtons.attachTo  (*proc.apvts.getParameter (ParamID::syncL));
    syncRButtons.attachTo  (*proc.apvts.getParameter (ParamID::syncR));

    // --- ヘッダー：画面サイズ・言語 ---
    addAndMakeVisible (zoom);
    addAndMakeVisible (langBtn);
    langBtn.onClick = [this] { applyLanguage (proc.getLanguage() == Lang::Japanese ? Lang::English : Lang::Japanese); };
    compare.setLanguage (proc.getLanguage());

    // --- 解説テキスト（レッスン欄と同じ色の箱） ---
    explain.setMultiLine (true);
    explain.setReadOnly (true);
    explain.setCaretVisible (false);
    explain.setScrollbarsShown (true);
    explain.setFont (theme::ui (13.0f));
    explain.setColour (juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    explain.setColour (juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    explain.setColour (juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    explain.setColour (juce::TextEditor::textColourId, theme::text);
    explain.setIndents (12, 10);
    explain.setScrollToShowCursor (false);   // テキスト差し替え時に末尾へスクロールしない
    addAndMakeVisible (explain);
    addAndMakeVisible (compare);
    addAndMakeVisible (timeline);

    // --- ノブ ---
    auto setupKnob = [this] (Knob& k, const char* id, const juce::String& name, const juce::String& suffix)
    {
        k.slider.setTextValueSuffix (suffix);
        k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 86, 18);
        k.label.setText (name.toUpperCase(), juce::dontSendNotification);
        k.label.setFont (theme::mono (10.0f, true));
        k.label.setColour (juce::Label::textColourId, theme::muted);
        k.label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (k.slider);
        addAndMakeVisible (k.label);
        k.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, id, k.slider);
    };
    setupKnob (timeL,    ParamID::timeL,    "Time L",    " ms");
    setupKnob (timeR,    ParamID::timeR,    "Time R",    " ms");
    setupKnob (feedback, ParamID::feedback, "Feedback",  " %");
    setupKnob (hp,       ParamID::hp,       "High Pass", " Hz");
    setupKnob (lp,       ParamID::lp,       "Low Pass",  " Hz");
    setupKnob (rate,     ParamID::rate,     "Mod Rate",  " Hz");
    setupKnob (depth,    ParamID::depth,    "Mod Depth", " ms");
    setupKnob (wet,      ParamID::wet,      "Wet",       " dB");

    // SYNC 中に Time ノブを触ったら、同期を外して「今の音価の ms」から手動で微調整できるようにする
    timeL.slider.onDragStart = [this] { releaseSync (ParamID::syncL); };
    timeR.slider.onDragStart = [this] { releaseSync (ParamID::syncR); };

    // --- Bypass / Wet Only / Wet Lock / Mono（EQ Anatomy の Auto gain 等と同じトグルボタン） ---
    for (auto* b : { &delayOff, &wetOnly, &wetLock, &monoCheck })
    {
        b->setClickingTogglesState (true);
        addAndMakeVisible (*b);
    }
    offAtt     = std::make_unique<APVTS::ButtonAttachment> (proc.apvts, ParamID::delayOff,  delayOff);
    wetOnlyAtt = std::make_unique<APVTS::ButtonAttachment> (proc.apvts, ParamID::wetOnly,   wetOnly);
    wetLockAtt = std::make_unique<APVTS::ButtonAttachment> (proc.apvts, ParamID::wetLock,   wetLock);
    monoAtt    = std::make_unique<APVTS::ButtonAttachment> (proc.apvts, ParamID::monoCheck, monoCheck);

    applyLanguage (proc.getLanguage());
    setSize (1000, 700);
    startTimerHz (20);
}

DelayBasicsLabView::~DelayBasicsLabView()
{
    setLookAndFeel (nullptr);
}

void DelayBasicsLabView::applyLanguage (Lang l)
{
    proc.setLanguage (l);
    compare.setLanguage (l);
    langBtn.setButtonText (l == Lang::Japanese ? juce::String ("EN") : juce::String::fromUTF8 ("日本語"));
    showPresetText (proc.getCurrentProgram());
}

void DelayBasicsLabView::showPresetText (int index)
{
    if (! juce::isPositiveAndBelow (index, (int) factoryPresets().size()))
        return;

    const auto lang = proc.getLanguage();
    const auto& pr  = factoryPresets()[(size_t) index];
    const bool  ja  = (lang == Lang::Japanese);
    const auto& en  = presetTextEN (index);

    auto txt = [ja] (const char* jpText, const char* enText) { return ja ? jp (jpText) : juce::String (enText); };

    explain.setText (juce::String (pr.name) + "  [" + txt (pr.tag, en.tag) + "]\n\n"
                     + ui (UiText::WhatHappens, lang)  + "\n" + txt (pr.what, en.what) + "\n\n"
                     + ui (UiText::WhereToUse, lang)   + "\n" + txt (pr.use,  en.use)  + "\n\n"
                     + ui (UiText::WhatToListen, lang) + "\n" + txt (pr.ear,  en.ear)  + "\n\n"
                     + ui (UiText::Footer, lang), false);
    explain.setCaretPosition (0);        // 差し替え後は先頭から表示
    shownPreset = index;
}

void DelayBasicsLabView::timerCallback()
{
    const auto s   = proc.currentSettings();
    const auto bpm = proc.getHostBpm();
    timeline.update (s, bpm);
    compare.update (proc.getCurrentProgram(), proc.apvts, bpm);

    const auto next = juce::String (bpm, 1) + " BPM    L " + juce::String (s.timeLms, 1)
                    + " ms    R " + juce::String (s.timeRms, 1) + " ms";
    if (next != readout) { readout = next; repaint (0, 0, getWidth(), headerH); }

    // SYNC で音価を選んでいる間は、Time ノブの値をその音価の ms に合わせる（テンポ変更にも追従）
    followSync (timeL.slider, ParamID::timeL, ParamID::syncL, bpm);
    followSync (timeR.slider, ParamID::timeR, ParamID::syncR, bpm);
    timeR.slider.setAlpha (s.mode == DelayMode::Mono ? 0.35f : 1.0f);   // Mono では R は使わない
    syncRButtons.setAlpha (s.mode == DelayMode::Mono ? 0.4f : 1.0f);

    // ホスト側でプリセットが切り替わった場合にも表示を追従
    if (proc.getCurrentProgram() != shownPreset)
    {
        presetButtons.setSelected (proc.getCurrentProgram());
        showPresetText (proc.getCurrentProgram());
    }
}

void DelayBasicsLabView::paint (juce::Graphics& g)
{
    g.fillAll (theme::bg);

    // ヘッダー
    g.setColour (theme::header); g.fillRect (0, 0, getWidth(), headerH);
    g.setColour (theme::line);   g.drawHorizontalLine (headerH, 0.0f, (float) getWidth());
    g.setColour (theme::text);   g.setFont (theme::mono (14.0f, true));
    g.drawText ("DELAY BASICS LAB", 14, 6, 170, 20, juce::Justification::centredLeft);
    g.setColour (theme::faint);  g.setFont (theme::ui (9.5f));
    g.drawText ("by Johan Plus Kei", 14, 24, 170, 14, juce::Justification::centredLeft);
    g.setColour (theme::muted);  g.setFont (theme::mono (11.0f));
    g.drawText ("HOST " + readout, 244, 12, 560, 20, juce::Justification::centredLeft);

    // ノブ欄
    g.setColour (theme::panel); g.fillRoundedRectangle (knobPanel.toFloat(), 6.0f);
    g.setColour (theme::line);  g.drawRoundedRectangle (knobPanel.toFloat().reduced (0.5f), 6.0f, 1.0f);

    // 解説（EQ Anatomy のレッスン欄と同じ配色）
    const auto box = explain.getBounds().toFloat();
    g.setColour (theme::lessonBg); g.fillRoundedRectangle (box, 6.0f);
    g.setColour (theme::lessonLn); g.drawRoundedRectangle (box.reduced (0.5f), 6.0f, 1.0f);
}

void DelayBasicsLabView::resized()
{
    // ヘッダー
    zoom.setBounds (178, 12, 46, 20);
    langBtn.setBounds (getWidth() - 12 - 64, 10, 64, 24);

    auto r = getLocalBounds().withTrimmedTop (headerH + 10).reduced (12, 0).withTrimmedBottom (12);
    const int labelW = 64, rowH = 28, gap = 8;

    // PRESET（8個）
    auto row = r.removeFromTop (rowH);
    presetTitle.setBounds (row.removeFromLeft (labelW));
    presetButtons.setBounds (row);
    r.removeFromTop (gap);

    // SAMPLE（4個）＋ MODE（4個）
    row = r.removeFromTop (rowH);
    auto left = row.removeFromLeft (row.getWidth() / 2 - 6);
    row.removeFromLeft (12);
    sampleTitle.setBounds (left.removeFromLeft (labelW));
    sampleButtons.setBounds (left);
    modeTitle.setBounds (row.removeFromLeft (labelW - 14));
    modeButtons.setBounds (row);
    r.removeFromTop (gap);

    // 解説（左・広く）と 初期値の比較表（右・狭く）
    auto infoRow = r.removeFromTop (220);
    compare.setBounds (infoRow.removeFromRight (330));
    infoRow.removeFromRight (8);
    explain.setBounds (infoRow);
    r.removeFromTop (gap);

    timeline.setBounds (r.removeFromTop (150));
    r.removeFromTop (gap);

    // ノブ ＋ 右端のスイッチ4つ
    knobPanel = r.removeFromTop (132);
    auto knobRow = knobPanel.reduced (8, 8);
    auto checks = knobRow.removeFromRight (112);
    knobRow.removeFromRight (8);
    const int bh = (checks.getHeight() - 3 * 4) / 4;
    for (auto* b : { &delayOff, &wetOnly, &wetLock, &monoCheck })
    {
        b->setBounds (checks.removeFromTop (bh));
        checks.removeFromTop (4);
    }

    Knob* knobs[] = { &timeL, &timeR, &feedback, &hp, &lp, &rate, &depth, &wet };
    const int w = knobRow.getWidth() / 8;
    for (auto* k : knobs)
    {
        auto cell = knobRow.removeFromLeft (w);
        k->label.setBounds (cell.removeFromTop (16));
        k->slider.setBounds (cell);
    }
    r.removeFromTop (gap);

    // テンポ同期 L / R（各9個）
    row = r.removeFromTop (rowH);
    left = row.removeFromLeft (row.getWidth() / 2 - 6);
    row.removeFromLeft (12);
    syncLTitle.setBounds (left.removeFromLeft (labelW));
    syncLButtons.setBounds (left);
    syncRTitle.setBounds (row.removeFromLeft (labelW - 14));
    syncRButtons.setBounds (row);
}

void DelayBasicsLabView::followSync (juce::Slider& knob, const char* timeId, const char* syncId, double bpm)
{
    const int sync = (int) proc.apvts.getRawParameterValue (syncId)->load();
    if (sync <= 0 || knob.isMouseButtonDown())
        return;

    auto* param  = proc.apvts.getParameter (timeId);
    const auto target = (float) juce::jlimit (1.0, 2000.0, resolveTimeMs (sync, 0.0, bpm));
    const auto now    = proc.apvts.getRawParameterValue (timeId)->load();
    if (std::abs (now - target) > 0.06f)                  // 0.1ms 刻みの丸め誤差は無視
        param->setValueNotifyingHost (param->convertTo0to1 (target));
}

void DelayBasicsLabView::releaseSync (const char* syncId)
{
    auto* param = proc.apvts.getParameter (syncId);
    if (param->getValue() <= 0.0f)
        return;
    // ノブは既に音価の ms を指しているので、同期を外しても音は途切れず変わらない
    param->beginChangeGesture();
    param->setValueNotifyingHost (0.0f);
    param->endChangeGesture();
}
