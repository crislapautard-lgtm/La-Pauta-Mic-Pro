#pragma once

#include "PluginProcessor.h"

// Interruptor tipo "tic-tac" con texto ON / OFF
class ToggleSwitch : public juce::Button, private juce::Timer
{
public:
    ToggleSwitch() : juce::Button ("power")
    {
        setClickingTogglesState (true);
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
        startTimerHz (60);
    }

    void paintButton (juce::Graphics& g, bool isOver, bool) override
    {
        auto r = getLocalBounds().toFloat().reduced (2.0f);
        const float h = r.getHeight();

        // pista
        auto offCol = juce::Colour (0xff2a2c2f);
        auto onCol  = juce::Colour (0xff3fae4a);
        auto track = offCol.interpolatedWith (onCol, pos);

        g.setColour (juce::Colours::black.withAlpha (0.55f));
        g.fillRoundedRectangle (r.translated (0.0f, 1.5f), h * 0.5f);
        g.setGradientFill (juce::ColourGradient (track.brighter (0.15f), r.getX(), r.getY(),
                                                 track.darker (0.35f), r.getX(), r.getBottom(), false));
        g.fillRoundedRectangle (r, h * 0.5f);
        g.setColour (juce::Colours::white.withAlpha (isOver ? 0.55f : 0.3f));
        g.drawRoundedRectangle (r, h * 0.5f, 1.2f);

        // perilla
        const float d = h - 6.0f;
        const float x0 = r.getX() + 3.0f;
        const float x1 = r.getRight() - 3.0f - d;
        const float kx = x0 + (x1 - x0) * pos;
        juce::Rectangle<float> knob (kx, r.getY() + 3.0f, d, d);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xfff2f2f2), knob.getX(), knob.getY(),
                                                 juce::Colour (0xff8d9094), knob.getX(), knob.getBottom(), false));
        g.fillEllipse (knob);
        g.setColour (juce::Colours::black.withAlpha (0.6f));
        g.drawEllipse (knob, 1.0f);

        // texto
        g.setColour (juce::Colours::white.withAlpha (0.95f));
        g.setFont (juce::Font (juce::FontOptions (h * 0.42f, juce::Font::bold)));
        const float pad = 8.0f;
        if (pos > 0.5f)
            g.drawText ("ON",  juce::Rectangle<float> (r.getX() + pad, r.getY(), r.getWidth() - d - pad * 2, h),
                        juce::Justification::centred);
        else
            g.drawText ("OFF", juce::Rectangle<float> (r.getX() + d + pad, r.getY(), r.getWidth() - d - pad * 2, h),
                        juce::Justification::centred);
    }

private:
    void timerCallback() override
    {
        const float target = getToggleState() ? 1.0f : 0.0f;
        if (std::abs (target - pos) > 0.002f)
        {
            pos += (target - pos) * 0.35f;
            repaint();
        }
        else if (pos != target)
        {
            pos = target;
            repaint();
        }
    }

    float pos = 0.0f;
};

class LaPautaMicProAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit LaPautaMicProAudioProcessorEditor (LaPautaMicProAudioProcessor&);
    ~LaPautaMicProAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void buildBackground();

    LaPautaMicProAudioProcessor& processor;
    ToggleSwitch toggle;
    juce::ButtonParameterAttachment attachment;
    juce::Image background;

    // 12 cm x 8 cm a 96 ppp
    static constexpr int W = 454;
    static constexpr int H = 302;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LaPautaMicProAudioProcessorEditor)
};
