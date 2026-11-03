#include "PluginEditor.h"
#include "Presets.h"

namespace
{
    const juce::Colour bg     (0xff15171b);
    const juce::Colour panel  (0xff1e2128);
    const juce::Colour panel2 (0xff252932);
    const juce::Colour line   (0xff323844);
    const juce::Colour ink    (0xffece9e3);
    const juce::Colour mute   (0xff9ba2ad);
    const juce::Colour accent (0xfff0a63a);

    // macOS では日本語を確実に表示するため Hiragino Sans を指定
    juce::Font uiFont (float size, bool bold = false)
    {
       #if JUCE_MAC
        return juce::Font (juce::FontOptions ("Hiragino Sans", size, bold ? juce::Font::bold : juce::Font::plain));
       #elif JUCE_WINDOWS
        // Windows 標準の日本語UIフォント（英語版Windowsにも入っている）
        return juce::Font (juce::FontOptions ("Yu Gothic UI", size, bold ? juce::Font::bold : juce::Font::plain));
       #else
        return juce::Font (juce::FontOptions (size, bold ? juce::Font::bold : juce::Font::plain));
       #endif
    }

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
}

DelayBasicsLabEditor::DelayBasicsLabEditor (DelayBasicsLabProcessor& p)
    : AudioProcessorEditor (&p), proc (p),
      presetButtons (presetLabels(), 1001),
      sampleButtons (demoNames(), 1002),
      modeButtons   (shortModeLabels(), 1003),
      syncLButtons  (shortSyncLabels(), 1004),
      syncRButtons  (shortSyncLabels(), 1005),
      langButtons   ({ juce::String::fromUTF8 ("日本語"), "English" }, 1006)
{
    using APVTS = juce::AudioProcessorValueTreeState;

    // --- 配色 ---
    lnf.setColour (juce::Slider::rotarySliderFillColourId,    accent);
    lnf.setColour (juce::Slider::rotarySliderOutlineColourId, line);
    lnf.setColour (juce::Slider::thumbColourId,               ink);
    lnf.setColour (juce::Slider::textBoxTextColourId,         ink);
    lnf.setColour (juce::TextButton::buttonColourId,          panel2);
    lnf.setColour (juce::TextButton::buttonOnColourId,        accent);
    lnf.setColour (juce::TextButton::textColourOffId,         mute);
    lnf.setColour (juce::TextButton::textColourOnId,          juce::Colour (0xff1a1305));
    lnf.setColour (juce::ComboBox::outlineColourId,           line);
    lnf.setColour (juce::TextEditor::backgroundColourId,      panel);
    lnf.setColour (juce::TextEditor::outlineColourId,         line);
    lnf.setColour (juce::TextEditor::textColourId,            ink);
    setLookAndFeel (&lnf);

    // --- 見出し ---
    title.setText ("DELAY BASICS LAB", juce::dontSendNotification);
    title.setFont (uiFont (20.0f, true));
    title.setColour (juce::Label::textColourId, ink);
    addAndMakeVisible (title);

    signature.setText ("by Johan Plus Kei", juce::dontSendNotification);
    signature.setFont (uiFont (12.0f));
    signature.setColour (juce::Label::textColourId, mute);
    signature.setJustificationType (juce::Justification::bottomLeft);
    addAndMakeVisible (signature);

    bpmLabel.setFont (uiFont (13.0f));
    bpmLabel.setColour (juce::Label::textColourId, mute);
    bpmLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (bpmLabel);

    auto setupTitle = [this] (juce::Label& l, const juce::String& text)
    {
        l.setText (text, juce::dontSendNotification);
        l.setFont (uiFont (11.0f, true));
        l.setColour (juce::Label::textColourId, accent);
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

    // 言語切替（日本語 / English）
    addAndMakeVisible (langButtons);
    langButtons.onSelect = [this] (int i) { applyLanguage (i == 0 ? Lang::Japanese : Lang::English); };
    langButtons.setSelected (proc.getLanguage() == Lang::Japanese ? 0 : 1);
    compare.setLanguage (proc.getLanguage());

    sampleButtons.attachTo (*proc.apvts.getParameter (ParamID::demo));
    modeButtons.attachTo   (*proc.apvts.getParameter (ParamID::mode));
    syncLButtons.attachTo  (*proc.apvts.getParameter (ParamID::syncL));
    syncRButtons.attachTo  (*proc.apvts.getParameter (ParamID::syncR));

    // --- 解説テキスト ---
    explain.setMultiLine (true);
    explain.setReadOnly (true);
    explain.setCaretVisible (false);
    explain.setScrollbarsShown (true);
    explain.setFont (uiFont (13.0f));
    // 角丸の背景は paint() で描くので、TextEditor 自身の背景と枠は透明に
    explain.setColour (juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    explain.setColour (juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    explain.setColour (juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    explain.setIndents (10, 8);
    explain.setScrollToShowCursor (false);   // テキスト差し替え時に末尾へスクロールしない
    addAndMakeVisible (explain);
    addAndMakeVisible (compare);
    addAndMakeVisible (timeline);

    // --- ノブ ---
    auto setupKnob = [this] (Knob& k, const char* id, const juce::String& name, const juce::String& suffix)
    {
        k.slider.setTextValueSuffix (suffix);
        k.slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        k.slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        k.label.setText (name, juce::dontSendNotification);
        k.label.setFont (uiFont (12.0f));
        k.label.setColour (juce::Label::textColourId, mute);
        k.label.setJustificationType (juce::Justification::centred);
        k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 18);
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

    // --- Bypass / Wet Only / Mono ---
    for (auto* b : { &delayOff, &wetOnly, &wetLock, &monoCheck })   // Wet ノブの横に並べるチェックボックス
    {
        b->setColour (juce::ToggleButton::textColourId, ink);
        b->setColour (juce::ToggleButton::tickColourId, accent);
        b->setColour (juce::ToggleButton::tickDisabledColourId, mute);
        addAndMakeVisible (*b);
    }
    offAtt     = std::make_unique<APVTS::ButtonAttachment> (proc.apvts, ParamID::delayOff,  delayOff);
    wetOnlyAtt = std::make_unique<APVTS::ButtonAttachment> (proc.apvts, ParamID::wetOnly,   wetOnly);
    wetLockAtt = std::make_unique<APVTS::ButtonAttachment> (proc.apvts, ParamID::wetLock,   wetLock);
    monoAtt    = std::make_unique<APVTS::ButtonAttachment> (proc.apvts, ParamID::monoCheck, monoCheck);

    showPresetText (proc.getCurrentProgram());
    setSize (900, 698);
    startTimerHz (20);
}

DelayBasicsLabEditor::~DelayBasicsLabEditor()
{
    setLookAndFeel (nullptr);
}

void DelayBasicsLabEditor::applyLanguage (Lang l)
{
    proc.setLanguage (l);
    compare.setLanguage (l);
    showPresetText (proc.getCurrentProgram());
}

void DelayBasicsLabEditor::showPresetText (int index)
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

void DelayBasicsLabEditor::timerCallback()
{
    const auto s   = proc.currentSettings();
    const auto bpm = proc.getHostBpm();
    timeline.update (s, bpm);
    compare.update (proc.getCurrentProgram(), proc.apvts, bpm);

    bpmLabel.setText ("Host " + juce::String (bpm, 1) + " BPM   |   L " + juce::String (s.timeLms, 1)
                      + " ms   R " + juce::String (s.timeRms, 1) + " ms", juce::dontSendNotification);

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

void DelayBasicsLabEditor::paint (juce::Graphics& g)
{
    g.fillAll (bg);
    g.setColour (panel);
    g.fillRoundedRectangle (knobPanel.toFloat(), 10.0f);

    // 説明ボックスの角丸パネル（比較表・タイムライン・ノブ欄と同じ半径10）
    const auto box = explain.getBounds().toFloat();
    g.setColour (panel);
    g.fillRoundedRectangle (box, 10.0f);
    g.setColour (line);
    g.drawRoundedRectangle (box.reduced (0.5f), 10.0f, 1.0f);
}

void DelayBasicsLabEditor::resized()
{
    auto r = getLocalBounds().reduced (12);
    const int labelW = 58, rowH = 28, gap = 8;

    auto header = r.removeFromTop (30);
    title.setBounds (header.removeFromLeft (196));
    signature.setBounds (header.removeFromLeft (140).withTrimmedBottom (7));
    langButtons.setBounds (header.removeFromRight (150).reduced (0, 3));
    header.removeFromRight (10);
    bpmLabel.setBounds (header);
    r.removeFromTop (gap);

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
    modeTitle.setBounds (row.removeFromLeft (labelW - 10));
    modeButtons.setBounds (row);
    r.removeFromTop (gap);

    // 解説（左）と パラメーター比較表（右）
    auto infoRow = r.removeFromTop (204);
    explain.setBounds (infoRow.removeFromLeft (infoRow.getWidth() / 2 - 4));
    infoRow.removeFromLeft (8);
    compare.setBounds (infoRow);
    r.removeFromTop (gap);

    timeline.setBounds (r.removeFromTop (170));
    r.removeFromTop (gap);

    // ノブ
    knobPanel = r.removeFromTop (120);
    auto knobRow = knobPanel.reduced (6, 6);
    // 右端：Bypass / Wet Only / Wet Lock / Mono のチェックボックス
    auto checks = knobRow.removeFromRight (118).reduced (4, 4);
    const int ch = checks.getHeight() / 4;
    delayOff.setBounds  (checks.removeFromTop (ch));
    wetOnly.setBounds   (checks.removeFromTop (ch));
    wetLock.setBounds   (checks.removeFromTop (ch));
    monoCheck.setBounds (checks);

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
    syncRTitle.setBounds (row.removeFromLeft (labelW - 10));
    syncRButtons.setBounds (row);
}

void DelayBasicsLabEditor::followSync (juce::Slider& knob, const char* timeId, const char* syncId, double bpm)
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

void DelayBasicsLabEditor::releaseSync (const char* syncId)
{
    auto* param = proc.apvts.getParameter (syncId);
    if (param->getValue() <= 0.0f)
        return;
    // ノブは既に音価の ms を指しているので、同期を外しても音は途切れず変わらない
    param->beginChangeGesture();
    param->setValueNotifyingHost (0.0f);
    param->endChangeGesture();
}
