/*
  ==============================================================================
    arxybin. — Glitch Particle Granular Effect
    Creator: nilotoo.

    ParticleBackground implementation.
  ==============================================================================
*/

#include "ParticleBackground.h"

namespace arxybin
{

ParticleBackground::ParticleBackground()
{
    particles.reserve(maxParticles);
    // Defer particle spawn + timer start to resized() — at construction time
    // getWidth()/getHeight() are 0, which causes NaN positions and crashes.
}

void ParticleBackground::visibilityChanged()
{
    if (isVisible() && !isTimerRunning())
    {
        if (particles.empty())
            respawnAll();
        startTimerHz(40);
    }
    else if (!isVisible() && isTimerRunning())
    {
        stopTimer();
    }
}

ParticleBackground::~ParticleBackground()
{
    stopTimer();
}

// ==============================================================================
void ParticleBackground::setAudioLevel(float level)
{
    audioLevel = juce::jlimit(0.0f, 1.0f, level);
}

// ==============================================================================
void ParticleBackground::respawnAll()
{
    particles.clear();
    for (int i = 0; i < maxParticles; ++i)
        spawnParticle();
}

void ParticleBackground::spawnParticle()
{
    Particle p;
    p.x         = random.nextFloat() * static_cast<float>(getWidth());
    p.y         = random.nextFloat() * static_cast<float>(getHeight());
    p.vx        = (random.nextFloat() - 0.5f) * 0.5f;
    p.vy        = (random.nextFloat() - 0.5f) * 0.3f - 0.1f; // bias upward
    p.size      = random.nextFloat() * 4.0f + 2.0f;
    p.alpha     = random.nextFloat() * 0.3f + 0.05f;
    p.shape     = random.nextInt(3); // 0=circle, 1=triangle, 2=diamond
    p.rotation  = random.nextFloat() * juce::MathConstants<float>::twoPi;
    p.rotSpeed  = (random.nextFloat() - 0.5f) * 0.02f;
    p.lifePhase = random.nextFloat();
    p.lifeSpeed = random.nextFloat() * 0.01f + 0.003f;

    if (particles.size() < static_cast<size_t>(maxParticles))
        particles.push_back(p);
    else
        particles[random.nextInt(maxParticles)] = p;
}

// ==============================================================================
void ParticleBackground::timerCallback()
{
    updateParticles();
    repaint();
}

// ==============================================================================
void ParticleBackground::updateParticles()
{
    const float w = static_cast<float>(getWidth());
    const float h = static_cast<float>(getHeight());
    const float speedBoost = 1.0f + audioLevel * 2.0f; // faster when loud

    for (auto& p : particles)
    {
        // Move
        p.x += p.vx * speedBoost;
        p.y += p.vy * speedBoost;

        // Wrap around edges
        if (p.x < -10.0f) p.x = w + 10.0f;
        if (p.x > w + 10.0f) p.x = -10.0f;
        if (p.y < -10.0f) p.y = h + 10.0f;
        if (p.y > h + 10.0f) p.y = -10.0f;

        // Rotate
        p.rotation += p.rotSpeed * speedBoost;

        // Pulsate
        p.lifePhase += p.lifeSpeed;
        if (p.lifePhase > 1.0f) p.lifePhase -= 1.0f;
    }
}

// ==============================================================================
void ParticleBackground::paint(juce::Graphics& g)
{
    const auto bg = juce::Colour(0xFFFAFAFC);
    g.fillAll(bg);

    const float audioBoost = 1.0f + audioLevel * 0.5f;

    for (const auto& p : particles)
    {
        // Pulsating alpha
        const float pulse = 0.5f + 0.5f * std::sin(p.lifePhase * juce::MathConstants<float>::twoPi);
        const float alpha = p.alpha * pulse * audioBoost;
        const auto colour = juce::Colour(0xFF3A6B8C).withAlpha(alpha);

        g.setColour(colour);

        const float cx = p.x;
        const float cy = p.y;
        const float s = p.size;

        switch (p.shape)
        {
            case 0: // Circle
                g.fillEllipse(cx - s, cy - s, s * 2.0f, s * 2.0f);
                break;

            case 1: // Triangle
            {
                juce::Path tri;
                tri.addTriangle(cx, cy - s,
                                cx - s * 0.866f, cy + s * 0.5f,
                                cx + s * 0.866f, cy + s * 0.5f);
                tri.applyTransform(juce::AffineTransform::rotation(p.rotation, cx, cy));
                g.fillPath(tri);
                break;
            }

            case 2: // Diamond
            {
                juce::Path diamond;
                diamond.addQuadrilateral(cx, cy - s,
                                         cx + s, cy,
                                         cx, cy + s,
                                         cx - s, cy);
                diamond.applyTransform(juce::AffineTransform::rotation(p.rotation, cx, cy));
                g.fillPath(diamond);
                break;
            }
        }
    }
}

} // namespace arxybin
