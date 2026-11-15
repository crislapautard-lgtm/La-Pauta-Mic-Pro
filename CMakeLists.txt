cmake_minimum_required(VERSION 3.22)

# Runtime estático para que el VST3 no pida instalar nada extra en Windows
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(LaPautaMicPro VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(LaPautaMicPro
    COMPANY_NAME "La Pauta"
    PLUGIN_MANUFACTURER_CODE LaPa
    PLUGIN_CODE LpMp
    FORMATS VST3
    PRODUCT_NAME "La Pauta Mic Pro"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)

target_sources(LaPautaMicPro PRIVATE
    PluginProcessor.cpp
    PluginEditor.cpp)

target_compile_definitions(LaPautaMicPro PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(LaPautaMicPro
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
