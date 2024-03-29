#

find_package(Python2 COMPONENTS Interpreter)

set(CONFIG_STATUS 1)
set(CONFIG_STATUS_MESSAGE "")

if(NOT Python2_FOUND)
  set(MSG "Could not find Python2")
  message(WARNING ${MSG})

  set(CONFIG_STATUS 0)
  string(APPEND CONFIG_STATUS_MESSAGE ${MSG} "\n")
endif()

set(FILECHECK_PATH "${LLVM_TOOLS_BINARY_DIR}/FileCheck")

if(NOT EXISTS ${FILECHECK_PATH})
  set(MSG "Could not find ${FILECHECK_PATH}")
  message(WARNING ${MSG})

  set(CONFIG_STATUS 0)
  string(APPEND CONFIG_STATUS_MESSAGE ${MSG} "\n")
endif()

# aggregate unit test targets under a pseudo-target
add_custom_target(check)

# create a dummy target that just prints a message when configuration failed
# this is more obvious than just a silent failure during configurarion
if(NOT CONFIG_STATUS)
  set(ERROR_MSG_FILE "${CMAKE_CURRENT_BINARY_DIR}/error_msg.txt")

  file(WRITE "${ERROR_MSG_FILE}" ${CONFIG_STATUS_MESSAGE})

  add_custom_target(
    regression_tests_message
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Tests disabled due to config errors: "
    COMMAND ${CMAKE_COMMAND} -E cat "${ERROR_MSG_FILE}")

  add_dependencies(check regression_tests_message)

  return()
endif()

set(LIT_CONFIG_FILE "lit.cfg")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${LIT_CONFIG_FILE}.in"
               ${LIT_CONFIG_FILE})

add_custom_target(
  lit_tests
  COMMAND ${Python2_EXECUTABLE} -m lit.main ${CMAKE_CURRENT_BINARY_DIR} -v --no-progress-bar
  COMMENT "This might require installation of lit via \"pip install lit\"")

add_dependencies(lit_tests ${OBJ_LIB_NAME})

add_dependencies(check lit_tests)
