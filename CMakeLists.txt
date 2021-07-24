cmake_minimum_required(VERSION 3.10)

project(utf-8)

function(cpp17 target)
    set_target_properties(${target}
        PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
    )

    target_compile_options(${target}
        PRIVATE
            -Wall -Wextra -Wconversion -pedantic -Wno-sign-conversion
    )
endfunction()

add_library(utf8 STATIC src/utf8.cpp)
cpp17(utf8)
target_include_directories(utf8 PUBLIC include)
