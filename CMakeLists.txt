cmake_minimum_required(VERSION 3.17)
project(dfio C)

set(CMAKE_C_STANDARD 11)

find_library(LIBM
        NAME m
        PATHS /usr/lib64
        NO_DEFAULT_PATH)

find_package(MPI)
if (MPI_FOUND)
  add_definitions("-DHAVE_MPI")
  message (STATUS "MPI headers at ${MPI_C_INCLUDE_DIRS}")  
  message (STATUS "MPI libs ${MPI_C_LIBRARIES}")  
else()
  message (STATUS "MPI is not defined. Building simple version")  
endif()


include_directories(${LIBM_INCLUDE_DIRS} ${MPI_C_INCLUDE_DIRS})

add_executable(dfio main.c stats.c processor.c dir.c)
target_link_libraries(dfio ${LIBM} ${MPI_C_LIBRARIES})
