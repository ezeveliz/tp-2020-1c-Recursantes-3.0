cmake_minimum_required(VERSION 3.5)
project(gameboy C)

set(CMAKE_C_STANDARD 99)

include_directories("/usr/include/commons")

add_executable(gameboy gameboy.c gameboy.h structures.h)

target_link_libraries(gameboy "/usr/lib/libcommons.so" pthread)