/*
  ==============================================================================
    arxybin. — Glitch Particle Granular Effect
    Creator: nilotoo.

    Grain — a single audio grain played back from the ring buffer.
    Each grain is a tiny slice of sound (1–500ms) with its own pitch,
    pan, direction, and amplitude envelope.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace arxybin
{

// ==============================================================================
class Grain
{
public:
    Grain() = default;

    // --- Configuration --------------------------------------------------------
    void setup(int writePosIn,              // Ring buffer write head (for position reference)
               int ringBufferSizeIn,         // Total ring buffer length in samples
               float grainLengthMs,          // Grain length in milliseconds
               float sampleRate,             // Current sample rate
               float pitchRatio,             // 0.25 – 4.0 (1.0 = original pitch)
               float positionRandom,         // 0.0 – 1.0 random offset
               float pan,                    // -1.0 – +1.0
               bool reverse,                 // Play backwards?
               float amplitude);             // 0.0 – 1.0

    // --- Per-sample processing ------------------------------------------------
    // Read one stereo sample pair from the ring buffer.
    // Returns false when the grain has finished playing.
    bool process(const float* ringBufferL,
                 const float* ringBufferR,
                 int ringBufferSize,
                 float& outL,
                 float& outR);

    // --- Lifecycle ------------------------------------------------------------
    bool isActive() const noexcept { return active; }
    bool isFinished() const noexcept { return finished; }

    // --- Envelope -------------------------------------------------------------
    // Returns the amplitude envelope value for the current position
    // Uses a Hann window (smooth fade-in / fade-out) to avoid clicks.
    float getEnvelope() const noexcept;

private:
    bool   active   = false;
    bool   finished = false;

    int    ringSize    = 0;
    int    fadeSamples = 0; // 10ms fade in/out to prevent clicks

    float  readPos  = 0.0f;    // Fractional read position in ring buffer
    float  pitch    = 1.0f;    // Playback speed (pitch ratio)
    float  pan      = 0.0f;    // -1.0 (full left) .. +1.0 (full right)
    float  amp      = 1.0f;    // Overall amplitude
    float  dir      = 1.0f;    // +1 = forward, -1 = reverse
    int    length   = 0;       // Grain length in samples
    int    age      = 0;       // How many samples processed so far

    // --- Internal -------------------------------------------------------------
    float readFromRing(const float* buffer, float pos, int size) const noexcept;
};

} // namespace arxybin
