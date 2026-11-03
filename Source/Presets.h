#pragma once
#include <juce_core/juce_core.h>
#include <vector>
#include "Params.h"

// =============================================================================
//  ファクトリープリセット（ブラウザ版の7プリセットと同じ値・同じ解説）
//  sync: 0=Off, 1=1/16, 2=1/8T, 3=1/8, 4=1/8D, 5=1/4T, 6=1/4, 7=1/4D, 8=1/2
// =============================================================================
struct FactoryPreset
{
    const char* name;
    const char* tag;
    DelayMode mode;
    float timeL, timeR;
    int   syncL, syncR;
    float feedbackPct, hp, lp, rate, depth, wetDb;
    const char* what;
    const char* use;
    const char* ear;
    const char* plugins;
};

inline juce::String jp (const char* s) { return juce::String::fromUTF8 (s); }

inline const std::vector<FactoryPreset>& factoryPresets()
{
    static const std::vector<FactoryPreset> presets
    {
        { "Default", "汎用", DelayMode::Mono, 500, 500, 6, 6, 25, 200, 6000, 0.5f, 0.2f, -12,
          "4分音符のエコーが、少しずつ暗くなりながら3〜4回返ってくる。ディレイの“いつもの出発点”。",
          "ボーカル、ギター、シンセ、何にでも。センド（Wet Only）に挿して、まずこの状態から好みに寄せていく。",
          "Feedbackで回数、Low Passで奥行き、Wetで存在感。この3つだけで多くの曲に馴染む。動かした量は右の比較表で確認できる。",
          "" },

        { "Slapback", "存在感", DelayMode::Mono, 110, 110, 0, 0, 5, 200, 5000, 0.5f, 0.0f, -9,
          "原音の約0.1秒後に、高域を少し落とした反射が1回だけ返ってくる。50年代のテープ・スラップが原型。",
          "ボーカル、ギター、スネア。リバーブほど空間を濁さずに「前に出つつ立体的」な存在感を足せる。",
          "Bypassと切り替えて、歌の輪郭が太る感覚を確認。100msを境に、短いほどタイト、長いほどロカビリー的に。",
          "Ableton Echo：Stereo・Link ON／110ms／Feedback 5%／HP 200Hz・LP 5kHz／Wobble 5〜10%\n"
          "Valhalla Delay：Tape または HiFi／Single／110ms／Feedback 0〜10%\n"
          "EchoBoy：Single Echo／Studio Tape／110ms／Feedback 5〜10%" },

        { "Doubling", "太さ", DelayMode::Stereo, 13, 21, 0, 0, 0, 150, 9000, 0.7f, 1.2f, -4,
          "約30ms以内の遅れは「別のエコー」ではなく「同じ音の厚み」として融合して聴こえる（先行音効果）。L/Rで時間を変え、ゆっくり揺らすと二度録り風に。",
          "ボーカルの太さ、シンセ・ギターの広がり。ダブル録りの代用。",
          "Mono CheckをONにすると、短いディレイ特有のコムフィルターが出る。Depthを0にすると“揺れない厚み”の不自然さも分かる。",
          "Ableton Echo：Stereo・Link OFF／L 13ms・R 21ms／Feedback 0%／Modulation Rate 0.7Hz\n"
          "Valhalla Delay：Dual／13ms・21ms／Feedback 0%／HiFi（またはPitchでDetune ±8〜12cent）\n"
          "EchoBoy：Dual Echo／13ms・21ms／Feedback 0%／Studio Tape＋Wobble少し" },

        { "Haas Wide", "左右へ広げる", DelayMode::Haas, 12, 12, 0, 0, 0, 20, 20000, 0.5f, 0.0f, 0,
          "左は原音、右だけ12ms遅らせる。耳は早く届いた左に定位を感じつつ、右からも音が来るため横に広がって聴こえる。",
          "モノ素材（ギター、パッド、コーラス）をステレオに広げたい時。※トラックにインサートで使う。",
          "必ずMono CheckをON。L＋Rが足されると特定の周波数が打ち消し合い、音がペラペラになる。これがHaasの落とし穴。",
          "Ableton Delay：インサート／Link OFF／L 1ms・R 12ms／Feedback 0%／Dry/Wet 100%\n"
          "Valhalla Delay：Dual／L 0ms・R 12ms／Feedback 0%／Mix 100%\n"
          "EchoBoy：Dual Echoで片側だけ10〜20ms、Mix 100%" },

        { "Ping-Pong", "動き", DelayMode::PingPong, 250, 250, 3, 3, 45, 250, 4500, 0.5f, 0.0f, -11,
          "エコーが左→右→左…と交互に返る。片側のフィードバックをもう片側へ入れる配線（クロスフィードバック）。",
          "シンセのアルペジオ、ギターのリフ、ボーカルの語尾。中央の主役を邪魔せず、左右に動きを作れる。",
          "タイムラインで青と赤が交互に並ぶのを確認。HPを250Hz以上にすると、エコーがベースやキックと喧嘩しない。",
          "Ableton Echo：Ping Pong／Sync 1/8／Feedback 40〜50%／HP 250Hz・LP 4.5kHz\n"
          "Valhalla Delay：PingPong／L・R 1/8／Feedback 45%\n"
          "EchoBoy：Ping-Pong／1/8 Sync／Feedback 45%" },

        { "Dotted 8th + 1/4", "グルーヴ", DelayMode::Stereo, 300, 300, 4, 6, 35, 300, 3500, 0.4f, 0.3f, -12,
          "左に付点8分、右に4分。ズレた2つのリズムが重なって、フレーズの隙間に16分の“合いの手”が生まれる。",
          "シンプルな8分のギターやシンセを細かいリズムに変える定番。ボーカルのロングトーンの後ろにも。",
          "タイムラインの拍線に対して、付点8分のエコーが裏に入るのを見る。LPを下げるほど、エコーが原音の後ろに下がる。",
          "Ableton Echo：Stereo・Link OFF／L 1/8D・R 1/4／Feedback 35%／LP 3.5kHz\n"
          "Valhalla Delay：Dual／L 1/8D・R 1/4／Feedback 35%／High Cut 3.5kHz\n"
          "EchoBoy：Dual Echo／L 1/8D・R 1/4／Feedback 35%" },

        { "Dub Throw", "質感・展開", DelayMode::Mono, 500, 500, 7, 7, 72, 450, 2200, 0.35f, 0.8f, -8,
          "フィードバックを上げ、ループ内のHP/LPで1回ごとに帯域を狭める。リピートは細く・暗く・揺れながら消えていく。",
          "語尾やスネアの“投げ”（ディレイ・スロー）。センド量を1拍だけ上げるオートメーションで展開を作る。",
          "タイムラインの色が暗くなる＝高域が削れていく様子に注目。Feedback 90%付近は発振寸前（音量注意）。",
          "Ableton Echo：Sync 1/4D／Feedback 70%／HP 450Hz・LP 2.2kHz／Wobble 20%・Noise少量\n"
          "Valhalla Delay：Tape／Feedback 70%／Age 30%前後／Low Cut 450Hz・High Cut 2.2kHz\n"
          "EchoBoy：Space Echo／Feedback 70%／Saturationを上げて汚れを足す" },

        { "Short Ambience", "リバーブとの境界", DelayMode::Stereo, 43, 67, 0, 0, 48, 300, 3200, 0.3f, 0.6f, -10,
          "40〜80msの短いエコーを左右別時間でフィードバックさせると、反射が密集して部屋鳴りのように聴こえ始める。",
          "リバーブだと奥に引っ込みすぎる時の“近い空気感”。ドラムやアコギを濁らせずに馴染ませる。",
          "Feedbackを0→60%と動かし、どこから「エコー」が「響き」に変わるかを探す。",
          "Ableton Echo：Stereo・Link OFF／L 43ms・R 67ms／Feedback 45〜50%／Reverb少量\n"
          "Valhalla Delay：Ratio（61.8%前後）／Feedback 50%／Diffusionで疑似リバーブ化\n"
          "EchoBoy：Dual Echo／43ms・67ms／Feedback 45%／Style Tweak › Diffusion" },
    };
    return presets;
}
