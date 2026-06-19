/*
  ==============================================================================
    arxybin. — Distortion implementation.
  ==============================================================================
*/

#include "Distortion.h"

namespace arxybin
{

float Distortion::processSample(float x, float effectiveDrive) const
{
    if (effectiveDrive <= 0.0f) return x;

    const float gain = 1.0f + effectiveDrive * 40.0f;
    float y = x * gain;

    switch (type)
    {
        case DistType::SoftClip:
            y = std::tanh(y); break;
        case DistType::HardClip:
            y = juce::jlimit(-1.0f, 1.0f, y); break;
        case DistType::Foldback:
            y = std::abs(y) > 1.0f
                ? (std::fmod(std::abs(y) + 1.0f, 4.0f) - 2.0f) * (y > 0 ? 1.0f : -1.0f)
                : y; break;
        case DistType::Fuzz:
            y = (y > 0.0f ? 1.0f : -1.0f) * (1.0f - std::exp(-std::abs(y))); break;
    }

    return y * (1.0f / juce::jmax(0.1f, 1.0f + effectiveDrive * 3.0f));
}

void Distortion::processBlock(juce::AudioBuffer<float>& buffer)
{
    float d = drive;
    if (random) d = juce::Random::getSystemRandom().nextFloat() * drive + 0.01f;
    d = juce::jlimit(0.0f, 1.0f, d + lfoMod);
    if (d <= 0.0f) return;

    const int n = buffer.getNumSamples();
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int s = 0; s < n; ++s)
            data[s] = processSample(data[s], d);
    }
}

} // namespace arxybin
