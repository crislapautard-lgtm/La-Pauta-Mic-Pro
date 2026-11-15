#pragma once

#include "PluginProcessor.h"

// Interruptor moderno tipo "pill" con texto ON / OFF y brillo verde al encender
class ToggleSwitch : public juce::Button, private juce::Timer
{
public:
    ToggleSwitch() : juce::Button ("power")
    {
        setClickingTogglesState (true);
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
        startTimerHz (60);
    }

    void paintButton (juce::Graphics& g, bool isOver, bool isDown) override
    {
        using namespace juce;

        auto r = getLocalBounds().toFloat().reduced (12.0f, 10.0f);
        const float h = r.getHeight();
        const float corner = h * 0.5f;

        Path pill;
        pill.addRoundedRectangle (r, corner);

        // sombra y resplandor
        DropShadow (Colours::black.withAlpha (0.65f), 7, Point<int> (0, 3)).drawForPath (g, pill);
        if (pos > 0.01f)
            DropShadow (Colour (0xff35e06a).withAlpha (0.70f * pos), 13, Point<int> (0, 0)).drawForPath (g, pill);

        // pista
        const auto offCol = Colour (0xff1c1e22);
        const auto onCol  = Colour (0xff22b34f);
        const auto track  = offCol.interpolatedWith (onCol, pos);
        g.setGradientFill (ColourGradient (track.darker (0.35f), 0.0f, r.getY(),
                                           track.brighter (0.15f), 0.0f, r.getBottom(), false));
        g.fillPath (pill);

        // brillo superior (efecto cristal)
        g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.16f), 0.0f, r.getY(),
                                           Colours::white.withAlpha (0.0f), 0.0f, r.getCentreY(), false));
        g.fillRoundedRectangle (r.reduced (2.0f, 1.5f).withHeight (h * 0.5f), corner);

        g.setColour (Colours::white.withAlpha (isOver ? 0.42f : 0.24f));
        g.drawRoundedRectangle (r, corner, 1.1f);

        // perilla
        const float d = h - 8.0f;
        const float x0 = r.getX() + 4.0f;
        const float x1 = r.getRight() - 4.0f - d;
        const float kx = x0 + (x1 - x0) * pos;
        Rectangle<float> knob (kx, r.getY() + 4.0f, d, d);

        Path kp;
        kp.addEllipse (knob);
        DropShadow (Colours::black.withAlpha (0.55f), 4, Point<int> (0, 2)).drawForPath (g, kp);

        g.setGradientFill (ColourGradient (Colour (0xfffafafa), knob.getX() + d * 0.3f, knob.getY() + d * 0.2f,
                                           Colour (0xff8e939b), knob.getRight(), knob.getBottom(), true));
        g.fillEllipse (isDown ? knob.reduced (0.8f) : knob);
        g.setColour (Colours::black.withAlpha (0.45f));
        g.drawEllipse (knob, 0.9f);

        // texto ON / OFF
        const float tw = r.getWidth() - d - 16.0f;
        g.setFont (Font (FontOptions ("Segoe UI", h * 0.40f, Font::bold).withKerningFactor (0.10f)));

        g.setColour (Colours::white.withAlpha (0.95f * pos));
        g.drawText ("ON", Rectangle<float> (r.getX() + 10.0f, r.getY(), tw, h), Justification::centred);

        g.setColour (Colour (0xffb9bec6).withAlpha (0.90f * (1.0f - pos)));
        g.drawText ("OFF", Rectangle<float> (r.getRight() - 10.0f - tw, r.getY(), tw, h), Justification::centred);
    }

private:
    void timerCallback() override
    {
        const float target = getToggleState() ? 1.0f : 0.0f;
        if (std::abs (target - pos) > 0.003f)
        {
            pos += (target - pos) * 0.30f;
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

class LaPautaMicProAudioProcessorEditor : public juce::AudioProcessorEditor,
                                          private juce::Timer
{
public:
    explicit LaPautaMicProAudioProcessorEditor (LaPautaMicProAudioProcessor&);
    ~LaPautaMicProAudioProcessorEditor() override { stopTimer(); }

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void drawBackdrop (juce::Graphics&);
    void drawMic (juce::Graphics&);

    LaPautaMicProAudioProcessor& proc;
    ToggleSwitch toggle;
    juce::ButtonParameterAttachment attachment;

    juce::Image backdrop, micLayer;
    float glow = 0.0f;

    // 12 cm x 8 cm a 96 ppp
    static constexpr int W = 454;
    static constexpr int H = 302;
    static constexpr float micX = 288.0f;   // centro del micrófono

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LaPautaMicProAudioProcessorEditor)
};
