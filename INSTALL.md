# Delay Basics Lab — インストール方法 / Installation

> **ベータ版です。** Mac の Ableton Live で動作確認済み。
> **Windows 版はビルドが通ることのみ確認しており、実機での動作確認はできていません。**
> 不具合は [Issues](https://github.com/kazufumi01/DelayBasicsLab/issues) へお知らせください。
>
> **Beta.** Tested with Ableton Live on Mac.
> **The Windows build compiles but has NOT been tested on a real Windows machine.**
> Please report problems via [Issues](https://github.com/kazufumi01/DelayBasicsLab/issues).

ダウンロード / Download：[Releases ページ](https://github.com/kazufumi01/DelayBasicsLab/releases/latest)

| ファイル | 内容 |
|---|---|
| `DelayBasicsLab-x.x.x-mac.zip` | Mac・手動インストール（おすすめ） |
| `DelayBasicsLab-x.x.x-mac.pkg` | Mac・インストーラー（ターミナルを使いたくない人向け） |
| `DelayBasicsLab-x.x.x-win64.zip` | Windows・手動インストール（未検証） |
| `DelayBasicsLab-x.x.x-win64.exe` | Windows・インストーラー（未検証） |

---

## 日本語

### Mac（macOS 11 以降・Apple Silicon / Intel）— 手動（zip）

1. zip をダブルクリックで展開。
2. Finder で option キーを押しながら「移動」メニュー › 「ライブラリ」を開き、次へドラッグでコピー（フォルダが無ければ作成）：
   - `Delay Basics Lab.component` → `ライブラリ/Audio/Plug-Ins/Components`
   - `Delay Basics Lab.vst3` → `ライブラリ/Audio/Plug-Ins/VST3`
   - `Delay Basics Lab.app` → `アプリケーション`（単体で使う場合のみ）
3. ターミナルで次の1行を実行（Apple の公証をしていないため、検疫フラグを外す）：
   ```bash
   xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/Components/"Delay Basics Lab.component" ~/Library/Audio/Plug-Ins/VST3/"Delay Basics Lab.vst3" /Applications/"Delay Basics Lab.app"
   ```
   .app を入れていない場合の「No such file」は無視して OK。**この手順を飛ばすと「壊れている」と表示され読み込めません。**
4. DAW を再起動して再スキャン。Live は「設定 › プラグイン › 再スキャン」。

### Mac — インストーラー（pkg）

1. `.pkg` をダブルクリック。「Apple は悪質なソフトウェアがないことを確認できません」と出たら **「完了」** で閉じる。
2. **システム設定 › プライバシーとセキュリティ** の下の方にある **「このまま開く」** をクリック → パスワード入力。
3. 入れたい形式（AU / VST3 / Standalone）を選んでインストール → DAW で再スキャン。

### Windows（10 / 11・64bit）— 未検証

**手動（zip）**
1. 展開前に zip を右クリック › プロパティ › **「許可する」** にチェック › OK。
2. 展開した `Delay Basics Lab.vst3` フォルダを丸ごと `C:\Program Files\Common Files\VST3` へコピー。
3. DAW で再スキャン（Live は「VST3 プラグイン システムフォルダ」をオン）。

**インストーラー（exe）**
1. `.exe` を実行。「Windows によって PC が保護されました」→ **「詳細情報」→「実行」**。
2. VST3 は `C:\Program Files\Common Files\VST3` に入ります。DAW で再スキャン。

### 読み込めない時
- **Mac・AU が出ない**：ターミナルで `killall -9 AudioComponentRegistrar` → DAW 再起動。
- **Mac・「壊れている」と出る**：上の手順3のコマンドを実行（pkg 版で入れた場合は `~/Library` を `/Library` に読み替え）。
- **アンインストール**：Mac は入れたファイルを削除（pkg 版は `/Library/Audio/Plug-Ins/…` と `/Applications`）。Windows は `.vst3` フォルダを削除、exe 版は「設定 › アプリ」から。

---

## English

### Mac (macOS 11+, Apple Silicon / Intel) — Manual (zip)

1. Unzip the file.
2. In Finder, hold Option › Go › Library, then copy (create folders if missing):
   - `Delay Basics Lab.component` → `Library/Audio/Plug-Ins/Components`
   - `Delay Basics Lab.vst3` → `Library/Audio/Plug-Ins/VST3`
   - `Delay Basics Lab.app` → `Applications` (optional, standalone)
3. Run this in Terminal (removes the quarantine flag; this beta is not notarized by Apple):
   ```bash
   xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/Components/"Delay Basics Lab.component" ~/Library/Audio/Plug-Ins/VST3/"Delay Basics Lab.vst3" /Applications/"Delay Basics Lab.app"
   ```
   Ignore "No such file" for the .app if you skipped it. **Without this step macOS reports the plug-in as "damaged".**
4. Restart your DAW and rescan plug-ins.

### Mac — Installer (pkg)

1. Double-click the `.pkg`. If macOS says it "cannot verify" the developer, click **Done**.
2. Open **System Settings › Privacy & Security**, click **Open Anyway**, enter your password.
3. Choose formats (AU / VST3 / Standalone), install, and rescan in your DAW.

### Windows (10 / 11, 64-bit) — untested

**Manual (zip)**
1. Before extracting, right-click the zip › Properties › tick **Unblock** › OK.
2. Copy the whole `Delay Basics Lab.vst3` folder to `C:\Program Files\Common Files\VST3`.
3. Rescan plug-ins in your DAW.

**Installer (exe)**
1. Run the `.exe`. If you see "Windows protected your PC", click **More info › Run anyway**.
2. VST3 goes to `C:\Program Files\Common Files\VST3`. Rescan in your DAW.

### Troubleshooting
- **Mac, AU not showing**: run `killall -9 AudioComponentRegistrar` in Terminal and restart the DAW.
- **Mac, "damaged"**: run the command in step 3 (use `/Library` instead of `~/Library` for the pkg install).
- **Uninstall**: delete the installed files. On Windows, the exe version can be removed from Settings › Apps.
