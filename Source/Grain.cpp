/*
  ==============================================================================
    arxybin. — Grain (updated for direct position control).
  ==============================================================================
*/

#include "Grain.h"

namespace arxybin
{

void Grain::setup(int writePosIn, int ringBufferSizeIn,
                  float grainLengthMs, float sampleRate,
                  float pitchRatio, float positionNorm,
                  float panIn, bool reverse, float amplitude)
{
    ringSize = ringBufferSizeIn;
    pitch    = juce::jlimit(0.125f, 8.0f, pitchRatio);
    pan      = juce::jlimit(-1.0f, 1.0f, panIn);
    amp      = juce::jlimit(0.0f, 1.0f, amplitude);
    dir      = reverse ? -1.0f : 1.0f;

    const float lenMs = juce::jlimit(1.0f, 500.0f, grainLengthMs);
    length = static_cast<int>(lenMs * sampleRate * 0.001f);
    length = juce::jmax(4, length);
    fadeSamples = static_cast<int>(sampleRate * 0.01f); // 10ms fade
    if (fadeSamples * 2 >= length) fadeSamples = length / 4; // don't over-fade short grains

    // positionNorm is 0-1 fraction of ring buffer
    const float posNorm = juce::jlimit(0.0f, 1.0f, positionNorm);
    // Map to ring buffer position (writePos is the "now" point, pos goes backwards)
    const int offset = static_cast<int>(posNorm * ringSize);
    readPos = static_cast<float>((writePosIn - offset + ringSize) % ringSize);

    age = 0; active = true; finished = false;
}

bool Grain::process(const float* ringL, const float* ringR, int rbSize,
                     float& outL, float& outR)
{
    if (!active || finished) { outL = 0; outR = 0; return false; }
    float env = getEnvelope();
    float sL = readFromRing(ringL, readPos, rbSize);
    float sR = readFromRing(ringR, readPos, rbSize);
    float pL = std::sqrt(0.5f * (1.0f - pan));
    float pR = std::sqrt(0.5f * (1.0f + pan));
    outL = sL * env * amp * pL;
    outR = sR * env * amp * pR;
    readPos += dir * pitch;
    if (readPos < 0) readPos += static_cast<float>(rbSize);
    if (readPos >= static_cast<float>(rbSize)) readPos -= static_cast<float>(rbSize);
    age++;
    if (age >= length) { finished = true; active = false; }
    return true;
}

float Grain::getEnvelope() const noexcept
{
    if (length <= 0) return 0;
    float phase = static_cast<float>(age) / static_cast<float>(length);
    float hann = 0.5f * (1.0f - std::cos(juce::MathConstants<float>::twoPi * phase));
    if (fadeSamples > 0)
    {
        if (age < fadeSamples)
            hann *= static_cast<float>(age) / static_cast<float>(fadeSamples);
        else if (age > length - fadeSamples)
            hann *= static_cast<float>(length - age) / static_cast<float>(fadeSamples);
    }
    return hann;
}

float Grain::readFromRing(const float* buf, float pos, int size) const noexcept
{
    int i0 = static_cast<int>(pos);
    int i1 = (i0 + 1) % size;
    float f = pos - static_cast<float>(i0);
    return buf[i0] * (1.0f - f) + buf[i1] * f;
}

} // namespace arxybin
