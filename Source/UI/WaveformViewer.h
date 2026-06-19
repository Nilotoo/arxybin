/*
  ==============================================================================
    arxybin. — Glitch Particle Granular Effect
    Creator: nilotoo.

    WaveformViewer — real-time waveform + spectral display with an
    impressionist, flowing visual style (white + azure palette).
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace arxybin
{

// ==============================================================================
class WaveformViewer : public juce::Component,
                       public juce::Timer
{
public:
    WaveformViewer();
    ~WaveformViewer() override;

    // --- Audio data feeding ---------------------------------------------------
    void pushBuffer(const juce::AudioBuffer<float>& buffer);

    // --- juce::Timer ----------------------------------------------------------
    void timerCallback() override;

    // --- juce::Component ------------------------------------------------------
    void paint(juce::Graphics& g) override;
    void resized() override;
    void visibilityChanged() override;

private:
    // Waveform history (ring buffer of recent samples for scrolling display)
    static constexpr int historySize = 1024;
    std::array<float, historySize> waveformL;
    std::array<float, historySize> waveformR;
    int writeIndex = 0;

    // Smoothed RMS level for glow intensity
    float smoothedLevel = 0.0f;

    // Spectrum data
    static constexpr int fftOrder = 10;
    static constexpr int fftSize  = 1 << fftOrder; // 1024
    juce::dsp::FFT forwardFFT;
    std::array<float, fftSize * 2> fftBuffer;
    std::array<float, fftSize / 2> spectrumData;
    int fftCount = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformViewer)
};

} // namespace arxybin
