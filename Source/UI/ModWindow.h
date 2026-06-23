/*
  ==============================================================================
    arxybin. — MOD window. KnobLabel-style controls (no Slider),
    animated LFO waveform preview. Matches main UI.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace arxybin
{

class ModWindowContent : public juce::Component, public juce::Timer
{
public:
    explicit ModWindowContent(juce::AudioProcessorValueTreeState& apvts);
    ~ModWindowContent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    static const juce::Colour azureBlue, lightAzure, paleAzure, bgWhite, darkText, divLine;

private:
    // KnobLabel clone for MOD window
    struct ModKnob : public juce::Label
    {
        ModKnob(const juce::String& pid, juce::AudioProcessorValueTreeState& a);
        void paint(juce::Graphics& g) override;
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void mouseEnter(const juce::MouseEvent&) override;
        void mouseExit(const juce::MouseEvent&) override;
        void updateValue(float v);

        juce::AudioProcessorValueTreeState& apvts;
        juce::String pid;
        float val = 0, dispVal = 0, dragStart = 0;
        int   dragY = 0;
        bool  hover = false;
    };

    void drawLfoPreview(juce::Graphics& g, juce::Rectangle<int> area,
                        int waveType, float phaseOffset);

    juce::AudioProcessorValueTreeState& apvts;

    ModKnob lfo1Rate{"lfo1Rate",apvts}, lfo1Depth{"lfo1Depth",apvts};
    ModKnob lfo2Rate{"lfo2Rate",apvts}, lfo2Depth{"lfo2Depth",apvts};
    ModKnob lfo3Rate{"lfo3Rate",apvts}, lfo3Depth{"lfo3Depth",apvts};
    ModKnob lfo4Rate{"lfo4Rate",apvts}, lfo4Depth{"lfo4Depth",apvts};
    juce::ComboBox lfo1Target, lfo1Wave, lfo2Target, lfo2Wave;
    juce::ComboBox lfo3Target, lfo3Wave, lfo4Target, lfo4Wave;
    juce::Label lfo1RateLbl, lfo1DepthLbl, lfo1TargetLbl, lfo1WaveLbl;
    juce::Label lfo2RateLbl, lfo2DepthLbl, lfo2TargetLbl, lfo2WaveLbl;
    juce::Label lfo3RateLbl, lfo3DepthLbl, lfo3TargetLbl, lfo3WaveLbl;
    juce::Label lfo4RateLbl, lfo4DepthLbl, lfo4TargetLbl, lfo4WaveLbl;

    float lfo1Phase = 0, lfo2Phase = 0, lfo3Phase = 0, lfo4Phase = 0;
};

juce::DialogWindow* createModWindow(juce::AudioProcessorValueTreeState& apvts);

} // namespace arxybin
