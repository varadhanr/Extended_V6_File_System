cmake_minimum_required(VERSION 3.10)
project(V6FileSystem)
add_executable(V6FileSystem main.c)
project(V6FileSystem VERSION 1.0)
configure_file(V6FileSystemConfig.h.in V6FileSystemConfig.h)
target_include_directories(V6FileSystem PUBLIC "${PROJECT_BINARY_DIR}")
SET(GCC_COVERAGE_LINK_FLAGS  "-lm")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

set(BINARY ${CMAKE_PROJECT_NAME})

file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp *.c)

set(SOURCES ${SOURCES})

add_executable(${BINARY}_run ${SOURCES})

add_library(${BINARY}_lib STATIC ${SOURCES})
