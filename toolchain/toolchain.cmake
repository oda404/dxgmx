
# the name of the target operating system
set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

# which compilers to use for C
set(CMAKE_C_COMPILER $ENV{CC})

# where is the target environment located
set(CMAKE_FIND_ROOT_PATH $ENV{DXGMX_SYSROOT})

set(CMAKE_SYSROOT $ENV{DXGMX_SYSROOT})

# adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)