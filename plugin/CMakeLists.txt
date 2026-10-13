# JUCE plugin wrapper (the only place that includes JUCE): processor, editor (SPEC §7 M5, DECISIONS D-044).
juce_add_plugin(amp2203_plugin
    COMPANY_NAME "CatastrophicCoder"
    PRODUCT_NAME "Amp2203"
    PLUGIN_MANUFACTURER_CODE Ctcd
    PLUGIN_CODE A223
    FORMATS AU VST3 Standalone
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)

target_sources(amp2203_plugin PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp)

target_compile_definitions(amp2203_plugin PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(amp2203_plugin
    PRIVATE
        amp2203_engine
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# Offline renders through the processor for the M5 null test (plugin/render/main.cpp).
juce_add_console_app(amp2203_render PRODUCT_NAME "amp2203_render")
target_sources(amp2203_render PRIVATE render/main.cpp src/PluginProcessor.cpp src/PluginEditor.cpp)
target_compile_definitions(amp2203_render PRIVATE AMP2203_NO_PLUGIN_ENTRY=1 JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(amp2203_render
    PRIVATE
        ampwav_core
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)
