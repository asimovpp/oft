#!/usr/bin/env bash

PRJ_ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../" && pwd)"
SRC_DIR=${1:-$PRJ_ROOT_DIR}
INSTALL_DIR=${2:-../install/}

source $PRJ_ROOT_DIR/utils/scripts/build/export_deps.sh

# Note: we require word splitting in CMAKE_OPTIONS, so leave it unquoted

cmake \
  -G Ninja \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=On \
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
  -DBUILD_SHARED_LIBS="${SHARED_LIBS}" \
  -DCMAKE_CXX_FLAGS="${CXX_FLAGS}" \
  -DCMAKE_EXE_LINKER_FLAGS="${LINKER_FLAGS}" \
  -DCMAKE_SHARED_LINKER_FLAGS="${LINKER_FLAGS}" \
  -DCMAKE_MODULE_LINKER_FLAGS="${LINKER_FLAGS}" \
  -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=On \
  -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
  -DOVERFLOWTOOL_BUILD_TESTING=On \
  ${CMAKE_OPTIONS} \
  "${SRC_DIR}"

