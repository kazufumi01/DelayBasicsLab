# Delay Basics Lab

**by Johan Plus Kei** · VST3 / AU / Standalone · 無料ベータ / Free beta

🇯🇵 ディレイの基本用途 7 種を、触りながら学べるプラグイン。ブログ用Webアプリ「[ディレイ基本用途ラボ](https://kazufumi01.github.io/delay-basics-lab/)」のプラグイン版です。
🇬🇧 A plug-in for learning the seven classic uses of delay hands-on. It is the plug-in version of the browser app [Delay Basics Lab](https://kazufumi01.github.io/delay-basics-lab/en/).

**[日本語](#日本語) · [English](#english) · [ダウンロード / Download](#ダウンロード--download) · [開発者向け / For developers](#開発者向け--for-developers)**

## ダウンロード / Download

**[最新版をダウンロード / Download latest (v0.1.1 beta)](https://github.com/kazufumi01/DelayBasicsLab/releases/latest)**

| | 手動インストール（おすすめ）<br>Manual (recommended) | インストーラー<br>Installer |
|---|---|---|
| **Mac**<br>macOS 11+, Apple Silicon / Intel | `…-mac.zip` | `…-mac.pkg` |
| **Windows** ⚠️ 未検証 / untested<br>10 / 11, 64-bit | `…-win64.zip` | `…-win64.exe` |

| | 日本語 | English |
|---|---|---|
| インストール方法 / How to install | [INSTALL.md（日本語）](INSTALL.md#日本語) | [INSTALL.md (English)](INSTALL.md#english) |
| ブラウザ版 / Browser version | [ディレイ基本用途ラボ](https://kazufumi01.github.io/delay-basics-lab/) | [Delay Basics Lab](https://kazufumi01.github.io/delay-basics-lab/en/) |
| 不具合・要望 / Bugs & requests | [Issues](https://github.com/kazufumi01/DelayBasicsLab/issues) | [Issues](https://github.com/kazufumi01/DelayBasicsLab/issues) |

> **ご注意 / Notes**
> - ベータ版です。Mac の Ableton Live で動作確認済み。<br>This is a beta, tested with Ableton Live on Mac.
> - ⚠️ Windows 版はビルドのみ確認済みで、実機での動作確認はできていません。<br>The Windows build compiles but has not been tested on real hardware.
> - Apple の公証・Windows のコード署名をしていないため、初回のみ警告が出ます（回避手順は INSTALL.md）。<br>The builds are not notarized or code-signed, so you will see a one-time warning (see INSTALL.md).

---

## 日本語

### 概要

7つのプリセット（スラップ、ダブリング、Haas、ピンポン、付点8分＋4分、ダブ・スロー、ショート・アンビエンス）と、
エコーのタイムライン表示、各プリセットの解説・他プラグインでの再現設定を、ブラウザ版からそのまま持っています。日本語 / English 切り替え対応。

### 使い方のポイント

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

### Ableton Live で読み込む

1. Live の「設定 › Plug-Ins」で Audio Units（または VST3 システムフォルダ）をオンにし、「再スキャン」。
2. ブラウザの「プラグイン」に `JPK › Delay Basics Lab` が出ます。
3. プリセットは画面上のメニュー、または Live のデバイスのプログラム切替から選べます。

AU が出てこない時は、ターミナルで検証してみてください：

```bash
auval -v aufx Dbl1 Jpkl
```

最後に `AU VALIDATION SUCCEEDED` と出れば OK。直後に Live を再スキャン
（キャッシュが残っている場合は `killall -9 AudioComponentRegistrar` してから）。

---

## English

### Overview

Seven presets — slapback, doubling, Haas, ping-pong, dotted eighth + quarter, dub throw and short ambience — with an echo timeline, an explanation of each preset and matching settings for other delay plug-ins, all carried over from the browser version. Japanese / English UI.

### Controls

| Control | What it does |
|---|---|
| Mode | Mono / Stereo (L→L, R→R) / Ping-Pong / Haas (left is dry, only the right is delayed) |
| Time L / R + Sync | The ms knobs work only when Sync is `Off (ms)`. While synced they are dimmed and the actual ms is shown top right |
| Feedback | 0–95%. Filters inside the loop make each repeat thinner and darker |
| High / Low Pass | Filters **inside** the feedback loop |
| Mod Rate / Depth | Delay-time wobble; L and R move in opposite phase |
| Wet | Echo level (dB) |
| Bypass / Wet Only / Mono check | A/B compare, hear only the echoes, and sum L+R to check (shows how Haas thins out) |

- **On a send (Return)**, turn on `Wet Only` and set `Wet` around 0 dB.
- **Haas** is meant as an insert on the track (the left side is the dry signal).

### Loading in Ableton Live

1. In Live › Settings › Plug-Ins, turn on Audio Units (or the VST3 system folder) and click Rescan.
2. The plug-in appears under Plug-Ins as `JPK › Delay Basics Lab`.
3. Choose presets from the menu on screen or via Live's program change for the device.

If the AU does not appear, validate it in Terminal: `auval -v aufx Dbl1 Jpkl`. When it ends with `AU VALIDATION SUCCEEDED`, rescan in Live (if a cache is stuck, run `killall -9 AudioComponentRegistrar` first).

---

## 開発者向け / For developers

> 🇬🇧 English summary: JUCE 8.0.16 (fetched by CMake), C++17. Build with `cmake -B build -G Xcode && cmake --build build --config Release` (add `-DJUCE_PATH=/path/to/JUCE` to use a local JUCE). The DSP lives in `Source/DelayEngine.h`; presets in `Source/Presets.h`; parameters in `Source/Params.h`. Mac packages: `packaging/mac/build_pkg.sh`; Windows: GitHub Actions (`.github/workflows/build.yml`). JUCE is used under the Starter licence; the source is public for learning, and redistribution or sale without permission is not allowed.

- JUCE 8.0.16（CMake から自動取得）／C++17
- 出力：AU、VST3、Standalone（macOS 11以降・Apple Silicon/Intel ユニバーサル）

### ビルド手順（macOS 13 ＋ Xcode）

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

### コードの構成（ブラウザ版との対応）

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

### 次の一歩（自作プラグインの雛形として）

このプロジェクトはそのまま「自作プラグインの雛形」として使えます。

- `DelayEngine.h` を差し替えれば、別のエフェクトになります（コンプなら `CompressorEngine`）。
- パラメーターの追加は `Params.h` に ID → `PluginProcessor.cpp` の `createLayout()` → `PluginEditor.cpp` にノブ、の3か所。
- 練習課題の例：
  - ループ内に `std::tanh` を入れてテープ風の飽和を足す（アナログディレイ機器ラボの Drive）
  - Freeze ボタン（Feedback 100%、入力を止める）
  - `juce::dsp::Oversampling` で飽和の折り返しノイズを減らす

### 補足

- `COMPANY_NAME`／`BUNDLE_ID`／`PLUGIN_MANUFACTURER_CODE` は `CMakeLists.txt` で変更できます。
  一度 Live に読み込ませた後に Plugin Code を変えると「別のプラグイン」として扱われるので注意。
- 配布：Mac は未公証の .pkg（`packaging/mac/build_pkg.sh`）、Windows は GitHub Actions でビルド（`.github/workflows/build.yml`）。
  インストール手順は `INSTALL.md`。
- ライセンス：JUCE は **Starter ライセンス** で使用。配布物はフリーウェア使用許諾（`packaging/mac/resources/license.txt`）に従う。
  ソースは学習目的で公開していますが、無断での再配布・販売はできません。

---

## 気に入ったら / If you like it

**🇯🇵 気に入ったら、曲の再生とページの拡散をお願いします！** Delay Basics Lab は無料です。友人や授業へのシェア、[ブログ記事](https://jpk.seesaa.net/article/521891797.html)の拡散、曲の再生が何よりの応援になります。

**🇬🇧 If you like it, please play my music and share this page!** Delay Basics Lab is free, and streams and shares help more than anything else.

[![Listen on Spotify](https://img.shields.io/badge/Spotify-Listen-1DB954?logo=spotify&logoColor=white)](https://open.spotify.com/artist/30PV2gqUbDDwvuWtKaok0x)
[![Apple Music](https://img.shields.io/badge/Apple_Music-Listen-FA243C?logo=applemusic&logoColor=white)](https://music.apple.com/us/artist/johan-plus-kei/519360150)
[![Linktree](https://img.shields.io/badge/Linktree-All_links-43E55E?logo=linktree&logoColor=white)](https://linktr.ee/jpk01)
[![X](https://img.shields.io/badge/X-Follow-000000?logo=x&logoColor=white)](https://x.com/johanpluskei)
[![Instagram](https://img.shields.io/badge/Instagram-Follow-E4405F?logo=instagram&logoColor=white)](https://www.instagram.com/johanpluskei)

**Johan Plus Kei** — dream pop / indie pop / post-rock · [Learning labs](https://kazufumi01.github.io/)

PayPal からの投げ銭も歓迎です / Tips via [PayPal](https://paypal.me/johanpluskei) are welcome too. More: <https://kazufumi01.github.io/support/>

---

**© 2026 Johan Plus Kei. All rights reserved.**
プラグイン・ソースコード・画像・文章の著作権その他の権利は Johan Plus Kei に帰属します。プラグインは無料で使えますが、ソースは学習のために公開しているもので、無断での転用・再配布・販売はできません。JUCE（Starter ライセンス）と VST3 SDK を使用しています。VST は Steinberg Media Technologies GmbH の登録商標です。
The copyright and all other rights in the plug-in, source code, images and text belong to Johan Plus Kei. The plug-in is free to use; the source is published for learning only and may not be reused, redistributed or sold without permission. Built with JUCE (Starter licence) and the VST3 SDK. VST is a registered trademark of Steinberg Media Technologies GmbH.
