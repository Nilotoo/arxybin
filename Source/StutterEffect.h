/*
  ==============================================================================
    arxybin. — Glitch Particle Granular Effect
    Creator: nilotoo.

    StutterEffect — randomly captures and repeats short audio segments,
    like a glitching playback device.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace arxybin
{

class StutterEffect
{
public:
    StutterEffect() = default;

    void prepare(double sampleRate, int maxBlockSize);
    void reset();

    // --- Parameters -----------------------------------------------------------
    void setProbability(float pct)    { probability = juce::jlimit(0.0f, 1.0f, pct * 0.01f); }
    void setProbRandom(bool r)        { probRandom = r; }
    void setLengthMs(float ms)        { stutterLenMs = juce::jlimit(1.0f, 2000.0f, ms); }
    void setLenRandom(bool r)         { lenRandom = r; }
    void setLfoMod(float pMod, float lMod) { lfoProbMod = pMod; lfoLenMod = lMod; }

    // --- Processing -----------------------------------------------------------
    void processBlock(juce::AudioBuffer<float>& buffer);

private:
    double sampleRate    = 44100.0;
    float  probability   = 0.1f;
    float  stutterLenMs  = 100.0f;
    bool   probRandom     = false;
    bool   lenRandom      = false;
    float  lfoProbMod     = 0.0f;
    float  lfoLenMod      = 0.0f;

    // Stutter state
    bool   isStuttering    = false;
    int    stutterRemaining = 0;
    int    stutterLength    = 0;

    // Capture buffer for the stutter segment
    std::vector<float> captureL;
    std::vector<float> captureR;
    int    capturePos = 0;
    int    maxCaptureLen = 88200; // 2 seconds at 44.1kHz

    // Random
    juce::Random random;
};

} // namespace arxybin
