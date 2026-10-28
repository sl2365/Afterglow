cmake_minimum_required(VERSION 3.25)
project(Afterglow VERSION 1.57.3 LANGUAGES C CXX)

set(JUCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../_Tools/JUCE/_9.0.4")
if(NOT EXISTS "${JUCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "JUCE 9.0.4 not found at: ${JUCE_DIR}")
endif()
add_subdirectory("${JUCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE")

juce_add_plugin(Afterglow
    VERSION 1.57.3
    COMPANY_NAME "sl23"
    BUNDLE_ID "com.sl23.afterglow"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_AUTO_MANIFEST FALSE
    PLUGIN_MANUFACTURER_CODE sl23
    PLUGIN_CODE afg1
    FORMATS VST3
    VST3_CATEGORIES Fx Delay
    PRODUCT_NAME "Afterglow")

juce_add_binary_data(AfterglowResources
    SOURCES "${CMAKE_CURRENT_LIST_DIR}/../Resources/logo.png")

juce_generate_juce_header(Afterglow)
target_sources(Afterglow PRIVATE
    PluginProcessor.cpp PluginProcessor.h
    PluginEditor.cpp PluginEditor.h
    AfterglowSequencer.h)
target_compile_features(Afterglow PRIVATE cxx_std_17)
target_compile_definitions(Afterglow PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(Afterglow PRIVATE juce::juce_audio_utils AfterglowResources
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
