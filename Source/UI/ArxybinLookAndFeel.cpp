/*
  ==============================================================================
    arxybin. — Glitch Particle Granular Effect
    Creator: nilotoo.

    ArxybinLookAndFeel implementation.
  ==============================================================================
*/

#include "ArxybinLookAndFeel.h"

namespace arxybin
{

const juce::Colour ArxybinLookAndFeel::bgWhite    { 0xFFFAFAFC };
const juce::Colour ArxybinLookAndFeel::azureBlue  { 0xFF3A6B8C };
const juce::Colour ArxybinLookAndFeel::lightAzure { 0xFF8AACC8 };
const juce::Colour ArxybinLookAndFeel::paleAzure  { 0xFFCDDBE8 };
const juce::Colour ArxybinLookAndFeel::darkText   { 0xFF2C4050 };

// ==============================================================================
ArxybinLookAndFeel::ArxybinLookAndFeel()
{
    // Intentionally empty — all colours are applied directly to components
    // via setColour() in PluginEditor. Modifying global JUCE state here
    // (setDefaultSansSerifTypefaceName, global setColour) can crash FL Studio.
}

// ==============================================================================
void ArxybinLookAndFeel::drawLinearSlider(juce::Graphics& g,
                                           int x, int y, int w, int h,
                                           float sliderPos, float minPos, float maxPos,
                                           juce::Slider::SliderStyle style,
                                           juce::Slider& slider)
{
    if (style != juce::Slider::LinearHorizontal)
        return LookAndFeel_V4::drawLinearSlider(g, x, y, w, h, sliderPos,
                                                 minPos, maxPos, style, slider);

    const float trackY = y + h * 0.5f;
    const float trackH = 4.0f;
    const float thumbW = 14.0f;
    const float thumbH = 20.0f;

    // Track background
    g.setColour(paleAzure);
    g.fillRoundedRectangle(static_cast<float>(x), trackY - trackH * 0.5f,
                           static_cast<float>(w), trackH, 2.0f);

    // Track fill (left of thumb)
    g.setColour(lightAzure);
    g.fillRoundedRectangle(static_cast<float>(x), trackY - trackH * 0.5f,
                           sliderPos - static_cast<float>(x), trackH, 2.0f);

    // Thumb — rounded rectangle with "organic" capsule shape
    const float thumbX = sliderPos - thumbW * 0.5f;
    const float thumbY = y + h * 0.5f - thumbH * 0.5f;

    g.setColour(azureBlue);
    g.fillRoundedRectangle(thumbX, thumbY, thumbW, thumbH, 4.0f);

    // Thumb highlight
    g.setColour(azureBlue.withAlpha(0.3f));
    g.fillRoundedRectangle(thumbX + 3.0f, thumbY + 2.0f, thumbW * 0.35f, thumbH - 4.0f, 2.0f);
}

// ==============================================================================
void ArxybinLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                           int x, int y, int w, int h,
                                           float sliderPos,
                                           float startAngle, float endAngle,
                                           juce::Slider&)
{
    const float radius = juce::jmin(w, h) * 0.4f;
    const float centreX = x + w * 0.5f;
    const float centreY = y + h * 0.5f;
    const float angle = startAngle + sliderPos * (endAngle - startAngle);

    // Outer ring
    g.setColour(paleAzure);
    g.drawEllipse(centreX - radius, centreY - radius,
                  radius * 2.0f, radius * 2.0f, 3.0f);

    // Arc (filled portion)
    juce::Path arc;
    arc.addCentredArc(centreX, centreY, radius, radius, 0.0f,
                      startAngle, angle, true);
    g.setColour(azureBlue);
    g.strokePath(arc, juce::PathStrokeType(3.0f));

    // Pointer dot
    const float dotX = centreX + radius * 0.75f * std::cos(angle);
    const float dotY = centreY + radius * 0.75f * std::sin(angle);
    g.setColour(azureBlue);
    g.fillEllipse(dotX - 3.0f, dotY - 3.0f, 6.0f, 6.0f);
}

// ==============================================================================
juce::Slider::SliderLayout ArxybinLookAndFeel::getSliderLayout(juce::Slider& slider)
{
    auto layout = LookAndFeel_V4::getSliderLayout(slider);

    if (slider.getSliderStyle() == juce::Slider::LinearHorizontal)
    {
        // Give more room to the slider track vs the text box
        layout.textBoxBounds.setWidth(56);
        layout.sliderBounds = slider.getLocalBounds()
            .withTrimmedRight(layout.textBoxBounds.getWidth() + 8);
    }

    return layout;
}

// ==============================================================================
juce::Label* ArxybinLookAndFeel::createSliderTextBox(juce::Slider& slider)
{
    auto* label = LookAndFeel_V4::createSliderTextBox(slider);
    label->setFont(juce::Font("Consolas", 9.0f, juce::Font::plain));
    label->setColour(juce::Label::textColourId, darkText);
    label->setColour(juce::Label::backgroundColourId, bgWhite);
    label->setColour(juce::Label::outlineColourId, lightAzure);
    return label;
}

// ==============================================================================
void ArxybinLookAndFeel::drawGroupComponentOutline(juce::Graphics& g,
                                                     int w, int h,
                                                     const juce::String&,
                                                     const juce::Justification&,
                                                     juce::GroupComponent&)
{
    // Rounded rect outline — soft impressionist feel
    g.setColour(paleAzure);
    g.drawRoundedRectangle(2.0f, 10.0f,
                           static_cast<float>(w) - 4.0f,
                           static_cast<float>(h) - 12.0f,
                           8.0f, 1.0f);
}

// ==============================================================================
void ArxybinLookAndFeel::drawButtonBackground(juce::Graphics& g,
                                               juce::Button& button,
                                               const juce::Colour&,
                                               bool isHighlighted,
                                               bool isDown)
{
    const auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);

    juce::Colour fill = azureBlue;
    if (isDown)        fill = azureBlue.darker(0.2f);
    if (isHighlighted) fill = azureBlue.brighter(0.2f);

    g.setColour(fill);
    g.fillRoundedRectangle(bounds, 5.0f);

    if (isHighlighted && !isDown)
    {
        g.setColour(bgWhite.withAlpha(0.15f));
        g.fillRoundedRectangle(bounds.withHeight(bounds.getHeight() * 0.5f), 5.0f);
    }
}

} // namespace arxybin
