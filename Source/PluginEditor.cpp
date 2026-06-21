/*
  ==============================================================================
    arxybin. — No Slider components. All float params use KnobLabel (direct
    APVTS write, proven state persistence). MIX uses MixLabel with progress bar.
  ==============================================================================
*/

#include "PluginEditor.h"
#include "UI/ModWindow.h"

namespace
{
    const juce::Colour azure   { 0xFF2D5A78 };
    const juce::Colour lightAz { 0xFF7A9DB8 };
    const juce::Colour paleAz  { 0xFFBCCCD8 };
    const juce::Colour whiteBg { 0xFFF8F8FA };
    const juce::Colour darkTx  { 0xFF1A2A35 };
    const juce::Colour dryBlue { 0xFF2D5AB8 };
    const juce::Colour wetGreen{ 0xFF5DA870 };
    const juce::Colour glassBg { 0xFFEEF0F4 };
    const juce::Colour divLine { 0xFFC0C8D4 };
    const juce::Colour barFill { 0xFF2D5A78 };
    const juce::Colour shadowCol { 0x18000000 };
}
static juce::Random& getRng() { static juce::Random rng; return rng; }

// ==============================================================================
// KnobLabel — circular knob drawn on Label, hover+drag to adjust
// ==============================================================================
ArxybinAudioProcessorEditor::KnobLabel::KnobLabel(
    const juce::String& paramID, juce::AudioProcessorValueTreeState& a)
    : apvts(a), pid(paramID)
{
    setInterceptsMouseClicks(true, true);
    if (auto* p = apvts.getParameter(pid))
        currentValue = p->getValue();
    displayValue = currentValue;
}
void ArxybinAudioProcessorEditor::KnobLabel::updateValue(float v)
{
    currentValue = v;
    displayValue = v;
    repaint();
}
void ArxybinAudioProcessorEditor::KnobLabel::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced(2.0f);
    float cx = b.getCentreX(), cy = b.getCentreY();
    float r  = juce::jmin(b.getWidth(), b.getHeight()) * 0.42f;

    // Drop shadow behind the knob
    g.setColour(juce::Colour(0x18000000));
    g.fillEllipse(cx - r + 2.0f, cy - r + 2.0f, r * 2, r * 2);

    // Full outer ring
    g.setColour(paleAz.withAlpha(0.5f));
    g.drawEllipse(cx - r, cy - r, r * 2, r * 2, 2.0f);

    // Filled wedge arc (scribe from center)
    const float startAng = juce::MathConstants<float>::pi * 1.25f;
    const float endAng   = juce::MathConstants<float>::pi * 2.75f;
    const float ang = startAng + displayValue * (endAng - startAng);

    juce::Path arcPath;
    arcPath.addCentredArc(cx, cy, r, r, 0.0f, startAng, ang, true);
    // Glow layer
    g.setColour(azure.withAlpha(0.15f));
    g.strokePath(arcPath, juce::PathStrokeType(7.0f));
    g.setColour(azure.withAlpha(0.30f));
    g.strokePath(arcPath, juce::PathStrokeType(4.5f));
    // Core arc
    g.setColour(azure);
    g.strokePath(arcPath, juce::PathStrokeType(2.8f));

    // Dot: take arc endpoint from path, then move inward to r*0.72
    auto ep = arcPath.getCurrentPosition();
    float dx = cx + (ep.x - cx) * 0.72f;
    float dy = cy + (ep.y - cy) * 0.72f;
    g.setColour(azure);
    g.fillEllipse(dx - 2.0f, dy - 2.0f, 4.0f, 4.0f);
    g.setColour(whiteBg);
    g.fillEllipse(dx - 1.0f, dy - 1.0f, 2.0f, 2.0f);

    // LFO modulation green line — shows modulation around current value
    if (std::abs(lfoMod) > 0.001f)
    {
        const float modulatedVal = juce::jlimit(0.0f, 1.0f, displayValue + lfoMod);
        const float lfoAng = startAng + modulatedVal * (endAng - startAng);
        juce::Path lfoPath;
        lfoPath.addCentredArc(cx, cy, r + 4.0f, r + 4.0f, 0.0f, startAng, lfoAng, true);
        g.setColour(juce::Colour(0xFF4A8C5C).withAlpha(0.85f));
        g.strokePath(lfoPath, juce::PathStrokeType(2.5f));
    }

    // Value text
    g.setColour(darkTx);
    g.setFont(juce::Font { juce::FontOptions {}.withPointHeight(9.0f) });
    juce::String txt;
    if (auto* p = apvts.getParameter(pid))
        txt = p->getCurrentValueAsText();
    g.drawText(txt, b.withY(cy + r + 4.0f).withHeight(14.0f), juce::Justification::centred);

    // Hover highlight
    if (hovering)
    {
        g.setColour(juce::Colour(0x0C000000));
        g.fillEllipse(cx - r - 4.0f, cy - r - 4.0f, (r + 4.0f) * 2, (r + 4.0f) * 2);
    }
}
void ArxybinAudioProcessorEditor::KnobLabel::mouseEnter(const juce::MouseEvent&)
    { hovering = true; repaint(); }
void ArxybinAudioProcessorEditor::KnobLabel::mouseExit(const juce::MouseEvent&)
    { hovering = false; repaint(); }
void ArxybinAudioProcessorEditor::KnobLabel::mouseDown(const juce::MouseEvent& e)
{
    // Double-click: reset to default
    if (e.getNumberOfClicks() == 2)
    {
        if (auto* p = apvts.getParameter(pid))
        {
            float defVal = p->getDefaultValue();
            currentValue = defVal; displayValue = defVal;
            p->setValueNotifyingHost(defVal); repaint();
        }
        return;
    }
    // Alt+click: reset to default
    if (e.mods.isAltDown())
    {
        if (auto* p = apvts.getParameter(pid))
        {
            float defVal = p->getDefaultValue();
            currentValue = defVal; displayValue = defVal;
            p->setValueNotifyingHost(defVal); repaint();
        }
        return;
    }
    dragStartVal = currentValue;
    dragStartY   = e.getScreenY();
    setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
}
void ArxybinAudioProcessorEditor::KnobLabel::mouseDrag(const juce::MouseEvent& e)
{
    float nv = juce::jlimit(0.0f, 1.0f, dragStartVal + (dragStartY - e.getScreenY()) * 0.005f);
    currentValue = nv;
    displayValue = nv;
    if (auto* p = apvts.getParameter(pid))
        p->setValueNotifyingHost(nv);
    repaint();
}
void ArxybinAudioProcessorEditor::KnobLabel::mouseUp(const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown())
    {
        if (auto* p = apvts.getParameter(pid))
        {
            auto* ed = new juce::TextEditor();
            ed->setText(p->getCurrentValueAsText(), juce::dontSendNotification);
            ed->setFont(juce::Font{ juce::FontOptions{}.withPointHeight(11.0f) });
            ed->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xFFF8F8FA));
            ed->setColour(juce::TextEditor::textColourId, juce::Colour(0xFF1A2A35));
            ed->setColour(juce::TextEditor::outlineColourId, juce::Colour(0xFF3A6B8C));
            ed->setSize(70, 22);
            ed->setTopLeftPosition(getScreenX() + getWidth() / 2 - 35, getScreenY() - 28);
            ed->setAlwaysOnTop(true); ed->addToDesktop(0);
            ed->grabKeyboardFocus();
            ed->onReturnKey = [this, ed, p] {
                float val = ed->getText().getFloatValue();
                auto rng = p->getNormalisableRange();
                val = juce::jlimit(rng.start, rng.end, val);
                float norm = p->convertTo0to1(val);
                currentValue = norm; displayValue = norm;
                p->setValueNotifyingHost(norm); repaint(); delete ed;
            };
            ed->onEscapeKey = [ed] { delete ed; };
            ed->onFocusLost = [ed] { delete ed; };
        }
    }
}

// ==============================================================================
// MixLabel — progress bar + text, hover+drag
// ==============================================================================
ArxybinAudioProcessorEditor::MixLabel::MixLabel(
    const juce::String& paramID, juce::AudioProcessorValueTreeState& a)
    : apvts(a), pid(paramID)
{
    setJustificationType(juce::Justification::centred);
    setFont(juce::Font { juce::FontOptions {}.withPointHeight(12.0f).withStyle("Bold") });
    setInterceptsMouseClicks(true, true);
    updateDisplay();
}
float ArxybinAudioProcessorEditor::MixLabel::getNormValue() const
{
    if (auto* p = apvts.getParameter(pid)) return p->getValue();
    return 0.0f;
}
void ArxybinAudioProcessorEditor::MixLabel::updateDisplay()
{
    if (auto* p = apvts.getParameter(pid))
    {
        juce::String t = p->getCurrentValueAsText();
        if (pid == "inputGain" || pid == "outputGain")
            t += " dB";
        else t += "%";
        setText(t, juce::dontSendNotification);
        repaint();
    }
}
void ArxybinAudioProcessorEditor::MixLabel::paint(juce::Graphics& g)
{
    float val = getNormValue();
    auto b = getLocalBounds().toFloat().reduced(2.0f, 4.0f);
    g.setColour(paleAz.withAlpha(0.4f));
    g.fillRoundedRectangle(b, 4.0f);
    if (val > 0.0f)
    {
        g.setColour(barFill.withAlpha(0.5f));
        g.fillRoundedRectangle(b.withWidth(b.getWidth() * val), 4.0f);
    }
    if (hovering)
    {
        g.setColour(juce::Colour(0x0C000000));
        g.fillRoundedRectangle(b, 4.0f);
    }
    g.setColour(darkTx);
    g.setFont(juce::Font { juce::FontOptions {}.withPointHeight(12.0f).withStyle("Bold") });
    g.drawText(getText(), getLocalBounds(), juce::Justification::centred);
}
void ArxybinAudioProcessorEditor::MixLabel::mouseEnter(const juce::MouseEvent&)
    { hovering = true; repaint(); }
void ArxybinAudioProcessorEditor::MixLabel::mouseExit(const juce::MouseEvent&)
    { hovering = false; repaint(); }
void ArxybinAudioProcessorEditor::MixLabel::mouseDown(const juce::MouseEvent& e)
{
    // Double-click: reset to default
    if (e.getNumberOfClicks() == 2)
    {
        if (auto* p = apvts.getParameter(pid))
        {
            p->setValueNotifyingHost(p->getDefaultValue());
            updateDisplay();
        }
        return;
    }
    // Alt+click: reset to default
    if (e.mods.isAltDown())
    {
        if (auto* p = apvts.getParameter(pid))
        {
            p->setValueNotifyingHost(p->getDefaultValue());
            updateDisplay();
        }
        return;
    }
    if (auto* p = apvts.getParameter(pid)) dragStartVal = p->getValue();
    dragStartY = e.getScreenY();
    setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
}
void ArxybinAudioProcessorEditor::MixLabel::mouseDrag(const juce::MouseEvent& e)
{
    if (auto* p = apvts.getParameter(pid))
    {
        float nv = juce::jlimit(0.0f, 1.0f, dragStartVal + (dragStartY - e.getScreenY()) * 0.005f);
        p->setValueNotifyingHost(nv);
        updateDisplay();
    }
}
void ArxybinAudioProcessorEditor::MixLabel::mouseUp(const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown())
    {
        if (auto* p = apvts.getParameter(pid))
        {
            auto* ed = new juce::TextEditor();
            ed->setText(p->getCurrentValueAsText(), juce::dontSendNotification);
            ed->setFont(juce::Font{ juce::FontOptions{}.withPointHeight(11.0f) });
            ed->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xFFF8F8FA));
            ed->setColour(juce::TextEditor::textColourId, juce::Colour(0xFF1A2A35));
            ed->setColour(juce::TextEditor::outlineColourId, juce::Colour(0xFF3A6B8C));
            ed->setSize(70, 22);
            ed->setTopLeftPosition(getScreenX() + getWidth() / 2 - 35, getScreenY() - 28);
            ed->setAlwaysOnTop(true); ed->addToDesktop(0);
            ed->grabKeyboardFocus();
            ed->onReturnKey = [this, ed, p] {
                float val = ed->getText().getFloatValue();
                auto rng = p->getNormalisableRange();
                val = juce::jlimit(rng.start, rng.end, val);
                p->setValueNotifyingHost(p->convertTo0to1(val));
                updateDisplay(); delete ed;
            };
            ed->onEscapeKey = [ed] { delete ed; };
            ed->onFocusLost = [ed] { delete ed; };
        }
    }
}

// ==============================================================================
// PresetSavePopup
// ==============================================================================
ArxybinAudioProcessorEditor::PresetSavePopup::PresetSavePopup(
    std::function<void(const juce::String&, const juce::String&)> cb) : onSave(std::move(cb))
{
    auto tf=juce::Font{juce::FontOptions{}.withPointHeight(11.0f)};
    auto lf=juce::Font{juce::FontOptions{}.withPointHeight(9.0f)};
    nameBox.setFont(tf); nameBox.setText("");
    nameBox.setSelectAllWhenFocused(true);
    nameBox.setColour(juce::TextEditor::backgroundColourId,whiteBg);
    nameBox.setColour(juce::TextEditor::textColourId,darkTx);
    nameBox.setColour(juce::TextEditor::outlineColourId,azure.withAlpha(0.4f));
    nameBox.setColour(juce::TextEditor::focusedOutlineColourId,azure);
    addAndMakeVisible(nameBox);
    authorBox.setFont(tf); authorBox.setText("nilotoo.");
    authorBox.setSelectAllWhenFocused(true);
    authorBox.setColour(juce::TextEditor::backgroundColourId,whiteBg);
    authorBox.setColour(juce::TextEditor::textColourId,darkTx);
    authorBox.setColour(juce::TextEditor::outlineColourId,azure.withAlpha(0.4f));
    authorBox.setColour(juce::TextEditor::focusedOutlineColourId,azure);
    addAndMakeVisible(authorBox);
    nameLbl.setFont(lf); nameLbl.setColour(juce::Label::textColourId,darkTx); addAndMakeVisible(nameLbl);
    authorLbl.setFont(lf); authorLbl.setColour(juce::Label::textColourId,darkTx); addAndMakeVisible(authorLbl);
    okBtn.setColour(juce::TextButton::buttonColourId,azure);
    okBtn.setColour(juce::TextButton::textColourOffId,whiteBg);
    okBtn.onClick=[this]{if(onSave)onSave(nameBox.getText().trim(),authorBox.getText().trim());
        if(auto* p=getParentComponent())p->removeChildComponent(this);};
    addAndMakeVisible(okBtn);
    cancelBtn.setColour(juce::TextButton::buttonColourId,juce::Colours::white.withAlpha(0.6f));
    cancelBtn.setColour(juce::TextButton::textColourOffId,darkTx);
    cancelBtn.onClick=[this]{if(auto* p=getParentComponent())p->removeChildComponent(this);};
    addAndMakeVisible(cancelBtn);
    setSize(240, 130);
}
void ArxybinAudioProcessorEditor::PresetSavePopup::resized()
{
    nameLbl.setBounds(4,6,50,18);  nameBox.setBounds(56,4,174,24);
    authorLbl.setBounds(4,34,50,18); authorBox.setBounds(56,32,174,24);
    okBtn.setBounds(4,66,112,28); cancelBtn.setBounds(120,66,112,28);
}
void ArxybinAudioProcessorEditor::PresetSavePopup::paint(juce::Graphics& g)
{
    g.setColour(whiteBg); g.fillRoundedRectangle(getLocalBounds().toFloat(),6);
    g.setColour(azure.withAlpha(0.3f)); g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f),6,1);
}

// ==============================================================================
void ArxybinAudioProcessorEditor::applyComboStyle(juce::ComboBox& c)
{
    c.setColour(juce::ComboBox::backgroundColourId, juce::Colours::transparentWhite);
    c.setColour(juce::ComboBox::outlineColourId, azure.withAlpha(0.35f));
    c.setColour(juce::ComboBox::textColourId, darkTx);
    c.setColour(juce::ComboBox::arrowColourId, azure);
    c.setScrollWheelEnabled(false);
}
void ArxybinAudioProcessorEditor::applyToggleStyle(juce::TextButton& b)
{
    b.setColour(juce::TextButton::buttonColourId, juce::Colours::white.withAlpha(0.5f));
    b.setColour(juce::TextButton::textColourOffId, darkTx);
    b.setColour(juce::TextButton::buttonOnColourId, azure);
    b.setColour(juce::TextButton::textColourOnId, whiteBg);
    b.setClickingTogglesState(true); b.setTriggeredOnMouseDown(true);
}

// ==============================================================================
ArxybinAudioProcessorEditor::ArxybinAudioProcessorEditor(ArxybinAudioProcessor& p)
    : juce::GenericAudioProcessorEditor(p), proc(p)
{
    for (auto* c : getChildren())
        if (dynamic_cast<juce::TreeView*>(c)) c->setVisible(false);

    const auto uF = juce::Font { juce::FontOptions {}.withPointHeight(10.5f) };
    const auto bF = juce::Font { juce::FontOptions {}.withPointHeight(10.5f).withStyle("Bold") };
    const auto tF = juce::Font { juce::FontOptions {}.withPointHeight(9.0f) };

    titleLabel.setText("arxybin.", juce::dontSendNotification);
    titleLabel.setFont(juce::Font { juce::FontOptions {}.withPointHeight(20.0f).withStyle("Bold") });
    titleLabel.setColour(juce::Label::textColourId, azure);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);

    menuButton.setColour(juce::TextButton::buttonColourId, juce::Colours::white.withAlpha(0.55f));
    menuButton.setColour(juce::TextButton::textColourOffId, darkTx);
    menuButton.setButtonText("..."); menuButton.onClick = [this] { showSettingsMenu(); };
    addAndMakeVisible(menuButton);

    subLabel.setText("glitch . particle . granular    ::    nilotoo.", juce::dontSendNotification);
    subLabel.setFont(uF); subLabel.setColour(juce::Label::textColourId, azure.withAlpha(0.6f));
    subLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(subLabel);

    tabSwitch.setColour(juce::TextButton::buttonColourId, juce::Colours::white.withAlpha(0.55f));
    tabSwitch.setColour(juce::TextButton::textColourOffId, darkTx);
    tabSwitch.setButtonText("Granulator");
    tabSwitch.onClick = [this] { switchTab(1 - activeTab); };
    addAndMakeVisible(tabSwitch);

    buildParamControls();

    auto setupMix = [&](MixLabel& lbl, juce::Label& cap, const juce::String& txt) {
        addAndMakeVisible(lbl);
        lbl.setColour(juce::Label::backgroundColourId, juce::Colours::transparentWhite);
        lbl.setColour(juce::Label::outlineColourId, azure.withAlpha(0.25f));
        addAndMakeVisible(cap); cap.setText(txt, juce::dontSendNotification);
        cap.setFont(tF); cap.setColour(juce::Label::textColourId, darkTx.withAlpha(0.65f));
        cap.setJustificationType(juce::Justification::centred);
    };
    setupMix(mixDryWet, mixDryWetLbl, "Dry/Wet");
    setupMix(mixInput,  mixInputLbl,  "Input");
    setupMix(mixOutput, mixOutputLbl, "Output");

    addAndMakeVisible(mixSectionLbl);
    mixSectionLbl.setText("MIX", juce::dontSendNotification);
    mixSectionLbl.setFont(bF); mixSectionLbl.setColour(juce::Label::textColourId, azure);
    mixSectionLbl.setJustificationType(juce::Justification::centredLeft);

    presetLabel.setText("particle.granular", juce::dontSendNotification);
    presetLabel.setFont(bF); presetLabel.setColour(juce::Label::textColourId, azure);
    presetLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(presetLabel);

    applyComboStyle(presetBox); presetBox.onChange = [this] { loadPresetFromBox(); };
    addAndMakeVisible(presetBox);

    saveButton.setColour(juce::TextButton::buttonColourId, juce::Colours::white.withAlpha(0.55f));
    saveButton.setColour(juce::TextButton::textColourOffId, darkTx);
    saveButton.onClick = [this] { showSavePopup(); };
    addAndMakeVisible(saveButton);

    loadButton.setColour(juce::TextButton::buttonColourId, juce::Colours::white.withAlpha(0.55f));
    loadButton.setColour(juce::TextButton::textColourOffId, darkTx);
    loadButton.onClick = [this] { loadPresetFile(); };
    addAndMakeVisible(loadButton);

    modButton.setColour(juce::TextButton::buttonColourId, juce::Colours::white.withAlpha(0.55f));
    modButton.setColour(juce::TextButton::textColourOffId, darkTx);
    modButton.onClick = [this] { showModWindow(); };
    addAndMakeVisible(modButton);
    addAndMakeVisible(ampFader);

    footerTag.setText("glitch.digital", juce::dontSendNotification);
    footerTag.setFont(bF); footerTag.setColour(juce::Label::textColourId, azure);
    footerTag.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(footerTag);

    initParticles();

    updatePresetList();
    switchTab(0);

    if (proc.selfBackup.getNumChildren() > 0)
    {
        proc.apvts.replaceState(proc.selfBackup);
        proc.selfBackup = juce::ValueTree("SELF_BACKUP");
    }

    setResizable(true, true);
    setResizeLimits(800, 686, 1400, 1000);
    setSize(800, 686);
}
ArxybinAudioProcessorEditor::~ArxybinAudioProcessorEditor()
{
    proc.selfBackup = proc.apvts.copyState();
    stopTimer();
    modWindow.deleteAndZero();
}

// ==============================================================================
void ArxybinAudioProcessorEditor::syncKnobsFromAPVTS()
{
    for (auto& w : widgets)
        if (w->style == 0 && w->knob)
            if (auto* p = proc.apvts.getParameter(w->paramID))
                w->knob->updateValue(p->getValue());
}

void ArxybinAudioProcessorEditor::buildParamControls()
{
    auto& apv = proc.apvts;
    const auto tF = juce::Font { juce::FontOptions {}.withPointHeight(9.0f) };

    struct PD { juce::String id,name; int style,grp; };
    const std::vector<PD> defs = {
        {"grainSize","Sz",0,0},{"grainDensity","Dens",0,0},
        {"grainPitch","Pitch",0,0},{"pitchRandom","PtRnd",0,0},
        {"panSpread","Pan",0,0},{"reverseProb","Rev",0,0},
        {"grainFeedback","Fbk",0,0},
        {"grainRandom","SzRnd",3,0},{"panRandom","PanRnd",3,0},
        {"scanMode","Scan",2,1},{"scanSpeed","ScnSpd",0,1},
        {"grainPosition","Pos",0,1},{"positionRandom","Spray",0,1},
        {"placeOffset","Ofs",0,2},{"placeGrid","Grid",2,2},
        {"placeRate","Rate",0,2},{"placeSnap","Snap",3,2},{"placeRandom","Rnd",0,2},
        {"distType","Dist",2,3},{"distDrive","Drive",0,3},
        {"distRandom","Rnd",3,3},{"distMix","Mix",0,3},
        {"bitDepth","Bits",0,4},{"sampleRateRedux","Rate",2,4},
        {"bitRandom","Rnd",3,4},{"rateRandom","Rnd",3,4},
        {"bitMix","Mix",0,4},
        {"stutterProb","Stut%",0,5},{"stutterLength","Len",0,5},
        {"stutProbRandom","Rnd",3,5},{"stutLenRandom","Rnd",3,5},
        {"stutterMix","Mix",0,5},
        {"shuffleAmount","ShfAmt",0,6},{"shuffleSegment","ShfSz",0,6},
        {"shufAmtRandom","Rnd",3,6},{"shufSizeRandom","Rnd",3,6},
        {"shuffleMix","Mix",0,6},
    };

    int lastGrp=-1;
    for(int i=0;i<(int)defs.size();++i)
        if(defs[i].grp!=lastGrp){groupStartIndices.push_back(i);lastGrp=defs[i].grp;}
    groupStartIndices.push_back((int)defs.size());
    widgets.reserve(defs.size());

    for(const auto& d : defs)
    {
        auto w = std::make_unique<ParamWidget>();
        w->paramID = d.id; w->style = d.style;
        addAndMakeVisible(w->label);
        w->label.setText(d.name, juce::dontSendNotification);
        w->label.setFont(tF); w->label.setColour(juce::Label::textColourId, darkTx);
        w->label.setJustificationType(juce::Justification::centred);

        if(d.style == 2)
        {
            addAndMakeVisible(w->combo); applyComboStyle(w->combo);
            if(auto* p=apv.getParameter(d.id))
                if(auto* ch=dynamic_cast<juce::AudioParameterChoice*>(p))
                {
                    w->combo.addItemList(ch->getAllValueStrings(),1);
                    w->combo.setSelectedItemIndex(ch->getIndex(),juce::dontSendNotification);
                    w->combo.onChange=[this,id=d.id,&c=w->combo]{
                        if(auto* pp=proc.apvts.getParameter(id))
                            pp->setValueNotifyingHost((float)c.getSelectedItemIndex()
                                /juce::jmax(1.0f,(float)(c.getNumItems()-1)));
                    };
                }
        }
        else if(d.style == 3)
        {
            addAndMakeVisible(w->button); applyToggleStyle(w->button);
            w->button.setButtonText("OFF");
            if(auto* p=apv.getParameter(d.id))
            {
                bool iv=p->getValue()>=0.5f;
                w->button.setToggleState(iv,juce::dontSendNotification);
                w->button.setButtonText(iv?"ON":"OFF");
                w->button.onClick=[this,id=d.id,&b=w->button]{
                    bool on=b.getToggleState();b.setButtonText(on?"ON":"OFF");
                    if(auto* pp=proc.apvts.getParameter(id))
                        pp->setValueNotifyingHost(on?1.0f:0.0f);
                };
            }
        }
        else
        {
            w->knob = std::make_unique<KnobLabel>(d.id, proc.apvts);
            addAndMakeVisible(w->knob.get());
        }
        widgets.push_back(std::move(w));
    }
}

// ==============================================================================
void ArxybinAudioProcessorEditor::switchTab(int tab)
{
    activeTab = tab;
    tabSwitch.setButtonText(tab==0?"Granulator":"Glitch");
    // Clear old group visuals immediately
    dividerXs.clear(); groupLabelXs.clear(); groupLabelYs.clear(); activeGroupNames.clear();
    const int ge=18;
    for(int i=0;i<(int)widgets.size();++i)
    {
        bool v=(tab==0)?(i<ge):(i>=ge);
        auto& w=widgets[(size_t)i];
        w->label.setVisible(v);
        if(w->style==2)w->combo.setVisible(v);
        else if(w->style==3)w->button.setVisible(v);
        else if(w->knob)w->knob->setVisible(v);
    }
    resized();
    repaint(paramRect);
}

// ==============================================================================
void ArxybinAudioProcessorEditor::resized()
{
    juce::GenericAudioProcessorEditor::resized();
    const int w=getWidth(),h=getHeight();

    for(auto* c:getChildren())
        if(dynamic_cast<juce::TreeView*>(c))c->setVisible(false);

    titleLabel.setBounds(12,4,200,24); menuButton.setBounds(w-48,4,40,22);
    subLabel.setBounds(14,24,w-60,14);

    // Proportional heights
    const int wavH = (int)(h * 0.233f);
    const int ascH = (int)(h * 0.019f);
    const int footH= (int)(h * 0.049f);
    const int mixHH= (int)(h * 0.052f);
    const int tabH = (int)(h * 0.041f);

    waveRect={22,(int)(h*0.066f),w-28,wavH};
    ampFader.setBounds({6,waveRect.getY()+8,14,waveRect.getHeight()-16});
    asciiRect={0,waveRect.getBottom()+2,w,ascH};

    const int paramTop=asciiRect.getBottom()+2,footerY=h-footH;
    paramRect={6,paramTop,w-12,footerY-paramTop-4};

    const int mixY=paramRect.getBottom()-mixHH;
    mixRect={paramRect.getX()+4,mixY,paramRect.getWidth()-8,mixHH};
    mixSectionLbl.setBounds(mixRect.getX(),mixY+14,28,14);
    const int mx=mixRect.getX()+30,mw=(mixRect.getWidth()-30)/3;
    mixDryWetLbl.setBounds(mx,mixY,mw,12);    mixDryWet.setBounds(mx,mixY+13,mw,22);
    mixInputLbl.setBounds(mx+mw,mixY,mw,12);   mixInput.setBounds(mx+mw,mixY+13,mw,22);
    mixOutputLbl.setBounds(mx+mw*2,mixY,mw,12);mixOutput.setBounds(mx+mw*2,mixY+13,mw,22);

    const int tabBtnY=mixY-tabH-4;
    tabSwitch.setBounds(paramRect.getX()+4,tabBtnY,paramRect.getWidth()-8,tabH);

    const int gridX=paramRect.getX()+4,gridY=paramRect.getY()+4;
    const int gridW=paramRect.getWidth()-8,gridH=tabBtnY-gridY-4;

    std::vector<int> ai; const int ge=18;
    for(int i=0;i<(int)widgets.size();++i)
        if((activeTab==0&&i<ge)||(activeTab==1&&i>=ge))ai.push_back(i);
    layoutParamGrid(ai,{gridX,gridY,gridW,gridH});

    presetLabel.setBounds(6,footerY+2,130,18); presetBox.setBounds(140,footerY+1,130,22);
    saveButton.setBounds(274,footerY+1,44,22); loadButton.setBounds(322,footerY+1,44,22);
    modButton.setBounds(w-56,footerY+1,48,22);
    footerTag.setBounds(374,footerY+2,w-438,18);
}

void ArxybinAudioProcessorEditor::layoutParamGrid(const std::vector<int>& indices,
                                                    juce::Rectangle<int> area)
{
    dividerXs.clear(); groupLabelXs.clear(); groupLabelYs.clear(); activeGroupNames.clear();
    if(indices.empty())return;

    static const char* grpNamesGran[]={"grains.","scan.","placement."};
    static const char* grpNamesGlitch[]={"distortion.","bitcrusher.","stutter.","shuffler."};

    // Collect groups from indices
    struct G{int s,e,gn;};
    std::vector<G> grps;
    int cg=-1,cs=0;
    for(int idx=0;idx<(int)indices.size();++idx){
        int wi=indices[(size_t)idx],g=0;
        for(size_t gi=1;gi<groupStartIndices.size();++gi)
            if(wi>=groupStartIndices[gi-1]&&wi<groupStartIndices[gi]){g=(int)(gi-1);break;}
        if(g!=cg){if(cg>=0)grps.push_back({cs,idx,cg});cg=g;cs=idx;}
    }
    if(cg>=0)grps.push_back({cs,(int)indices.size(),cg});

    const int maxC=8,cellW=area.getWidth()/maxC,rowH=74,lblH=14,gap=2;
    int y=area.getY();

    for(size_t gi=0;gi<grps.size();++gi){
        auto& grp=grps[gi];
        int cnt=grp.e-grp.s,cols=juce::jmin(cnt,maxC),rows=(cnt+cols-1)/cols;

        // Label (no background)
        const char* nm=nullptr;
        int gn=grp.gn;
        if(activeTab==0&&gn>=0&&gn<3)nm=grpNamesGran[gn];
        else if(activeTab==1&&gn>=3&&gn<=6)nm=grpNamesGlitch[gn-3];
        if(nm){
            groupLabelXs.push_back(area.getX()+4);
            groupLabelYs.push_back(y);
            activeGroupNames.add(nm);
            y+=lblH-2;
        }

        // Widgets
        for(int i=0;i<cnt;++i){
            int wi=indices[(size_t)(grp.s+i)];
            int col=i%cols, cx=area.getX()+col*cellW, cy=y+(i/cols)*rowH;
            auto& w=widgets[(size_t)wi];
            w->label.setBounds(cx,cy,cellW,9);
            if(w->style==2)w->combo.setBounds(cx+4,cy+10,cellW-8,22);
            else if(w->style==3)w->button.setBounds(cx+cellW/2-22,cy+10,44,22);
            else if(w->knob)w->knob->setBounds(cx+2,cy+8,cellW-4,rowH-10);
            else if(w->knob) w->knob->setBounds(cx+2,cy+8,cellW-4,rowH-10);
        }
        y+=rows*rowH+gap;

        // Divider between groups
        if(gi+1<grps.size()) dividerXs.push_back(y);
    }
}

// ==============================================================================
void ArxybinAudioProcessorEditor::showSavePopup()
{
    if(savePopup&&savePopup->isVisible()){savePopup.reset();return;}
    auto& pm=proc.getPresetManager();
    savePopup=std::make_unique<PresetSavePopup>([this](const juce::String& name, const juce::String& author){
        if(name.isNotEmpty()){
            auto file=arxybin::PresetManager::getPresetFolder("users",proc.presetFolderPath)
                          .getChildFile(name+".arxybin");
            proc.getPresetManager().saveToFile(file,author,proc.apvts);
            proc.getPresetManager().saveUserPreset(name,author);
            updatePresetList();
            savePopup.reset();
        }
    });
    auto sb=saveButton.getScreenBounds();
    auto* top=getTopLevelComponent();
    auto tl=top->getLocalPoint(&saveButton,juce::Point<int>(0,0));
    savePopup->setTopLeftPosition(tl.x-80,tl.y-150);
    top->addAndMakeVisible(savePopup.get());
    savePopup->nameBox.grabKeyboardFocus();
}
void ArxybinAudioProcessorEditor::loadPresetFile()
{
    auto dir=arxybin::PresetManager::getPresetFolder("users",proc.presetFolderPath);
    auto* chooser=new juce::FileChooser("Load Preset",dir,"*.arxybin");
    chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,
        [this,chooser](const juce::FileChooser&){
            auto file=chooser->getResult();
            if(file!=juce::File()&&proc.getPresetManager().loadFromFile(file,proc.apvts)){
                updatePresetList();
                // Full sync: knobs, combos, toggles, MIX
                for(auto& w:widgets){
                    if(auto* p=proc.apvts.getParameter(w->paramID)){
                        if(w->style==2){
                            if(auto* c=dynamic_cast<juce::AudioParameterChoice*>(p))
                                w->combo.setSelectedItemIndex(c->getIndex(),juce::dontSendNotification);
                        }else if(w->style==3){
                            bool on=p->getValue()>=0.5f;
                            w->button.setToggleState(on,juce::dontSendNotification);
                            w->button.setButtonText(on?"ON":"OFF");
                        }else if(w->knob){
                            w->knob->updateValue(p->getValue());
                        }
                    }
                }
                mixDryWet.updateDisplay();mixInput.updateDisplay();mixOutput.updateDisplay();
            }
            delete chooser;
        });
}
void ArxybinAudioProcessorEditor::saveCurrentPreset(){showSavePopup();}

// ==============================================================================
void ArxybinAudioProcessorEditor::visibilityChanged()
{
    if(isVisible()&&!isTimerRunning())
    {
        syncKnobsFromAPVTS();
        mixDryWet.updateDisplay();mixInput.updateDisplay();mixOutput.updateDisplay();
        startTimerHz(40);
    }
    else if(!isVisible()&&isTimerRunning())stopTimer();
}
void ArxybinAudioProcessorEditor::timerCallback()
{
    static int tick=0;
    if(++tick>=80){tick=0;proc.selfBackup=proc.apvts.copyState();}

    // Push LFO mod values to knobs (non-zero mod = green line appears)
    {
        static int lfoTick=0;
        if(++lfoTick>=3){lfoTick=0;
            for(auto& w:widgets){
                if(w->style==0&&w->knob){
                    float mod=0, scale=1.0f;
                    if(w->paramID=="grainPitch")    {mod=proc.lfoModDisplay[0]; scale=24.0f;}
                    if(w->paramID=="grainPosition") {mod=proc.lfoModDisplay[1]; scale=0.5f;}
                    if(w->paramID=="panSpread")     {mod=proc.lfoModDisplay[2]; scale=1.0f;}
                    if(w->paramID=="grainSize")     {mod=proc.lfoModDisplay[3]; scale=0.8f;}
                    if(w->paramID=="grainDensity")  {mod=proc.lfoModDisplay[4]; scale=0.5f;}
                    if(w->paramID=="distDrive")     {mod=proc.lfoModDisplay[5]; scale=1.0f;}
                    if(w->paramID=="bitDepth")      {mod=proc.lfoModDisplay[6]; scale=1.0f;}
                    if(w->paramID=="stutterProb")   {mod=proc.lfoModDisplay[7]; scale=1.0f;}
                    if(w->paramID=="stutterLength") {mod=proc.lfoModDisplay[8]; scale=1.0f;}
                    if(w->paramID=="shuffleAmount") {mod=proc.lfoModDisplay[9]; scale=1.0f;}
                    if(w->paramID=="shuffleSegment"){mod=proc.lfoModDisplay[10];scale=1.0f;}
                    w->knob->setLfoMod(mod / juce::jmax(0.01f, scale));
                    w->knob->repaint();
                }
            }
        }
    }

    // Audio level for particles
    const auto *wv = proc.getWetWave(); int n = proc.getWaveSize();
    float wp = 0;
    for (int i = 0; i < n; ++i) wp = juce::jmax(wp, std::abs(wv[i]));
    audioLevel = audioLevel * 0.92f + wp * 0.08f;
    audioLevel = juce::jlimit(0.0f, 1.0f, audioLevel);

    updateParticles(); repaint(waveRect); repaint(asciiRect);
}
void ArxybinAudioProcessorEditor::initParticles()
{
    particles.resize(maxParticles);
    for(auto& p:particles){
        p.x=getRng().nextFloat()*960;p.y=getRng().nextFloat()*600;
        p.vx=(getRng().nextFloat()-0.5f)*0.4f;p.vy=(getRng().nextFloat()-0.5f)*0.25f-0.08f;
        p.size=getRng().nextFloat()*5+2;p.alpha=getRng().nextFloat()*0.18f+0.03f;
        p.shape=getRng().nextInt(3);p.rot=getRng().nextFloat()*juce::MathConstants<float>::twoPi;
        p.rotSpd=(getRng().nextFloat()-0.5f)*0.015f;
    }
}
void ArxybinAudioProcessorEditor::updateParticles()
{
    float W=(float)getWidth(),H=(float)getHeight(),sb=1+audioLevel*1.8f;
    for(auto& p:particles){p.x+=p.vx*sb;p.y+=p.vy*sb;p.rot+=p.rotSpd*sb;
        if(p.x<-10)p.x=W+10;if(p.x>W+10)p.x=-10;if(p.y<-10)p.y=H+10;if(p.y>H+10)p.y=-10;}
}
void ArxybinAudioProcessorEditor::updatePresetList()
{
    presetBox.clear(juce::dontSendNotification);
    auto& pm=proc.getPresetManager();const auto& ps=pm.getPresets();
    int id=1;juce::String lc;
    for(auto& i:ps){if(i.category!=lc){if(id>1)presetBox.addSeparator();
        presetBox.addSectionHeading(i.category=="factory"?"== Factory ==":"== User ==");lc=i.category;}
        juce::String displayName=i.name;
        if(i.author.isNotEmpty())displayName+="  -  "+i.author;
        presetBox.addItem(displayName,id++);}
    if(!ps.empty())presetBox.setSelectedId(pm.getCurrentIndex()+1,juce::dontSendNotification);
}
void ArxybinAudioProcessorEditor::loadPresetFromBox()
{
    int idx=presetBox.getSelectedId()-1;
    if(idx>=0){proc.getPresetManager().loadPreset(idx);
        presetLabel.setText(proc.getPresetManager().getCurrentName(),juce::dontSendNotification);
        syncKnobsFromAPVTS();
        mixDryWet.updateDisplay();mixInput.updateDisplay();mixOutput.updateDisplay();
        for(auto& w:widgets){
            if(w->style==2)
                if(auto* p=dynamic_cast<juce::AudioParameterChoice*>(proc.apvts.getParameter(w->paramID)))
                    w->combo.setSelectedItemIndex(p->getIndex(),juce::dontSendNotification);
            if(w->style==3)
                if(auto* p=proc.apvts.getParameter(w->paramID))
                    {bool on=p->getValue()>=0.5f;w->button.setToggleState(on,juce::dontSendNotification);w->button.setButtonText(on?"ON":"OFF");}
        }
    }
}
void ArxybinAudioProcessorEditor::showModWindow()
{
    if(modWindow!=nullptr){modWindow.deleteAndZero();return;}
    modWindow=arxybin::createModWindow(proc.apvts);
    if(auto* mw=modWindow.getComponent()){mw->setAlwaysOnTop(true);
        auto b=modButton.getScreenBounds();mw->setTopLeftPosition(b.getX()-300,b.getY()-320);}
}
void ArxybinAudioProcessorEditor::showSettingsMenu()
{
    juce::PopupMenu m;m.addSectionHeader("arxybin.  v1.0.0");m.addSeparator();
    m.addItem("Creator : nilotoo.",false,false,nullptr);
    m.addItem("sound designer / sound artist",false,false,nullptr);m.addSeparator();
    m.addItem("Granular + Glitch Effect",false,false,nullptr);
    m.addItem("VST3 + Standalone  |  Windows x64  |  JUCE",false,false,nullptr);m.addSeparator();
    m.addItem("Set Preset Path...",[this]{
        auto* chooser=new juce::FileChooser("Choose Preset Folder",
            arxybin::PresetManager::getPresetFolder(proc.presetFolderPath));
        chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectDirectories,
            [this,chooser](const juce::FileChooser&){
                auto r=chooser->getResult();
                if(r!=juce::File()){proc.presetFolderPath=r.getFullPathName();}
                delete chooser;
            });
    });
    m.addSeparator();
    m.addSectionHeader("Capture Buffer");
    m.addItem("Set Buffer (ms)...", [this]{
        auto* dw = new juce::DialogWindow("Capture Buffer Size", juce::Colour(0xFFD8DCE4), true, true);
        auto* ed = new juce::TextEditor();
        ed->setText(juce::String(proc.apvts.getRawParameterValue("captureBufferMs")->load(), 0));
        ed->setFont(juce::Font{ juce::FontOptions{}.withPointHeight(14.0f) });
        ed->setSize(150, 28);
        ed->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xFFFFFFFF));
        ed->setColour(juce::TextEditor::textColourId, juce::Colour(0xFF1A2A35));
        ed->setColour(juce::TextEditor::outlineColourId, juce::Colour(0xFF3A6B8C));
        dw->setContentOwned(ed, true);
        dw->setSize(200, 70);
        dw->centreAroundComponent(&menuButton, dw->getWidth(), dw->getHeight());
        dw->setVisible(true);
        ed->grabKeyboardFocus();
        auto apply = [this, ed, dw] {
            float val = ed->getText().getFloatValue();
            val = juce::jlimit(20.0f, 30000.0f, val);
            if (auto* p = proc.apvts.getParameter("captureBufferMs"))
                p->setValueNotifyingHost(p->convertTo0to1(val));
            dw->exitModalState(0);
        };
        ed->onReturnKey = apply;
        ed->onEscapeKey = [dw] { dw->exitModalState(0); };
        dw->enterModalState(true, juce::ModalCallbackFunction::create([dw](int) { delete dw; }), true);
    });
    bool bpmOn = proc.apvts.getRawParameterValue("bpmSync")->load() > 0.5f;
    m.addItem("BPM Sync: " + juce::String(bpmOn ? "ON" : "OFF"), true, bpmOn, [this, bpmOn]{
        if (auto* p = proc.apvts.getParameter("bpmSync"))
            p->setValueNotifyingHost(bpmOn ? 0.0f : 1.0f);
    });
    m.addSeparator();
    m.addItem("Close",true,false,nullptr);
    m.showMenuAsync(juce::PopupMenu::Options{}.withTargetComponent(&menuButton));
}

// ==============================================================================
void ArxybinAudioProcessorEditor::paint(juce::Graphics& g)
{
    const int w=getWidth();
    g.fillAll(whiteBg);
    for(const auto& p:particles){
        float a=p.alpha*(0.3f+audioLevel*0.5f);g.setColour(azure.withAlpha(a));
        float cx=p.x,cy=p.y,s=p.size;
        switch(p.shape){case 0:g.fillEllipse(cx-s,cy-s,s*2,s*2);break;
        case 1:{juce::Path t;t.addTriangle(cx,cy-s,cx-s*0.866f,cy+s*0.5f,cx+s*0.866f,cy+s*0.5f);
            t.applyTransform(juce::AffineTransform::rotation(p.rot,cx,cy));g.fillPath(t);break;}
        case 2:{juce::Path d;d.addQuadrilateral(cx,cy-s,cx+s,cy,cx,cy+s,cx-s,cy);
            d.applyTransform(juce::AffineTransform::rotation(p.rot,cx,cy));g.fillPath(d);break;}}
    }
    g.setColour(glassBg);g.fillRect(0,0,w,topBarH);
    g.setColour(divLine);g.drawHorizontalLine(topBarH,0,w);
    g.setColour(whiteBg);g.drawHorizontalLine(topBarH+1,0,w);

    // Waveform panel — soft feathered shadow (arc-fade)
    g.setColour(juce::Colour(0x0C000000)); g.fillRoundedRectangle(waveRect.toFloat().translated(2,3),10);
    g.setColour(juce::Colour(0x06000000)); g.fillRoundedRectangle(waveRect.toFloat().translated(4,6),10);
    g.setColour(juce::Colour(0x03000000)); g.fillRoundedRectangle(waveRect.toFloat().translated(7,10),10);
    g.setColour(juce::Colour(0x01000000)); g.fillRoundedRectangle(waveRect.toFloat().translated(11,15),10);
    g.setColour(glassBg);g.fillRoundedRectangle(waveRect.toFloat(),10);
    g.setColour(whiteBg.brighter(0.03f)); g.drawRoundedRectangle(waveRect.toFloat().reduced(1),10,1.5f);
    g.setColour(juce::Colour(0x10000000)); g.drawRoundedRectangle(waveRect.toFloat().translated(0,1),10,2.5f);
    g.setColour(azure.withAlpha(0.30f)); g.drawRoundedRectangle(waveRect.toFloat().reduced(0.5f),10,1.2f);
    float wfW=(float)(waveRect.getWidth()-24),wfH=(float)(waveRect.getHeight()-24);
    float wfX=(float)(waveRect.getX()+12),wfY=(float)(waveRect.getY()+12);

    // --- Buffer oscilloscope ---
    const int ringSz = proc.getRingSize();
    const int ringPos = proc.getRingPos();
    float scanPhase = proc.getScanPhase();
    if (scanPhase < 0.0f) scanPhase += std::floor(-scanPhase) + 1.0f;
    if (scanPhase >= 1.0f) scanPhase -= std::floor(scanPhase);

    if (ringSz > 0)
    {
        // Buffer waveform — engine ring buffer, oldest at left, newest at right
        const float* ringData = proc.getRingSnapshot();
        const int nPts = proc.getWaveSize();
        juce::Path rp; bool rf = true;
        for (int i = 0; i < nPts; ++i)
        {
            float frac = (float)i / (float)(nPts - 1);
            float x = wfX + wfW * (1.0f - frac);  // reversed: i=0 (newest) → right
            float y = wfY + wfH * 0.5f - ringData[i] * wfH * ampFader.value;
            if (rf) { rp.startNewSubPath(x, y); rf = false; }
            else rp.lineTo(x, y);
        }
        g.setColour(juce::Colour(0xCC7A9DB8));
        g.strokePath(rp, juce::PathStrokeType(1.8f));

        // Sweep cursor — white line at write head (newest data at right)
        float sweepFrac = (float)ringPos / (float)ringSz;
        float sweepX = wfX + wfW * (1.0f - sweepFrac);
        g.setColour(juce::Colour(0x88FFFFFF));
        g.fillRect(sweepX - 0.5f, wfY, 1.5f, wfH);

        // Position — blue line
        float posVal = proc.apvts.getRawParameterValue("grainPosition")->load() * 0.01f;
        float px = wfX + wfW * (1.0f - posVal);
        g.setColour(dryBlue.withAlpha(0.65f));
        g.fillRect(px - 1.5f, wfY, 3.0f, wfH);

        // Active grain read lines — green (reversed)
        float grainPos[256];
        int nGrains = proc.getActiveGrainPositions(grainPos, 256);
        for (int i = 0; i < nGrains; ++i)
        {
            float gx = wfX + wfW * (1.0f - grainPos[i] / (float)ringSz);
            g.setColour(wetGreen.withAlpha(0.7f));
            g.fillRect(gx - 0.5f, wfY + wfH * 0.15f, 1.0f, wfH * 0.7f);
        }
    }

    // Labels
    g.setColour(divLine.withAlpha(0.6f));g.drawRoundedRectangle(waveRect.toFloat().reduced(0.5f),10,1.3f);
    g.setFont(juce::Font{ juce::FontOptions{}.withPointHeight(8.0f) });
    g.setColour(paleAz.withAlpha(0.75f));
    float capMs = proc.apvts.getRawParameterValue("captureBufferMs")->load();
    g.drawText("cap: " + juce::String(capMs, 0) + "ms  buf:" + juce::String(proc.getBlockSize()),
        waveRect.getX() + 10, waveRect.getY() + 4, waveRect.getWidth() - 20, 12, juce::Justification::left);
    g.setColour(dryBlue.withAlpha(0.75f));
    g.drawText("pos", waveRect.getRight() - 40, waveRect.getY() + 4, 30, 12, juce::Justification::right);

    g.setColour(paleAz.withAlpha(0.5f));
    g.setFont(juce::Font{juce::FontOptions{}.withPointHeight(9)});
    g.drawText(".  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  ",asciiRect,juce::Justification::centred);

    g.setColour(divLine.withAlpha(0.5f));g.drawHorizontalLine(asciiRect.getY(),20,w-20);

    // Amp fader is drawn by AmpFader component

    // Parameter area — soft feathered shadow
    g.setColour(juce::Colour(0x0C000000)); g.fillRoundedRectangle(paramRect.toFloat().reduced(0,2).translated(2,3),6);
    g.setColour(juce::Colour(0x06000000)); g.fillRoundedRectangle(paramRect.toFloat().reduced(0,2).translated(4,6),6);
    g.setColour(juce::Colour(0x03000000)); g.fillRoundedRectangle(paramRect.toFloat().reduced(0,2).translated(7,10),6);
    g.setColour(juce::Colour(0x01000000)); g.fillRoundedRectangle(paramRect.toFloat().reduced(0,2).translated(11,15),6);
    g.setColour(glassBg);g.fillRoundedRectangle(paramRect.toFloat().reduced(0,2),6);
    g.setColour(whiteBg.brighter(0.03f)); g.drawRoundedRectangle(paramRect.toFloat().reduced(1,1),6,1.5f);
    g.setColour(juce::Colour(0x10000000)); g.drawRoundedRectangle(paramRect.toFloat().translated(0,1).reduced(1,3),6,2.5f);
    g.setColour(azure.withAlpha(0.30f));g.drawRoundedRectangle(paramRect.toFloat().reduced(0,2),6,1);

    // Group labels (no background, just embedded text)
    g.setFont(juce::Font{juce::FontOptions{}.withPointHeight(9.0f).withStyle("Bold")});
    for(int gi=0;gi<activeGroupNames.size();++gi){
        int lx=groupLabelXs[gi], ly=groupLabelYs[gi];
        g.setColour(azure.withAlpha(0.7f));
        g.drawText(activeGroupNames[gi],lx,ly,120,12,juce::Justification::left);
    }

    // Group dividers (horizontal lines between groups)
    for(int dy:dividerXs){
        g.setColour(azure.withAlpha(0.28f));
        g.drawHorizontalLine(dy,paramRect.getX()+8,paramRect.getRight()-8);
        g.setColour(whiteBg.withAlpha(0.5f));
        g.drawHorizontalLine(dy+1,paramRect.getX()+8,paramRect.getRight()-8);
    }
    // Tab button shadow (drawn in paint behind the component)
    auto tb = tabSwitch.getBounds().toFloat();
    g.setColour(juce::Colour(0x0C000000)); g.fillRoundedRectangle(tb.translated(2,3),5);
    g.setColour(juce::Colour(0x05000000)); g.fillRoundedRectangle(tb.translated(4,5),5);

    g.setColour(divLine.withAlpha(0.45f));
    g.drawHorizontalLine(mixRect.getY()-2,paramRect.getX()+10,paramRect.getRight()-10);
}

// ==============================================================================
// AmpFader — vertical fader for oscilloscope waveform amplitude
// ==============================================================================
ArxybinAudioProcessorEditor::AmpFader::AmpFader()
{
    setInterceptsMouseClicks(true, true);
}
void ArxybinAudioProcessorEditor::AmpFader::paint(juce::Graphics& g)
{
    auto ar = getLocalBounds().toFloat();
    float frac = (value - 0.05f) / 1.45f;
    float trackW = 4, trackX = ar.getCentreX() - trackW * 0.5f;
    float trackTop = ar.getY() + 12, trackBot = ar.getBottom() - 16;
    float trackH = trackBot - trackTop;
    float thumbR = 6;
    float thumbY = trackBot - frac * trackH;

    g.setColour(juce::Colour(0xFFD8DCE0));
    g.fillRoundedRectangle(trackX, trackTop, trackW, trackH, 2);
    g.setColour(azure.withAlpha(0.7f));
    g.fillRoundedRectangle(trackX, thumbY, trackW, trackBot - thumbY, 2);

    g.setColour(juce::Colour(0x20000000));
    g.fillEllipse(ar.getCentreX() - thumbR + 1, thumbY - thumbR + 2, thumbR * 2, thumbR * 2);
    g.setColour(juce::Colour(0xFFFFFFFF));
    g.fillEllipse(ar.getCentreX() - thumbR, thumbY - thumbR, thumbR * 2, thumbR * 2);
    g.setColour(juce::Colour(0x30FFFFFF));
    g.fillEllipse(ar.getCentreX() - thumbR * 0.5f, thumbY - thumbR * 0.8f, thumbR, thumbR * 0.7f);
    g.setColour(azure.withAlpha(0.6f));
    g.drawEllipse(ar.getCentreX() - thumbR, thumbY - thumbR, thumbR * 2, thumbR * 2, 1.2f);

    g.setFont(juce::Font{ juce::FontOptions{}.withPointHeight(6.5f) });
    g.setColour(paleAz.withAlpha(0.8f));
    g.drawText("AMP", ar.getX(), ar.getBottom() - 12, ar.getWidth(), 10, juce::Justification::centred);
}
void ArxybinAudioProcessorEditor::AmpFader::mouseDown(const juce::MouseEvent& e)
{
    dragStart = value;
    dragY = e.getScreenY();
}
void ArxybinAudioProcessorEditor::AmpFader::mouseDrag(const juce::MouseEvent& e)
{
    value = juce::jlimit(0.05f, 1.5f, dragStart + (dragY - e.getScreenY()) * 0.005f);
    repaint();
}
