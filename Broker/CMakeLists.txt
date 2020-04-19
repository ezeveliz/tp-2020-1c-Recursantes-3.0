cmake_minimum_required(VERSION 3.5)
project(broker C)

set(CMAKE_C_STANDARD 99)

include_directories("/usr/include/commons" "/usr/include/commLib")

add_executable(broker broker.c broker.h )

target_link_libraries(broker "/usr/lib/libcommons.so" pthread "/usr/lib/libcommLib.so")