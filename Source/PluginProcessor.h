/*
  ==============================================================================
    arxybin. — PluginProcessor.
    Signal: Input → InGain → Granular → Distortion → Bitcrusher → Stutter →
            Shuffler → DryWet → OutGain → Output
    LFO modulates grain parameters per-sample.
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Parameters.h"
#include "GranularEngine.h"
#include "Distortion.h"
#include "Bitcrusher.h"
#include "StutterEffect.h"
#include "BufferShuffler.h"
#include "LfoEngine.h"
#include "PresetManager.h"

class ArxybinAudioProcessor : public juce::AudioProcessor
{
public:
    ArxybinAudioProcessor();
    ~ArxybinAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    juce::ValueTree selfBackup { "SELF_BACKUP" };
    juce::String    presetFolderPath; // custom preset folder path

    // --- LFO target query (for UI green indicators) -------------------------
    int getLfo1Target() const { return lfo1TargetIdx; }
    int getLfo2Target() const { return lfo2TargetIdx; }
    int getLfo3Target() const { return lfo3TargetIdx; }

    // --- Preset access (for UI) ----------------------------------------------
    arxybin::PresetManager& getPresetManager() { return presetManager; }
    const arxybin::PresetManager& getPresetManager() const { return presetManager; }

    // --- LFO modulation values + active targets (for UI green indicators) ----
    float lfoModDisplay[12] = {};
    int   lfo1TargetIdx = 0, lfo2TargetIdx = 0, lfo3TargetIdx = 0;

    // --- Waveform snapshots (for UI) -----------------------------------------
    const float* getDryWave() const { return drySnapshot; }
    const float* getWetWave() const { return wetSnapshot; }
    int getWaveSize() const { return 512; }
    int getBlockSize() const { return currentBlockSize; }

    // Granular engine state for UI visualization
    int getRingPos() const { return granularEngine.getRingPos(); }
    int getRingSize() const { return granularEngine.getRingSize(); }
    float getScanPhase() const { return granularEngine.getScanPhase(); }
    int getActiveGrainPositions(float* out, int max) const { return granularEngine.getActiveGrainReadPositions(out, max); }
    void fillRingSnapshot() { granularEngine.getRingBufferDecimated(ringSnapshot, waveSnapLen); }
    const float* getRingSnapshot() const { return ringSnapshot; }

private:
    int currentBlockSize = 0;
    arxybin::GranularEngine  granularEngine;
    arxybin::Distortion      distortion;
    arxybin::Bitcrusher      bitcrusher;
    arxybin::StutterEffect   stutter;
    arxybin::BufferShuffler  shuffler;
    arxybin::DualLfo         lfo;
    arxybin::PresetManager   presetManager;

    static constexpr int waveSnapLen = 512;
    float drySnapshot[waveSnapLen] = {};
    float wetSnapshot[waveSnapLen] = {};
    float ringSnapshot[waveSnapLen] = {};
    // getRingSnapshot() + getDryRingSnapshot() declared above in public section

    void readParams();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArxybinAudioProcessor)
};
