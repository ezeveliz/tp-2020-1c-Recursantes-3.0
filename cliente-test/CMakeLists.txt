cmake_minimum_required(VERSION 3.5)
project(cliente_test C)

set(CMAKE_C_STANDARD 99)

include_directories("/usr/include/commons" "/usr/include/commLib")

add_executable(cliente-test cliente-test.c cliente-test.h)

target_link_libraries(cliente-test "/usr/lib/libcommons.so" pthread "/usr/lib/libcommLib.so")