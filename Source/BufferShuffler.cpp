/*
  ==============================================================================
    arxybin. — Glitch Particle Granular Effect
    Creator: nilotoo.

    BufferShuffler implementation.
  ==============================================================================
*/

#include "BufferShuffler.h"

namespace arxybin
{

void BufferShuffler::prepare(double sr, int maxBlockSize)
{
    sampleRate       = sr;
    shuffleBufferSize = maxBlockSize * 2; // extra room for shifts
    shuffleBuffer.setSize(2, shuffleBufferSize, false, true);
}

void BufferShuffler::reset()
{
    shuffleBuffer.clear();
}

void BufferShuffler::processBlock(juce::AudioBuffer<float>& buffer)
{
    // Apply randomisation per-block
    if (amtRandom)  amount       = juce::Random::getSystemRandom().nextFloat();
    if (sizeRandom) segmentSizeMs = juce::Random::getSystemRandom().nextFloat() * 999.0f + 1.0f;
    amount        = juce::jlimit(0.0f, 1.0f, amount + lfoAmtMod);
    segmentSizeMs = juce::jlimit(1.0f, 1000.0f, segmentSizeMs + lfoSizeMod * 300.0f);

    if (amount <= 0.0f)
        return;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    const int segSize = juce::jmax(2, static_cast<int>(segmentSizeMs * sampleRate * 0.001f));

    if (segSize >= numSamples)
        return; // Can't shuffle if segment is bigger than buffer

    const int numSegments = numSamples / segSize;
    if (numSegments < 2)
        return;

    // Copy to work buffer
    shuffleBuffer.clear();
    for (int ch = 0; ch < numChannels; ++ch)
        shuffleBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);

    // Determine how many segments to shuffle
    const int segmentsToShuffle = juce::jmax(1,
        static_cast<int>(static_cast<float>(numSegments) * amount));

    // Fisher-Yates partial shuffle of segment indices
    std::vector<int> indices(numSegments);
    std::iota(indices.begin(), indices.end(), 0);

    for (int i = 0; i < segmentsToShuffle; ++i)
    {
        const int j = random.nextInt(numSegments);
        std::swap(indices[i], indices[j]);
    }

    // Reassemble buffer in shuffled order
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* outData = buffer.getWritePointer(ch);
        const auto* inData = shuffleBuffer.getReadPointer(ch);

        for (int seg = 0; seg < numSegments; ++seg)
        {
            const int srcSeg = indices[seg];
            const int srcStart = srcSeg * segSize;
            const int dstStart = seg * segSize;

            for (int i = 0; i < segSize; ++i)
                outData[dstStart + i] = inData[srcStart + i];
        }
    }
}

} // namespace arxybin
