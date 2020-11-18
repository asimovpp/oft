#!/usr/bin/env bash

# LLVM/Clang should be in the PATH for this to work
export CC=clang
export CXX=clang++
export LLVMCONFIG=llvm-config

#COMPILER_VERSION=

# or picked up from a system package install
if [[ ! -z ${COMPILER_VERSION} ]]; then
  export CC=${CC}-${COMPILER_VERSION}
  export CXX=${CXX}-${COMPILER_VERSION}
  export LLVMCONFIG=${LLVMCONFIG}-${COMPILER_VERSION}
fi

#export BUILD_TYPE=Debug
export BUILD_TYPE=RelWithDebInfo

#export GTEST_ROOT=/usr/local/gtest-libcxx

export CXX_FLAGS=
export CXX_FLAGS="${CXX_FLAGS} -O1"
export CXX_FLAGS="${CXX_FLAGS} -stdlib=libc++"

export LINKER_FLAGS=
export LINKER_FLAGS="-Wl,-L$(${LLVMCONFIG} --libdir)"
export LINKER_FLAGS="${LINKER_FLAGS} -lc++ -lc++abi"

export SANITIZER_OPTIONS=""

OVERFLOWTOOL_SKIP_TESTS="OFF"
OVERFLOWTOOL_SKIP_TESTS=${GTEST_ROOT:=ON}

# find LLVM's cmake dir
export LLVM_DIR=$(${LLVMCONFIG} --cmakedir)

CMAKE_OPTIONS="-DLLVM_DIR=${LLVM_DIR}"

if [[ ! -z ${GTEST_ROOT} ]]; then
  CMAKE_OPTIONS="${CMAKE_OPTIONS} -DGTEST_ROOT=${GTEST_ROOT}"
fi

CMAKE_OPTIONS="${CMAKE_OPTIONS} -DOVERFLOWTOOL_SKIP_TESTS=${OVERFLOWTOOL_SKIP_TESTS}"

export CMAKE_OPTIONS
