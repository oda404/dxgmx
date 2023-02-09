cmake_minimum_required(VERSION 3.23)
project(libc)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_FLAGS "-fno-builtin -std=c2x ${CMAKE_C_FLAGS}")

# libc.a
add_library(c STATIC
    stdlib.c
    syscalls.c
    fcntl.c
    dxgmx.c
)
target_include_directories(c PRIVATE include $ENV{DXGMX_SRC_ROOT}/include)
install(TARGETS c DESTINATION ${CMAKE_SYSROOT}/usr/lib/)

# Install headers
install(DIRECTORY include/ DESTINATION ${CMAKE_SYSROOT}/usr/include/)

# crt0.o
add_library(crt0 STATIC crt0.c)
target_include_directories(crt0 PRIVATE include)
add_custom_command(
    TARGET crt0 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:crt0> ${CMAKE_SYSROOT}/usr/lib/crt0.o
)
