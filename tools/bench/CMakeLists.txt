add_executable(amp2203_bench main.cpp)
target_link_libraries(amp2203_bench PRIVATE ampwav_core)
target_compile_definitions(amp2203_bench PRIVATE AMP2203_DI_DIR="${PROJECT_SOURCE_DIR}/reference/di")
amp2203_set_warnings(amp2203_bench)
