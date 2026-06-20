/*
  ==============================================================================
    arxybin. — Complete parameter set (~40 params, 8 groups).
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

namespace arxybin
{

namespace ParamID
{
    #define DECLARE_ID(name) const inline juce::String name { #name }

    // Grains (10)
    DECLARE_ID(grainSize);      DECLARE_ID(grainRandom);
    DECLARE_ID(grainDensity);   DECLARE_ID(grainPitch);
    DECLARE_ID(pitchRandom);    DECLARE_ID(panSpread);
    DECLARE_ID(panRandom);      DECLARE_ID(reverseProb);
    DECLARE_ID(grainFeedback);

    // Scan (4)
    DECLARE_ID(scanMode);       DECLARE_ID(scanSpeed);
    DECLARE_ID(grainPosition);  DECLARE_ID(positionRandom);
    DECLARE_ID(captureBufferMs); DECLARE_ID(bpmSync);

    // Placement (5)
    DECLARE_ID(placeOffset);    DECLARE_ID(placeGrid);
    DECLARE_ID(placeRate);      DECLARE_ID(placeSnap);
    DECLARE_ID(placeRandom);

    // Distortion (3)
    DECLARE_ID(distType);       DECLARE_ID(distDrive);
    DECLARE_ID(distRandom);

    // Bitcrusher (4)
    DECLARE_ID(bitDepth);       DECLARE_ID(bitRandom);
    DECLARE_ID(sampleRateRedux); DECLARE_ID(rateRandom);

    // Stutter (4)
    DECLARE_ID(stutterProb);    DECLARE_ID(stutProbRandom);
    DECLARE_ID(stutterLength);  DECLARE_ID(stutLenRandom);

    // Shuffler (4)
    DECLARE_ID(shuffleAmount);  DECLARE_ID(shufAmtRandom);
    DECLARE_ID(shuffleSegment); DECLARE_ID(shufSizeRandom);

    // Mix (3)
    DECLARE_ID(dryWet);         DECLARE_ID(inputGain);
    DECLARE_ID(outputGain);

    // Per-effect Glitch dry/wet (4)
    DECLARE_ID(distMix);        DECLARE_ID(bitMix);
    DECLARE_ID(stutterMix);     DECLARE_ID(shuffleMix);

    // LFO1 (4)
    DECLARE_ID(lfo1Rate);       DECLARE_ID(lfo1Depth);
    DECLARE_ID(lfo1Target);     DECLARE_ID(lfo1Wave);

    // LFO2 (4)
    DECLARE_ID(lfo2Rate);       DECLARE_ID(lfo2Depth);
    DECLARE_ID(lfo2Target);     DECLARE_ID(lfo2Wave);

    // LFO3 (4)
    DECLARE_ID(lfo3Rate);       DECLARE_ID(lfo3Depth);
    DECLARE_ID(lfo3Target);     DECLARE_ID(lfo3Wave);

    #undef DECLARE_ID
}

// ==============================================================================
inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    using Range = juce::NormalisableRange<float>;
    using Layout = juce::AudioProcessorValueTreeState::ParameterLayout;
    Layout lay;

    auto fp = [&](const juce::String& id, const juce::String& name,
                  float lo, float hi, float step, float def) {
        lay.add(std::make_unique<juce::AudioParameterFloat>(id, name, Range(lo, hi, step), def));
    };
    auto in = [&](const juce::String& id, const juce::String& name, int lo, int hi, int def) {
        lay.add(std::make_unique<juce::AudioParameterInt>(id, name, lo, hi, def));
    };
    auto bo = [&](const juce::String& id, const juce::String& name, bool def) {
        lay.add(std::make_unique<juce::AudioParameterBool>(id, name, def));
    };
    auto ch = [&](const juce::String& id, const juce::String& name,
                  juce::StringArray opts, int def) {
        lay.add(std::make_unique<juce::AudioParameterChoice>(id, name, opts, def));
    };

    // ======================== GRAINS (10) ======================================
    fp(ParamID::grainSize,    "Grain Size",    30, 500, 0.1f, 50);
    bo(ParamID::grainRandom,  "Grain Random",  false);
    in(ParamID::grainDensity, "Density",       1,  100, 10);
    fp(ParamID::grainPitch,   "Pitch",        -24, 24,  1,    0);
    fp(ParamID::pitchRandom,  "Pitch Random",  0,  100, 1,    0);
    fp(ParamID::panSpread,    "Pan Spread",    0,  100, 1,   80);
    bo(ParamID::panRandom,    "Pan Random",    false);
    fp(ParamID::reverseProb,  "Reverse",       0,  100, 1,   20);
    fp(ParamID::grainFeedback,"Feedback",      0,  100, 1,    0);

    // ======================== SCAN (4) =========================================
    ch(ParamID::scanMode,     "Scan Mode",
       juce::StringArray{"Forward","Reverse","Bidirectional","Random"}, 0);
    fp(ParamID::scanSpeed,    "Scan Speed",    0.1f, 10, 0.1f, 1);
    fp(ParamID::grainPosition,"Position",      0,   100, 1,   50);
    fp(ParamID::positionRandom,"Spray",         0,   100, 1,   50);
    // Capture
    fp(ParamID::captureBufferMs,"CapBuf",        20,  30000, 1,  4000);
    bo(ParamID::bpmSync,        "BPM Sync",      false);

    // ======================== PLACEMENT (5) ====================================
    fp(ParamID::placeOffset,  "Offset",        0,   100, 1,   0);
    ch(ParamID::placeGrid,    "Grid",
       juce::StringArray{"Off","1/4","1/8","1/16","1/32"}, 0);
    fp(ParamID::placeRate,    "Rate",          0.1f, 10, 0.1f, 1);
    bo(ParamID::placeSnap,    "Snap",          true);
    fp(ParamID::placeRandom,  "Rand",          0,   100, 1,   0);

    // ======================== DISTORTION (3) ===================================
    ch(ParamID::distType,     "Dist Type",
       juce::StringArray{"Soft Clip","Hard Clip","Foldback","Fuzz"}, 0);
    fp(ParamID::distDrive,    "Drive",         0,   100, 1,   0);
    bo(ParamID::distRandom,   "Dist Random",   false);

    // ======================== BITCRUSHER (4) ===================================
    in(ParamID::bitDepth,     "Bit Depth",     5,   24,  24);
    bo(ParamID::bitRandom,    "Bit Random",    false);
    ch(ParamID::sampleRateRedux,"Rate Reduce",
       juce::StringArray{"1x","2x","4x","8x","16x","32x","64x"}, 0);
    bo(ParamID::rateRandom,   "Rate Random",   false);

    // ======================== STUTTER (4) ======================================
    fp(ParamID::stutterProb,  "Stutter %",     0,   100, 1,   10);
    bo(ParamID::stutProbRandom,"Stut% Random", false);
    fp(ParamID::stutterLength,"Stutter Len",   1,  2000, 0.1f, 100);
    bo(ParamID::stutLenRandom,"StutLen Random",false);

    // ======================== SHUFFLER (4) =====================================
    fp(ParamID::shuffleAmount,"Shuffle Amt",   0,   100, 1,   0);
    bo(ParamID::shufAmtRandom,"ShufAmt Random",false);
    fp(ParamID::shuffleSegment,"Shuffle Size", 1,  1000, 0.1f, 50);
    bo(ParamID::shufSizeRandom,"ShufSz Random",false);

    // ======================== MIX (3) ==========================================
    fp(ParamID::dryWet,       "Dry / Wet",     0,   100, 1,   50);
    fp(ParamID::inputGain,    "Input",        -24,  24,  0.1f, 0);
    fp(ParamID::outputGain,   "Output",       -24,  24,  0.1f, 0);

    // ======================== GLITCH MIX (4) ===================================
    fp(ParamID::distMix,      "Dist Mix",      0,   100, 1,   100);
    fp(ParamID::bitMix,       "Bit Mix",       0,   100, 1,   100);
    fp(ParamID::stutterMix,   "Stutter Mix",   0,   100, 1,   100);
    fp(ParamID::shuffleMix,   "Shuffle Mix",   0,   100, 1,   100);

    // ======================== LFO1 (4) =========================================
    fp(ParamID::lfo1Rate,     "LFO1 Rate",     0.01f, 1000, 0.01f, 1);
    fp(ParamID::lfo1Depth,    "LFO1 Depth",    0,     100,  1,     0);
    ch(ParamID::lfo1Target,   "LFO1 Target",
       juce::StringArray{"Off","Pitch","Pos","Pan","Size","Density",
                         "DistDrv","BitDepth","Stut%","StutLen","ShfAmt","ShfSz"}, 0);
    ch(ParamID::lfo1Wave,     "LFO1 Wave",
       juce::StringArray{"Sine","Triangle","Saw","Square","S&H"}, 0);

    // ======================== LFO2 (4) =========================================
    fp(ParamID::lfo2Rate,     "LFO2 Rate",     0.01f, 1000, 0.01f, 1);
    fp(ParamID::lfo2Depth,    "LFO2 Depth",    0,     100,  1,     0);
    ch(ParamID::lfo2Target,   "LFO2 Target",
       juce::StringArray{"Off","Pitch","Pos","Pan","Size","Density",
                         "DistDrv","BitDepth","Stut%","StutLen","ShfAmt","ShfSz"}, 0);
    ch(ParamID::lfo2Wave,     "LFO2 Wave",
       juce::StringArray{"Sine","Triangle","Saw","Square","S&H"}, 0);

    // ======================== LFO3 (4) =========================================
    fp(ParamID::lfo3Rate,     "LFO3 Rate",     0.01f, 1000, 0.01f, 1);
    fp(ParamID::lfo3Depth,    "LFO3 Depth",    0,     100,  1,     0);
    ch(ParamID::lfo3Target,   "LFO3 Target",
       juce::StringArray{"Off","Pitch","Pos","Pan","Size","Density",
                         "DistDrv","BitDepth","Stut%","StutLen","ShfAmt","ShfSz"}, 0);
    ch(ParamID::lfo3Wave,     "LFO3 Wave",
       juce::StringArray{"Sine","Triangle","Saw","Square","S&H"}, 0);

    return lay;
}

inline float getRawParam(const juce::AudioProcessorValueTreeState& apvts,
                         const juce::String& paramID)
{
    return apvts.getRawParameterValue(paramID)->load();
}

} // namespace arxybin
