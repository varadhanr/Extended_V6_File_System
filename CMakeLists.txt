cmake_minimum_required(VERSION 3.10)
project(V6FileSystem)
add_executable(V6FileSystem fsaccess.c)
project(V6FileSystem VERSION 1.0)
configure_file(V6FileSystemConfig.h.in V6FileSystemConfig.h)
target_include_directories(V6FileSystem PUBLIC "${PROJECT_BINARY_DIR}")

