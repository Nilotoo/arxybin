arxybin. v1.0.0
================
Granular + Glitch Particle Effect
by arxybin.

Formats : VST3 + Standalone
Platform: Windows x64
Tech    : JUCE 8 + C++17 (static link, no external DLLs)

INSTALL
-------
1. Run "Install.cmd" as Administrator
   OR manually copy:
   - "arxybin..vst3" to C:\Program Files\Common Files\VST3\
   - "default.arxybin" to Documents\arxybin\Presets\

2. Launch your DAW and scan for new plugins.

3. Standalone "arxybin..exe" runs directly.

SIGNAL CHAIN
------------
Input -> Granular Engine -> Distortion -> Bitcrusher ->
Stutter -> BufferShuffler -> Dry/Wet -> Output

KEY FEATURES
------------
- Granular synthesis: 4s buffer, up to 256 grains, 30-500ms
- Scan modes: Forward / Reverse / Bidirectional / Random
- Distortion: Soft Clip / Hard Clip / Foldback / Fuzz
- Bitcrusher: 5-24 bit, 1x-64x rate reduction
- Stutter: probabilistic buffer freeze
- BufferShuffler: segment reordering
- 3 LFOs with 12 modulation targets each
- Per-effect Glitch dry/wet mixing
- Grain fade 10ms (anti-click)
- Preset file format: .arxybin

PRESETS
-------
Save: [Save] button -> name + author -> saved to Documents\arxybin\Presets\
Load: [Load] button -> browse to any .arxybin file
Default: included in package, copied during install

MOD MATRIX
----------
[MOD] button -> LFO1/2/3 config window
Each LFO can target any Granulator or Glitch parameter
Green arc on targeted knob shows real-time modulation

UNINSTALL
---------
Delete: C:\Program Files\Common Files\VST3\arxybin..vst3\
Delete: Documents\arxybin\
Delete: arxybin..exe wherever placed

(c) 2026 arxybin.
