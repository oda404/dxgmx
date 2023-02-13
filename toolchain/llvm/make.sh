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
        git clone $URL $REPO_ROOT || exit 1

        pushd $REPO_ROOT
            git checkout 1e1f60c605a9b1c803f3bbb1a1339c9bb1af4e34 || exit 1
            git apply ../patches/1.patch || exit 1
        popd
    fi
}

build() {
    echo Building $PACKAGE_NAME...

    echo "Prefix is \"$PREFIX\""

    pushd $REPO_ROOT
        cmake -S llvm -B build-llvm -G "Unix Makefiles" \
            -DLLVM_DEFAULT_TARGET_TRIPLE='i686-unknown-dxgmx' \
            -DLLVM_TARGETS_TO_BUILD='X86' \
            -DLLVM_ENABLE_PROJECTS='clang;lld' \
            -DLLVM_ENABLE_RUNTIMES='compiler-rt' \
            -DCMAKE_INSTALL_PREFIX="$PREFIX" \
            -DCMAKE_C_COMPILER=clang \
            -DCMAKE_CXX_COMPILER=clang++ \
            -DCMAKE_BUILD_TYPE='Release' \
            -DCOMPILER_RT_BUILD_BUILTINS=ON \
            -DCOMPILER_RT_BUILD_CRT=ON \
            -DCOMPILER_RT_BUILD_SANITIZERS=OFF \
            -DCOMPILER_RT_BUILD_XRAY=OFF \
            -DCOMPILER_RT_BUILD_LIBFUZZER=OFF \
            -DCOMPILER_RT_BUILD_PROFILE=OFF \
            -DCOMPILER_RT_BUILD_MEMPROF=OFF \
            -DCOMPILER_RT_BUILD_XRAY_NO_PREINIT=OFF \
            -DCOMPILER_RT_BUILD_ORC=OFF \
            -DCOMPILER_RT_BUILD_GWP_ASAN=OFF \
            -DCOMPILER_RT_ENABLE_CET=OFF \
            -DCOMPILER_RT_INCLUDE_TESTS=OFF

	make -j$(nproc) -C build-llvm
    popd
}

install() {

    pushd $REPO_ROOT
        make -C build-llvm install
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
