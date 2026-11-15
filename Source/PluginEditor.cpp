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

    // Panel acústico de espuma con pirámides (luz desde arriba a la izquierda)
    void drawFoam (Graphics& g, Rectangle<float> area, int cols, int rows, Random& r)
    {
        const float cw = area.getWidth() / (float) cols;
        const float ch = area.getHeight() / (float) rows;

        for (int row = 0; row < rows; ++row)
        {
            for (int col = 0; col < cols; ++col)
            {
                const float x = area.getX() + col * cw;
                const float y = area.getY() + row * ch;
                const float v = 1.0f + (r.nextFloat() - 0.5f) * 0.16f;

                const Point<float> tl (x, y), tr (x + cw, y), bl (x, y + ch), br (x + cw, y + ch);
                const Point<float> ce (x + cw * 0.5f, y + ch * 0.5f);

                auto facet = [&] (Point<float> p1, Point<float> p2, Colour c)
                {
                    Path p;
                    p.addTriangle (p1.x, p1.y, p2.x, p2.y, ce.x, ce.y);
                    g.setColour (c.withMultipliedBrightness (v));
                    g.fillPath (p);
                };

                facet (tl, tr, Colour (0xff5a5e65));   // cara superior (iluminada)
                facet (tl, bl, Colour (0xff3c3f45));   // izquierda
                facet (tr, br, Colour (0xff222427));   // derecha
                facet (bl, br, Colour (0xff141516));   // inferior (sombra)

                g.setColour (Colours::white.withAlpha (0.05f));
                g.fillEllipse (ce.x - 1.6f, ce.y - 1.6f, 3.2f, 3.2f);
            }
        }

        // juntas oscuras entre pirámides
        g.setColour (Colours::black.withAlpha (0.60f));
        for (int col = 0; col <= cols; ++col)
            g.drawLine (area.getX() + col * cw, area.getY(), area.getX() + col * cw, area.getBottom(), 0.8f);
        for (int row = 0; row <= rows; ++row)
            g.drawLine (area.getX(), area.getY() + row * ch, area.getRight(), area.getY() + row * ch, 0.8f);
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
    g.setGradientFill (ColourGradient (Colour (0xff6a6e75), 0.0f, 0.0f, Colour (0xff202328), fw, fh, false));
    g.fillRect (0.0f, 0.0f, fw, fh);

    for (int i = 0; i < 2400; ++i)
    {
        const float x = r.nextFloat() * fw, y = r.nextFloat() * fh, len = 30.0f + r.nextFloat() * 220.0f;
        g.setColour ((r.nextBool() ? Colours::white : Colours::black).withAlpha (r.nextFloat() * 0.055f));
        g.drawLine (x, y, x + len, y + (r.nextFloat() - 0.5f) * 0.5f, 0.5f);
    }
    for (int i = 0; i < 1600; ++i)   // grano fino
    {
        g.setColour ((r.nextBool() ? Colours::white : Colours::black).withAlpha (0.03f + r.nextFloat() * 0.06f));
        g.fillRect (r.nextFloat() * fw, r.nextFloat() * fh, 1.0f, 1.0f);
    }

    // ---------- óxido sutil en los bordes ----------
    const Colour rust (0xffa4501f);
    for (int i = 0; i < 10; ++i)
    {
        const float x = r.nextBool() ? r.nextFloat() * 40.0f : fw - r.nextFloat() * 40.0f;
        const float y = r.nextFloat() * fh;
        const float rad = 14.0f + r.nextFloat() * 34.0f;
        g.setGradientFill (ColourGradient (rust.withAlpha (0.30f), x, y, rust.withAlpha (0.0f), x + rad, y, true));
        g.fillEllipse (x - rad, y - rad, rad * 2.0f, rad * 2.0f);
    }
    for (int i = 0; i < 16; ++i)     // arañazos finos
    {
        const float x = r.nextFloat() * fw, y = r.nextFloat() * fh;
        g.setColour (Colours::white.withAlpha (0.07f));
        g.drawLine (x, y, x + (r.nextFloat() - 0.5f) * 30.0f, y + (r.nextFloat() - 0.5f) * 12.0f, 0.6f);
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

    // ---------- panel acústico ----------
    const Rectangle<float> panel (150.0f, 16.0f, 276.0f, 238.0f);
    const auto foam = panel.reduced (7.0f);

    Path panelPath;
    panelPath.addRoundedRectangle (panel, 4.0f);
    DropShadow (Colours::black.withAlpha (0.70f), 20, Point<int> (0, 9)).drawForPath (g, panelPath);

    // marco metálico negro
    g.setGradientFill (ColourGradient (Colour (0xff43464c), 0.0f, panel.getY(),
                                       Colour (0xff121315), 0.0f, panel.getBottom(), false));
    g.fillPath (panelPath);
    g.setColour (Colours::white.withAlpha (0.16f));
    g.drawRoundedRectangle (panel, 4.0f, 1.0f);

    // espuma con pirámides
    drawFoam (g, foam, 12, 10, r);

    // iluminación: luz desde arriba, bordes y base más oscuros
    g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.12f), 0.0f, foam.getY(),
                                       Colours::transparentBlack, 0.0f, foam.getY() + 110.0f, false));
    g.fillRect (foam);
    g.setGradientFill (ColourGradient (Colours::black.withAlpha (0.55f), foam.getX(), 0.0f,
                                       Colours::transparentBlack, foam.getX() + 40.0f, 0.0f, false));
    g.fillRect (foam);
    g.setGradientFill (ColourGradient (Colours::black.withAlpha (0.55f), foam.getRight(), 0.0f,
                                       Colours::transparentBlack, foam.getRight() - 40.0f, 0.0f, false));
    g.fillRect (foam);
    g.setGradientFill (ColourGradient (Colours::transparentBlack, 0.0f, foam.getBottom() - 60.0f,
                                       Colours::black.withAlpha (0.55f), 0.0f, foam.getBottom(), false));
    g.fillRect (foam);

    // borde interior hundido
    g.setColour (Colours::black.withAlpha (0.85f));
    g.drawRect (foam, 1.2f);

    // tornillos del marco
    for (float sx : { panel.getX() + 3.5f, panel.getRight() - 3.5f })
        for (float sy : { panel.getY() + 3.5f, panel.getBottom() - 3.5f })
            drawScrew (g, sx, sy, 2.2f, sx + sy);

    // ---------- tornillos, bisel y viñeta ----------
    drawScrew (g, 8.0f, 8.0f, 3.6f, 0.4f);
    drawScrew (g, fw - 8.0f, 8.0f, 3.6f, 1.9f);
    drawScrew (g, 8.0f, fh - 8.0f, 3.6f, 1.1f);
    drawScrew (g, fw - 8.0f, fh - 8.0f, 3.6f, 2.6f);

    g.setGradientFill (ColourGradient (Colours::transparentBlack, fw * 0.55f, fh * 0.45f,
                                       Colours::black.withAlpha (0.45f), 0.0f, 0.0f, true));
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
        g.setColour (Colour (0xff7e8289));
        g.fillEllipse (box.getX() + 5.0f, box.getY() + 5.0f, 3.0f, 3.0f);
        g.fillEllipse (box.getX() + 5.0f, box.getBottom() - 8.0f, 3.0f, 3.0f);
    }

    // ---- brazo que une el micrófono con el soporte ----
    {
        Rectangle<float> arm (cx + 24.0f, 182.0f, 34.0f, 11.0f);
        g.setGradientFill (ColourGradient (Colour (0xff3a3c41), 0.0f, arm.getY(), Colour (0xff0e0e10), 0.0f, arm.getBottom(), false));
        g.fillRoundedRectangle (arm, 3.0f);
        g.setColour (Colours::white.withAlpha (0.14f));
        g.drawRoundedRectangle (arm, 3.0f, 0.8f);
        g.setGradientFill (ColourGradient (Colour (0xffe6e8ec), cx + 44.0f, 185.0f, Colour (0xff60636a), cx + 50.0f, 194.0f, true));
        g.fillEllipse (cx + 41.0f, 183.5f, 8.0f, 8.0f);
    }

    // ---- sombra del micrófono sobre el panel ----
    {
        Path shadow;
        shadow.addRoundedRectangle (cx - 30.0f + 9.0f, 34.0f + 6.0f, 60.0f, 182.0f, 22.0f);
        DropShadow (Colours::black.withAlpha (0.60f), 13, Point<int> (0, 0)).drawForPath (g, shadow);
    }

    // ---- cuerpo (con el interruptor bien visible) ----
    {
        Rectangle<float> body (cx - 27.0f, 132.0f, 54.0f, 82.0f);
        g.setGradientFill (cylinder (body, Colour (0xff070708), Colour (0xff1c1d20), Colour (0xff45474c)));
        g.fillRect (body);

        // interruptor deslizante
        g.setColour (Colour (0xff0a0a0b));
        g.fillRoundedRectangle (cx - 18.0f, 148.5f, 20.0f, 7.0f, 3.5f);
        g.setColour (Colours::white.withAlpha (0.18f));
        g.drawRoundedRectangle (cx - 18.0f, 148.5f, 20.0f, 7.0f, 3.5f, 0.7f);
        g.setGradientFill (ColourGradient (Colour (0xffe9ebee), cx - 14.0f, 149.0f, Colour (0xff6f7279), cx - 8.0f, 156.0f, true));
        g.fillEllipse (cx - 15.5f, 149.5f, 5.0f, 5.0f);

        // marcas del interruptor
        g.setColour (Colours::white.withAlpha (0.35f));
        g.drawEllipse (cx - 23.0f, 149.0f, 4.5f, 4.5f, 0.8f);
        g.drawEllipse (cx + 5.0f, 149.0f, 4.5f, 4.5f, 0.8f);
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

        for (float k = -head.getHeight(); k < head.getWidth() + head.getHeight(); k += 3.4f)
        {
            g.setColour (Colours::white.withAlpha (0.11f));
            g.drawLine (head.getX() + k, head.getY(), head.getX() + k - head.getHeight(), head.getBottom(), 0.6f);
            g.setColour (Colours::black.withAlpha (0.30f));
            g.drawLine (head.getX() + k, head.getY(), head.getX() + k + head.getHeight(), head.getBottom(), 0.6f);
        }

        g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.26f), 0.0f, head.getY() + 6.0f,
                                           Colours::white.withAlpha (0.0f), 0.0f, head.getBottom(), false));
        g.fillRoundedRectangle (head.getX() + 11.0f, head.getY() + 6.0f, 6.0f, head.getHeight() - 14.0f, 3.0f);
        g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.18f), cx - 8.0f, head.getY() + 14.0f,
                                           Colours::white.withAlpha (0.0f), cx + 14.0f, head.getY() + 30.0f, true));
        g.fillEllipse (cx - 24.0f, head.getY() + 3.0f, 40.0f, 26.0f);
    }

    // ---- anillos plateados ----
    for (float ry : { 130.0f, 209.0f })
    {
        Rectangle<float> ring (cx - 29.0f, ry, 58.0f, 5.0f);
        g.setGradientFill (cylinder (ring, Colour (0xff595c62), Colour (0xff9da1a8), Colour (0xffe9ebee)));
        g.fillRoundedRectangle (ring, 1.5f);
        g.setColour (Colours::black.withAlpha (0.5f));
        g.drawRoundedRectangle (ring, 1.5f, 0.7f);
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

    // al encender, el panel se ilumina con una luz cálida detrás del micrófono
    if (glow > 0.01f)
    {
        g.setGradientFill (ColourGradient (Colour (0xffffc46b).withAlpha (0.34f * glow), micX, 110.0f,
                                           Colour (0xffffc46b).withAlpha (0.0f), micX + 140.0f, 110.0f, true));
        g.fillRect (157.0f, 23.0f, 262.0f, 224.0f);
    }

    g.drawImage (micLayer, bounds);

    // LED de encendido (debajo del interruptor del micrófono)
    if (glow > 0.01f)
    {
        g.setGradientFill (ColourGradient (Colour (0xff5dff6b).withAlpha (0.85f * glow), micX, 176.0f,
                                           Colour (0xff5dff6b).withAlpha (0.0f), micX + 11.0f, 176.0f, true));
        g.fillEllipse (micX - 11.0f, 165.0f, 22.0f, 22.0f);
    }
    g.setColour (Colour (0xff3a1a1a).interpolatedWith (Colour (0xff8dff98), glow));
    g.fillEllipse (micX - 3.0f, 173.0f, 6.0f, 6.0f);
    g.setColour (Colours::white.withAlpha (0.35f * glow));
    g.fillEllipse (micX - 1.8f, 173.8f, 2.2f, 1.6f);
}
