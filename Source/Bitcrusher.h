/*
  ==============================================================================
    arxybin. — Glitch Particle Granular Effect
    Creator: nilotoo.

    Bitcrusher — Lo-fi digital destruction.
    Reduces bit depth (1–24 bits) and sample rate (1x–64x reduction).
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace arxybin
{

class Bitcrusher
{
public:
    Bitcrusher() = default;

    void prepare(double sampleRate);
    void reset();

    // --- Parameters -----------------------------------------------------------
    void setBitDepth(int bits)       { targetBitDepth = juce::jlimit(1, 24, bits); }
    void setRandom(bool r)            { randomBit = r; }
    void setRateReduction(int factor);
    void setRateRandom(bool r)        { randomRate = r; }
    void setLfoMod(float m)            { lfoMod = m; }

    // --- Processing -----------------------------------------------------------
    void processBlock(juce::AudioBuffer<float>& buffer);

private:
    double sampleRate     = 44100.0;
    int    targetBitDepth = 24;
    float  smoothBitDepth = 24.0f; // smoothed value for click-free transitions
    int    bitDepth       = 24;
    int    rateReduxIdx   = 0;
    int    holdCounter    = 0;
    int    holdMax        = 1;
    float  lastSampleL    = 0.0f;
    float  lastSampleR    = 0.0f;
    bool   randomBit      = false;
    bool   randomRate     = false;
    float  lfoMod         = 0.0f;
};

} // namespace arxybin
