#include "PluginEditor.h"

using namespace juce;

LaPautaMicProAudioProcessorEditor::LaPautaMicProAudioProcessorEditor (LaPautaMicProAudioProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      attachment (*p.apvts.getParameter ("power"), toggle, nullptr)
{
    setSize (W, H);          // ~12 cm x 8 cm
    setResizable (false, false);
    addAndMakeVisible (toggle);
    toggle.onClick = [this] { repaint(); };
    buildBackground();
}

void LaPautaMicProAudioProcessorEditor::resized()
{
    toggle.setBounds (18, 228, 104, 38);
}

void LaPautaMicProAudioProcessorEditor::buildBackground()
{
    background = Image (Image::ARGB, W, H, true);
    Graphics g (background);
    Random r (1234);

    const auto fw = (float) W;
    const auto fh = (float) H;

    // ---------- fondo metálico rústico ----------
    g.setGradientFill (ColourGradient (Colour (0xff868a90), 0, 0, Colour (0xff45484d), 0, fh, false));
    g.fillAll();

    for (int i = 0; i < 1400; ++i)   // vetas de metal cepillado
    {
        const float x = r.nextFloat() * fw, y = r.nextFloat() * fh, len = 20.0f + r.nextFloat() * 140.0f;
        g.setColour ((r.nextBool() ? Colours::white : Colours::black).withAlpha (r.nextFloat() * 0.07f));
        g.drawLine (x, y, x + len, y, 0.6f);
    }

    for (int i = 0; i < 16; ++i)     // manchas de óxido
    {
        const float x = r.nextFloat() * fw, y = r.nextFloat() * fh, rad = 15.0f + r.nextFloat() * 45.0f;
        g.setGradientFill (ColourGradient (Colour (0xff9a4a1c).withAlpha (0.38f), x, y,
                                           Colour (0xff9a4a1c).withAlpha (0.0f), x + rad, y, true));
        g.fillEllipse (x - rad, y - rad, rad * 2, rad * 2);
    }

    for (int i = 0; i < 40; ++i)     // arañazos
    {
        const float x = r.nextFloat() * fw, y = r.nextFloat() * fh;
        g.setColour (Colours::white.withAlpha (0.10f));
        g.drawLine (x, y, x + r.nextFloat() * 30.0f - 15.0f, y + r.nextFloat() * 14.0f - 7.0f, 0.7f);
    }

    // ---------- placa del título ----------
    Rectangle<float> plate (12.0f, 12.0f, 112.0f, 62.0f);
    g.setColour (Colours::black.withAlpha (0.45f));
    g.fillRoundedRectangle (plate.translated (1.5f, 2.0f), 4.0f);
    g.setGradientFill (ColourGradient (Colour (0xff3a3d41), 0, plate.getY(), Colour (0xff1f2123), 0, plate.getBottom(), false));
    g.fillRoundedRectangle (plate, 4.0f);
    g.setColour (Colours::white.withAlpha (0.25f));
    g.drawRoundedRectangle (plate, 4.0f, 1.0f);
    g.setColour (Colour (0xffe6d9b8));
    g.setFont (Font (FontOptions (19.0f, Font::bold)));
    g.drawText ("LA PAUTA", plate.withTrimmedBottom (28.0f), Justification::centred);
    g.setFont (Font (FontOptions (15.0f, Font::bold)));
    g.setColour (Colour (0xffc9b27a));
    g.drawText ("MIC PRO", plate.withTrimmedTop (30.0f), Justification::centred);

    // ---------- caja de madera abierta (detrás del micrófono) ----------
    const float cx = 270.0f;
    Rectangle<float> crate (145.0f, 20.0f, 250.0f, 204.0f);

    g.setColour (Colours::black.withAlpha (0.45f));                       // sombra
    g.fillEllipse (crate.getX() - 6.0f, crate.getBottom() - 14.0f, crate.getWidth() + 12.0f, 30.0f);

    g.setColour (Colour (0xff1a0f07));                                    // interior oscuro
    g.fillRect (crate);

    // tablas del fondo
    const float frame = 16.0f;
    auto inner = crate.reduced (frame);
    const int planks = 4;
    for (int i = 0; i < planks; ++i)
    {
        Rectangle<float> pl (inner.getX(), inner.getY() + inner.getHeight() * i / planks,
                             inner.getWidth(), inner.getHeight() / planks);
        g.setGradientFill (ColourGradient (Colour (0xff4b2e17), 0, pl.getY(), Colour (0xff2e1b0d), 0, pl.getBottom(), false));
        g.fillRect (pl.reduced (0.0f, 0.5f));
        g.setColour (Colours::black.withAlpha (0.55f));
        g.drawLine (pl.getX(), pl.getBottom(), pl.getRight(), pl.getBottom(), 1.5f);
    }
    // sombra interior
    g.setGradientFill (ColourGradient (Colours::black.withAlpha (0.55f), inner.getX(), inner.getY(),
                                       Colours::transparentBlack, inner.getX(), inner.getY() + 40.0f, false));
    g.fillRect (inner);

    auto drawBoard = [&] (Rectangle<float> b, bool horizontal)
    {
        g.setGradientFill (horizontal
            ? ColourGradient (Colour (0xffa06a34), 0, b.getY(), Colour (0xff6b4220), 0, b.getBottom(), false)
            : ColourGradient (Colour (0xffa06a34), b.getX(), 0, Colour (0xff6b4220), b.getRight(), 0, false));
        g.fillRect (b);
        for (int i = 0; i < 18; ++i)   // vetas
        {
            g.setColour (Colours::black.withAlpha (0.10f + r.nextFloat() * 0.12f));
            if (horizontal)
            {
                const float y = b.getY() + r.nextFloat() * b.getHeight();
                g.drawLine (b.getX(), y, b.getRight(), y + r.nextFloat() * 2.0f - 1.0f, 0.8f);
            }
            else
            {
                const float x = b.getX() + r.nextFloat() * b.getWidth();
                g.drawLine (x, b.getY(), x + r.nextFloat() * 2.0f - 1.0f, b.getBottom(), 0.8f);
            }
        }
        g.setColour (Colours::black.withAlpha (0.65f));
        g.drawRect (b, 1.2f);
    };

    drawBoard (crate.removeFromTop (frame).withTrimmedLeft (0), true);           // marco superior
    drawBoard (Rectangle<float> (145.0f, 224.0f - frame, 250.0f, frame), true);  // marco inferior
    drawBoard (Rectangle<float> (145.0f, 20.0f + frame, frame, 204.0f - frame * 2), false);   // izquierdo
    drawBoard (Rectangle<float> (395.0f - frame, 20.0f + frame, frame, 204.0f - frame * 2), false); // derecho

    g.setColour (Colour (0xff2b2b2b));                                            // clavos
    for (float nx : { 153.0f, 387.0f })
        for (float ny : { 28.0f, 216.0f })
        {
            g.fillEllipse (nx - 2.5f, ny - 2.5f, 5.0f, 5.0f);
            g.setColour (Colours::white.withAlpha (0.35f));
            g.fillEllipse (nx - 1.5f, ny - 1.5f, 1.5f, 1.5f);
            g.setColour (Colour (0xff2b2b2b));
        }

    // ---------- micrófono de condensador (dibujado en vectores) ----------
    // soporte
    g.setGradientFill (ColourGradient (Colour (0xff101010), cx + 48.0f, 0, Colour (0xff3a3a3a), cx + 58.0f, 0, false));
    g.fillRect (cx + 48.0f, 175.0f, 9.0f, fh - 175.0f);

    // caja de alimentación con disipador
    {
        Rectangle<float> box (cx + 28.0f, 112.0f, 62.0f, 56.0f);
        g.setColour (Colour (0xff0d0d0d));
        g.fillRoundedRectangle (box, 3.0f);
        for (int i = 0; i < 11; ++i)
        {
            const float fx = box.getX() + 22.0f + i * 3.6f;
            g.setColour (Colour (0xff2b2b2b));
            g.fillRect (fx, box.getY() + 2.0f, 2.0f, box.getHeight() - 4.0f);
        }
        g.setColour (Colour (0xff1a1a1a));
        g.fillRect (box.getX(), box.getY(), 22.0f, box.getHeight());
    }

    // cuerpo
    Rectangle<float> body (cx - 28.0f, 118.0f, 56.0f, 78.0f);
    g.setGradientFill (ColourGradient (Colour (0xff050505), body.getX(), 0, Colour (0xff050505), body.getRight(), 0, false));
    {
        ColourGradient cg (Colour (0xff050505), body.getX(), 0, Colour (0xff050505), body.getRight(), 0, false);
        cg.addColour (0.35, Colour (0xff3b3b3b));
        cg.addColour (0.55, Colour (0xff202020));
        g.setGradientFill (cg);
        g.fillRect (body);
    }

    // cabeza / rejilla
    Rectangle<float> head (cx - 31.0f, 36.0f, 62.0f, 92.0f);
    {
        ColourGradient cg (Colour (0xff050505), head.getX(), 0, Colour (0xff050505), head.getRight(), 0, false);
        cg.addColour (0.3, Colour (0xff3f3f3f));
        cg.addColour (0.6, Colour (0xff1a1a1a));
        g.setGradientFill (cg);
        g.fillRoundedRectangle (head, 22.0f);

        Graphics::ScopedSaveState ss (g);
        Path clip; clip.addRoundedRectangle (head, 22.0f);
        g.reduceClipRegion (clip);
        g.setColour (Colours::white.withAlpha (0.12f));
        for (float x = head.getX(); x < head.getRight(); x += 3.5f)
            g.drawLine (x, head.getY(), x, head.getBottom(), 0.7f);
        for (float y = head.getY(); y < head.getBottom(); y += 3.5f)
            g.drawLine (head.getX(), y, head.getRight(), y, 0.7f);
        g.setColour (Colours::white.withAlpha (0.10f));
        g.fillEllipse (head.getX() + 8.0f, head.getY() + 14.0f, 18.0f, 60.0f);
    }

    // anillo plateado y conector
    g.setColour (Colour (0xff8c8f93));
    g.fillRect (cx - 28.0f, 190.0f, 56.0f, 4.0f);
    g.setGradientFill (ColourGradient (Colour (0xff1a1a1a), 0, 194, Colour (0xff070707), 0, 222, false));
    g.fillRect (cx - 25.0f, 194.0f, 50.0f, 28.0f);

    g.setGradientFill (ColourGradient (Colour (0xffc9ccd0), cx - 8.0f, 0, Colour (0xff6f7276), cx + 8.0f, 0, false));
    g.fillRect (cx - 8.0f, 224.0f, 16.0f, 20.0f);
    g.setColour (Colours::black.withAlpha (0.5f));
    for (float y = 228.0f; y < 244.0f; y += 4.0f) g.drawLine (cx - 8.0f, y, cx + 8.0f, y, 0.8f);

    // cable
    g.setColour (Colour (0xff77797c));
    g.drawLine (cx, 244.0f, cx, fh - 8.0f, 5.0f);

    // suspensión elástica
    g.setColour (Colour (0xff0b0b0b));
    for (int i = 0; i < 7; ++i)
    {
        const float a = -0.9f + i * 0.3f;
        g.drawLine (cx + a * 36.0f, 168.0f, cx + a * 46.0f, 200.0f, 1.2f);
        g.drawLine (cx + a * 46.0f, 200.0f, cx + a * 36.0f, 218.0f, 1.2f);
    }
    g.setColour (Colour (0xff141414));
    g.drawEllipse (cx - 46.0f, 186.0f, 92.0f, 28.0f, 7.0f);
    g.setColour (Colours::white.withAlpha (0.12f));
    g.drawEllipse (cx - 46.0f, 186.0f, 92.0f, 28.0f, 1.0f);

    // ---------- remaches y viñeta ----------
    for (float rx : { 8.0f, fw - 8.0f })
        for (float ry : { 8.0f, fh - 8.0f })
        {
            g.setGradientFill (ColourGradient (Colour (0xffd0d2d6), rx - 2, ry - 2, Colour (0xff4a4c50), rx + 4, ry + 4, true));
            g.fillEllipse (rx - 4.0f, ry - 4.0f, 8.0f, 8.0f);
            g.setColour (Colours::black.withAlpha (0.6f));
            g.drawEllipse (rx - 4.0f, ry - 4.0f, 8.0f, 8.0f, 0.8f);
        }

    g.setGradientFill (ColourGradient (Colours::transparentBlack, fw * 0.5f, fh * 0.5f,
                                       Colours::black.withAlpha (0.5f), 0, 0, true));
    g.fillAll();
    g.setColour (Colour (0xff111213));
    g.drawRect (0, 0, W, H, 2);
}

void LaPautaMicProAudioProcessorEditor::paint (Graphics& g)
{
    g.drawImageAt (background, 0, 0);

    // pequeño LED de encendido sobre el micrófono
    const bool on = processor.apvts.getRawParameterValue ("power")->load() > 0.5f;
    const float cx = 270.0f;
    if (on)
    {
        g.setGradientFill (ColourGradient (Colour (0xff5dff6b).withAlpha (0.8f), cx, 203.0f,
                                           Colour (0xff5dff6b).withAlpha (0.0f), cx + 10.0f, 203.0f, true));
        g.fillEllipse (cx - 10.0f, 193.0f, 20.0f, 20.0f);
    }
    g.setColour (on ? Colour (0xff7dff89) : Colour (0xff3a1a1a));
    g.fillEllipse (cx - 3.0f, 200.0f, 6.0f, 6.0f);
}
