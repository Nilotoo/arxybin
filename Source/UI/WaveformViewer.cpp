/*
  ==============================================================================
    arxybin. — Glitch Particle Granular Effect
    Creator: nilotoo.

    WaveformViewer implementation — impressionist waveform + spectrum.
  ==============================================================================
*/

#include "WaveformViewer.h"

namespace arxybin
{

WaveformViewer::WaveformViewer()
    : forwardFFT(fftOrder)
{
    waveformL.fill(0.0f);
    waveformR.fill(0.0f);
    spectrumData.fill(0.0f);
    fftBuffer.fill(0.0f);
    // Timer deferred to visibilityChanged() to avoid init-time crashes
}

void WaveformViewer::visibilityChanged()
{
    if (isVisible() && !isTimerRunning())
        startTimerHz(30);
    else if (!isVisible() && isTimerRunning())
        stopTimer();
}

WaveformViewer::~WaveformViewer()
{
    stopTimer();
}

// ==============================================================================
void WaveformViewer::pushBuffer(const juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    if (numSamples <= 0) return;

    // Store a decimated version of the waveform for the scrolling display
    const int step = juce::jmax(1, numSamples / 64);
    for (int i = 0; i < numSamples; i += step)
    {
        float sum = 0.0f;
        int count = 0;
        for (int j = 0; j < step && (i + j) < numSamples; ++j)
        {
            sum += std::abs(buffer.getSample(0, i + j));
            count++;
        }
        waveformL[writeIndex % historySize] = sum / static_cast<float>(count);
        waveformR[writeIndex % historySize] = waveformL[writeIndex % historySize]; // mono-ish
        writeIndex++;
    }

    // Compute RMS for glow
    float rms = 0.0f;
    for (int i = 0; i < numSamples; ++i)
    {
        const float s = buffer.getSample(0, i);
        rms += s * s;
    }
    rms = std::sqrt(rms / static_cast<float>(numSamples));
    smoothedLevel = smoothedLevel * 0.9f + rms * 0.1f;

    // FFT accumulation (decimate to fit FFT size)
    const int fftStep = juce::jmax(1, numSamples / fftSize);
    for (int i = 0; i < fftSize && fftCount < fftSize; ++i)
    {
        const int idx = i * fftStep;
        fftBuffer[fftCount * 2]     = (idx < numSamples) ? buffer.getSample(0, idx) : 0.0f;
        fftBuffer[fftCount * 2 + 1] = 0.0f;
        fftCount++;
    }
}

// ==============================================================================
void WaveformViewer::timerCallback()
{
    // Run FFT periodically
    if (fftCount >= fftSize)
    {
        forwardFFT.performRealOnlyForwardTransform(fftBuffer.data(), true);

        for (int i = 0; i < fftSize / 2; ++i)
        {
            const float re = fftBuffer[i * 2];
            const float im = fftBuffer[i * 2 + 1];
            const float mag = std::sqrt(re * re + im * im) / static_cast<float>(fftSize);
            spectrumData[i] = spectrumData[i] * 0.7f + mag * 0.3f; // smooth
        }

        fftCount = 0;
    }

    repaint();
}

// ==============================================================================
void WaveformViewer::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const float w = bounds.getWidth();
    const float h = bounds.getHeight();
    const float centreY = h * 0.5f;

    // Background fill
    g.setColour(juce::Colour(0xFFF5F6FA));
    g.fillRoundedRectangle(bounds, 6.0f);

    // --- Spectrum bars (background layer, very subtle) -------------------------
    const int numBars = 40;
    const float barWidth = w / static_cast<float>(numBars);

    for (int i = 0; i < numBars; ++i)
    {
        const float specVal = spectrumData[juce::jmin(i * 2, fftSize / 2 - 1)];
        const float barH = specVal * h * 0.6f;
        const float alpha = 0.08f + smoothedLevel * 0.3f;

        g.setColour(juce::Colour(0xFF3A6B8C).withAlpha(alpha));
        g.fillRoundedRectangle(i * barWidth + 1.0f, centreY - barH * 0.5f,
                               barWidth - 2.0f, barH, 2.0f);
    }

    // --- Waveform line (flowing, impressionist) --------------------------------
    juce::Path wavePath;
    bool firstPoint = true;

    const int displayPoints = static_cast<int>(w);
    const int startIdx = juce::jmax(0, writeIndex - displayPoints);

    for (int x = 0; x < displayPoints && (startIdx + x) < writeIndex; ++x)
    {
        const int idx = (startIdx + x) % historySize;
        const float amplitude = waveformL[idx] * h * 0.4f;
        const float y = centreY - amplitude;

        if (firstPoint)
        {
            wavePath.startNewSubPath(static_cast<float>(x), y);
            firstPoint = false;
        }
        else
        {
            wavePath.lineTo(static_cast<float>(x), y);
        }
    }

    // Draw waveform with glow
    {
        juce::Graphics::ScopedSaveState save(g);
        g.setColour(juce::Colour(0xFF3A6B8C).withAlpha(0.15f));
        g.strokePath(wavePath, juce::PathStrokeType(4.0f));
    }

    g.setColour(juce::Colour(0xFF3A6B8C));
    g.strokePath(wavePath, juce::PathStrokeType(1.5f));

    // --- Centre line (subtle) --------------------------------------------------
    g.setColour(juce::Colour(0xFFCDDBE8).withAlpha(0.5f));
    g.drawHorizontalLine(static_cast<int>(centreY), 0.0f, w);

    // --- Glow at centre when audio is loud -------------------------------------
    if (smoothedLevel > 0.05f)
    {
        const float glowAlpha = juce::jmin(0.2f, smoothedLevel * 0.5f);
        juce::ColourGradient glow(
            juce::Colour(0xFF3A6B8C).withAlpha(glowAlpha),
            bounds.getCentreX(), centreY,
            juce::Colour(0xFF3A6B8C).withAlpha(0.0f),
            bounds.getCentreX(), centreY + h * 0.3f,
            true);
        g.setGradientFill(glow);
        g.fillRoundedRectangle(bounds, 6.0f);
    }
}

void WaveformViewer::resized() {}

} // namespace arxybin
