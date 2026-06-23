/*
  ==============================================================================
    arxybin. — LFO Engine implementation.
  ==============================================================================
*/

#include "LfoEngine.h"

namespace arxybin
{

// ==============================================================================
// Single LFO
// ==============================================================================
void LfoEngine::prepare(double sampleRate)
{
    sr    = sampleRate;
    phase = 0.0;
    lastRandom = 0.0f;
    randomCounter = 0;
}

void LfoEngine::reset()
{
    phase = 0.0;
    lastRandom = 0.0f;
}

float LfoEngine::process()
{
    if (params.depth <= 0.0f || params.target == LfoTarget::Off)
        return 0.0f;

    // Advance phase
    const double delta = static_cast<double>(params.rate) / sr;
    phase += delta;
    if (phase >= 1.0) phase -= std::floor(phase);

    float raw = 0.0f;

    switch (params.wave)
    {
        case LfoWaveform::Sine:
            raw = std::sin(phase * juce::MathConstants<double>::twoPi);
            break;

        case LfoWaveform::Triangle:
            raw = 1.0f - 4.0f * std::abs(static_cast<float>(phase) - 0.5f);
            break;

        case LfoWaveform::Saw:
            raw = static_cast<float>(phase) * 2.0f - 1.0f;
            break;

        case LfoWaveform::Square:
            raw = (phase < 0.5) ? 1.0f : -1.0f;
            break;

        case LfoWaveform::SampleAndHold:
            randomCounter++;
            if (randomCounter >= static_cast<int>(sr / juce::jmax(0.01, params.rate * 4.0)))
            {
                lastRandom = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
                randomCounter = 0;
            }
            raw = lastRandom;
            break;
    }

    return raw * (params.depth * 0.01f);
}

float LfoEngine::modulate(float base, float range) const
{
    juce::ignoreUnused(base, range);
    return 0.0f;
}

// ==============================================================================
// Quad LFO
// ==============================================================================
void DualLfo::prepare(double sampleRate)
{
    lfo1.prepare(sampleRate);
    lfo2.prepare(sampleRate);
    lfo3.prepare(sampleRate);
    lfo4.prepare(sampleRate);
}

void DualLfo::reset()
{
    lfo1.reset();
    lfo2.reset();
    lfo3.reset();
    lfo4.reset();
}

DualLfo::ModValues DualLfo::process()
{
    ModValues mv;
    const float m1 = lfo1.process();
    const float m2 = lfo2.process();
    const float m3 = lfo3.process();
    const float m4 = lfo4.process();

    auto apply = [&](LfoTarget t, float mod, float& out, LfoTarget match, float scale) {
        if (t == match) out += mod * scale;
    };

    #define APPLY(tgt, mod, field, targetEnum, scale) apply(tgt, mod, mv.field, LfoTarget::targetEnum, scale)
    #define APPLY_LFO(lfoTgt, modVal) \
        APPLY(lfoTgt, modVal, pitchMod,    GrainPitch,    24.0f); \
        APPLY(lfoTgt, modVal, positionMod, GrainPosition, 0.5f); \
        APPLY(lfoTgt, modVal, panMod,      GrainPan,      1.0f); \
        APPLY(lfoTgt, modVal, sizeMod,     GrainSize,     0.8f); \
        APPLY(lfoTgt, modVal, densityMod,  GrainDensity,  0.5f); \
        APPLY(lfoTgt, modVal, distMod,     DistDrive,     1.0f); \
        APPLY(lfoTgt, modVal, bitMod,      BitDepth,      1.0f); \
        APPLY(lfoTgt, modVal, stutProbMod, StutterProb,   1.0f); \
        APPLY(lfoTgt, modVal, stutLenMod,  StutterLen,    1.0f); \
        APPLY(lfoTgt, modVal, shufAmtMod,  ShuffleAmt,    1.0f); \
        APPLY(lfoTgt, modVal, shufSzMod,   ShuffleSize,   1.0f); \
        APPLY(lfoTgt, modVal, dryWetMod,   DryWet,        1.0f); \
        APPLY(lfoTgt, modVal, inGainMod,   InGain,        24.0f); \
        APPLY(lfoTgt, modVal, outGainMod,  OutGain,       24.0f); \
        APPLY(lfoTgt, modVal, distMixMod,  DistMix,       1.0f); \
        APPLY(lfoTgt, modVal, bitMixMod,   BitMix,        1.0f); \
        APPLY(lfoTgt, modVal, stutMixMod,  StutterMix,    1.0f); \
        APPLY(lfoTgt, modVal, shfMixMod,   ShuffleMix,    1.0f);

    APPLY_LFO(lfo1Params.target, m1)
    APPLY_LFO(lfo2Params.target, m2)
    APPLY_LFO(lfo3Params.target, m3)
    APPLY_LFO(lfo4Params.target, m4)

    #undef APPLY
    #undef APPLY_LFO

    return mv;
}

} // namespace arxybin
