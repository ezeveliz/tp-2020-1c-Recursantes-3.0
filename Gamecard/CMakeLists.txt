cmake_minimum_required(VERSION 3.5)
project(gamecard C)

set(CMAKE_C_STANDARD 99)

include_directories("/usr/include/commons")

add_executable(gamecard gamecard.c gamecard.h)

target_link_libraries(gamecard "/usr/lib/libcommons.so" pthread)