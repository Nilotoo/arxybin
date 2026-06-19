/*
  ==============================================================================
    arxybin. — LFO Engine.
    Two independent LFOs. Each computes a modulation value per sample.
    Waveforms: Sine, Triangle, Saw, Square, Sample & Hold.
    Rate: 0.01 – 1000 Hz.  Depth: 0 – 100%.  Target: assignable.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace arxybin
{

// ==============================================================================
enum class LfoWaveform { Sine, Triangle, Saw, Square, SampleAndHold };
enum class LfoTarget    { Off, GrainPitch, GrainPosition, GrainPan, GrainSize, GrainDensity,
                             DistDrive, BitDepth, StutterProb, StutterLen, ShuffleAmt, ShuffleSize };

struct LfoParams
{
    float rate     = 1.0f;
    float depth    = 0.0f;
    LfoWaveform wave = LfoWaveform::Sine;
    LfoTarget  target = LfoTarget::Off;
};

// ==============================================================================
class LfoEngine
{
public:
    LfoEngine() = default;

    void prepare(double sampleRate);
    void reset();

    void setParams(const LfoParams& p) { params = p; }

    // Returns modulation value in [-1, 1] scaled by depth
    float process();

    // Convenience: apply modulation to a base value
    float modulate(float base, float range) const;

private:
    double sr    = 44100.0;
    double phase = 0.0;
    LfoParams params;

    float lastRandom = 0.0f;
    int   randomCounter = 0;
};

// ==============================================================================
// Dual LFO manager
// ==============================================================================
class DualLfo
{
public:
    void prepare(double sampleRate);
    void reset();

    void setLfo1(const LfoParams& p) { lfo1.setParams(p); }
    void setLfo2(const LfoParams& p) { lfo2.setParams(p); }
    void setLfo3(const LfoParams& p) { lfo3.setParams(p); }

    const LfoParams& getLfo1Params() const { return lfo1Params; }
    const LfoParams& getLfo2Params() const { return lfo2Params; }
    const LfoParams& getLfo3Params() const { return lfo3Params; }
    void setLfo1Params(const LfoParams& p) { lfo1Params = p; lfo1.setParams(p); }
    void setLfo2Params(const LfoParams& p) { lfo2Params = p; lfo2.setParams(p); }
    void setLfo3Params(const LfoParams& p) { lfo3Params = p; lfo3.setParams(p); }

    // Process both LFOs. Returns modulated values for pitch, position, pan,
    // size, density (applied by caller).
    struct ModValues
    {
        float pitchMod    = 0.0f;
        float positionMod = 0.0f;
        float panMod      = 0.0f;
        float sizeMod     = 0.0f;
        float densityMod  = 0.0f;
        float distMod     = 0.0f;
        float bitMod      = 0.0f;
        float stutProbMod = 0.0f;
        float stutLenMod  = 0.0f;
        float shufAmtMod  = 0.0f;
        float shufSzMod   = 0.0f;
    };

    ModValues process();

private:
    LfoEngine  lfo1, lfo2, lfo3;
    LfoParams  lfo1Params, lfo2Params, lfo3Params;
};

} // namespace arxybin
