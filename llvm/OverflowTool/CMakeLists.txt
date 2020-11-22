# cmake file

cmake_minimum_required(VERSION 3.11.0)

#
# handle policies

# CMP0056 is required for any libc++abi dependency of the try_compile() command
# this bumps required version to 3.2 otherwise LD_LIBRARY_DIR needs to be set
if(POLICY CMP0056)
  cmake_policy(SET CMP0056 NEW)
endif()

# CMP0077 is used to handle option() and other variables with the same name.
# With the new behaviour the option() variable does nothing, yielding over the
# user-specified variable with the same name. This caused warnings when
# importing the cmake option() variables set in the LLVM cmake config on which
# we are dependent, even though we don't override those variables, but merely
# comply with them. This was added with CMake 3.13
if(POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif()

# enable project-specific debug if build mode is also debug
set(PRJ_DEBUG_VALUE 0)
string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE)

if(${CMAKE_BUILD_TYPE} MATCHES "DEBUG")
  set(PRJ_DEBUG_VALUE 1)
endif()

set(PRJ_VERSION "0.0.1")

set(PRJ_NAME OverflowTool)
string(TOLOWER ${PRJ_NAME} PRJ_NAME_LOWER)

project(${PRJ_NAME} C CXX)

#
# use standard CMake modules
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(OVERFLOWTOOL_EXPORT ${PRJ_NAME})

#
# requirements

#
# detect LLVM

# FIXME hardcode out-of-source mode for now
set(STANDALONE_BUILD ON)

if(STANDALONE_BUILD)
  # CONFIG option influences search towards the location pointed by LLVM_DIR
  find_package(LLVM 7 REQUIRED CONFIG)

  if(NOT LLVM_FOUND)
    message(FATAL_ERROR "Could not find LLVM")
  endif()

  message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
  message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

  # TODO determine how much of that we can actually use required if these
  # functions are used: add_llvm_loadable_module(), add_llvm_library
  list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")

  include(AddLLVM)
  include(HandleLLVMOptions)

  #
  # detect LLVM's shared library mode

  get_target_property(EXTERNAL_LLVM_SHARED_MODE LLVMSupport TYPE)

  if(EXTERNAL_LLVM_SHARED_MODE)
    message(STATUS "Detected LLVM shared mode: ${EXTERNAL_LLVM_SHARED_MODE}")
  else()
    message(WARNING "Cannot determine shared mode from LLVMSupport library.")
  endif()

  set(PRJ_SHARED_MODE "")

  if(${EXTERNAL_LLVM_SHARED_MODE} STREQUAL "SHARED_LIBRARY"
     OR ${EXTERNAL_LLVM_SHARED_MODE} STREQUAL "MODULE_LIBRARY")
    set(PRJ_SHARED_MODE "MODULE")
    message(STATUS "Plugin shared mode: ${PRJ_SHARED_MODE}")
  endif()
endif()

# TODO: maybe simplify these and get them from llvm-config --cxxflags
if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

message(STATUS "RTTI support: ${LLVM_ENABLE_RTTI}")

if(NOT LLVM_ENABLE_EH)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
endif()

message(STATUS "EH support: ${LLVM_ENABLE_EH}")

# when we are building the project as standalone, we'd like to leave the install
# RPATH empty so that potentially we can dynamically link against other
# installed LLVM toolchains

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

if(CMAKE_VERSION VERSION_EQUAL "3.8.0" OR CMAKE_VERSION VERSION_GREATER "3.8.0")
  set(CMAKE_BUILD_RPATH ${LLVM_LIBRARY_DIRS})
else()
  # unfortunately this also sets the install RPATH for older cmake versions
  # set(PRJ_LINKER_FLAGS "${PRJ_LINKER_FLAGS} -Wl,-rpath ${LLVM_LIBRARY_DIRS}")
endif()

set(PRJ_LINKER_FLAGS "${PRJ_LINKER_FLAGS} -Wl,--disable-new-dtags")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${PRJ_LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} ${PRJ_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS
    "${CMAKE_MODULE_LINKER_FLAGS} ${PRJ_LINKER_FLAGS}")

set(LIB_SOURCES
    "lib/PassSupport/RegisterPasses.cpp"
    "lib/PassSupport/PassCommandLineOptions.cpp"
    "lib/Analysis/ManualAnnotationSelection.cpp"
    "lib/Analysis/OverflowTracking.cpp"
    "lib/Analysis/Passes/ManualAnnotationSelectionPass.cpp")

set(LIB_NAME "LLVM${PRJ_NAME}Pass")
set(OBJ_LIB_NAME "${LIB_NAME}_obj")
set(TEST_LIB_NAME "${LIB_NAME}_test")

list(APPEND PRJ_LIB_NAMES ${OBJ_LIB_NAME} ${LIB_NAME} ${TEST_LIB_NAME})

add_library(${OBJ_LIB_NAME} OBJECT ${LIB_SOURCES})
add_library(${LIB_NAME} ${PRJ_SHARED_MODE} $<TARGET_OBJECTS:${OBJ_LIB_NAME}>)
add_library(${TEST_LIB_NAME} STATIC $<TARGET_OBJECTS:${OBJ_LIB_NAME}>)

# TODO this is apparently resolved in cmake 3.12 and later see
# https://cmake.org/cmake/help/latest/release/3.12.html decide how to simplify
# this for now

# workaround for cmake's current inability to transitively provide requirements
# from an object library to its users
foreach(PRJ_LIB ${PRJ_LIB_NAMES})
  set_target_properties(
    ${PRJ_LIB} PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON
                          CXX_EXTENSIONS OFF)

  set_target_properties(${PRJ_LIB} PROPERTIES LIBRARY_OUTPUT_DIRECTORY
                                              ${CMAKE_INSTALL_LIBDIR})

  target_compile_options(${PRJ_LIB} PRIVATE "-pedantic")
  target_compile_options(${PRJ_LIB} PRIVATE "-Wall")
  target_compile_options(${PRJ_LIB} PRIVATE "-Wextra")
  target_compile_options(${PRJ_LIB} PRIVATE "-Wno-unused-parameter")
  target_compile_options(${PRJ_LIB} PRIVATE "-Wno-unused-function")

  target_compile_definitions(${PRJ_LIB} PUBLIC ${LLVM_DEFINITIONS})
  target_compile_definitions(${PRJ_LIB} PRIVATE VERSION_STRING=${PRJ_VERSION})

  target_include_directories(${PRJ_LIB} PUBLIC ${LLVM_INCLUDE_DIRS})
  target_include_directories(
    ${PRJ_LIB} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
  target_include_directories(
    ${PRJ_LIB} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>)
  target_include_directories(${PRJ_LIB} PUBLIC $<INSTALL_INTERFACE:include>)

  target_include_directories(
    ${PRJ_LIB}
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/include>)
endforeach()

# workaround for OBJECT libraries since they are not first class citizens should
# be able to use target_link_libraries() against an INTERFACE library
# target_include_directories(${OBJ_LIB_NAME} PUBLIC $<TARGET_PROPERTY:foo-
# interface-lib,INTERFACE_INCLUDE_DIRECTORIES>)

target_include_directories(
  ${LIB_NAME}
  PUBLIC $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${PATH_PREFIX}/include>)

if(${EXTERNAL_LLVM_SHARED_MODE} STREQUAL "STATIC_LIBRARY")
  # according to http://llvm.org/docs/CMake.html we do not need the below
  # library dependencies since the plugin will be loaded via opt or clang which
  # already have those libs in their dependencies
  llvm_map_components_to_libnames(LLVM_LIBS core support analysis passes)

  target_link_libraries(${TEST_LIB_NAME} PUBLIC ${LLVM_LIBS})

  target_link_libraries(${LIB_NAME} PUBLIC ${LLVM_LIBS})
endif()

# TODO document this
set(DEPENDEE "")

#
# generated files

set(PRJ_CONFIG_HEADER_FILE "Config.hpp")
set(PRJ_CMAKE_CONFIG_FILE "config.cmake")
set(PRJ_CMAKE_CONFIG_EXTRA_FILE "config-extra.cmake")

set(BIN_SCRIPTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/utils/scripts/run/)

# GNUInstallDirs provides: CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_DATAROOTDIR
# CMAKE_INSTALL_LIBDIR This is based on where files are expected on *nix
# filesystems: https://cmake.org/cmake/help/latest/command/find_package.html
# https://stackoverflow.com/a/55863486/3048763

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_INCLUDEDIR}")

configure_file(
  "${CMAKE_INSTALL_INCLUDEDIR}/${PRJ_NAME}/${PRJ_CONFIG_HEADER_FILE}.in"
  "${CMAKE_INSTALL_INCLUDEDIR}/${PRJ_NAME}/${PRJ_CONFIG_HEADER_FILE}")

configure_file(
  "${CMAKE_INSTALL_DATAROOTDIR}/cmake/${PRJ_CMAKE_CONFIG_FILE}.in"
  "${CMAKE_INSTALL_LIBDIR}/cmake/${PRJ_NAME_LOWER}-${PRJ_CMAKE_CONFIG_FILE}"
  @ONLY)

configure_file(
  "${CMAKE_INSTALL_DATAROOTDIR}/cmake/${PRJ_CMAKE_CONFIG_EXTRA_FILE}.in"
  "${CMAKE_INSTALL_LIBDIR}/cmake/${PRJ_NAME_LOWER}-${PRJ_CMAKE_CONFIG_EXTRA_FILE}"
  @ONLY)

# gather up all .in templates in the scripts dir
file(
  GLOB BIN_SCRIPTS
  LIST_DIRECTORIES FALSE
  RELATIVE ${BIN_SCRIPTS_DIR}
  ${BIN_SCRIPTS_DIR}/*.in)

foreach(BIN_SCRIPT_TEMPLATE ${BIN_SCRIPTS})
  get_filename_component(BIN_SCRIPT ${BIN_SCRIPT_TEMPLATE} NAME_WE)
  configure_file("${BIN_SCRIPTS_DIR}/${BIN_SCRIPT_TEMPLATE}"
                 "${CMAKE_INSTALL_BINDIR}/${BIN_SCRIPT}" @ONLY)
endforeach()

write_basic_package_version_file(
  "${CMAKE_INSTALL_DATAROOTDIR}/cmake/${PRJ_NAME_LOWER}-config-version.cmake"
  VERSION "${PRJ_VERSION}"
  COMPATIBILITY SameMajorVersion)

#
# installation

set(CMAKE_INSTALL_MESSAGE "LAZY")

install(
  TARGETS ${LIB_NAME}
  EXPORT ${OVERFLOWTOOL_EXPORT}
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")

install(
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  PATTERN "*.in" EXCLUDE)

install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/share/"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}")

install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}")

install(
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin/"
  DESTINATION "${CMAKE_INSTALL_BINDIR}"
  USE_SOURCE_PERMISSIONS)

#

install(
  EXPORT ${OVERFLOWTOOL_EXPORT}
  FILE ${PRJ_NAME_LOWER}.cmake
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake")
