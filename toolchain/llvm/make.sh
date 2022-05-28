#!/bin/bash

PACKAGE_NAME="dxgmx_llvm"

URL="https://github.com/llvm/llvm-project"

PACKAGE_ROOT_DIR=$(dirname $0)
REPO_ROOT=${PACKAGE_ROOT_DIR}/llvm-project

setup() {
    echo Setting up $PACKAGE_NAME...

    if [[ -d $REPO_ROOT ]]; then
        echo "Skipping clone..."
    else
        git clone --depth 1 $URL $REPO_ROOT

        pushd $REPO_ROOT
            git checkout 1e1f60c605a9b1c803f3bbb1a1339c9bb1af4e34 
        popd
    fi
}

build() {
    echo Building $PACKAGE_NAME...

    echo "Prefix is \"$PREFIX\""

    pushd $REPO_ROOT
        cmake -S llvm -B build-llvm -G "Unix Makefiles" \
            -DLLVM_ENABLE_PROJECTS='clang;compiler-rt;lld' \
            -DLLVM_ENABLE_RUNTIMES='' \
            -DLLVM_TARGETS_TO_BUILD='X86' \
            -DCMAKE_INSTALL_PREFIX="$PREFIX" \
            -DCMAKE_C_COMPILER=clang \
            -DCMAKE_CXX_COMPILER=clang++ \
            -DCMAKE_BUILD_TYPE=Release \
            -DCOMPILER_RT_BUILD_CRT=OFF \
            -DCOMPILER_RT_CRT_USE_EH_FRAME_REGISTRY=OFF \
            -DCOMPILER_RT_BUILD_SANITIZERS=OFF \
            -DCOMPILER_RT_BUILD_XRAY=OFF \
            -DCOMPILER_RT_BUILD_LIBFUZZER=OFF \
            -DCOMPILER_RT_BUILD_PROFILE=OFF \
            -DCOMPILER_RT_BUILD_MEMPROF=OFF \
            -DCOMPILER_RT_BUILD_XRAY_NO_PREINIT=OFF \
            -DCOMPILER_RT_BUILD_ORC=OFF \
            -DCOMPILER_RT_BUILD_GWP_ASAN=OFF \
            -DCOMPILER_RT_ENABLE_CET=OFF \
            -DCOMPILER_RT_BAREMETAL_BUILD=ON \
            -DCOMPILER_RT_INCLUDE_TESTS=NO \
            -DCOMPILER_RT_DEFAULT_TARGET_TRIPLE=i686-unknown-dxgmx

        make -j$(nproc) -C build-llvm/

    popd
}

install() {
    pushd $REPO_ROOT
        make install -C build-llvm/
    popd 
}

clean() {
    rm -rf $SRC_DIR
}

mrclean() {
    clean
    rm -rf $BUILD_DIR
}

setup
build
install
