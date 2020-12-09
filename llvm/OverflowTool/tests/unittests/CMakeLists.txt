# cmake file

# some useful doc on testing practices
# https://cliutils.gitlab.io/modern-cmake/chapters/testing.html

#
# use standard CMake modules
include(FetchContent)

fetchcontent_declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v2.13.3)

fetchcontent_makeavailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)
include(Catch)

# disable Catch options since it's a subproject
set(CATCH_BUILD_TESTING OFF)
set(CATCH_INSTALL_DOCS OFF)
set(CATCH_INSTALL_HELPERS OFF)

# TODO qualify these names or add a namespace
add_executable(unittests test.cpp)

target_link_libraries(unittests PRIVATE ${OBJ_LIB_NAME} ${LLVM_LIBS}
                                        Catch2::Catch2)

# make tests discoverable via CTest
catch_discover_tests(unittests)
