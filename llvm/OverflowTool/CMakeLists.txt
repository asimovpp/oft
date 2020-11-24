# cmake file

if(NOT DEFINED LLVM_MAIN_SRC_DIR)
  set(STANDALONE_BUILD ON)
  cmake_minimum_required(VERSION 3.12.0)
endif()

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

# use the lowercase name of the project for exporting CMake targets and naming
# the related files
string(TOLOWER ${PRJ_NAME} PRJ_NAME_LOWER)

if(STANDALONE_BUILD)
  project(${PRJ_NAME} C CXX)
endif()

#
# use standard CMake modules
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# name of the export set of this CMake package
set(OVERFLOWTOOL_EXPORT ${PRJ_NAME})

#
# requirements

#
# detect LLVM

if(STANDALONE_BUILD)
  # CONFIG option influences search towards the location pointed by LLVM_DIR
  find_package(LLVM 7 REQUIRED CONFIG)

  if(NOT LLVM_FOUND)
    message(FATAL_ERROR "Could not find LLVM")
  endif()

  message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
  message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

  # if these functions are used: add_llvm_loadable_module, add_llvm_library,
  # add_llvm_plugin_pass
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

  # these compile options can also be acquired and processed externally using
  # llvm-config --cxxflags
  if(NOT LLVM_ENABLE_RTTI)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
  endif()

  message(STATUS "RTTI support: ${LLVM_ENABLE_RTTI}")

  if(NOT LLVM_ENABLE_EH)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
  endif()

  message(STATUS "EH support: ${LLVM_ENABLE_EH}")
endif()

# if we are building the project as standalone and we'd like to leave the
# install RPATH empty so that potentially we can dynamically link against other
# installed LLVM toolchains

# set(CMAKE_SKIP_BUILD_RPATH FALSE)

# set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# this requires version 3.8 or later and only affects the RPATH/RUNPATH for the
# build tree shared object for older version user *_LINKER_FLAGS

# set(CMAKE_BUILD_RPATH ${LLVM_LIBRARY_DIRS}) set(PRJ_LINKER_FLAGS
# "${PRJ_LINKER_FLAGS} -Wl,-rpath ${LLVM_LIBRARY_DIRS}")

set(LIB_SOURCES
    "lib/PassSupport/RegisterPasses.cpp"
    "lib/PassSupport/PassCommandLineOptions.cpp"
    "lib/Analysis/ManualAnnotationSelection.cpp"
    "lib/Analysis/OverflowTracking.cpp"
    "lib/Analysis/Passes/ManualAnnotationSelectionPass.cpp")

set(LIB_NAME "LLVM${PRJ_NAME}Pass")
set(OBJ_LIB_NAME "${LIB_NAME}.obj")

# create a object library to avoid potential recompilations
add_library(${OBJ_LIB_NAME} OBJECT ${LIB_SOURCES})

# set properties on the object library which can be transitively passed to users
# since CMake 3.12 and later
# https://cmake.org/cmake/help/latest/release/3.12.html

target_compile_definitions(${OBJ_LIB_NAME} PUBLIC ${LLVM_DEFINITIONS})
target_compile_definitions(${OBJ_LIB_NAME} PRIVATE
                           VERSION_STRING=${PRJ_VERSION})

target_include_directories(${OBJ_LIB_NAME} PUBLIC ${LLVM_INCLUDE_DIRS})

target_include_directories(
  ${OBJ_LIB_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_include_directories(
  ${OBJ_LIB_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>)
target_include_directories(
  ${OBJ_LIB_NAME}
  PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/include>)

if(${EXTERNAL_LLVM_SHARED_MODE} STREQUAL "STATIC_LIBRARY")
  llvm_map_components_to_libnames(LLVM_LIBS core support analysis passes)
  target_link_libraries(${LIB_NAME} PUBLIC ${LLVM_LIBS})
endif()

# this option controls if the object file will use the RPATH or RUNPATH for
# detecting other shared object dependencies man 8 ld.so for details on the
# fields (DT_RPATH and DT_RUNPATH) uncomment this to use RPATH if using CMake
# 3.13 and later for older CMake versions append to the
# CMAKE_SHARED_LINKER_FLAGS, CMAKE_MODULE_LINKER_FLAGS, CMAKE_EXE_LINKER_FLAGS
# accordingly

# here is another reason that we might want to enforce this: https://reviews.llvm.org/D42390

# target_link_options(${OBJ_LIB_NAME} PUBLIC "-Wl,--disable-new-dtags")

# create plugin library
add_library(${LIB_NAME} ${PRJ_SHARED_MODE} $<TARGET_OBJECTS:${OBJ_LIB_NAME}>)

# GNUInstallDirs provides: CMAKE_INSTALL_INCLUDEDIR

target_include_directories(
  ${LIB_NAME} PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

set_target_properties(${LIB_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY
                                             ${CMAKE_INSTALL_LIBDIR})

# this allows to "inject" dependencies in config-extra.cmake.in if needed for
# the project installation
set(DEPENDEE "")

#
# generated files

# GNUInstallDirs provides: CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_DATAROOTDIR
# CMAKE_INSTALL_LIBDIR This is based on where files are expected on *nix
# filesystems: https://cmake.org/cmake/help/latest/command/find_package.html
# https://stackoverflow.com/a/55863486/3048763

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_INCLUDEDIR}")

# generate a general configuration header file for the project
set(PRJ_CONFIG_HEADER_FILE "Config.hpp")

configure_file(
  "${CMAKE_INSTALL_INCLUDEDIR}/${PRJ_NAME}/${PRJ_CONFIG_HEADER_FILE}.in"
  "${CMAKE_INSTALL_INCLUDEDIR}/${PRJ_NAME}/${PRJ_CONFIG_HEADER_FILE}")

set(PRJ_CMAKE_CONFIG_FILE "config.cmake")

configure_file(
  "${CMAKE_INSTALL_DATAROOTDIR}/cmake/${PRJ_CMAKE_CONFIG_FILE}.in"
  "${CMAKE_INSTALL_LIBDIR}/cmake/${PRJ_NAME_LOWER}-${PRJ_CMAKE_CONFIG_FILE}"
  @ONLY)

set(PRJ_CMAKE_CONFIG_EXTRA_FILE "config-extra.cmake")

configure_file(
  "${CMAKE_INSTALL_DATAROOTDIR}/cmake/${PRJ_CMAKE_CONFIG_EXTRA_FILE}.in"
  "${CMAKE_INSTALL_LIBDIR}/cmake/${PRJ_NAME_LOWER}-${PRJ_CMAKE_CONFIG_EXTRA_FILE}"
  @ONLY)

# gather up all .in templates in the scripts dir
set(BIN_SCRIPTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/utils/scripts/run/)

file(
  GLOB BIN_SCRIPTS
  LIST_DIRECTORIES FALSE
  RELATIVE ${BIN_SCRIPTS_DIR}
  ${BIN_SCRIPTS_DIR}/*.in)

foreach(BIN_SCRIPT_TEMPLATE ${BIN_SCRIPTS})
  # perform 2-step generation: 1) for normal variable 2) for generator
  # expressions
  get_filename_component(BIN_SCRIPT ${BIN_SCRIPT_TEMPLATE} NAME_WE)

  configure_file("${BIN_SCRIPTS_DIR}/${BIN_SCRIPT_TEMPLATE}"
                 "${CMAKE_INSTALL_BINDIR}-cfg/${BIN_SCRIPT}" @ONLY)

  file(
    GENERATE
    OUTPUT "${CMAKE_INSTALL_BINDIR}/${BIN_SCRIPT}"
    INPUT
      "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}-cfg/${BIN_SCRIPT}")
endforeach()

# this enables exporting the version of the project as a CMake artifact
write_basic_package_version_file(
  "${CMAKE_INSTALL_DATAROOTDIR}/cmake/${PRJ_NAME_LOWER}-config-version.cmake"
  VERSION "${PRJ_VERSION}"
  COMPATIBILITY SameMajorVersion)

#
# installation

# show only the necessary messages when performing an install
set(CMAKE_INSTALL_MESSAGE "LAZY")

# the cmake files installed here are required for filling in the requirements
# for allowing this installation to provide imported targets. See CMake
# documentation on cmake-packages(7)

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

# this allows importing of the installed project in other CMake projects
install(
  EXPORT ${OVERFLOWTOOL_EXPORT}
  FILE ${PRJ_NAME_LOWER}.cmake
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake")
