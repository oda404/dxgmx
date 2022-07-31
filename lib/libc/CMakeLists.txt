cmake_minimum_required(VERSION 3.23)
project(libc)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_FLAGS "-fno-builtin ${CMAKE_C_FLAGS}")

# libc.a
add_library(c STATIC
    stdlib.c
    syscalls.c
)
target_include_directories(c PRIVATE include)
install(TARGETS c DESTINATION ${CMAKE_SYSROOT}/usr/lib/)

# Install headers
set(LIBC_HEADERS
    include/stdlib.h
)
install(FILES ${LIBC_HEADERS} DESTINATION ${CMAKE_SYSROOT}/usr/include/)

# crt0.o
add_library(crt0 STATIC crt0.c)
add_custom_command(
    TARGET crt0 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:crt0> ${CMAKE_SYSROOT}/usr/lib/crt0.o
)
