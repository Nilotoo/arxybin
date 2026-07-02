/*
  ==============================================================================
    arxybin. — GranularEngine with scan modes, placement, LFO support.
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Grain.h"

namespace arxybin
{

class GranularEngine
{
public:
    GranularEngine();
    void prepare(double sampleRate, int maxBlockSize);
    void reset();

    // Setters
    void setGrainSize(float ms)       { grainSizeMs = ms; }
    void setGrainRandom(bool r)       { grainRandom = r; }
    void setDensity(int g)            { density = g; }
    void setPitch(float s)            { pitchSemis = s; }
    void setPitchRandom(float p)      { pitchRandomPct = p * 0.01f; }
    void setPitchChroma(int c)        { pitchChroma = c; }
    void setPitchRandomSync(int s)    { pitchRandSyncIdx = s; }
    void setPitchSyncRate(float r)    { pitchSyncRate = r; }
    void setPitchSyncInterval(int i)  { pitchSyncInterval = juce::jmax(1, i); }
    void setPanSpread(float p)        { panSpreadPct = p * 0.01f; }
    void setPanRandom(bool r)         { panRandom = r; }
    void setReverseProb(float p)      { reverseProbPct = p * 0.01f; }
    void setFeedback(float p)         { feedbackPct = p * 0.01f; }
    void setScanMode(int m)           { scanMode = m; }
    void setScanSpeed(float s)        { scanSpeed = s; }
    void setGrainPosition(float p)    { grainPositionPct = p * 0.01f; }
    void setPositionRandom(float p)   { positionRandomPct = p * 0.01f; }
    void setPlaceOffset(float o)      { placeOffset = o * 0.01f; }
    void setPlaceGrid(int g)          { placeGrid = g; }
    void setPlaceRate(float r)        { placeRate = r; }
    void setPlaceSnap(bool s)         { placeSnap = s; }
    void setPlaceRandom(float r)      { placeRandom = r * 0.01f; }

    void applyLfoMod(float pitchM, float posM, float panM, float sizeM, float densM);

    void processBlock(const juce::AudioBuffer<float>& input,
                      juce::AudioBuffer<float>& output);

    // Getters for UI visualization
    int getRingPos() const { return ringPos; }
    int getRingSize() const { return ringSz; }
    float getScanPhase() const { return static_cast<float>(scanPhase); }
    int getActiveGrainReadPositions(float* out, int maxGrains) const;
    void getRingBufferDecimated(float* out, int outLen) const;
    void setCaptureBufferMs(double sr, float ms);

private:
    int maxRingSamples = 44100 * 4; // dynamic, default 4s
    static constexpr int maxGrains = 256;
    std::vector<float> ringL, ringR;
    std::array<Grain, maxGrains> grains;
    int ringPos = 0, ringSz = 0, grainIdx = 0;
    int samplesSinceGrain = 0, grainInterval = 44100;

    std::vector<float> fbL, fbR;
    float sampleRate = 44100;

    // Parameters
    float grainSizeMs = 50, pitchSemis = 0, pitchRandomPct = 0;
    float positionRandomPct = 0.5f, panSpreadPct = 0.8f;
    float reverseProbPct = 0.2f, feedbackPct = 0;
    int   density = 10;
    bool  grainRandom = false, panRandom = false;
    int   scanMode = 0;
    float scanSpeed = 1, grainPositionPct = 0.5f;
    float placeOffset = 0, placeRate = 1, placeRandom = 0;
    int   placeGrid = 0;
    bool  placeSnap = true;

    // LFO modulation accumulators
    float lfoPitchMod = 0, lfoPosMod = 0, lfoPanMod = 0;
    float lfoSizeMod = 0, lfoDensMod = 0;
    double scanPhase = 0;

    // Pitch random chroma + sync
    int   pitchChroma = 0, pitchRandSyncIdx = 0;
    float pitchSyncRate = 0, lastPitchRand = 0;
    int   pitchSyncCounter = 0, pitchSyncInterval = 0;

    float semitonesToRatio(float s) const { return std::pow(2.0f, s / 12.0f); }
    void spawnGrain();
};

} // namespace arxybin
