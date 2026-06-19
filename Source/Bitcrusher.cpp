/*
  ==============================================================================
    arxybin. — Glitch Particle Granular Effect
    Creator: nilotoo.

    Bitcrusher implementation.
  ==============================================================================
*/

#include "Bitcrusher.h"

namespace arxybin
{

void Bitcrusher::prepare(double sr)
{
    sampleRate  = sr;
    holdCounter = 0;
    holdMax     = 1;
    lastSampleL = 0.0f;
    lastSampleR = 0.0f;
}

void Bitcrusher::reset()
{
    holdCounter = 0;
    lastSampleL = 0.0f;
    lastSampleR = 0.0f;
}

void Bitcrusher::setRateReduction(int factor)
{
    rateReduxIdx = juce::jlimit(0, 6, factor);
    // Map: 0→1x, 1→2x, 2→4x, 3→8x, 4→16x, 5→32x, 6→64x
    holdMax = 1 << rateReduxIdx;
    if (holdMax < 1) holdMax = 1;
}

void Bitcrusher::processBlock(juce::AudioBuffer<float>& buffer)
{
    // Apply randomisation + LFO per-block
    if (randomBit)  targetBitDepth = juce::Random::getSystemRandom().nextInt(23) + 1;
    if (randomRate) rateReduxIdx   = juce::Random::getSystemRandom().nextInt(7);
    targetBitDepth = juce::jlimit(1, 24, targetBitDepth + static_cast<int>(lfoMod * 12.0f));

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Smooth bit depth transitions to avoid zipper noise
    const float target = static_cast<float>(targetBitDepth);
    const float smoothing = 0.995f;

    for (int s = 0; s < numSamples; ++s)
    {
        smoothBitDepth = smoothBitDepth * smoothing + target * (1.0f - smoothing);
    }

    bitDepth = juce::roundToInt(smoothBitDepth);
    bitDepth = juce::jlimit(1, 24, bitDepth);

    if (bitDepth >= 24 && holdMax <= 1)
        return;

    // Quantisation step
    const int levels = 1 << bitDepth;
    const float scale = static_cast<float>(levels - 1);
    const float invScale = 1.0f / scale;

    for (int s = 0; s < numSamples; ++s)
    {
        // Sample rate reduction: sample-and-hold
        if (holdCounter > 0)
        {
            // Hold previous value
            for (int ch = 0; ch < numChannels; ++ch)
                buffer.getWritePointer(ch)[s] = (ch == 0) ? lastSampleL : lastSampleR;
            holdCounter--;
            continue;
        }
        holdCounter = holdMax - 1;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float& sample = buffer.getWritePointer(ch)[s];

            // Bit depth reduction via quantisation
            if (bitDepth < 24)
            {
                // Map from [-1, 1] to [0, levels-1], quantise, map back
                const float shifted = (sample + 1.0f) * 0.5f; // [0, 1]
                const float quantised = std::round(shifted * scale) * invScale;
                sample = quantised * 2.0f - 1.0f; // back to [-1, 1]
            }

            // Store for sample-and-hold
            if (ch == 0) lastSampleL = sample;
            else         lastSampleR = sample;
        }
    }
}

} // namespace arxybin
