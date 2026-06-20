/*
  ==============================================================================
    arxybin. — Landscape UI. All float params use HoverDragLabel (no Slider).
    Proven state persistence via direct APVTS write.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class ArxybinAudioProcessorEditor : public juce::GenericAudioProcessorEditor,
                                     public juce::Timer
{
public:
    explicit ArxybinAudioProcessorEditor(ArxybinAudioProcessor& p);
    ~ArxybinAudioProcessorEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void visibilityChanged() override;
    void timerCallback() override;

private:
    // --- Hover-drag knob label (replaces Slider for ALL float params) -----------
    struct KnobLabel : public juce::Label
    {
        KnobLabel(const juce::String& paramID, juce::AudioProcessorValueTreeState& apvts);
        void paint(juce::Graphics& g) override;
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void mouseEnter(const juce::MouseEvent&) override;
        void mouseExit(const juce::MouseEvent&) override;
        void updateValue(float v);
        void setLfoMod(float m) { lfoMod = m; }

        juce::AudioProcessorValueTreeState& apvts;
        juce::String pid;
        float lfoMod = 0.0f;
        float currentValue = 0.0f;
        float dragStartVal  = 0.0f;
        int   dragStartY    = 0;
        bool  hovering      = false;
        float displayValue  = 0.0f;
    };

    // --- MIX label (same as before, with progress bar) -------------------------
    struct MixLabel : public juce::Label
    {
        MixLabel(const juce::String& paramID, juce::AudioProcessorValueTreeState& apvts);
        void paint(juce::Graphics& g) override;
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void mouseEnter(const juce::MouseEvent&) override;
        void mouseExit(const juce::MouseEvent&) override;
        void updateDisplay();
        float getNormValue() const;

        juce::AudioProcessorValueTreeState& apvts;
        juce::String pid;
        float dragStartVal = 0.0f;
        int   dragStartY = 0;
        bool  hovering = false;
    };

    // --- Preset save popup -----------------------------------------------------
    struct PresetSavePopup : public juce::Component
    {
        PresetSavePopup(std::function<void(const juce::String&, const juce::String&)> cb);
        void resized() override;
        void paint(juce::Graphics& g) override;
        juce::TextEditor nameBox, authorBox;
        juce::Label      nameLbl { "Name","Name" }, authorLbl { "Author","Author" };
        juce::TextButton okBtn { "OK" }, cancelBtn { "Cancel" };
        std::function<void(const juce::String&, const juce::String&)> onSave;
    };

    // --- Parameter widget (no Slider, all float params use KnobLabel) ----------
    struct ParamWidget
    {
        juce::String             paramID;
        std::unique_ptr<KnobLabel> knob;    // for float params
        juce::Label              label;
        juce::ComboBox           combo;     // for choice params
        juce::TextButton         button { "X" }; // for bool params
        int style = 0; // 0=knob(float), 2=combo, 3=toggle
    };

    void buildParamControls();
    void applyComboStyle(juce::ComboBox& c);
    void applyToggleStyle(juce::TextButton& b);
    void switchTab(int tab);
    void layoutParamGrid(const std::vector<int>& indices, juce::Rectangle<int> area);
    void showSavePopup();
    void loadPresetFile();
    void syncKnobsFromAPVTS();

    ArxybinAudioProcessor& proc;

    juce::TextButton tabSwitch { "Granulator" };
    int activeTab = 0;

    std::vector<std::unique_ptr<ParamWidget>> widgets;
    std::vector<int> dividerXs, groupStartIndices, groupLabelXs, groupLabelYs;
    juce::StringArray activeGroupNames;

    MixLabel mixDryWet  { "dryWet",    proc.apvts };
    MixLabel mixInput   { "inputGain",  proc.apvts };
    MixLabel mixOutput  { "outputGain", proc.apvts };
    juce::Label mixDryWetLbl, mixInputLbl, mixOutputLbl, mixSectionLbl;

    static constexpr int maxParticles = 50;
    struct Particle { float x,y,vx,vy,size,alpha,rot,rotSpd; int shape; };
    std::vector<Particle> particles;
    float audioLevel = 0.0f;
    void initParticles(); void updateParticles();



    juce::Label titleLabel, subLabel;
    juce::TextButton menuButton { "..." };
    juce::Label presetLabel, footerTag;
    juce::ComboBox   presetBox;
    juce::TextButton saveButton { "Save" }, loadButton { "Load" }, modButton { "MOD" };
    std::unique_ptr<PresetSavePopup> savePopup;
    juce::Component::SafePointer<juce::DialogWindow> modWindow;

    juce::Rectangle<int> waveRect, asciiRect, paramRect, mixRect;

    void showSettingsMenu(); void showModWindow();
    void updatePresetList(); void loadPresetFromBox(); void saveCurrentPreset();

    static constexpr int topBarH = 40, waveH = 170, footerH = 36;
};
