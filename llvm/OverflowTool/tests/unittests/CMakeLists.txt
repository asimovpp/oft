# cmake file

#
# use standard CMake modules
include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v2.13.3)

FetchContent_MakeAvailable(Catch2)

# TODO qualify these names or add a namespace
add_executable(unittests test.cpp)

target_link_libraries(unittests Catch2::Catch2)

