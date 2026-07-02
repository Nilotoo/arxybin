cmake_minimum_required(VERSION 3.22)

# ==============================================================================
# arxybin. — Glitch Particle Granular Effect
# Creator: nilotoo.
# ==============================================================================

# Static runtime — MUST be set before any targets are created.
# Prevents "missing MSVC runtime DLL" errors on end-user machines.
# Windows/MSVC only — Apple Clang on macOS does not use MSVC runtime.
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

project(arxybin VERSION 1.0.2 LANGUAGES C CXX)

# --- JUCE setup ----------------------------------------------------------------
# JUCE_PATH can be overridden three ways (first wins):
#   1. cmake -DJUCE_PATH=/path/to/JUCE   (explicit)
#   2. JUCE_PATH environment variable
#   3. Platform default (Windows: C:/Users/Administrator/JUCE, macOS: ~/JUCE)
if(NOT DEFINED JUCE_PATH)
    if(DEFINED ENV{JUCE_PATH})
        set(JUCE_PATH "$ENV{JUCE_PATH}")
    elseif(WIN32)
        set(JUCE_PATH "C:/Users/Administrator/JUCE")
    else()
        set(JUCE_PATH "$ENV{HOME}/JUCE")
    endif()
endif()
message(STATUS "JUCE_PATH = ${JUCE_PATH}")
add_subdirectory(${JUCE_PATH} ${CMAKE_BINARY_DIR}/JUCE)

# --- Plugin target -------------------------------------------------------------
# macOS also builds AudioUnit (AU) for Logic Pro etc.
if(APPLE)
    set(ARXYBIN_FORMATS VST3 Standalone AudioUnit)
else()
    set(ARXYBIN_FORMATS VST3 Standalone)
endif()

juce_add_plugin(arxybin
    # Plugin metadata
    PRODUCT_NAME             "arxybin."
    COMPANY_NAME             "nilotoo."
    PLUGIN_MANUFACTURER_CODE nilo
    PLUGIN_CODE              ArXy
    VERSION                  1.0.0

    # Plugin formats
    FORMATS                  ${ARXYBIN_FORMATS}
    VST3_CATEGORIES          "Fx|Delay" "Fx|Dynamics" "Fx"
)

# --- Source files (added AFTER juce_add_plugin, per JUCE convention) -----------
juce_generate_juce_header(arxybin)

target_sources(arxybin
    PRIVATE
        Source/PluginProcessor.h
        Source/PluginProcessor.cpp
        Source/PluginEditor.h
        Source/PluginEditor.cpp
        Source/Parameters.h
        Source/Grain.h
        Source/Grain.cpp
        Source/GranularEngine.h
        Source/GranularEngine.cpp
        Source/Distortion.h
        Source/Distortion.cpp
        Source/Bitcrusher.h
        Source/Bitcrusher.cpp
        Source/StutterEffect.h
        Source/StutterEffect.cpp
        Source/BufferShuffler.h
        Source/BufferShuffler.cpp
        Source/LfoEngine.h
        Source/LfoEngine.cpp
        Source/PresetManager.h
        Source/PresetManager.cpp
        Source/UI/ArxybinLookAndFeel.h
        Source/UI/ArxybinLookAndFeel.cpp
        Source/UI/WaveformViewer.h
        Source/UI/WaveformViewer.cpp
        Source/UI/ParticleBackground.h
        Source/UI/ParticleBackground.cpp
        Source/UI/ModWindow.h
        Source/UI/ModWindow.cpp
        $<$<PLATFORM_ID:Windows>:Source/arxybin.rc>
)

# --- Compile definitions -------------------------------------------------------
target_compile_definitions(arxybin
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0   # VST3-only, no VST2 history
)

# --- JUCE module dependencies --------------------------------------------------
target_link_libraries(arxybin
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_devices
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# --- Compiler settings ---------------------------------------------------------
target_compile_features(arxybin PRIVATE cxx_std_17)
