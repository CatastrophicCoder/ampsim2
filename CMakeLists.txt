cmake_minimum_required(VERSION 3.22)

project(amp2203 VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(APPLE)
    set(AMP2203_PLUGIN_DEFAULT ON)
else()
    set(AMP2203_PLUGIN_DEFAULT OFF)
endif()

option(AMP2203_BUILD_TESTS "Build the Catch2 unit tests" ON)
option(AMP2203_BUILD_TOOLS "Build the ampwav CLI and the benchmark" ON)
option(AMP2203_BUILD_PLUGIN "Build the JUCE plugin (AU, VST3, Standalone)" ${AMP2203_PLUGIN_DEFAULT})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(Warnings)
include(Dependencies)

add_subdirectory(engine)

if(AMP2203_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(AMP2203_BUILD_TOOLS)
    add_subdirectory(tools/ampwav)
    add_subdirectory(tools/bench)
endif()

if(AMP2203_BUILD_PLUGIN)
    add_subdirectory(plugin)
endif()
