# Strict warnings for this project's own targets (SPEC §8). Third-party targets are not affected; their
# headers are consumed as SYSTEM includes (see Dependencies.cmake).
function(amp2203_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /WX)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endfunction()
