# cmake file

set(PRJ_RT_NAME oft_rt)

set(SOURCES "src/oft_mark.c")

add_library(${PRJ_RT_NAME} STATIC ${SOURCES})

install(
  TARGETS ${PRJ_RT_NAME}
  EXPORT ${PRJ_NAME}
  ARCHIVE DESTINATION "runtime/lib"
  LIBRARY DESTINATION "runtime/lib")
