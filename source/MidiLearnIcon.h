#pragma once

// Vector MIDI socket designed for the shared 26 px toolbar button.
// No raster resampling, cached images or process-lifetime graphics objects.
namespace afterglow::artwork
{
inline void drawMidiLearnIcon(juce::Graphics& graphics,
                              juce::Rectangle<float> bounds,
                              juce::Colour colour)
{
    const auto centre = bounds.getCentre();
    const float scale = juce::jmin(bounds.getWidth(), bounds.getHeight())
                      / 26.0f;
    if (scale <= 0.0f)
        return;

    juce::Path socket;
    socket.startNewSubPath(-2.5f, -8.15f);
    socket.cubicTo(-6.05f, -7.15f, -8.5f, -3.9f, -8.5f, 0.0f);
    socket.cubicTo(-8.5f, 4.7f, -4.7f, 8.5f, 0.0f, 8.5f);
    socket.cubicTo(4.7f, 8.5f, 8.5f, 4.7f, 8.5f, 0.0f);
    socket.cubicTo(8.5f, -3.9f, 6.05f, -7.15f, 2.5f, -8.15f);
    socket.lineTo(2.5f, -6.1f);
    socket.cubicTo(2.5f, -2.9f, -2.5f, -2.9f, -2.5f, -6.1f);
    socket.closeSubPath();
    socket.applyTransform(juce::AffineTransform::scale(scale)
                              .translated(centre.x, centre.y));

    graphics.setColour(colour);
    graphics.strokePath(
        socket,
        juce::PathStrokeType(1.6f * scale,
                             juce::PathStrokeType::curved,
                             juce::PathStrokeType::rounded));

    constexpr float pins[][2] =
    {
        { -5.25f, 0.75f }, { -3.65f, 4.05f }, { 0.0f, 5.55f },
        { 3.65f, 4.05f }, { 5.25f, 0.75f }
    };
    const float radius = 1.18f * scale;
    for (const auto& pin : pins)
        graphics.fillEllipse(centre.x + pin[0] * scale - radius,
                             centre.y + pin[1] * scale - radius,
                             radius * 2.0f, radius * 2.0f);
}
} // namespace afterglow::artwork
