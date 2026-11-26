cmake_minimum_required(VERSION 3.22)
project(ATELIER VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.12
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(juce)

juce_add_plugin(ATELIER
    COMPANY_NAME "N6"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Nm6x
    PLUGIN_CODE Atl1
    FORMATS VST3
    PRODUCT_NAME "ATELIER"
    VERSION "1.0.0"
)

target_sources(ATELIER PRIVATE
    AtelierMain.cpp
    AtelierVersion.h
    AtelierFormat.h
    AtelierFormat.cpp
    AtelierDsp.h
    AtelierDsp.cpp
    AtelierProcessor.h
    AtelierProcessor.cpp
    AtelierEditor.h
    AtelierEditor.cpp
)

target_compile_definitions(ATELIER PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_MODAL_LOOPS_PERMITTED=1
)

target_link_libraries(ATELIER PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
