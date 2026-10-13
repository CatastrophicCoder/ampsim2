add_library(amp2203_engine STATIC
    src/Amp2203.cpp
    src/Preamp.cpp
    src/StageV1aV1b.cpp
    src/StageV2aV2b.cpp
    src/StageV2aV2bToneStack.cpp
    src/ToneStackMaster.cpp)

target_include_directories(amp2203_engine
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(amp2203_engine PUBLIC chowdsp::chowdsp_wdf)
target_compile_features(amp2203_engine PUBLIC cxx_std_20)
set_target_properties(amp2203_engine PROPERTIES POSITION_INDEPENDENT_CODE ON)
amp2203_set_warnings(amp2203_engine)
