add_executable(amp2203_tests
    test_amp2203.cpp
    test_chowdsp_wdf.cpp)

target_link_libraries(amp2203_tests PRIVATE amp2203_engine Catch2::Catch2WithMain)
amp2203_set_warnings(amp2203_tests)

include(Catch)
catch_discover_tests(amp2203_tests)
