/*
  ==============================================================================
    arxybin. — GranularEngine with scan modes + placement + LFO.
  ==============================================================================
*/

#include "GranularEngine.h"

namespace arxybin
{

GranularEngine::GranularEngine()
{
    ringL.resize(maxRingSamples, 0); ringR.resize(maxRingSamples, 0);
}

void GranularEngine::setCaptureBufferMs(double sr, float ms)
{
    maxRingSamples = static_cast<int>(sr * ms * 0.001);
    maxRingSamples = juce::jmax(64, maxRingSamples);
}

void GranularEngine::prepare(double sr, int maxBlk)
{
    sampleRate = sr;
    ringSz = juce::jmin(maxRingSamples, static_cast<int>(sr * 4));
    ringPos = 0;
    ringL.resize(maxRingSamples, 0); ringR.resize(maxRingSamples, 0);
    std::fill(ringL.begin(), ringL.end(), 0);
    std::fill(ringR.begin(), ringR.end(), 0);
    fbL.resize(maxBlk, 0); fbR.resize(maxBlk, 0);
    grainInterval = juce::jmax(1, static_cast<int>(sr / juce::jmax(1.0f, static_cast<float>(density))));
    samplesSinceGrain = 0; scanPhase = 0;
    for (auto& g : grains) g = Grain{};
}

void GranularEngine::reset()
{
    ringPos = 0;
    std::fill(ringL.begin(), ringL.end(), 0);
    std::fill(ringR.begin(), ringR.end(), 0);
    std::fill(fbL.begin(), fbL.end(), 0);
    std::fill(fbR.begin(), fbR.end(), 0);
    scanPhase = 0;
}

void GranularEngine::applyLfoMod(float pm, float posm, float panm, float sm, float dm)
{
    lfoPitchMod = pm; lfoPosMod = posm; lfoPanMod = panm;
    lfoSizeMod = sm; lfoDensMod = dm;
}

void GranularEngine::processBlock(const juce::AudioBuffer<float>& input,
                                   juce::AudioBuffer<float>& output)
{
    const int ns = output.getNumSamples();
    const int nc = juce::jmin(input.getNumChannels(), 2);

    const float baseDensity = static_cast<float>(density) * (1.0f + lfoDensMod);
    grainInterval = juce::jmax(1, static_cast<int>(sampleRate / juce::jmax(0.5f, baseDensity)));

    // Scan sweep: scanSpeed=1 → one full buffer sweep per buffer duration
    const float scanDelta = (ringSz > 0) ? scanSpeed / ringSz : 0;

    for (int s = 0; s < ns; ++s)
    {
        // Write input
        float inL = input.getReadPointer(0)[s] + fbL[s] * feedbackPct;
        float inR = (nc > 1) ? input.getReadPointer(1)[s] + fbR[s] * feedbackPct : inL;
        ringL[ringPos] = inL;
        ringR[ringPos] = inR;

        // Spawn grain
        samplesSinceGrain++;
        if (samplesSinceGrain >= grainInterval) { samplesSinceGrain = 0; spawnGrain(); }

        // Process active grains
        float outL = 0, outR = 0; int active = 0;
        for (auto& g : grains)
        {
            if (!g.isActive()) continue;
            float gl, gr;
            if (g.process(ringL.data(), ringR.data(), ringSz, gl, gr))
                { outL += gl; outR += gr; active++; }
        }
        float norm = active > 1 ? 1.0f / std::sqrt(static_cast<float>(active)) : 1.0f;
        outL *= norm; outR *= norm;

        output.getWritePointer(0)[s] = outL;
        if (nc > 1) output.getWritePointer(1)[s] = outR;
        fbL[s] = outL; fbR[s] = outR;

        ringPos = (ringPos + 1) % ringSz;
        scanPhase += scanDelta;
        if (scanPhase >= 1.0) scanPhase -= std::floor(scanPhase);

        // Pitch random sync counter
        if (pitchSyncInterval > 0) {
            ++pitchSyncCounter;
            if (pitchSyncCounter >= pitchSyncInterval) {
                pitchSyncCounter = 0;
                lastPitchRand = juce::Random::getSystemRandom().nextFloat();
            }
        }
    }
}

void GranularEngine::spawnGrain()
{
    auto& rng = juce::Random::getSystemRandom();

    // Grain size
    float size = grainRandom ? rng.nextFloat() * grainSizeMs + 1.0f : grainSizeMs;
    size *= (1.0f + lfoSizeMod);
    size = juce::jlimit(1.0f, 500.0f, size);

    // Pitch
    float randPitch = 0;
    if (pitchRandomPct > 0) {
        float r = (pitchSyncInterval > 0) ? lastPitchRand : rng.nextFloat();
        randPitch = (r * 2.0f - 1.0f) * pitchRandomPct * 24.0f;
        // Chroma mode: snap to chromatic intervals
        if (pitchChroma > 0) {
            int chromaSemis[] = {0, 3, 5, 7, 12}; // Off, 3rd(+/-3), 5th(+/-4or5), ...wait
            // Actually: Off=0, 3rd=1 (snap to ±3 or ±4), 5th=2 (snap to ±5 or ±7), Octave=3 (snap to ±12)
            int intervals_3rd[] = {0, 3, 4, -3, -4};
            int intervals_5th[] = {0, 5, 7, -5, -7};
            int intervals_oct[] = {0, 12, -12};
            int* target = nullptr; int count = 0;
            if (pitchChroma == 1) { target = intervals_3rd; count = 5; }
            else if (pitchChroma == 2) { target = intervals_5th; count = 5; }
            else if (pitchChroma == 3) { target = intervals_oct; count = 3; }
            if (target && count > 0) {
                // Find nearest interval
                float nearest = 0; float bestDist = 999;
                for (int i = 0; i < count; ++i) {
                    float dist = std::abs(randPitch - (float)target[i]);
                    if (dist < bestDist) { bestDist = dist; nearest = (float)target[i]; }
                }
                randPitch = nearest;
            }
        }
    }
    float pitch = pitchSemis + randPitch + lfoPitchMod;
    float ratio = semitonesToRatio(pitch);

    // Position — base from parameter, plus scan mode, plus LFO
    float pos = grainPositionPct;
    // Scan mode
    switch (scanMode)
    {
        case 0: pos = static_cast<float>(scanPhase); break;                              // Forward
        case 1: pos = 1.0f - static_cast<float>(scanPhase); break;                      // Reverse
        case 2: pos = std::abs(1.0f - 2.0f * static_cast<float>(scanPhase)); break;     // Bidirectional
        case 3: pos = rng.nextFloat(); break;                                            // Random
    }
    pos += lfoPosMod;
    // Placement offset + random
    pos += placeOffset + (placeRandom > 0 ? rng.nextFloat() * placeRandom : 0);
    // Grid snap
    if (placeSnap && placeGrid > 0) {
        int divs[] = {1, 4, 8, 16, 32};
        int d = divs[juce::jlimit(0, 4, placeGrid)];
        pos = std::round(pos * d) / static_cast<float>(d);
    }
    // Spray
    float spray = positionRandomPct > 0 ? rng.nextFloat() * positionRandomPct : 0;
    pos = juce::jlimit(0.0f, 1.0f, pos + spray);

    // Pan
    float pan = panSpreadPct > 0 ? (rng.nextFloat() * 2 - 1) * panSpreadPct : 0;
    if (panRandom) pan = (rng.nextFloat() * 2 - 1);
    pan += lfoPanMod;
    pan = juce::jlimit(-1.0f, 1.0f, pan);

    bool rev = rng.nextFloat() < reverseProbPct;
    float amp = 0.5f;

    grainIdx = (grainIdx + 1) % maxGrains;
    grains[grainIdx].setup(ringPos, ringSz, size, static_cast<float>(sampleRate),
                            ratio, pos, pan, rev, amp);
}

int GranularEngine::getActiveGrainReadPositions(float* out, int maxOut) const
{
    int count = 0;
    for (int i = 0; i < maxGrains && count < maxOut; ++i)
    {
        if (grains[i].isActive())
        {
            out[count] = grains[i].getReadPos();
            ++count;
        }
    }
    return count;
}

void GranularEngine::getRingBufferDecimated(float* out, int outLen) const
{
    if (ringSz <= 0) { std::fill(out, out + outLen, 0.0f); return; }
    const int step = juce::jmax(1, ringSz / outLen);
    // Average over step window to prevent jitter, newest→oldest order
    for (int i = 0; i < outLen; ++i)
    {
        float sum = 0.0f; int cnt = 0;
        for (int j = 0; j < step; ++j)
        {
            int idx = (ringPos - (i * step + j) - 1 + ringSz * 2) % ringSz;
            sum += ringL[idx]; ++cnt;
        }
        out[i] = sum / (float)cnt;
    }
}

} // namespace arxybin
