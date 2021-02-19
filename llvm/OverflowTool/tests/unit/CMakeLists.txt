#

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

set(UNIT_TEST_TARGET UnitTests)

set(UNIT_TEST_SOURCES UnitTestsMain.cpp ManualAnnotationTest.cpp)

add_executable(${UNIT_TEST_TARGET} ${UNIT_TEST_SOURCES})

target_link_libraries(${UNIT_TEST_TARGET} PRIVATE ${OBJ_LIB_NAME} ${LLVM_LIBS}
                                                  Catch2::Catch2)

target_include_directories(${UNIT_TEST_TARGET}
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(${UNIT_TEST_TARGET}
                           PRIVATE CATCH_CONFIG_FAST_COMPILE)

# make tests discoverable via CTest
catch_discover_tests(${UNIT_TEST_TARGET})
