/*
  ==============================================================================
    arxybin. — Glitch Particle Granular Effect
    Creator: nilotoo.

    BufferShuffler — randomly reorders segments of the audio buffer,
    creating glitchy, unpredictable rearrangements.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace arxybin
{

class BufferShuffler
{
public:
    BufferShuffler() = default;

    void prepare(double sampleRate, int maxBlockSize);
    void reset();

    // --- Parameters -----------------------------------------------------------
    void setAmount(float pct)       { amount = juce::jlimit(0.0f, 1.0f, pct * 0.01f); }
    void setAmtRandom(bool r)       { amtRandom = r; }
    void setSegmentSizeMs(float ms) { segmentSizeMs = juce::jlimit(1.0f, 1000.0f, ms); }
    void setSizeRandom(bool r)      { sizeRandom = r; }
    void setLfoMod(float aMod, float sMod) { lfoAmtMod = aMod; lfoSizeMod = sMod; }

    // --- Processing -----------------------------------------------------------
    void processBlock(juce::AudioBuffer<float>& buffer);

private:
    double sampleRate   = 44100.0;
    float  amount       = 0.0f;
    float  segmentSizeMs = 50.0f;
    bool   amtRandom     = false;
    bool   sizeRandom    = false;
    float  lfoAmtMod     = 0.0f;
    float  lfoSizeMod    = 0.0f;

    // Shuffle workspace
    juce::AudioBuffer<float> shuffleBuffer;
    int    shuffleBufferSize = 0;

    juce::Random random;
};

} // namespace arxybin
