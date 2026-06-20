/*
  ==============================================================================
    arxybin. — Full DSP chain with LFO modulation.
    Input → InGain → Granular → Distortion → Bitcrusher → Stutter →
            Shuffler → DryWet → OutGain → Output
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

ArxybinAudioProcessor::ArxybinAudioProcessor()
    : juce::AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , apvts(*this, nullptr, "PARAMETERS", arxybin::createParameterLayout())
    , presetManager(apvts) {}

ArxybinAudioProcessor::~ArxybinAudioProcessor() = default;

const juce::String ArxybinAudioProcessor::getName() const { return JucePlugin_Name; }
bool ArxybinAudioProcessor::acceptsMidi() const   { return false; }
bool ArxybinAudioProcessor::producesMidi() const   { return false; }
bool ArxybinAudioProcessor::isMidiEffect() const   { return false; }
double ArxybinAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int ArxybinAudioProcessor::getNumPrograms()         { return 1; }
int ArxybinAudioProcessor::getCurrentProgram()       { return 0; }
void ArxybinAudioProcessor::setCurrentProgram(int)    {}
const juce::String ArxybinAudioProcessor::getProgramName(int) { return {}; }
void ArxybinAudioProcessor::changeProgramName(int, const juce::String&) {}

void ArxybinAudioProcessor::prepareToPlay(double sr, int blockSize)
{
    currentBlockSize = blockSize;
    granularEngine.prepare(sr, blockSize);
    distortion.prepare(sr);
    bitcrusher.prepare(sr);
    stutter.prepare(sr, blockSize);
    shuffler.prepare(sr, blockSize);
    lfo.prepare(sr);
}

void ArxybinAudioProcessor::releaseResources()
{
    granularEngine.reset();
    distortion.reset();
    bitcrusher.reset();
    stutter.reset();
    shuffler.reset();
    lfo.reset();
}

bool ArxybinAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet()
        && (layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
         || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo());
}

void ArxybinAudioProcessor::readParams()
{
    using namespace arxybin;
    auto gp = [&](const juce::String& id) { return getRawParam(apvts, id); };

    // Grains
    granularEngine.setGrainSize(gp(ParamID::grainSize));
    granularEngine.setGrainRandom(gp(ParamID::grainRandom) > 0.5f);
    granularEngine.setDensity(static_cast<int>(gp(ParamID::grainDensity)));
    granularEngine.setPitch(gp(ParamID::grainPitch));
    granularEngine.setPitchRandom(gp(ParamID::pitchRandom));
    granularEngine.setPanSpread(gp(ParamID::panSpread));
    granularEngine.setPanRandom(gp(ParamID::panRandom) > 0.5f);
    granularEngine.setReverseProb(gp(ParamID::reverseProb));
    granularEngine.setFeedback(gp(ParamID::grainFeedback));

    // Scan
    granularEngine.setScanMode(static_cast<int>(gp(ParamID::scanMode)));
    granularEngine.setScanSpeed(gp(ParamID::scanSpeed));
    granularEngine.setGrainPosition(gp(ParamID::grainPosition));
    granularEngine.setPositionRandom(gp(ParamID::positionRandom));

    // Placement
    granularEngine.setPlaceOffset(gp(ParamID::placeOffset));
    granularEngine.setPlaceGrid(static_cast<int>(gp(ParamID::placeGrid)));
    granularEngine.setPlaceRate(gp(ParamID::placeRate));
    granularEngine.setPlaceSnap(gp(ParamID::placeSnap) > 0.5f);
    granularEngine.setPlaceRandom(gp(ParamID::placeRandom));

    // Distortion
    distortion.setType(static_cast<arxybin::DistType>(static_cast<int>(gp(ParamID::distType))));
    distortion.setDrive(gp(ParamID::distDrive) * 0.01f);
    distortion.setRandom(gp(ParamID::distRandom) > 0.5f);

    // Bitcrusher
    bitcrusher.setBitDepth(static_cast<int>(gp(ParamID::bitDepth)));
    bitcrusher.setRandom(gp(ParamID::bitRandom) > 0.5f);
    bitcrusher.setRateReduction(static_cast<int>(gp(ParamID::sampleRateRedux)));
    bitcrusher.setRateRandom(gp(ParamID::rateRandom) > 0.5f);

    // Stutter
    stutter.setProbability(gp(ParamID::stutterProb));
    stutter.setProbRandom(gp(ParamID::stutProbRandom) > 0.5f);
    stutter.setLengthMs(gp(ParamID::stutterLength));
    stutter.setLenRandom(gp(ParamID::stutLenRandom) > 0.5f);

    // Shuffler
    shuffler.setAmount(gp(ParamID::shuffleAmount));
    shuffler.setAmtRandom(gp(ParamID::shufAmtRandom) > 0.5f);
    shuffler.setSegmentSizeMs(gp(ParamID::shuffleSegment));
    shuffler.setSizeRandom(gp(ParamID::shufSizeRandom) > 0.5f);

    // LFO
    {
        arxybin::LfoParams p1, p2;
        p1.rate  = gp(ParamID::lfo1Rate);
        p1.depth = gp(ParamID::lfo1Depth);
        p1.wave  = static_cast<arxybin::LfoWaveform>(static_cast<int>(gp(ParamID::lfo1Wave)));
        p1.target= static_cast<arxybin::LfoTarget>(static_cast<int>(gp(ParamID::lfo1Target)));
        p2.rate  = gp(ParamID::lfo2Rate);
        p2.depth = gp(ParamID::lfo2Depth);
        p2.wave  = static_cast<arxybin::LfoWaveform>(static_cast<int>(gp(ParamID::lfo2Wave)));
        p2.target= static_cast<arxybin::LfoTarget>(static_cast<int>(gp(ParamID::lfo2Target)));
        lfo.setLfo1Params(p1);
        lfo.setLfo2Params(p2);

        arxybin::LfoParams p3;
        p3.rate  = gp(ParamID::lfo3Rate);
        p3.depth = gp(ParamID::lfo3Depth);
        p3.wave  = static_cast<arxybin::LfoWaveform>(static_cast<int>(gp(ParamID::lfo3Wave)));
        p3.target= static_cast<arxybin::LfoTarget>(static_cast<int>(gp(ParamID::lfo3Target)));
        lfo.setLfo3Params(p3);
    }

    // Capture buffer size + BPM sync
    float capMs = gp(ParamID::captureBufferMs);
    bool bpmSync = gp(ParamID::bpmSync) > 0.5f;
    if (bpmSync)
    {
        if (auto* ph = getPlayHead())
        {
            auto pos = ph->getPosition();
            if (pos && pos->getBpm())
            {
                double bpm = *pos->getBpm();
                if (bpm > 1.0)
                {
                    double beatMs = 60000.0 / bpm;
                    // Snap to nearest beat fraction
                    double options[] = { beatMs * 0.25, beatMs * 0.5, beatMs, beatMs * 2, beatMs * 4, beatMs * 8 };
                    double best = capMs;
                    for (double opt : options) {
                        if (std::abs(opt - capMs) < std::abs(best - capMs)) best = opt;
                    }
                    capMs = juce::jlimit(20.0f, 30000.0f, (float)best);
                }
            }
        }
    }
    granularEngine.setCaptureBufferMs(getSampleRate(), capMs);
}

void ArxybinAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    readParams();
    auto lfoMod = lfo.process();

    // Store for UI display
    lfoModDisplay[0] = lfoMod.pitchMod;
    lfoModDisplay[1] = lfoMod.positionMod;
    lfoModDisplay[2] = lfoMod.panMod;
    lfoModDisplay[3] = lfoMod.sizeMod;
    lfoModDisplay[4] = lfoMod.densityMod;
    lfoModDisplay[5] = lfoMod.distMod;
    lfoModDisplay[6] = lfoMod.bitMod;
    lfoModDisplay[7] = lfoMod.stutProbMod;
    lfoModDisplay[8] = lfoMod.stutLenMod;
    lfoModDisplay[9] = lfoMod.shufAmtMod;
    lfoModDisplay[10]= lfoMod.shufSzMod;

    lfo1TargetIdx = static_cast<int>(lfo.getLfo1Params().target);
    lfo2TargetIdx = static_cast<int>(lfo.getLfo2Params().target);
    lfo3TargetIdx = static_cast<int>(lfo.getLfo3Params().target);

    const float inputDb  = arxybin::getRawParam(apvts, arxybin::ParamID::inputGain);
    const float outputDb = arxybin::getRawParam(apvts, arxybin::ParamID::outputGain);
    const float dryWet   = arxybin::getRawParam(apvts, arxybin::ParamID::dryWet) * 0.01f;

    const float inGain  = juce::Decibels::decibelsToGain(inputDb);
    const float outGain = juce::Decibels::decibelsToGain(outputDb);

    const int ns = buffer.getNumSamples();
    const int nc = buffer.getNumChannels();

    // Save original dry input (for master dry/wet)
    juce::AudioBuffer<float> dryBuf;
    dryBuf.makeCopyOf(buffer);

    // Apply LFO modulations to granular engine
    granularEngine.applyLfoMod(lfoMod.pitchMod, lfoMod.positionMod,
                                lfoMod.panMod, lfoMod.sizeMod, lfoMod.densityMod);

    // Apply LFO modulations to Glitch params (temporary per-block modulation)
    distortion.setLfoMod(lfoMod.distMod);
    bitcrusher.setLfoMod(lfoMod.bitMod);
    stutter.setLfoMod(lfoMod.stutProbMod, lfoMod.stutLenMod);
    shuffler.setLfoMod(lfoMod.shufAmtMod, lfoMod.shufSzMod);

    // Granular synthesis
    juce::AudioBuffer<float> wet(nc, ns);
    wet.makeCopyOf(buffer);
    wet.applyGain(inGain);

    juce::AudioBuffer<float> grainOut(nc, ns);
    grainOut.clear();
    granularEngine.processBlock(wet, grainOut);

    // === Glitch chain — each effect with individual dry/wet ===
    const float distMix    = arxybin::getRawParam(apvts, arxybin::ParamID::distMix)    * 0.01f;
    const float bitMix     = arxybin::getRawParam(apvts, arxybin::ParamID::bitMix)     * 0.01f;
    const float stutterMix = arxybin::getRawParam(apvts, arxybin::ParamID::stutterMix) * 0.01f;
    const float shuffleMix = arxybin::getRawParam(apvts, arxybin::ParamID::shuffleMix) * 0.01f;

    // Helper: apply effect with dry/wet blend
    auto applyWithMix = [&](auto& effect, float mix) {
        if (mix <= 0.0f) return;                     // fully dry → skip
        if (mix >= 1.0f) { effect.processBlock(grainOut); return; } // fully wet → no blend needed

        juce::AudioBuffer<float> dry(nc, ns);
        dry.makeCopyOf(grainOut);
        effect.processBlock(grainOut);
        for (int ch = 0; ch < nc; ++ch)
            for (int s = 0; s < ns; ++s)
                grainOut.getWritePointer(ch)[s] =
                    dry.getReadPointer(ch)[s] * (1.0f - mix)
                    + grainOut.getReadPointer(ch)[s] * mix;
    };

    applyWithMix(distortion, distMix);
    applyWithMix(bitcrusher,  bitMix);
    applyWithMix(stutter,    stutterMix);
    applyWithMix(shuffler,   shuffleMix);

    // Master dry/wet (granular output vs original dry input)
    if (dryWet < 1.0f)
        for (int ch = 0; ch < nc; ++ch)
            for (int s = 0; s < ns; ++s)
                grainOut.getWritePointer(ch)[s] =
                    dryBuf.getReadPointer(ch)[s] * (1.0f - dryWet)
                    + grainOut.getReadPointer(ch)[s] * dryWet;

    for (int ch = 0; ch < nc; ++ch)
        buffer.copyFrom(ch, 0, grainOut, ch, 0, ns);
    buffer.applyGain(outGain);

    // Waveform snapshots for UI (decimate to waveSnapLen points)
    const int step = juce::jmax(1, ns / waveSnapLen);
    for (int i = 0; i < waveSnapLen; ++i)
    {
        const int idx = i * step;
        if (idx < ns) {
            drySnapshot[i] = dryBuf.getSample(0, idx);
            wetSnapshot[i] = buffer.getSample(0, idx);
        }
    }
    granularEngine.getRingBufferDecimated(ringSnapshot, waveSnapLen);
}

bool ArxybinAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* ArxybinAudioProcessor::createEditor()
{
    return new ArxybinAudioProcessorEditor(*this);
}

void ArxybinAudioProcessor::getStateInformation(juce::MemoryBlock& d)
{
    // Save APVTS state + preset bank
    auto state = apvts.copyState().createXml();
    auto presets = presetManager.getStateXml();

    juce::XmlElement root("arxybin");
    if (state)   root.addChildElement(state.release());
    root.addChildElement(new juce::XmlElement(presets));

    copyXmlToBinary(root, d);
}

void ArxybinAudioProcessor::setStateInformation(const void* data, int size)
{
    auto xml = getXmlFromBinary(data, size);
    if (!xml || !xml->hasTagName("arxybin"))
        return;

    // Restore APVTS state
    if (auto* paramTree = xml->getChildByName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*paramTree));

    // Restore preset bank
    if (auto* bankXml = xml->getChildByName("PRESET_BANK"))
        presetManager.setStateXml(*bankXml);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ArxybinAudioProcessor();
}
