# dr_wav implementation in its own target, built without the project's warning flags.
add_library(ampwav_drwav STATIC src/dr_wav_impl.c)
target_link_libraries(ampwav_drwav PUBLIC dr_wav)

add_library(ampwav_core STATIC
    src/Cli.cpp
    src/Runner.cpp
    src/WavIO.cpp)
target_include_directories(ampwav_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ampwav_core PUBLIC amp2203_engine PRIVATE ampwav_drwav)
amp2203_set_warnings(ampwav_core)

add_executable(ampwav main.cpp)
target_link_libraries(ampwav PRIVATE ampwav_core)
amp2203_set_warnings(ampwav)

if(AMP2203_BUILD_TESTS)
    include(Catch)
    add_executable(ampwav_tests tests/test_ampwav.cpp)
    target_link_libraries(ampwav_tests PRIVATE ampwav_core ampwav_drwav Catch2::Catch2WithMain)
    amp2203_set_warnings(ampwav_tests)
    catch_discover_tests(ampwav_tests)

    add_test(NAME ampwav_cli_help COMMAND ampwav --help)
    add_test(NAME ampwav_cli_rejects_bad_args COMMAND ampwav --block nope --in a.wav --out b.wav)
    set_tests_properties(ampwav_cli_rejects_bad_args PROPERTIES WILL_FAIL TRUE)
endif()
