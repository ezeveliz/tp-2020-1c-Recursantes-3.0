cmake_minimum_required(VERSION 3.5)
project(team C)

set(CMAKE_C_STANDARD 99)

include_directories("/usr/include/commons")

add_executable(team team.c team.h teamStructures.h)

target_link_libraries(team "/usr/lib/libcommons.so" pthread)