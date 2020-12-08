# cmake file

# some useful doc on testing practices
# https://cliutils.gitlab.io/modern-cmake/chapters/testing.html

# TODO ways to determine enabling/disabling testing policy from other projects:
# - https://github.com/google/re2/issues/138 -
# https://github.com/CLIUtils/CLI11/issues/273#issuecomment-491967380

#
# use standard CMake modules
include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v2.13.3)

FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)
include(Catch)

# disable Catch options since it's a subproject
set(CATCH_BUILD_TESTING OFF)
set(CATCH_INSTALL_DOCS OFF)
set(CATCH_INSTALL_HELPERS OFF)

# TODO qualify these names or add a namespace
add_executable(unittests test.cpp)

target_link_libraries(unittests Catch2::Catch2)

# make tests discoverable via CTest
catch_discover_tests(unittests)
