file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp *.c)
cmake_minimum_required(VERSION 3.10)
project(V6FileSystem)

set(CMAKE_CXX_STANDARD 14)

include_directories(src)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(lib/googletest)
