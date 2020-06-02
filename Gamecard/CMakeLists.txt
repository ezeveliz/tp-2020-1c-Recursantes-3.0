cmake_minimum_required(VERSION 3.5)
#project(gamecard C)
project(tallGrass C)
set(CMAKE_C_STANDARD 99)

include_directories("/usr/include/commLib" "/usr/include/commons")

#add_executable(gamecard gamecard.c gamecard.h structures.h tallGrass.c tallGrass.h)
add_executable(tallGrass tallGrass.c tallGrass.h)

#target_link_libraries(gamecard  "/usr/lib/libcommLib.so" "/usr/lib/libcommons.so" pthread)
target_link_libraries(tallGrass  "/usr/lib/libcommLib.so" "/usr/lib/libcommons.so" pthread)