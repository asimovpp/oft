#

find_package(Python2 COMPONENTS Interpreter)

if(NOT Python2_FOUND)
  message(WARNING "Could not find Python2; skipping tests")
  return()
endif()

if(NOT EXISTS ${LLVM_TOOLS_BINARY_DIR}/FileCheck)
  message(WARNING "Could not find FileCheck; skipping tests")
  return()
endif()

# aggregate unit test targets under a pseudo-target
add_custom_target(check)

set(LIT_CONFIG_FILE "lit.cfg")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${LIT_CONFIG_FILE}.in"
               ${LIT_CONFIG_FILE})

add_custom_target(
  lit_tests
  COMMAND ${Python2_EXECUTABLE} -m lit.main ${CMAKE_CURRENT_BINARY_DIR} -v
  COMMENT "This might require installation of lit via \"pip install lit\"")

add_dependencies(lit_tests ${OBJ_LIB_NAME})

add_dependencies(check lit_tests)
