#!/bin/zsh
# Delay Basics Lab を再ビルドする
# 使い方：
#   ~/Developer/DelayBasicsLab/build.sh            → ビルドして ~/Library/Audio/Plug-Ins にコピー（開発用）
#   ~/Developer/DelayBasicsLab/build.sh --no-copy  → ビルドのみ。コピーしない（配布版 .pkg のテスト用）
set -e
cd "$(dirname "$0")"

COPY=ON
for arg in "$@"; do
  case "$arg" in
    --no-copy) COPY=OFF ;;
    *) echo "不明なオプション: $arg（使えるのは --no-copy）"; exit 1 ;;
  esac
done

JUCE_DIR="$HOME/Documents/jpkplugins/JUCE"   # 手元の JUCE 9.0.4

if [ ! -f build/CMakeCache.txt ]; then
  # Xcode ジェネレーターは CMake 4.5 RC + Xcode 14 の組み合わせで失敗するため Makefile を使う
  cmake -B build -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER="$(xcrun -f clang)" \
    -DCMAKE_CXX_COMPILER="$(xcrun -f clang++)" \
    -DCMAKE_OSX_SYSROOT="$(xcrun --show-sdk-path)" \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DJUCE_PATH="$JUCE_DIR"
fi

# コピーする／しないを毎回反映（前回の設定が残らないように）
cmake -B build -DDBL_COPY_AFTER_BUILD="$COPY" > /dev/null

cmake --build build --config Release -j"$(sysctl -n hw.ncpu)"

if [ "$COPY" = "ON" ]; then
  # AU のキャッシュを更新して検証
  killall -9 AudioComponentRegistrar 2>/dev/null || true
  auval -v aufx Dbl1 Jpkl | tail -3
  echo "完了：Ableton Live で Plug-Ins を再スキャンしてください"
else
  echo "完了（コピーなし）：成果物は build/DelayBasicsLab_artefacts/Release/"
  echo "配布用 .pkg を作るには： packaging/mac/build_pkg.sh 1.0.0"
fi
