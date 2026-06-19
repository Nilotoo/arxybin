/*
  ==============================================================================
    arxybin. — Glitch Particle Granular Effect
    Creator: nilotoo.

    ParticleBackground — floating geometric particles (circles, triangles,
    diamonds) that drift in the background and respond subtly to audio.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace arxybin
{

// ==============================================================================
class ParticleBackground : public juce::Component,
                            public juce::Timer
{
public:
    ParticleBackground();
    ~ParticleBackground() override;

    // --- Audio reactivity -----------------------------------------------------
    void setAudioLevel(float level);

    // --- juce::Timer ----------------------------------------------------------
    void timerCallback() override;

    // --- juce::Component ------------------------------------------------------
    void visibilityChanged() override;
    void paint(juce::Graphics& g) override;

private:
    // A single floating particle
    struct Particle
    {
        float x, y;           // position
        float vx, vy;         // velocity
        float size;           // radius / half-size
        float alpha;          // opacity
        int   shape;          // 0=circle, 1=triangle, 2=diamond
        float rotation;       // current rotation angle
        float rotSpeed;       // rotation speed
        float lifePhase;      // 0..1 for pulsating alpha
        float lifeSpeed;       // how fast the pulse cycles
    };

    static constexpr int maxParticles = 60;
    std::vector<Particle> particles;

    float audioLevel = 0.0f;
    juce::Random random;

    void spawnParticle();
    void respawnAll();
    void updateParticles();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParticleBackground)
};

} // namespace arxybin
