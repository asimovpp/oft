# cmake file

find_package(MPI REQUIRED)
if(NOT MPI_FOUND)
    message(WARNING "Could not find MPI. Skipping building of the runtime library.")
    return()
endif()

set(PRJ_RT_NAME oft_rt)

set(SOURCES "src/oft_mark.c"
            "src/instrumentation_functions.c")

add_library(${PRJ_RT_NAME} STATIC ${SOURCES})

target_link_libraries(${PRJ_RT_NAME} PUBLIC MPI::MPI_C)

install(
  TARGETS ${PRJ_RT_NAME}
  EXPORT ${PRJ_NAME}
  ARCHIVE DESTINATION "runtime/lib"
  LIBRARY DESTINATION "runtime/lib")
