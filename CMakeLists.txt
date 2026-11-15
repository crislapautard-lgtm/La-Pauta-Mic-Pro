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
    BUNDLE_ID com.lapauta.micpro
    PLUGIN_MANUFACTURER_CODE LaPa
    PLUGIN_CODE LpMp
    FORMATS VST3
    PRODUCT_NAME "La Pauta Mic Pro"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)

# Busca los archivos .cpp en la raíz del repo o en carpetas comunes (Source, src, plugin)
set(PLUGIN_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
    ${CMAKE_CURRENT_SOURCE_DIR}/source
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin)

set(PLUGIN_SOURCES "")
set(PLUGIN_INCLUDE_DIRS "")
foreach(dir ${PLUGIN_DIRS})
    file(GLOB found "${dir}/PluginProcessor.cpp" "${dir}/PluginEditor.cpp")
    if(found)
        list(APPEND PLUGIN_SOURCES ${found})
        list(APPEND PLUGIN_INCLUDE_DIRS ${dir})
    endif()
endforeach()

if(NOT PLUGIN_SOURCES)
    message(FATAL_ERROR "No se encontraron PluginProcessor.cpp / PluginEditor.cpp. Súbelos a la raíz del repo o a la carpeta Source/.")
endif()

target_sources(LaPautaMicPro PRIVATE ${PLUGIN_SOURCES})
target_include_directories(LaPautaMicPro PRIVATE ${PLUGIN_INCLUDE_DIRS})

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
        
