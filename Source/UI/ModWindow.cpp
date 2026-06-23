/*
  ==============================================================================
    arxybin. — MOD window. LFO1/2/3 config + animated waveform previews.
  ==============================================================================
*/

#include "ModWindow.h"

namespace arxybin
{

const juce::Colour ModWindowContent::azureBlue{0xFF4A8C5C}; // LFO green
const juce::Colour ModWindowContent::lightAzure{0xFF8CB89A};
const juce::Colour ModWindowContent::paleAzure{0xFFC8DCD0};
const juce::Colour ModWindowContent::bgWhite{0xFFF8F8FA};
const juce::Colour ModWindowContent::darkText{0xFF1A2A35};
const juce::Colour ModWindowContent::divLine{0xFFB0C8B8};

// ==============================================================================
ModWindowContent::ModKnob::ModKnob(const juce::String& p, juce::AudioProcessorValueTreeState& a)
    : apvts(a), pid(p)
{
    setInterceptsMouseClicks(true, true);
    if (auto* param = apvts.getParameter(pid)) val = param->getValue();
    dispVal = val;
}
void ModWindowContent::ModKnob::updateValue(float v) { val = v; dispVal = v; repaint(); }
void ModWindowContent::ModKnob::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced(2);
    float cx = b.getCentreX(), cy = b.getCentreY();
    float r = juce::jmin(b.getWidth(), b.getHeight()) * 0.4f;

    // Drop shadow
    g.setColour(juce::Colour(0x18000000));
    g.fillEllipse(cx - r + 2.0f, cy - r + 2.0f, r * 2, r * 2);

    // Outer ring
    g.setColour(paleAzure.withAlpha(0.5f));
    g.drawEllipse(cx - r, cy - r, r * 2, r * 2, 2.0f);

    const float sa = juce::MathConstants<float>::pi * 1.25f;
    const float ea = juce::MathConstants<float>::pi * 2.75f;
    const float ang = sa + dispVal * (ea - sa);

    // Glow layer
    juce::Path ap; ap.addCentredArc(cx, cy, r, r, 0.0f, sa, ang, true);
    g.setColour(azureBlue.withAlpha(0.15f));
    g.strokePath(ap, juce::PathStrokeType(7.0f));
    g.setColour(azureBlue.withAlpha(0.30f));
    g.strokePath(ap, juce::PathStrokeType(4.5f));
    // Core arc
    g.setColour(azureBlue); g.strokePath(ap, juce::PathStrokeType(2.8f));

    // Dot indicator
    auto ep = ap.getCurrentPosition();
    float dx = cx + (ep.x - cx) * 0.72f, dy = cy + (ep.y - cy) * 0.72f;
    g.setColour(azureBlue); g.fillEllipse(dx - 2.0f, dy - 2.0f, 4.0f, 4.0f);
    g.setColour(bgWhite); g.fillEllipse(dx - 1.0f, dy - 1.0f, 2.0f, 2.0f);
    if (hover) { g.setColour(juce::Colour(0x0C000000)); g.fillEllipse(cx - r - 4, cy - r - 4, (r+4)*2, (r+4)*2); }
}
void ModWindowContent::ModKnob::mouseEnter(const juce::MouseEvent&) { hover=true; repaint(); }
void ModWindowContent::ModKnob::mouseExit(const juce::MouseEvent&)  { hover=false; repaint(); }
void ModWindowContent::ModKnob::mouseDown(const juce::MouseEvent& e) { dragStart=val; dragY=e.getScreenY(); setMouseCursor(juce::MouseCursor::UpDownResizeCursor); }
void ModWindowContent::ModKnob::mouseDrag(const juce::MouseEvent& e) { float nv=juce::jlimit(0.0f,1.0f,dragStart+(dragY-e.getScreenY())*0.005f); val=nv; dispVal=nv; if(auto* p=apvts.getParameter(pid))p->setValueNotifyingHost(nv); repaint(); }

// ==============================================================================
ModWindowContent::ModWindowContent(juce::AudioProcessorValueTreeState& a) : apvts(a)
{
    auto setupCb = [&](juce::ComboBox& c, const juce::String& pid, juce::Label& lbl, const juce::String& txt) {
        addAndMakeVisible(c);
        c.setColour(juce::ComboBox::backgroundColourId, juce::Colours::transparentWhite);
        c.setColour(juce::ComboBox::outlineColourId, azureBlue.withAlpha(0.35f));
        c.setColour(juce::ComboBox::textColourId, darkText);
        c.setColour(juce::ComboBox::arrowColourId, azureBlue);
        c.setScrollWheelEnabled(false);
        if (auto* p = apvts.getParameter(pid))
            if (auto* ch = dynamic_cast<juce::AudioParameterChoice*>(p)) {
                c.addItemList(ch->getAllValueStrings(), 1);
                c.setSelectedItemIndex(ch->getIndex(), juce::dontSendNotification);
                c.onChange = [&c, pid, this] { if (auto* pp = apvts.getParameter(pid)) pp->setValueNotifyingHost((float)c.getSelectedItemIndex() / juce::jmax(1.0f, (float)(c.getNumItems()-1))); };
            }
        addAndMakeVisible(lbl); lbl.setText(txt, juce::dontSendNotification);
        lbl.setFont(juce::Font{ juce::FontOptions{}.withPointHeight(10.5f) }); lbl.setColour(juce::Label::textColourId, darkText);
        lbl.setJustificationType(juce::Justification::centred);
    };
    auto addKLbl = [&](juce::Label& l, const juce::String& t) {
        addAndMakeVisible(l); l.setText(t, juce::dontSendNotification);
        l.setFont(juce::Font{ juce::FontOptions{}.withPointHeight(10.0f) }); l.setColour(juce::Label::textColourId, darkText);
        l.setJustificationType(juce::Justification::centred);
    };

    // LFO1
    addAndMakeVisible(lfo1Rate); addKLbl(lfo1RateLbl, "Rate");
    addAndMakeVisible(lfo1Depth); addKLbl(lfo1DepthLbl, "Depth");
    setupCb(lfo1Target, "lfo1Target", lfo1TargetLbl, "Target");
    setupCb(lfo1Wave, "lfo1Wave", lfo1WaveLbl, "Wave");
    // LFO2
    addAndMakeVisible(lfo2Rate); addKLbl(lfo2RateLbl, "Rate");
    addAndMakeVisible(lfo2Depth); addKLbl(lfo2DepthLbl, "Depth");
    setupCb(lfo2Target, "lfo2Target", lfo2TargetLbl, "Target");
    setupCb(lfo2Wave, "lfo2Wave", lfo2WaveLbl, "Wave");
    // LFO3
    addAndMakeVisible(lfo3Rate); addKLbl(lfo3RateLbl, "Rate");
    addAndMakeVisible(lfo3Depth); addKLbl(lfo3DepthLbl, "Depth");
    setupCb(lfo3Target, "lfo3Target", lfo3TargetLbl, "Target");
    setupCb(lfo3Wave, "lfo3Wave", lfo3WaveLbl, "Wave");
    // LFO4
    addAndMakeVisible(lfo4Rate); addKLbl(lfo4RateLbl, "Rate");
    addAndMakeVisible(lfo4Depth); addKLbl(lfo4DepthLbl, "Depth");
    setupCb(lfo4Target, "lfo4Target", lfo4TargetLbl, "Target");
    setupCb(lfo4Wave, "lfo4Wave", lfo4WaveLbl, "Wave");

    setSize(510, 700);
    startTimerHz(40);
}
ModWindowContent::~ModWindowContent() { stopTimer(); }

void ModWindowContent::timerCallback()
{
    float r1=0,r2=0,r3=0;
    if(auto* p=apvts.getParameter("lfo1Rate"))r1=p->getValue();
    if(auto* p=apvts.getParameter("lfo2Rate"))r2=p->getValue();
    if(auto* p=apvts.getParameter("lfo3Rate"))r3=p->getValue();
    lfo1Phase+=0.005f+r1*0.15f; if(lfo1Phase>1.0f)lfo1Phase-=1.0f;
    lfo2Phase+=0.005f+r2*0.15f; if(lfo2Phase>1.0f)lfo2Phase-=1.0f;
    lfo3Phase+=0.005f+r3*0.15f; if(lfo3Phase>1.0f)lfo3Phase-=1.0f;
    float r4=0;
    if(auto* p=apvts.getParameter("lfo4Rate"))r4=p->getValue();
    lfo4Phase+=0.005f+r4*0.15f; if(lfo4Phase>1.0f)lfo4Phase-=1.0f;
    repaint();
}

void ModWindowContent::drawLfoPreview(juce::Graphics& g, juce::Rectangle<int> area,
                                       int waveType, float phaseOffset)
{
    // Shadow + glow
    g.setColour(juce::Colour(0x0C000000)); g.fillRoundedRectangle(area.toFloat().translated(2,3),5);
    g.setColour(juce::Colour(0x05000000)); g.fillRoundedRectangle(area.toFloat().translated(4,5),5);
    g.setColour(bgWhite); g.fillRoundedRectangle(area.toFloat(),5);
    g.setColour(azureBlue.withAlpha(0.07f)); g.drawRoundedRectangle(area.toFloat().expanded(2),7,3);
    g.setColour(divLine); g.drawRoundedRectangle(area.toFloat().reduced(0.5f),5,1);

    const float w=(float)(area.getWidth()-8),h=(float)(area.getHeight()-8);
    const float x0=(float)(area.getX()+4),cy=(float)area.getCentreY();
    const int pts=160;
    juce::Path wp; bool first=true;
    for(int i=0;i<pts;++i){
        float ph=(float)i/(float)(pts-1)+phaseOffset; if(ph>1.0f)ph-=1.0f;
        float val=0.0f;
        switch(waveType){
        case 0: val=std::sin(ph*juce::MathConstants<float>::twoPi); break;
        case 1: val=1.0f-std::abs(ph*4.0f-2.0f); break;
        case 2: val=ph*2.0f-1.0f; break;
        case 3: val=(ph<0.5f)?1.0f:-1.0f; break;
        case 4: {int st=(int)(ph*10); float rv=(float)(std::abs(std::sin((double)st*12.9898+78.233)*43758.5453)); val=std::fmod(rv,2.0f)-1.0f; break;}
        default: break;
        }
        float px=x0+(float)i/(float)(pts-1)*w, py=cy-val*h*0.32f;
        if(first){wp.startNewSubPath(px,py);first=false;}else wp.lineTo(px,py);
    }
    g.setColour(azureBlue.withAlpha(0.15f)); g.strokePath(wp,juce::PathStrokeType(3.5f));
    g.setColour(azureBlue); g.strokePath(wp,juce::PathStrokeType(1.5f));
    g.setColour(paleAzure.withAlpha(0.5f)); g.drawHorizontalLine((int)cy,area.getX()+4,area.getRight()-4);
}

void ModWindowContent::paint(juce::Graphics& g)
{
    g.fillAll(bgWhite);
    const auto hdrF=juce::Font{juce::FontOptions{}.withPointHeight(14.0f).withStyle("Bold")};
    const int wfH=52;

    // LFO1: header(2) → controls(18) → waveform(118) → separator(176)
    g.setFont(hdrF); g.setColour(azureBlue);
    g.drawText("LFO 1",12,2,200,18,juce::Justification::left);
    drawLfoPreview(g,{8,118,getWidth()-16,wfH},lfo1Wave.getSelectedItemIndex(),lfo1Phase);
    g.setColour(divLine); g.drawHorizontalLine(176,12,getWidth()-12);

    // LFO2: header(182) → controls(198) → waveform(298) → separator(356)
    g.drawText("LFO 2",12,182,200,18,juce::Justification::left);
    drawLfoPreview(g,{8,298,getWidth()-16,wfH},lfo2Wave.getSelectedItemIndex(),lfo2Phase);
    g.setColour(divLine); g.drawHorizontalLine(356,12,getWidth()-12);

    // LFO3: header(362) → controls(378) → waveform(478) → separator(536)
    g.drawText("LFO 3",12,362,200,18,juce::Justification::left);
    drawLfoPreview(g,{8,478,getWidth()-16,wfH},lfo3Wave.getSelectedItemIndex(),lfo3Phase);
    g.setColour(divLine); g.drawHorizontalLine(536,12,getWidth()-12);

    // LFO4: header(542) → controls(558) → waveform(658)
    g.drawText("LFO 4",12,542,200,18,juce::Justification::left);
    drawLfoPreview(g,{8,658,getWidth()-16,wfH},lfo4Wave.getSelectedItemIndex(),lfo4Phase);
}

void ModWindowContent::resized()
{
    const int kw=80,kh=72;
    auto lay=[&](int y, ModKnob& rate, ModKnob& depth, juce::Label& rl, juce::Label& dl,
                 juce::ComboBox& tgt, juce::ComboBox& wav, juce::Label& tl, juce::Label& wl){
        rl.setBounds(8,y,kw,14);     rate.setBounds(8,y+14,kw,kh);
        dl.setBounds(98,y,kw,14);    depth.setBounds(98,y+14,kw,kh);
        tl.setBounds(200,y,110,14);  tgt.setBounds(200,y+14,110,24);
        wl.setBounds(330,y,110,14);  wav.setBounds(330,y+14,110,24);
    };
    lay(18,  lfo1Rate,lfo1Depth,lfo1RateLbl,lfo1DepthLbl,lfo1Target,lfo1Wave,lfo1TargetLbl,lfo1WaveLbl);
    lay(198, lfo2Rate,lfo2Depth,lfo2RateLbl,lfo2DepthLbl,lfo2Target,lfo2Wave,lfo2TargetLbl,lfo2WaveLbl);
    lay(378, lfo3Rate,lfo3Depth,lfo3RateLbl,lfo3DepthLbl,lfo3Target,lfo3Wave,lfo3TargetLbl,lfo3WaveLbl);
    lay(558, lfo4Rate,lfo4Depth,lfo4RateLbl,lfo4DepthLbl,lfo4Target,lfo4Wave,lfo4TargetLbl,lfo4WaveLbl);
}

juce::DialogWindow* createModWindow(juce::AudioProcessorValueTreeState& apvts)
{
    auto* c=new ModWindowContent(apvts);
    juce::DialogWindow::LaunchOptions opts;
    opts.content.setOwned(c);
    opts.dialogTitle="arxybin.  MOD MATRIX";
    opts.dialogBackgroundColour=ModWindowContent::bgWhite;
    opts.resizable=false; opts.useNativeTitleBar=true;
    return opts.launchAsync();
}

} // namespace arxybin
