/*
  ==============================================================================
    arxybin. — Distortion module.
    Waveshaping distortion: Soft Clip, Hard Clip, Foldback, Fuzz.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace arxybin
{

enum class DistType { SoftClip, HardClip, Foldback, Fuzz };

class Distortion
{
public:
    Distortion() = default;

    void prepare(double sampleRate)  { juce::ignoreUnused(sampleRate); }
    void reset()                     {}

    void setType(DistType t)         { type = t; }
    void setDrive(float d)           { drive = juce::jlimit(0.0f, 1.0f, d); }
    void setRandom(bool r)           { random = r; }
    void setLfoMod(float m)          { lfoMod = m; }

    void processBlock(juce::AudioBuffer<float>& buffer);

private:
    DistType type   = DistType::SoftClip;
    float    drive  = 0.0f;
    bool     random = false;
    float    lfoMod = 0.0f;

    float processSample(float x, float effectiveDrive) const;
};

} // namespace arxybin
