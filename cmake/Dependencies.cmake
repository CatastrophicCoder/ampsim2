# Third-party dependencies, pinned by commit and archive SHA-256 (see docs/DECISIONS.md D-011).
# Archives are GitHub source tarballs of the pinned commits; downloads are verified against URL_HASH.
include(FetchContent)

# Extracted files get the extraction time as timestamp (CMake ≥ 3.24), so a pin change rebuilds cleanly.
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

# Marks a target's interface include directories as SYSTEM so -Werror on our targets ignores them.
function(amp2203_mark_system target)
    get_target_property(dirs ${target} INTERFACE_INCLUDE_DIRECTORIES)
    if(dirs)
        set_target_properties(${target} PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${dirs}")
    endif()
endfunction()

# chowdsp_wdf — main @ 43bcd29 (2026-07-02, "Updated adapted r-type computation")
set(CHOWDSP_WDF_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CHOWDSP_WDF_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(chowdsp_wdf
    URL https://github.com/Chowdhury-DSP/chowdsp_wdf/archive/43bcd295e8403de913f9966f4fd7ff9d17c9f1a5.tar.gz
    URL_HASH SHA256=4fa7919de8ee893ea3810e5a46728a8fdf04d319dea51359670bed90a29eeb70)
FetchContent_MakeAvailable(chowdsp_wdf)
amp2203_mark_system(chowdsp_wdf)

# dr_wav — dr_libs tag wav-0.14.5 @ fa931f3. Header only; the repository's CMake project is not used.
FetchContent_Declare(dr_libs
    URL https://github.com/mackron/dr_libs/archive/fa931f3285ced10ace628f7f1ac951e1951e7ea6.tar.gz
    URL_HASH SHA256=b56be5930399b9ea81591a837e0c9ee959a5d57e90dac96e0af37dde1fa0bb7e
    SOURCE_SUBDIR do-not-add-subdirectory)
FetchContent_MakeAvailable(dr_libs)
add_library(dr_wav INTERFACE)
target_include_directories(dr_wav SYSTEM INTERFACE ${dr_libs_SOURCE_DIR})

if(AMP2203_BUILD_TESTS)
    # Catch2 — tag v3.16.1 @ 5010924
    FetchContent_Declare(Catch2
        URL https://github.com/catchorg/Catch2/archive/5010924c4d8e912e64fd64a11819eecacf572c06.tar.gz
        URL_HASH SHA256=66e15ad3f8896fb1a4ef9a868b1de573be766f45c9495eb3c4ba4cc944ea4e22)
    FetchContent_MakeAvailable(Catch2)
    amp2203_mark_system(Catch2)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
endif()

if(AMP2203_BUILD_PLUGIN)
    # JUCE — tag 8.0.15 @ 91ad83a (SPEC §2: JUCE 8.x)
    FetchContent_Declare(JUCE
        URL https://github.com/juce-framework/JUCE/archive/91ad83ae34a81e0833b1a2b0866f54846370ae53.tar.gz
        URL_HASH SHA256=04f8d5055382582c757be9da069ea98338005f98248facd9c2804435ac853e70)
    FetchContent_MakeAvailable(JUCE)
endif()
