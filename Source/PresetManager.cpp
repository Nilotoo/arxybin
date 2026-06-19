/*
  ==============================================================================
    arxybin. — Glitch Particle Granular Effect
    Creator: nilotoo.

    PresetManager implementation — factory presets + user preset management.
  ==============================================================================
*/

#include "PresetManager.h"

namespace arxybin
{

// ==============================================================================
PresetManager::PresetManager(juce::AudioProcessorValueTreeState& apvtsRef)
    : apvts(apvtsRef)
    , presetBank("PRESET_BANK")
{
    // Default preset — user-specified values
    juce::ValueTree preset("PRESET");
    preset.setProperty("name", "default", nullptr);
    preset.setProperty("category", "factory", nullptr);
    preset.setProperty("author", "arxybin.", nullptr);
    // GRAINS
    preset.setProperty("grainSize", 200, nullptr);
    preset.setProperty("grainDensity", 40, nullptr);
    preset.setProperty("grainPitch", 0, nullptr);
    preset.setProperty("pitchRandom", 0, nullptr);
    preset.setProperty("panSpread", 85, nullptr);
    preset.setProperty("reverseProb", 0, nullptr);
    preset.setProperty("grainFeedback", 35, nullptr);
    preset.setProperty("grainRandom", 0, nullptr);
    preset.setProperty("panRandom", 0, nullptr);
    // SCAN
    preset.setProperty("scanMode", 0, nullptr);
    preset.setProperty("scanSpeed", 2, nullptr);
    preset.setProperty("grainPosition", 30, nullptr);
    preset.setProperty("positionRandom", 50, nullptr);
    // PLACEMENT
    preset.setProperty("placeOffset", 0, nullptr);
    preset.setProperty("placeGrid", 2, nullptr);
    preset.setProperty("placeRate", 1, nullptr);
    preset.setProperty("placeSnap", 0, nullptr);
    preset.setProperty("placeRandom", 0, nullptr);
    // DISTORTION
    preset.setProperty("distType", 0, nullptr);
    preset.setProperty("distDrive", 0, nullptr);
    preset.setProperty("distRandom", 0, nullptr);
    preset.setProperty("distMix", 0, nullptr);
    // BITCRUSHER
    preset.setProperty("bitDepth", 24, nullptr);
    preset.setProperty("sampleRateRedux", 0, nullptr);
    preset.setProperty("bitRandom", 0, nullptr);
    preset.setProperty("rateRandom", 0, nullptr);
    preset.setProperty("bitMix", 0, nullptr);
    // STUTTER
    preset.setProperty("stutterProb", 35, nullptr);
    preset.setProperty("stutterLength", 259, nullptr);
    preset.setProperty("stutProbRandom", 0, nullptr);
    preset.setProperty("stutLenRandom", 0, nullptr);
    preset.setProperty("stutterMix", 0, nullptr);
    // SHUFFLER
    preset.setProperty("shuffleAmount", 0, nullptr);
    preset.setProperty("shuffleSegment", 90, nullptr);
    preset.setProperty("shufAmtRandom", 0, nullptr);
    preset.setProperty("shufSizeRandom", 0, nullptr);
    preset.setProperty("shuffleMix", 0, nullptr);
    // MIX
    preset.setProperty("dryWet", 50, nullptr);
    preset.setProperty("inputGain", 0, nullptr);
    preset.setProperty("outputGain", 0, nullptr);
    // LFO
    for (int lfo = 1; lfo <= 3; ++lfo)
    {
        auto pfx = "lfo" + juce::String(lfo);
        preset.setProperty(pfx + "Rate",   1.0f, nullptr);
        preset.setProperty(pfx + "Depth",  0.0f, nullptr);
        preset.setProperty(pfx + "Target", 0,    nullptr);
        preset.setProperty(pfx + "Wave",   0,    nullptr);
    }
    presetBank.appendChild(preset, nullptr);
    rebuildIndex();
    scanPresetFolders(apvts);
    // Defer loadPreset() — calling setValueNotifyingHost during plugin
    // construction crashes some hosts (FL Studio). Call initialise() from
    // prepareToPlay() instead.
}

void PresetManager::initialise()
{
    if (!presets.empty() && currentIndex == 0)
        loadPreset(0);
}

// ==============================================================================
void PresetManager::createFactoryPresets()
{
    // 11 factory presets covering different sonic territories
    using ParamPair = std::pair<juce::String, float>;

    addFactoryPreset("Default", {
        {"grainSize", 50}, {"grainDensity", 10}, {"grainPitch", 0},
        {"pitchRandom", 30}, {"positionRandom", 50}, {"panSpread", 80},
        {"reverseProb", 20}, {"grainFeedback", 0},
        {"bitDepth", 24}, {"sampleRateRedux", 0}, {"stutterProb", 10},
        {"stutterLength", 100}, {"shuffleAmount", 0}, {"shuffleSegment", 50},
        {"dryWet", 50}, {"inputGain", 0}, {"outputGain", 0}
    });

    addFactoryPreset("Cloud Drift", {
        {"grainSize", 120}, {"grainDensity", 25}, {"grainPitch", 0},
        {"pitchRandom", 40}, {"positionRandom", 70}, {"panSpread", 90},
        {"reverseProb", 15}, {"grainFeedback", 20},
        {"bitDepth", 24}, {"sampleRateRedux", 0}, {"stutterProb", 0},
        {"stutterLength", 100}, {"shuffleAmount", 0}, {"shuffleSegment", 50},
        {"dryWet", 60}, {"inputGain", 0}, {"outputGain", 0}
    });

    addFactoryPreset("Digital Rain", {
        {"grainSize", 15}, {"grainDensity", 60}, {"grainPitch", 7},
        {"pitchRandom", 60}, {"positionRandom", 40}, {"panSpread", 70},
        {"reverseProb", 30}, {"grainFeedback", 10},
        {"bitDepth", 8}, {"sampleRateRedux", 2}, {"stutterProb", 15},
        {"stutterLength", 80}, {"shuffleAmount", 10}, {"shuffleSegment", 30},
        {"dryWet", 70}, {"inputGain", 0}, {"outputGain", -2}
    });

    addFactoryPreset("Glitch Machine", {
        {"grainSize", 30}, {"grainDensity", 40}, {"grainPitch", -5},
        {"pitchRandom", 80}, {"positionRandom", 80}, {"panSpread", 60},
        {"reverseProb", 50}, {"grainFeedback", 30},
        {"bitDepth", 4}, {"sampleRateRedux", 3}, {"stutterProb", 40},
        {"stutterLength", 200}, {"shuffleAmount", 50}, {"shuffleSegment", 80},
        {"dryWet", 80}, {"inputGain", 3}, {"outputGain", -4}
    });

    addFactoryPreset("Shattered Glass", {
        {"grainSize", 8}, {"grainDensity", 80}, {"grainPitch", 12},
        {"pitchRandom", 90}, {"positionRandom", 90}, {"panSpread", 100},
        {"reverseProb", 60}, {"grainFeedback", 5},
        {"bitDepth", 6}, {"sampleRateRedux", 1}, {"stutterProb", 25},
        {"stutterLength", 50}, {"shuffleAmount", 30}, {"shuffleSegment", 20},
        {"dryWet", 65}, {"inputGain", 0}, {"outputGain", -3}
    });

    addFactoryPreset("Underwater", {
        {"grainSize", 200}, {"grainDensity", 8}, {"grainPitch", -7},
        {"pitchRandom", 20}, {"positionRandom", 60}, {"panSpread", 50},
        {"reverseProb", 10}, {"grainFeedback", 40},
        {"bitDepth", 24}, {"sampleRateRedux", 0}, {"stutterProb", 0},
        {"stutterLength", 100}, {"shuffleAmount", 0}, {"shuffleSegment", 50},
        {"dryWet", 55}, {"inputGain", 0}, {"outputGain", 0}
    });

    addFactoryPreset("Tape Rot", {
        {"grainSize", 45}, {"grainDensity", 15}, {"grainPitch", -2},
        {"pitchRandom", 50}, {"positionRandom", 30}, {"panSpread", 40},
        {"reverseProb", 35}, {"grainFeedback", 15},
        {"bitDepth", 10}, {"sampleRateRedux", 1}, {"stutterProb", 20},
        {"stutterLength", 150}, {"shuffleAmount", 15}, {"shuffleSegment", 60},
        {"dryWet", 60}, {"inputGain", 2}, {"outputGain", -1}
    });

    addFactoryPreset("Stutter Storm", {
        {"grainSize", 60}, {"grainDensity", 20}, {"grainPitch", 0},
        {"pitchRandom", 10}, {"positionRandom", 20}, {"panSpread", 60},
        {"reverseProb", 5}, {"grainFeedback", 0},
        {"bitDepth", 24}, {"sampleRateRedux", 0}, {"stutterProb", 70},
        {"stutterLength", 300}, {"shuffleAmount", 0}, {"shuffleSegment", 50},
        {"dryWet", 85}, {"inputGain", 0}, {"outputGain", 0}
    });

    addFactoryPreset("Cosmic Dust", {
        {"grainSize", 250}, {"grainDensity", 6}, {"grainPitch", 5},
        {"pitchRandom", 60}, {"positionRandom", 85}, {"panSpread", 100},
        {"reverseProb", 25}, {"grainFeedback", 50},
        {"bitDepth", 24}, {"sampleRateRedux", 0}, {"stutterProb", 5},
        {"stutterLength", 500}, {"shuffleAmount", 5}, {"shuffleSegment", 100},
        {"dryWet", 50}, {"inputGain", 0}, {"outputGain", 0}
    });

    addFactoryPreset("Circuit Bend", {
        {"grainSize", 10}, {"grainDensity", 100}, {"grainPitch", 0},
        {"pitchRandom", 100}, {"positionRandom", 100}, {"panSpread", 100},
        {"reverseProb", 70}, {"grainFeedback", 25},
        {"bitDepth", 3}, {"sampleRateRedux", 4}, {"stutterProb", 50},
        {"stutterLength", 120}, {"shuffleAmount", 70}, {"shuffleSegment", 40},
        {"dryWet", 90}, {"inputGain", 4}, {"outputGain", -6}
    });

    addFactoryPreset("Vinyl Decay", {
        {"grainSize", 80}, {"grainDensity", 12}, {"grainPitch", -3},
        {"pitchRandom", 35}, {"positionRandom", 45}, {"panSpread", 30},
        {"reverseProb", 40}, {"grainFeedback", 10},
        {"bitDepth", 12}, {"sampleRateRedux", 1}, {"stutterProb", 30},
        {"stutterLength", 180}, {"shuffleAmount", 20}, {"shuffleSegment", 70},
        {"dryWet", 55}, {"inputGain", 1}, {"outputGain", -2}
    });
}

// ==============================================================================
void PresetManager::addFactoryPreset(const juce::String& name,
                                      const std::vector<std::pair<juce::String, float>>& params)
{
    juce::ValueTree preset("PRESET");
    preset.setProperty("name", name, nullptr);
    preset.setProperty("category", "factory", nullptr);
    preset.setProperty("author", "nilotoo.", nullptr);

    for (const auto& [id, value] : params)
        preset.setProperty(id, value, nullptr);

    // Ensure LFO params are always present (default: Off, no modulation)
    for (int lfo = 1; lfo <= 3; ++lfo)
    {
        auto pfx = "lfo" + juce::String(lfo);
        if (!preset.hasProperty(pfx + "Rate"))   preset.setProperty(pfx + "Rate",   1.0f, nullptr);
        if (!preset.hasProperty(pfx + "Depth"))  preset.setProperty(pfx + "Depth",  0.0f, nullptr);
        if (!preset.hasProperty(pfx + "Target")) preset.setProperty(pfx + "Target", 0,    nullptr);
        if (!preset.hasProperty(pfx + "Wave"))   preset.setProperty(pfx + "Wave",   0,    nullptr);
    }

    presetBank.appendChild(preset, nullptr);
}

// ==============================================================================
void PresetManager::rebuildIndex()
{
    presets.clear();
    for (int i = 0; i < presetBank.getNumChildren(); ++i)
    {
        auto child = presetBank.getChild(i);
        PresetInfo info;
        info.name     = child.getProperty("name").toString();
        info.category = child.getProperty("category").toString();
        info.author   = child.getProperty("author").toString();
        info.index    = i;
        presets.push_back(info);
    }
}

// ==============================================================================
juce::String PresetManager::getCurrentName() const
{
    if (currentIndex >= 0 && currentIndex < static_cast<int>(presets.size()))
        return presets[static_cast<size_t>(currentIndex)].name;
    return "Default";
}

int PresetManager::getNumFactoryPresets() const
{
    return static_cast<int>(std::count_if(presets.begin(), presets.end(),
        [](const PresetInfo& p) { return p.category == "factory"; }));
}

// ==============================================================================
void PresetManager::loadPreset(int index)
{
    if (index < 0 || index >= presetBank.getNumChildren())
        return;

    currentIndex = index;
    auto preset = presetBank.getChild(index);

    for (int i = 0; i < preset.getNumProperties(); ++i)
    {
        const auto propName = preset.getPropertyName(i).toString();
        if (propName == "name" || propName == "category" || propName == "author")
            continue;

        if (auto* param = apvts.getParameter(propName))
        {
            float rawVal = static_cast<float>(preset.getProperty(propName));
            float normVal;
            if (auto* choice = dynamic_cast<juce::AudioParameterChoice*>(param))
            {
                // Choice: stored as index, normalize directly
                int idx = juce::jlimit(0, choice->getAllValueStrings().size()-1,
                                       static_cast<int>(rawVal));
                normVal = static_cast<float>(idx) / juce::jmax(1.0f,
                    static_cast<float>(choice->getAllValueStrings().size() - 1));
            }
            else
            {
                // Float/Int/Bool: use getValueForText for correct range mapping
                normVal = param->getValueForText(juce::String(rawVal));
            }
            param->setValueNotifyingHost(normVal);
        }
    }
}

// ==============================================================================
void PresetManager::saveUserPreset(const juce::String& name, const juce::String& author)
{
    // Also save to disk in users/ folder
    auto file = getPresetFolder("users").getChildFile(name + ".arxybin");
    saveToFile(file, author, apvts);

    juce::ValueTree preset("PRESET");
    preset.setProperty("name", name, nullptr);
    preset.setProperty("category", "user", nullptr);
    preset.setProperty("author", author, nullptr);

    // Save all current parameter values
    auto state = apvts.copyState();
    for (int i = 0; i < state.getNumChildren(); ++i)
    {
        auto param = state.getChild(i);
        auto id    = param.getProperty("id").toString();
        auto value = param.getProperty("value");
        preset.setProperty(id, value, nullptr);
    }

    presetBank.appendChild(preset, nullptr);
    rebuildIndex();
}

void PresetManager::deleteUserPreset(int index)
{
    if (index < 0 || index >= presetBank.getNumChildren())
        return;

    auto child = presetBank.getChild(index);
    auto cat = child.getProperty("category").toString();
    if (cat == "user" || cat == "expansion")
    {
        presetBank.removeChild(index, nullptr);
        rebuildIndex();
        if (currentIndex >= static_cast<int>(presets.size()))
            currentIndex = juce::jmax(0, static_cast<int>(presets.size()) - 1);
    }
}

// ==============================================================================
juce::XmlElement PresetManager::getStateXml() const
{
    return *presetBank.createXml();
}

void PresetManager::setStateXml(const juce::XmlElement& xml)
{
    presetBank = juce::ValueTree::fromXml(xml);
    rebuildIndex();
}

juce::File PresetManager::getPresetFolder(const juce::String& subFolder,
                                            const juce::String& customPath)
{
    juce::File base;
    if (customPath.isNotEmpty())
        base = juce::File(customPath);
    else
        base = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                   .getChildFile("arxybin").getChildFile("Presets");

    if (subFolder.isNotEmpty())
        base = base.getChildFile(subFolder);
    if (!base.exists()) base.createDirectory();
    return base;
}

void PresetManager::scanPresetFolders(juce::AudioProcessorValueTreeState& apvts)
{
    struct Cat { const char* name; const char* folder; };
    Cat cats[] = {{"factory","factory"},{"user","users"},{"expansion","expansions"}};
    for (auto& cat : cats)
    {
        auto dir = getPresetFolder(cat.folder);
        auto files = dir.findChildFiles(juce::File::findFiles, false, "*.arxybin");
        for (auto& f : files)
        {
            // Skip if already loaded (check by name)
            bool exists = false;
            for (int i = 0; i < presetBank.getNumChildren(); ++i)
                if (presetBank.getChild(i).getProperty("name").toString() == f.getFileNameWithoutExtension())
                    { exists = true; break; }
            if (exists) continue;

            juce::XmlDocument doc(f);
            auto root = doc.getDocumentElement();
            if (!root || !root->hasTagName("arxybinPreset")) continue;

            juce::ValueTree p("PRESET");
            p.setProperty("name", root->getStringAttribute("name", f.getFileNameWithoutExtension()), nullptr);
            p.setProperty("category", cat.name, nullptr);
            p.setProperty("author", root->getStringAttribute("author", ""), nullptr);

            if (auto* stateElem = root->getChildByName("State"))
            {
                auto b64 = stateElem->getStringAttribute("data");
                if (b64.isNotEmpty())
                {
                    juce::MemoryBlock mb;
                    if (mb.fromBase64Encoding(b64))
                    {
                        auto xml = juce::AudioProcessor::getXmlFromBinary(mb.getData(), (int)mb.getSize());
                        if (xml && xml->hasTagName(apvts.state.getType()))
                        {
                            auto vt = juce::ValueTree::fromXml(*xml);
                            for (int i = 0; i < vt.getNumChildren(); ++i)
                            {
                                auto c = vt.getChild(i);
                                p.setProperty(c.getProperty("id").toString(), c.getProperty("value"), nullptr);
                            }
                        }
                    }
                }
            }
            presetBank.appendChild(p, nullptr);
        }
    }
    rebuildIndex();
}

bool PresetManager::saveToFile(const juce::File& file, const juce::String& author,
                                juce::AudioProcessorValueTreeState& apvts) const
{
    juce::XmlElement root("arxybinPreset");
    root.setAttribute("name", file.getFileNameWithoutExtension());
    root.setAttribute("author", author);
    root.setAttribute("version", "1.0");
    root.setAttribute("date", juce::Time::getCurrentTime().toISO8601(true));

    // Serialise entire APVTS state as binary (base64) — avoids XML parsing type issues
    juce::MemoryBlock mb;
    auto stateXml = apvts.copyState().createXml();
    if (stateXml) juce::AudioProcessor::copyXmlToBinary(*stateXml, mb);
    root.createNewChildElement("State")->setAttribute("data", mb.toBase64Encoding());

    return root.writeTo(file);
}

bool PresetManager::loadFromFile(const juce::File& file,
                                  juce::AudioProcessorValueTreeState& apvts)
{
    juce::XmlDocument doc(file);
    auto root = doc.getDocumentElement();
    if (!root || !root->hasTagName("arxybinPreset")) return false;

    // Restore from base64-encoded binary — same format as host save/load
    if (auto* stateElem = root->getChildByName("State"))
    {
        auto b64 = stateElem->getStringAttribute("data");
        if (b64.isNotEmpty())
        {
            juce::MemoryBlock mb;
            if (mb.fromBase64Encoding(b64))
            {
                auto stateXml = juce::AudioProcessor::getXmlFromBinary(mb.getData(), (int)mb.getSize());
                if (stateXml && stateXml->hasTagName(apvts.state.getType()))
                    apvts.replaceState(juce::ValueTree::fromXml(*stateXml));
            }
        }
    }

    // Add as user preset (replace existing with same name to avoid duplicates)
    juce::String presetName = root->getStringAttribute("name", file.getFileNameWithoutExtension());
    // Remove existing preset with same name
    for (int i = 0; i < presetBank.getNumChildren(); ++i)
        if (presetBank.getChild(i).getProperty("name").toString() == presetName)
            presetBank.removeChild(i, nullptr);

    juce::ValueTree preset("PRESET");
    preset.setProperty("name", presetName, nullptr);
    preset.setProperty("category", "user", nullptr);
    preset.setProperty("author", root->getStringAttribute("author", ""), nullptr);
    auto state = apvts.copyState();
    for (int i = 0; i < state.getNumChildren(); ++i)
    {
        auto child = state.getChild(i);
        preset.setProperty(child.getProperty("id").toString(), child.getProperty("value"), nullptr);
    }
    int newIdx = presetBank.getNumChildren();
    presetBank.appendChild(preset, nullptr);
    rebuildIndex();
    loadPreset(newIdx);

    return true;
}

} // namespace arxybin
