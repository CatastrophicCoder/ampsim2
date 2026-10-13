add_executable(amp2203_tests
    test_amp2203.cpp
    test_chowdsp_wdf.cpp
    test_mna.cpp
    test_tonestack.cpp
    test_newton.cpp
    test_stage12.cpp
    test_stage3.cpp
    test_stage34.cpp
    test_preamp.cpp
    test_triode.cpp
    test_pentode.cpp
    test_ot.cpp
    test_supply.cpp)

target_link_libraries(amp2203_tests PRIVATE amp2203_engine Catch2::Catch2WithMain)
# white-box tests of engine internals (engine/src) and the SPICE / Python fixtures in tests/data
target_include_directories(amp2203_tests PRIVATE ${PROJECT_SOURCE_DIR}/engine/src)
target_compile_definitions(amp2203_tests PRIVATE AMP2203_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
amp2203_set_warnings(amp2203_tests)

include(Catch)
catch_discover_tests(amp2203_tests)
