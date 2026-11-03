# Delay Basics Lab — インストール方法 / Installation

## 日本語

### Mac（macOS 11 以降・Apple Silicon / Intel）
1. `DelayBasicsLab-x.x.x-mac.pkg` をダブルクリック。
2. 「Apple は悪質なソフトウェアがないことを確認できません」と出たら **「完了」**（または「OK」）を押して閉じる。
3. **システム設定 › プライバシーとセキュリティ** を開き、下の方にある「"DelayBasicsLab…pkg" は開けませんでした」の横の **「このまま開く」** をクリック → パスワードを入力。
4. インストーラーが起動したら、入れたい形式（AU / VST3 / Standalone）を選んでインストール。
5. DAW でプラグインを再スキャン。Live なら「設定 › プラグイン › 再スキャン」。

> 開発者登録（Apple の公証）をしていない無料配布のため、最初の一度だけ手順 2〜3 が必要です。インストール後のプラグインは通常どおり読み込めます。

### Windows（10 / 11・64bit）
1. `DelayBasicsLab-x.x.x-win64.exe` を実行。
2. 「Windows によって PC が保護されました」と出たら **「詳細情報」→「実行」**。
3. インストール先：VST3 は `C:\Program Files\Common Files\VST3`。
4. DAW でプラグインを再スキャン。

### 読み込めない時
- **Mac・AU が出ない**：ターミナルで `killall -9 AudioComponentRegistrar` を実行してから DAW を再起動。
- **Mac・zip 版などで「壊れている」と出る**：
  `xattr -dr com.apple.quarantine "/Library/Audio/Plug-Ins/VST3/Delay Basics Lab.vst3"`
- **アンインストール**：Mac は上記フォルダと `/Applications/Delay Basics Lab.app` を削除。Windows は「設定 › アプリ」から削除。

---

## English

### Mac (macOS 11+, Apple Silicon / Intel)
1. Double-click `DelayBasicsLab-x.x.x-mac.pkg`.
2. If macOS says it "cannot verify" the developer, click **Done** (or OK).
3. Open **System Settings › Privacy & Security**, scroll down and click **Open Anyway** next to the installer message, then enter your password.
4. Choose the formats you want (AU / VST3 / Standalone) and install.
5. Rescan plug-ins in your DAW.

> This free release is not notarized by Apple, so steps 2–3 are needed once. The installed plug-ins load normally afterwards.

### Windows (10 / 11, 64-bit)
1. Run `DelayBasicsLab-x.x.x-win64.exe`.
2. If you see "Windows protected your PC", click **More info › Run anyway**.
3. VST3 is installed to `C:\Program Files\Common Files\VST3`.
4. Rescan plug-ins in your DAW.
