cmake_minimum_required(VERSION 3.22)

project(ChannelStrip VERSION 2.0.4 LANGUAGES CXX)

include(FetchContent)
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.8
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(ChannelStripArtwork
    HEADER_NAME ChannelStripArtworkData.h
    NAMESPACE ChannelStripArtworkData
    SOURCES
        assets/faceplate.png
)

juce_add_plugin(ChannelStrip
    COMPANY_NAME "Loops"
    BUNDLE_ID "audio.independent.channelstrip"
    PLUGIN_MANUFACTURER_CODE Indp
    PLUGIN_CODE ChSt
    PRODUCT_NAME "Channel Strip"
    DESCRIPTION "Vintage-style channel strip"
    VERSION ${PROJECT_VERSION}
    FORMATS VST3
    VST3_CATEGORIES Fx
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(ChannelStrip)

target_sources(ChannelStrip
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
)

target_compile_features(ChannelStrip PRIVATE cxx_std_17)
target_compile_definitions(ChannelStrip
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)
target_link_libraries(ChannelStrip
    PRIVATE
        ChannelStripArtwork
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
