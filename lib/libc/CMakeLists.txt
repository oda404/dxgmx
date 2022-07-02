cmake_minimum_required(VERSION 3.23)
project(libc)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_FLAGS "-fno-builtin ${CMAKE_C_FLAGS}")

add_library(c STATIC
    stdlib.c
)
add_custom_command(
    TARGET c
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:c> ${CMAKE_SYSROOT}/usr/lib/libc.a
)

add_library(crt0 STATIC crt0.c)
add_custom_command(
    TARGET crt0
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:crt0> ${CMAKE_SYSROOT}/usr/lib/crt0.o
)
