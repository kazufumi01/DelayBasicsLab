# Delay Basics Lab（JUCE プラグイン版）

ブログ用Webアプリ「ディレイ基本用途ラボ」を AU / VST3 / Standalone にしたものです。
7つのプリセット（スラップ、ダブリング、Haas、ピンポン、付点8分＋4分、ダブ・スロー、ショート・アンビエンス）と、
エコーのタイムライン表示、各プリセットの解説・他プラグインでの再現設定をそのまま持っています。

- JUCE 8.0.16（CMake から自動取得）／C++17
- 出力：AU、VST3、Standalone（macOS 11以降・Apple Silicon/Intel ユニバーサル）

---

## 1. ビルド手順（macOS 13 ＋ Xcode）

ターミナルでこのフォルダに移動して：

```bash
cmake -B build -G Xcode
cmake --build build --config Release
```

- 初回は JUCE のダウンロードと全体のコンパイルで数分かかります。2回目以降は差分だけ。
- ビルドが終わると自動で以下にコピーされます。
  - AU：`~/Library/Audio/Plug-Ins/Components/Delay Basics Lab.component`
  - VST3：`~/Library/Audio/Plug-Ins/VST3/Delay Basics Lab.vst3`
- Standalone アプリは `build/DelayBasicsLab_artefacts/Release/Standalone/` にできます。

**Projucer と一緒に入れた JUCE を使いたい場合**（ダウンロードを省略）：

```bash
cmake -B build -G Xcode -DJUCE_PATH=/Applications/JUCE
```

（`JUCE_PATH` は JUCE フォルダの場所に合わせて変更。中に `modules/` と `CMakeLists.txt` があるフォルダです。JUCE 8 系を推奨）

**Xcode で開いて作業したい場合**：`build/DelayBasicsLab.xcodeproj` を開き、スキームを
`DelayBasicsLab_AU` などにして ⌘B。デバッグ時は Standalone スキームが一番手軽です。

**VS Code で作業したい場合**：拡張機能「CMake Tools」を入れてこのフォルダを開くだけ。
ジェネレーターは Xcode／Unix Makefiles どちらでも動きます。

## 2. Ableton Live で読み込む

1. Live の「設定 › Plug-Ins」で Audio Units（または VST3 システムフォルダ）をオンにし、「再スキャン」。
2. ブラウザの「プラグイン」に `JPK › Delay Basics Lab` が出ます。
3. プリセットは画面上のメニュー、または Live のデバイスのプログラム切替から選べます。

AU が出てこない時は、ターミナルで検証してみてください：

```bash
auval -v aufx Dbl1 Jpkl
```

最後に `AU VALIDATION SUCCEEDED` と出れば OK。直後に Live を再スキャン
（キャッシュが残っている場合は `killall -9 AudioComponentRegistrar` してから）。

## 3. 使い方のポイント

| つまみ | 意味 |
|---|---|
| Mode | Mono／Stereo（L→L, R→R）／Ping-Pong／Haas（左は原音、右だけ遅延） |
| Time L / R ＋ Sync | Sync が `Off (ms)` の時だけ ms ノブが有効。同期中はノブが薄く表示され、実際の ms は右上に表示 |
| Feedback | 0〜95%。ループ内のフィルターで、繰り返すほど細く・暗くなる |
| High / Low Pass | フィードバックの**ループ内**にあるフィルター |
| Mod Rate / Depth | ディレイタイムの揺れ。L と R は逆相 |
| Wet | エコーの音量（dB） |
| Bypass / Wet Only / Mono check | A/B比較、エコーだけ試聴、L+R を足して確認（Haas の痩せ方が分かる） |

- **センド（Return）で使う場合**は `Wet Only` をオン、`Wet` を 0dB 付近に。
- **Haas** はトラックにインサートで使う前提です（左が原音になるため）。

## 4. コードの構成（ブラウザ版との対応）

```
Source/
├── Params.h           パラメーターID、モード名、テンポ同期の計算（resolveTimeMs）
├── DelayEngine.h      ★DSPの本体（ブラウザ版の build()/apply() に相当）
├── Presets.h          7つのファクトリープリセットと日本語の解説文
├── PluginProcessor.*  ホストとのやり取り（パラメーター、テンポ取得、保存/復元、プリセット）
├── PluginEditor.*     画面（ノブ・メニュー・解説テキスト）
└── Timeline.h         エコーのタイムライン描画（ブラウザ版の draw() の移植）
```

| ブラウザ版（Web Audio） | プラグイン版（C++） |
|---|---|
| `DelayNode` | `FractionalDelay`（リングバッファ＋4点エルミート補間） |
| `BiquadFilter` ×2（HP/LP） | `juce::dsp::StateVariableTPTFilter` ×2 |
| `OscillatorNode` → `delayTime` | `std::sin(phase)` をディレイ時間に加算 |
| `setTargetAtTime(…, 0.05)` | `juce::SmoothedValue`（50msで滑らかに追従） |
| `GainNode`（feedback／wet／dry） | 1サンプルごとの掛け算 |
| 毎回 `build()` で配線し直す | `switch (mode)` で書き込み先を切り替えるだけ |

ブラウザ版はノードを「配線」して音を流しますが、プラグインでは**1サンプルずつ自分で計算**します。
`DelayEngine::process()` の for ループの中が、ブラウザ版の音の流れ図そのものです。

1. ディレイから読み出す（`read`）
2. HP → LP を通す（ループ内フィルター）
3. 入力＋フィルター後の音×Feedback を書き込む（`write`）← ここがモードごとに違う
4. Dry と Wet を混ぜて出力

**ブラウザ版で直したバグの反映**：モードを切り替えた瞬間にバッファをクリアし
（`if (s.mode != lastMode) reset();`）、平滑化の初期値もゼロ／目標値から始めています。
古いエコーや一瞬の大音量が出ないための処理です。

## 5. 次の一歩（コンプレッサー制作に向けて）

このプロジェクトはそのまま「自作プラグインの雛形」として使えます。

- `DelayEngine.h` を差し替えれば、別のエフェクトになります（コンプなら `CompressorEngine`）。
- パラメーターの追加は `Params.h` に ID → `PluginProcessor.cpp` の `createLayout()` → `PluginEditor.cpp` にノブ、の3か所。
- 練習課題の例：
  - ループ内に `std::tanh` を入れてテープ風の飽和を足す（アナログディレイ機器ラボの Drive）
  - Freeze ボタン（Feedback 100%、入力を止める）
  - `juce::dsp::Oversampling` で飽和の折り返しノイズを減らす

## 6. 補足

- `COMPANY_NAME`／`BUNDLE_ID`／`PLUGIN_MANUFACTURER_CODE` は `CMakeLists.txt` で変更できます。
  一度 Live に読み込ませた後に Plugin Code を変えると「別のプラグイン」として扱われるので注意。
- 配布：Mac は未公証の .pkg（`packaging/mac/build_pkg.sh`）、Windows は GitHub Actions でビルド（`.github/workflows/build.yml`）。
  インストール手順は `INSTALL.md`。
- ライセンス：JUCE は **Starter ライセンス** で使用。配布物はフリーウェア使用許諾（`packaging/mac/resources/license.txt`）に従う。
  ソースは学習目的で公開していますが、無断での再配布・販売はできません。
