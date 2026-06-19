/*
  ==============================================================================
    arxybin. — Glitch Particle Granular Effect
    Creator: nilotoo.

    ArxybinLookAndFeel — custom LookAndFeel with impressionist white + azure
    palette. Rounded, organic slider thumbs and gentle colour transitions.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace arxybin
{

// ==============================================================================
class ArxybinLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ArxybinLookAndFeel();
    ~ArxybinLookAndFeel() override = default;

    // --- Slider / Rotary ------------------------------------------------------
    void drawLinearSlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          juce::Slider::SliderStyle style,
                          juce::Slider& slider) override;

    void drawRotarySlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPos, float startAngle, float endAngle,
                          juce::Slider& slider) override;

    juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override;

    juce::Label* createSliderTextBox(juce::Slider& slider) override;

    // --- Group Component ------------------------------------------------------
    void drawGroupComponentOutline(juce::Graphics& g,
                                   int w, int h,
                                   const juce::String& text,
                                   const juce::Justification& position,
                                   juce::GroupComponent& group) override;

    // --- Buttons --------------------------------------------------------------
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    // --- Colour palette -------------------------------------------------------
    static const juce::Colour bgWhite;
    static const juce::Colour azureBlue;
    static const juce::Colour lightAzure;
    static const juce::Colour paleAzure;
    static const juce::Colour darkText;
};

} // namespace arxybin
