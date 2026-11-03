#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

// =============================================================================
//  ChoiceButtons：メニューの代わりに「横並びのボタン」で選ぶ部品
//  ・パラメーター（Choice）に attachTo() すると、DAWのオートメーションとも連動
//  ・パラメーターでないもの（プリセット）は onSelect / setSelected で使う
// =============================================================================
class ChoiceButtons : public juce::Component
{
public:
    ChoiceButtons (const juce::StringArray& labels, int radioGroupId)
    {
        for (int i = 0; i < labels.size(); ++i)
        {
            auto* b = buttons.add (new juce::TextButton (labels[i]));
            b->setClickingTogglesState (true);
            b->setRadioGroupId (radioGroupId);
            b->onClick = [this, i]
            {
                if (buttons[i]->getToggleState() && onSelect)
                    onSelect (i);
            };
            addAndMakeVisible (b);
        }
    }

    void attachTo (juce::RangedAudioParameter& param)
    {
        attachment = std::make_unique<juce::ParameterAttachment> (
            param, [this] (float v) { setSelected ((int) std::lround (v)); }, nullptr);
        onSelect = [this] (int i) { attachment->setValueAsCompleteGesture ((float) i); };
        attachment->sendInitialUpdate();
    }

    void setSelected (int index)
    {
        for (int i = 0; i < buttons.size(); ++i)
            buttons[i]->setToggleState (i == index, juce::dontSendNotification);
    }

    void resized() override
    {
        auto r = getLocalBounds();
        const int n = buttons.size();
        for (int i = 0; i < n; ++i)
        {
            const int x0 = r.getX() + r.getWidth() * i / n;
            const int x1 = r.getX() + r.getWidth() * (i + 1) / n;
            buttons[i]->setBounds (x0, r.getY(), x1 - x0, r.getHeight());
            int edges = 0;
            if (i > 0)     edges |= juce::Button::ConnectedOnLeft;
            if (i < n - 1) edges |= juce::Button::ConnectedOnRight;
            buttons[i]->setConnectedEdges (edges);
        }
    }

    std::function<void (int)> onSelect;

private:
    juce::OwnedArray<juce::TextButton> buttons;
    std::unique_ptr<juce::ParameterAttachment> attachment;
};
