cmake_minimum_required(VERSION 3.5)
project(team C)

set(CMAKE_C_STANDARD 99)

include_directories("/usr/include/commons" "/usr/include/commLib")

add_executable(servidor_simulacion servidor_simulacion.c servidor_simulacion.h)

target_link_libraries(servidor_simulacion "/usr/lib/libcommons.so" "/usr/lib/libcommLib.so" pthread )