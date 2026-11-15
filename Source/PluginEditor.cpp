#include "PluginEditor.h"

using namespace juce;

namespace
{
    // Gradiente horizontal tipo cilindro (luz desde la izquierda)
    ColourGradient cylinder (Rectangle<float> r, Colour dark, Colour mid, Colour light)
    {
        ColourGradient cg (dark, r.getX(), 0.0f, dark.darker (0.5f), r.getRight(), 0.0f, false);
        cg.addColour (0.16, mid);
        cg.addColour (0.32, light);
        cg.addColour (0.50, mid);
        cg.addColour (0.78, dark);
        return cg;
    }

    // Madera con vetas, nudos y bordes
    void drawWood (Graphics& g, Rectangle<float> b, bool horizontal, Colour light, Colour dark, Random& r)
    {
        g.setGradientFill (horizontal
            ? ColourGradient (light, 0.0f, b.getY(), dark, 0.0f, b.getBottom(), false)
            : ColourGradient (light, b.getX(), 0.0f, dark, b.getRight(), 0.0f, false));
        g.fillRect (b);

        Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (b.toNearestInt());

        const int lines = (int) ((horizontal ? b.getHeight() : b.getWidth()) * 1.8f) + 4;
        for (int i = 0; i < lines; ++i)
        {
            Path p;
            if (horizontal)
            {
                const float y = b.getY() + r.nextFloat() * b.getHeight();
                p.startNewSubPath (b.getX(), y + (r.nextFloat() - 0.5f) * 1.5f);
                p.quadraticTo (b.getCentreX(), y + (r.nextFloat() - 0.5f) * 4.0f,
                               b.getRight(), y + (r.nextFloat() - 0.5f) * 2.0f);
            }
            else
            {
                const float x = b.getX() + r.nextFloat() * b.getWidth();
                p.startNewSubPath (x + (r.nextFloat() - 0.5f) * 1.5f, b.getY());
                p.quadraticTo (x + (r.nextFloat() - 0.5f) * 4.0f, b.getCentreY(),
                               x + (r.nextFloat() - 0.5f) * 2.0f, b.getBottom());
            }
            g.setColour ((r.nextFloat() < 0.75f ? Colours::black : Colours::white)
                             .withAlpha (0.04f + r.nextFloat() * 0.10f));
            g.strokePath (p, PathStrokeType (0.5f + r.nextFloat() * 0.7f));
        }

        if (r.nextFloat() < 0.45f)   // nudo
        {
            const float kx = b.getX() + b.getWidth() * (0.2f + 0.6f * r.nextFloat());
            const float ky = b.getY() + b.getHeight() * (0.2f + 0.6f * r.nextFloat());
            for (int k = 0; k < 4; ++k)
            {
                const float rr = 2.0f + k * 2.2f;
                g.setColour (Colours::black.withAlpha (0.16f - k * 0.025f));
                g.drawEllipse (kx - rr * 1.5f, ky - rr * 0.7f, rr * 3.0f, rr * 1.4f, 0.8f);
            }
        }

        g.setColour (Colours::black.withAlpha (0.55f));
        g.drawRect (b, 1.0f);
        g.setColour (Colours::white.withAlpha (0.10f));
        if (horizontal) g.drawLine (b.getX(), b.getY() + 1.0f, b.getRight(), b.getY() + 1.0f, 1.0f);
        else            g.drawLine (b.getX() + 1.0f, b.getY(), b.getX() + 1.0f, b.getBottom(), 1.0f);
    }

    void drawScrew (Graphics& g, float x, float y, float rad, float angle)
    {
        g.setGradientFill (ColourGradient (Colour (0xffe6e8ec), x - rad * 0.4f, y - rad * 0.4f,
                                           Colour (0xff50535a), x + rad, y + rad, true));
        g.fillEllipse (x - rad, y - rad, rad * 2.0f, rad * 2.0f);
        g.setColour (Colours::black.withAlpha (0.65f));
        g.drawEllipse (x - rad, y - rad, rad * 2.0f, rad * 2.0f, 0.8f);

        Path slot;
        slot.addRectangle (-rad * 0.72f, -0.8f, rad * 1.44f, 1.6f);
        g.setColour (Colour (0xff25272b));
        g.fillPath (slot, AffineTransform::rotation (angle).translated (x, y));
    }

    void drawNail (Graphics& g, float x, float y)
    {
        g.setColour (Colours::black.withAlpha (0.45f));
        g.fillEllipse (x - 3.0f, y - 2.0f, 6.0f, 6.0f);
        g.setGradientFill (ColourGradient (Colour (0xff6a6d73), x - 1.5f, y - 1.5f,
                                           Colour (0xff1c1d20), x + 3.0f, y + 3.0f, true));
        g.fillEllipse (x - 2.5f, y - 2.5f, 5.0f, 5.0f);
        g.setColour (Colours::white.withAlpha (0.4f));
        g.fillEllipse (x - 1.5f, y - 1.6f, 1.6f, 1.2f);
    }
}

LaPautaMicProAudioProcessorEditor::LaPautaMicProAudioProcessorEditor (LaPautaMicProAudioProcessor& p)
    : AudioProcessorEditor (&p),
      proc (p),
      attachment (*p.apvts.getParameter ("power"), toggle, nullptr)
{
    setSize (W, H);          // ~12 cm x 8 cm
    setResizable (false, false);
    addAndMakeVisible (toggle);

    constexpr int S = 2;     // supermuestreo para bordes suaves en pantallas HD
    backdrop = Image (Image::ARGB, W * S, H * S, true);
    micLayer = Image (Image::ARGB, W * S, H * S, true);
    {
        Graphics g (backdrop);
        g.addTransform (AffineTransform::scale ((float) S));
        drawBackdrop (g);
    }
    {
        Graphics g (micLayer);
        g.addTransform (AffineTransform::scale ((float) S));
        drawMic (g);
    }

    glow = proc.apvts.getRawParameterValue ("power")->load() > 0.5f ? 1.0f : 0.0f;
    startTimerHz (30);
}

void LaPautaMicProAudioProcessorEditor::resized()
{
    toggle.setBounds (10, 222, 130, 56);
}

void LaPautaMicProAudioProcessorEditor::timerCallback()
{
    const float target = proc.apvts.getRawParameterValue ("power")->load() > 0.5f ? 1.0f : 0.0f;
    if (std::abs (target - glow) > 0.01f)
    {
        glow += (target - glow) * 0.22f;
        repaint();
    }
    else if (glow != target)
    {
        glow = target;
        repaint();
    }
}

// ---------------------------------------------------------------------------------------
void LaPautaMicProAudioProcessorEditor::drawBackdrop (Graphics& g)
{
    Random r (20240);
    const float fw = (float) W, fh = (float) H;

    // ---------- metal oscuro cepillado ----------
    g.setGradientFill (ColourGradient (Colour (0xff72767d), 0.0f, 0.0f, Colour (0xff24272b), fw, fh, false));
    g.fillRect (0.0f, 0.0f, fw, fh);

    for (int i = 0; i < 2400; ++i)
    {
        const float x = r.nextFloat() * fw, y = r.nextFloat() * fh, len = 30.0f + r.nextFloat() * 220.0f;
        g.setColour ((r.nextBool() ? Colours::white : Colours::black).withAlpha (r.nextFloat() * 0.06f));
        g.drawLine (x, y, x + len, y + (r.nextFloat() - 0.5f) * 0.5f, 0.5f);
    }
    for (int i = 0; i < 1800; ++i)   // grano
    {
        g.setColour ((r.nextBool() ? Colours::white : Colours::black).withAlpha (0.04f + r.nextFloat() * 0.07f));
        g.fillRect (r.nextFloat() * fw, r.nextFloat() * fh, 1.0f, 1.0f);
    }

    // ---------- óxido ----------
    const Colour rust (0xffa4501f);
    for (int i = 0; i < 20; ++i)
    {
        const bool edge = r.nextFloat() < 0.65f;
        const float x = edge ? (r.nextBool() ? r.nextFloat() * 45.0f : fw - r.nextFloat() * 45.0f) : r.nextFloat() * fw;
        const float y = r.nextFloat() * fh;
        const float rad = 14.0f + r.nextFloat() * 42.0f;
        g.setGradientFill (ColourGradient (rust.withAlpha (0.40f), x, y, rust.withAlpha (0.0f), x + rad, y, true));
        g.fillEllipse (x - rad, y - rad, rad * 2.0f, rad * 2.0f);
    }
    for (int i = 0; i < 12; ++i)     // chorreados
    {
        const float x = r.nextFloat() * fw, y0 = r.nextFloat() * fh * 0.4f, len = 20.0f + r.nextFloat() * 80.0f;
        g.setColour (rust.withAlpha (0.06f + r.nextFloat() * 0.10f));
        g.drawLine (x, y0, x + (r.nextFloat() - 0.5f) * 3.0f, y0 + len, 1.0f + r.nextFloat() * 1.6f);
    }
    for (int i = 0; i < 40; ++i)     // arañazos
    {
        const float x = r.nextFloat() * fw, y = r.nextFloat() * fh;
        g.setColour (Colours::white.withAlpha (0.09f));
        g.drawLine (x, y, x + (r.nextFloat() - 0.5f) * 34.0f, y + (r.nextFloat() - 0.5f) * 14.0f, 0.6f);
    }

    // ---------- placa del título ----------
    Rectangle<float> plate (14.0f, 14.0f, 120.0f, 64.0f);
    Path pp;
    pp.addRoundedRectangle (plate, 6.0f);
    DropShadow (Colours::black.withAlpha (0.55f), 10, Point<int> (0, 4)).drawForPath (g, pp);
    g.setGradientFill (ColourGradient (Colour (0xff2e3136), 0.0f, plate.getY(),
                                       Colour (0xff121315), 0.0f, plate.getBottom(), false));
    g.fillPath (pp);
    g.setColour (Colours::white.withAlpha (0.14f));
    g.drawRoundedRectangle (plate, 6.0f, 1.0f);

    g.setFont (Font (FontOptions ("Segoe UI", 21.0f, Font::bold).withKerningFactor (0.14f)));
    g.setColour (Colour (0xfff1e6cc));
    g.drawText ("LA PAUTA", Rectangle<float> (plate.getX() + 12.0f, plate.getY() + 5.0f, plate.getWidth() - 20.0f, 28.0f),
                Justification::centredLeft);

    g.setColour (Colour (0xffe9a23b));
    g.fillRoundedRectangle (plate.getX() + 13.0f, plate.getY() + 37.0f, 26.0f, 2.0f, 1.0f);

    g.setFont (Font (FontOptions ("Segoe UI", 14.0f, Font::bold).withKerningFactor (0.30f)));
    g.setColour (Colour (0xffe9a23b));
    g.drawText ("MIC PRO", Rectangle<float> (plate.getX() + 12.0f, plate.getY() + 40.0f, plate.getWidth() - 20.0f, 20.0f),
                Justification::centredLeft);

    // ---------- caja de madera abierta ----------
    const Rectangle<float> crate (150.0f, 16.0f, 276.0f, 238.0f);
    const float post = 18.0f, rail = 22.0f;

    DropShadow (Colours::black.withAlpha (0.65f), 20, Point<int> (0, 9)).drawForRectangle (g, crate.toNearestInt());

    // pared del fondo (tablas horizontales)
    Rectangle<float> inner (crate.getX() + post, crate.getY(), crate.getWidth() - post * 2.0f, crate.getHeight() - rail);
    const int planks = 5;
    for (int i = 0; i < planks; ++i)
    {
        Rectangle<float> pl (inner.getX(), inner.getY() + inner.getHeight() * i / planks,
                             inner.getWidth(), inner.getHeight() / planks);
        const float v = r.nextFloat() * 0.25f;
        drawWood (g, pl, true, Colour (0xff6a4222).darker (v), Colour (0xff3d2412).darker (v), r);
    }

    // luz desde arriba (la caja está abierta) y sombras laterales
    g.setGradientFill (ColourGradient (Colour (0xffffe2a8).withAlpha (0.20f), 0.0f, inner.getY(),
                                       Colours::transparentBlack, 0.0f, inner.getY() + 110.0f, false));
    g.fillRect (inner);
    g.setGradientFill (ColourGradient (Colours::black.withAlpha (0.55f), inner.getX(), 0.0f,
                                       Colours::transparentBlack, inner.getX() + 28.0f, 0.0f, false));
    g.fillRect (inner);
    g.setGradientFill (ColourGradient (Colours::black.withAlpha (0.55f), inner.getRight(), 0.0f,
                                       Colours::transparentBlack, inner.getRight() - 28.0f, 0.0f, false));
    g.fillRect (inner);
    g.setGradientFill (ColourGradient (Colours::transparentBlack, 0.0f, inner.getBottom() - 50.0f,
                                       Colours::black.withAlpha (0.50f), 0.0f, inner.getBottom(), false));
    g.fillRect (inner);

    // postes laterales y travesaño inferior
    const Colour woodL (0xffb98245), woodD (0xff74491f);
    drawWood (g, Rectangle<float> (crate.getX(), crate.getY(), post, crate.getHeight()), false, woodL, woodD, r);
    drawWood (g, Rectangle<float> (crate.getRight() - post, crate.getY(), post, crate.getHeight()), false, woodL, woodD, r);
    drawWood (g, Rectangle<float> (crate.getX(), crate.getBottom() - rail, crate.getWidth(), rail), true,
              Colour (0xffc08948), Colour (0xff7b4c20), r);

    g.setColour (Colour (0xffe0b074).withAlpha (0.85f));   // veta de testa arriba de los postes
    g.fillRect (crate.getX(), crate.getY(), post, 3.0f);
    g.fillRect (crate.getRight() - post, crate.getY(), post, 3.0f);

    for (float ny : { crate.getY() + 26.0f, crate.getBottom() - rail - 24.0f })
    {
        drawNail (g, crate.getX() + post * 0.5f, ny);
        drawNail (g, crate.getRight() - post * 0.5f, ny);
    }
    for (float nx : { crate.getX() + 30.0f, crate.getRight() - 30.0f })
        drawNail (g, nx, crate.getBottom() - rail * 0.5f);

    // luz cálida baja la zona del micrófono
    g.setGradientFill (ColourGradient (Colour (0xffffc46b).withAlpha (0.16f), micX, 110.0f,
                                       Colour (0xffffc46b).withAlpha (0.0f), micX + 120.0f, 110.0f, true));
    g.fillRect (inner);

    // ---------- tornillos, bisel y viñeta ----------
    drawScrew (g, 8.0f, 8.0f, 3.6f, 0.4f);
    drawScrew (g, fw - 8.0f, 8.0f, 3.6f, 1.9f);
    drawScrew (g, 8.0f, fh - 8.0f, 3.6f, 1.1f);
    drawScrew (g, fw - 8.0f, fh - 8.0f, 3.6f, 2.6f);

    g.setGradientFill (ColourGradient (Colours::transparentBlack, fw * 0.55f, fh * 0.45f,
                                       Colours::black.withAlpha (0.50f), 0.0f, 0.0f, true));
    g.fillRect (0.0f, 0.0f, fw, fh);

    g.setColour (Colours::white.withAlpha (0.18f));
    g.drawLine (0.5f, 0.5f, fw - 0.5f, 0.5f, 1.0f);
    g.drawLine (0.5f, 0.5f, 0.5f, fh - 0.5f, 1.0f);
    g.setColour (Colours::black.withAlpha (0.8f));
    g.drawLine (0.5f, fh - 0.5f, fw - 0.5f, fh - 0.5f, 1.0f);
    g.drawLine (fw - 0.5f, 0.5f, fw - 0.5f, fh - 0.5f, 1.0f);
}

// ---------------------------------------------------------------------------------------
void LaPautaMicProAudioProcessorEditor::drawMic (Graphics& g)
{
    const float cx = micX;
    const float fh = (float) H;

    // ---- poste del soporte (detrás) ----
    {
        Rectangle<float> pole (cx + 54.0f, 176.0f, 10.0f, fh - 176.0f);
        g.setGradientFill (cylinder (pole, Colour (0xff0b0b0c), Colour (0xff26272a), Colour (0xff55585e)));
        g.fillRect (pole);
        g.setColour (Colour (0xff17181a));
        g.fillRoundedRectangle (cx + 50.0f, 190.0f, 18.0f, 14.0f, 3.0f);
        g.setColour (Colours::white.withAlpha (0.12f));
        g.drawRoundedRectangle (cx + 50.0f, 190.0f, 18.0f, 14.0f, 3.0f, 0.8f);
    }

    // ---- caja de alimentación con disipador ----
    {
        Rectangle<float> box (cx + 24.0f, 106.0f, 68.0f, 62.0f);
        Path bp;
        bp.addRoundedRectangle (box, 3.0f);
        DropShadow (Colours::black.withAlpha (0.6f), 9, Point<int> (6, 5)).drawForPath (g, bp);
        g.setColour (Colour (0xff0b0b0d));
        g.fillPath (bp);
        for (int i = 0; i < 12; ++i)
        {
            const float fx = box.getX() + 20.0f + i * 3.9f;
            g.setGradientFill (ColourGradient (Colour (0xff3c3d41), fx, 0.0f, Colour (0xff0f0f11), fx + 2.4f, 0.0f, false));
            g.fillRect (fx, box.getY() + 2.0f, 2.4f, box.getHeight() - 4.0f);
        }
        g.setGradientFill (ColourGradient (Colour (0xff25262a), box.getX(), 0.0f, Colour (0xff111214), box.getX() + 20.0f, 0.0f, false));
        g.fillRect (box.getX(), box.getY(), 20.0f, box.getHeight());
        g.setColour (Colours::white.withAlpha (0.10f));
        g.drawRoundedRectangle (box, 3.0f, 0.8f);
        g.setColour (Colour (0xff7e8289));                       // tornillitos
        g.fillEllipse (box.getX() + 5.0f, box.getY() + 5.0f, 3.0f, 3.0f);
        g.fillEllipse (box.getX() + 5.0f, box.getBottom() - 8.0f, 3.0f, 3.0f);
    }

    // ---- sombra del micrófono sobre la pared ----
    {
        Path shadow;
        shadow.addRoundedRectangle (cx - 30.0f + 9.0f, 34.0f + 6.0f, 60.0f, 172.0f, 22.0f);
        DropShadow (Colours::black.withAlpha (0.55f), 12, Point<int> (0, 0)).drawForPath (g, shadow);
    }

    // ---- cuerpo ----
    {
        Rectangle<float> body (cx - 27.0f, 132.0f, 54.0f, 76.0f);
        g.setGradientFill (cylinder (body, Colour (0xff070708), Colour (0xff1c1d20), Colour (0xff45474c)));
        g.fillRect (body);

        g.setColour (Colour (0xff3a3c41));                       // ranura del interruptor
        g.fillRoundedRectangle (cx - 17.0f, 150.0f, 15.0f, 4.0f, 2.0f);
        g.setColour (Colours::white.withAlpha (0.30f));
        g.drawEllipse (cx - 21.0f, 148.0f, 5.0f, 5.0f, 0.8f);
        g.drawEllipse (cx + 12.0f, 148.0f, 5.0f, 5.0f, 0.8f);
    }

    // ---- cabeza con rejilla ----
    {
        Rectangle<float> head (cx - 30.0f, 34.0f, 60.0f, 100.0f);
        Path hp;
        hp.addRoundedRectangle (head.getX(), head.getY(), head.getWidth(), head.getHeight(),
                                28.0f, 28.0f, true, true, false, false);

        g.setGradientFill (cylinder (head, Colour (0xff060607), Colour (0xff202226), Colour (0xff50535a)));
        g.fillPath (hp);

        Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (hp);

        // malla (dos diagonales cruzadas)
        for (float k = -head.getHeight(); k < head.getWidth() + head.getHeight(); k += 3.4f)
        {
            g.setColour (Colours::white.withAlpha (0.11f));
            g.drawLine (head.getX() + k, head.getY(), head.getX() + k - head.getHeight(), head.getBottom(), 0.6f);
            g.setColour (Colours::black.withAlpha (0.30f));
            g.drawLine (head.getX() + k, head.getY(), head.getX() + k + head.getHeight(), head.getBottom(), 0.6f);
        }

        // reflejo especular vertical y brillo de la cúpula
        g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.26f), 0.0f, head.getY() + 6.0f,
                                           Colours::white.withAlpha (0.0f), 0.0f, head.getBottom(), false));
        g.fillRoundedRectangle (head.getX() + 11.0f, head.getY() + 6.0f, 6.0f, head.getHeight() - 14.0f, 3.0f);
        g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.18f), cx - 8.0f, head.getY() + 14.0f,
                                           Colours::white.withAlpha (0.0f), cx + 14.0f, head.getY() + 30.0f, true));
        g.fillEllipse (cx - 24.0f, head.getY() + 3.0f, 40.0f, 26.0f);
    }

    // ---- anillos plateados ----
    for (float ry : { 130.0f, 206.0f })
    {
        Rectangle<float> ring (cx - 29.0f, ry, 58.0f, 5.0f);
        g.setGradientFill (cylinder (ring, Colour (0xff595c62), Colour (0xff9da1a8), Colour (0xffe9ebee)));
        g.fillRoundedRectangle (ring, 1.5f);
        g.setColour (Colours::black.withAlpha (0.5f));
        g.drawRoundedRectangle (ring, 1.5f, 0.7f);
    }

    // ---- suspensión elástica ----
    {
        const float ringY = 188.0f;
        g.setColour (Colour (0xff0a0a0b));
        for (int i = 0; i < 9; ++i)
        {
            const float a = -1.0f + i * 0.25f;
            g.drawLine (cx + a * 48.0f, ringY + std::abs (a) * 2.0f, cx + a * 30.0f, 164.0f, 1.3f);
            g.drawLine (cx + a * 48.0f, ringY + std::abs (a) * 2.0f, cx + a * 30.0f, 214.0f, 1.3f);
        }
        g.setColour (Colours::white.withAlpha (0.07f));
        for (int i = 0; i < 9; ++i)
        {
            const float a = -1.0f + i * 0.25f;
            g.drawLine (cx + a * 48.0f + 0.8f, ringY + std::abs (a) * 2.0f, cx + a * 30.0f + 0.8f, 164.0f, 0.6f);
        }

        Rectangle<float> ring (cx - 52.0f, ringY - 10.0f, 104.0f, 28.0f);
        g.setColour (Colour (0xff0d0d0f));
        g.drawEllipse (ring, 8.0f);
        g.setColour (Colours::white.withAlpha (0.16f));
        g.drawEllipse (ring.reduced (1.5f), 1.0f);
    }

    // ---- conector XLR y cable ----
    {
        Rectangle<float> cap (cx - 12.0f, 214.0f, 24.0f, 6.0f);
        g.setColour (Colour (0xff16171a));
        g.fillRoundedRectangle (cap, 2.0f);

        Rectangle<float> conn (cx - 8.5f, 220.0f, 17.0f, 26.0f);
        g.setGradientFill (cylinder (conn, Colour (0xff6b6e74), Colour (0xffb0b4ba), Colour (0xfff2f3f5)));
        g.fillRect (conn);
        g.setColour (Colours::black.withAlpha (0.35f));
        for (float y = conn.getY() + 3.0f; y < conn.getBottom(); y += 3.2f)
            g.drawLine (conn.getX(), y, conn.getRight(), y, 0.7f);

        Rectangle<float> cable (cx - 3.0f, 246.0f, 6.0f, fh - 246.0f);
        g.setGradientFill (cylinder (cable, Colour (0xff4a4c50), Colour (0xff8a8d92), Colour (0xffc2c5ca)));
        g.fillRect (cable);
    }
}

// ---------------------------------------------------------------------------------------
void LaPautaMicProAudioProcessorEditor::paint (Graphics& g)
{
    g.setImageResamplingQuality (Graphics::highResamplingQuality);
    const auto bounds = getLocalBounds().toFloat();

    g.drawImage (backdrop, bounds);

    // al encender, la caja se ilumina con luz cálida
    if (glow > 0.01f)
    {
        g.setGradientFill (ColourGradient (Colour (0xffffc46b).withAlpha (0.30f * glow), micX, 100.0f,
                                           Colour (0xffffc46b).withAlpha (0.0f), micX + 140.0f, 100.0f, true));
        g.fillRect (168.0f, 16.0f, 240.0f, 216.0f);
    }

    g.drawImage (micLayer, bounds);

    // LED de encendido
    if (glow > 0.01f)
    {
        g.setGradientFill (ColourGradient (Colour (0xff5dff6b).withAlpha (0.85f * glow), micX, 160.0f,
                                           Colour (0xff5dff6b).withAlpha (0.0f), micX + 11.0f, 160.0f, true));
        g.fillEllipse (micX - 11.0f, 149.0f, 22.0f, 22.0f);
    }
    g.setColour (Colour (0xff3a1a1a).interpolatedWith (Colour (0xff8dff98), glow));
    g.fillEllipse (micX - 3.0f, 157.0f, 6.0f, 6.0f);
    g.setColour (Colours::white.withAlpha (0.35f * glow));
    g.fillEllipse (micX - 1.8f, 157.8f, 2.2f, 1.6f);
}
