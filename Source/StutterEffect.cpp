/*
  ==============================================================================
    arxybin. — Glitch Particle Granular Effect
    Creator: nilotoo.

    StutterEffect implementation.
  ==============================================================================
*/

#include "StutterEffect.h"

namespace arxybin
{

void StutterEffect::prepare(double sr, int maxBlockSize)
{
    sampleRate    = sr;
    maxCaptureLen = static_cast<int>(sr * 2.0); // 2 seconds max
    captureL.resize(maxCaptureLen, 0.0f);
    captureR.resize(maxCaptureLen, 0.0f);

    reset();
}

void StutterEffect::reset()
{
    isStuttering     = false;
    stutterRemaining = 0;
    stutterLength    = 0;
    capturePos       = 0;
    std::fill(captureL.begin(), captureL.end(), 0.0f);
    std::fill(captureR.begin(), captureR.end(), 0.0f);
}

void StutterEffect::processBlock(juce::AudioBuffer<float>& buffer)
{
    // Apply randomisation + LFO per-block
    if (probRandom) probability  = juce::Random::getSystemRandom().nextFloat();
    if (lenRandom)  stutterLenMs = juce::Random::getSystemRandom().nextFloat() * 1999.0f + 1.0f;
    probability  = juce::jlimit(0.0f, 1.0f, probability + lfoProbMod);
    stutterLenMs = juce::jlimit(1.0f, 2000.0f, stutterLenMs + lfoLenMod * 500.0f);

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    stutterLength = static_cast<int>(stutterLenMs * sampleRate * 0.001f);
    stutterLength = juce::jmax(4, juce::jmin(stutterLength, maxCaptureLen));

    for (int s = 0; s < numSamples; ++s)
    {
        // --- Decide whether to trigger a new stutter --------------------------
        if (!isStuttering)
        {
            // Pass audio through normally AND record into capture buffer
            if (capturePos < maxCaptureLen)
            {
                captureL[capturePos] = buffer.getReadPointer(0)[s];
                captureR[capturePos] = (numChannels > 1)
                    ? buffer.getReadPointer(1)[s]
                    : captureL[capturePos];
                capturePos++;
            }

            // Random trigger check (once per sample)
            if (random.nextFloat() < probability * 0.01f && capturePos >= stutterLength)
            {
                isStuttering     = true;
                stutterRemaining = stutterLength;
            }
        }

        // --- If stuttering, loop the captured segment -------------------------
        if (isStuttering)
        {
            const int readIdx = (capturePos - stutterLength)
                              + ((stutterLength - stutterRemaining) % stutterLength);

            if (readIdx >= 0 && readIdx < capturePos)
            {
                buffer.getWritePointer(0)[s] = captureL[readIdx];
                if (numChannels > 1)
                    buffer.getWritePointer(1)[s] = captureR[readIdx];
            }

            stutterRemaining--;
            if (stutterRemaining <= 0)
            {
                isStuttering = false;
                // Reset capture to start fresh after stutter
                capturePos = 0;
            }
        }
    }
}

} // namespace arxybin
