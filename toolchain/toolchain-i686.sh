
export DXGMX_SRC_ROOT=/home/oda/Documents/cpp/dxgmx

export DXGMX_TOOLCHAIN_ROOT=/home/oda/Documents/cpp/dxgmx-toolchain
export DXGMX_SYSROOT=$DXGMX_SRC_ROOT/sysroot

export CC=$DXGMX_TOOLCHAIN_ROOT/usr/bin/clang
export AS=$DXGMX_TOOLCHAIN_ROOT/usr/bin/clang
export LD=$DXGMX_TOOLCHAIN_ROOT/usr/bin/ld.lld
export NM=$DXGMX_TOOLCHAIN_ROOT/usr/bin/llvm-nm
export OBJCOPY=$DXGMX_TOOLCHAIN_ROOT/usr/bin/llvm-objcopy

export RUSTC=rustc

export DXGMX_ARCH=i686
export CMAKE_TOOLCHAIN_FILE=$DXGMX_SRC_ROOT/toolchain/toolchain.cmake
