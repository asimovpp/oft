#!/bin/bash

if [ ! -d llvm_flang ]; then
  mkdir llvm_flang
fi
cd llvm_flang

buildtype=RelWithDebInfo
installdir=$WORK/llvm_clang_flang

if [ ! -d classic-flang-llvm-project ]; then
  git clone https://github.com/flang-compiler/classic-flang-llvm-project.git
  (cd classic-flang-llvm-project && git checkout tags/flang_20211215_12x)
fi
if [ ! -d flang ]; then
  git clone https://github.com/flang-compiler/flang
  (cd flang && git checkout tags/flang_20211221 && cd ..)
fi

BINUTILS_INC_DIR=$WORK/binutils_source/include/

BUILD_LLVM=true
CONFIGURE_LLVM=true
BUILD_LIBPGMATH=true
BUILD_FLANG=true


if $BUILD_LLVM; then
        mkdir -p build/llvm
        pushd build/llvm
        if $CONFIGURE_LLVM; then
                cmake -G "Ninja" \
                  -DCMAKE_BUILD_TYPE=$buildtype \
                  -DBUILD_SHARED_LIBS=On \
                  -DCMAKE_INSTALL_PREFIX=$installdir \
                  -DCMAKE_C_COMPILER=cc \
                  -DCMAKE_CXX_COMPILER=CC \
                  -DLLVM_ENABLE_PROJECTS="clang;openmp" \
                  -DLLVM_ENABLE_CLASSIC_FLANG=on \
                  -DLLVM_ENABLE_ASSERTIONS=on \
                  -DLLVM_OPTIMIZED_TABLEGEN=on \
                  -DLLVM_TARGETS_TO_BUILD="X86" \
                  -DLLVM_BINUTILS_INCDIR=${BINUTILS_INC_DIR} \
                  ../../classic-flang-llvm-project/llvm || exit 1
        fi
        nice -n 10 ninja -j8 install || exit 1
        popd
fi


if ! llvm_lit="$(realpath --canonicalize-existing $PWD/build/llvm/bin/llvm-lit 2>/dev/null)" || \
    [ ! -x "$llvm_lit" ]; then
  echo "no usable llvm-lit"
  exit 1
fi


if $BUILD_LIBPGMATH; then
        mkdir -p build/libpgmath
        pushd build/libpgmath
        cmake -G "Ninja" \
          -DCMAKE_BUILD_TYPE=$buildtype \
          -DCMAKE_INSTALL_PREFIX=$installdir \
          -DCMAKE_C_COMPILER=$installdir/bin/clang \
          -DCMAKE_CXX_COMPILER=$installdir/bin/clang++ \
          -DLLVM_CONFIG=$installdir/bin/llvm-config \
          -DLLVM_TARGETS_TO_BUILD="X86" \
          -DLIBPGMATH_LLVM_LIT_EXECUTABLE=$llvm_lit \
          ../../flang/runtime/libpgmath || exit 1
        nice -n 10 ninja -j1 check-libpgmath install || exit 1
        popd
fi


if $BUILD_FLANG; then
        mkdir -p build/flang
        pushd build/flang
        cmake -Wno-dev -G 'Unix Makefiles' \
          -DCMAKE_BUILD_TYPE=$buildtype \
          -DCMAKE_INSTALL_PREFIX=$installdir \
          -DCMAKE_C_COMPILER=$installdir/bin/clang \
          -DCMAKE_CXX_COMPILER=$installdir/bin/clang++ \
          -DCMAKE_Fortran_COMPILER=$installdir/bin/flang \
          -DCMAKE_Fortran_COMPILER_ID=Flang \
          -DLLVM_CONFIG=$installdir/bin/llvm-config \
          -DLLVM_TARGETS_TO_BUILD="X86" \
          -DLLVM_EXTERNAL_LIT=$llvm_lit \
          -DFLANG_LLVM_EXTENSIONS=ON \
          ../../flang || exit 1
        nice -n 10 make -j8 install || exit 1
        nice -n 10 make -j1 check-all || exit 1
        popd
fi


echo "SUCCESS"
