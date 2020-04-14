cmake_minimum_required(VERSION 3.5)
project(commLib C)

set(CMAKE_C_STANDARD 99)

include_directories("/usr/include/commons")

add_library(commLib SHARED connections.c connections.h structures.h)

target_link_libraries(commLib "/usr/lib/libcommons.so" pthread)

install(TARGETS commLib DESTINATION /usr/lib)
install(FILES connections.h structures.h DESTINATION /usr/include/commLib)