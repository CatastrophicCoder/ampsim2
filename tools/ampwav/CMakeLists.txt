add_executable(ampwav main.cpp)
target_link_libraries(ampwav PRIVATE amp2203_engine dr_wav)
amp2203_set_warnings(ampwav)
