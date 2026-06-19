/*
  ==============================================================================
    arxybin. — Glitch Particle Granular Effect
    Creator: nilotoo.

    PresetManager — load/save presets as XML.
    Manages factory presets and user presets.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace arxybin
{

// ==============================================================================
struct PresetInfo
{
    juce::String name;
    juce::String category;
    juce::String author;
    int         index = 0;
};

// ==============================================================================
class PresetManager
{
public:
    PresetManager(juce::AudioProcessorValueTreeState& apvtsRef);

    // Called once the host is fully ready (from prepareToPlay)
    void initialise();

    // --- Preset data ----------------------------------------------------------
    const std::vector<PresetInfo>& getPresets() const { return presets; }
    int  getCurrentIndex() const                   { return currentIndex; }
    juce::String getCurrentName() const;

    // --- Actions --------------------------------------------------------------
    void loadPreset(int index);
    void saveUserPreset(const juce::String& name, const juce::String& author = {});
    void deleteUserPreset(int index);
    int  getNumFactoryPresets() const;
    void scanPresetFolders(juce::AudioProcessorValueTreeState& apvts);

    // --- File-based save/load -------------------------------------------------
    bool saveToFile(const juce::File& file, const juce::String& author,
                    juce::AudioProcessorValueTreeState& apvts) const;
    bool loadFromFile(const juce::File& file,
                      juce::AudioProcessorValueTreeState& apvts);
    static juce::File getPresetFolder(const juce::String& subFolder = {},
                                        const juce::String& customPath = {});

    // --- Serialisation --------------------------------------------------------
    juce::XmlElement getStateXml() const;
    void setStateXml(const juce::XmlElement& xml);

private:
    juce::AudioProcessorValueTreeState& apvts;
    std::vector<PresetInfo> presets;
    juce::ValueTree presetBank; // contains all preset data
    int currentIndex = 0;

    void createFactoryPresets();
    void rebuildIndex();
    void addFactoryPreset(const juce::String& name,
                          const std::vector<std::pair<juce::String, float>>& params);
};

} // namespace arxybin
