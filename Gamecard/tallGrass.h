//
// Created by emi on 31/5/20.
//

#ifndef GAMECARD_TALLGRASS_H
#define GAMECARD_TALLGRASS_H
#define BLOCK_SIZE 5
#define BLOCKS 5192
#define MAGIC_NUMBER "TALL_GRASS"

#include <sys/types.h>
#include <sys/stat.h>
#include <commLib/connections.h>
#include <commLib/structures.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#endif //GAMECARD_TALLGRASS_H

int montar(char* punto_montaje);
int crear_estructura_carpetas(char* punto_montaje);
int remove_directory(const char *path);
char* concatenar_strings(char*path1, char* path2);
int crear_carpeta(char* path, int modo);
int crear_metadata(char* path);
t_bitarray* create_bitmap(int cantidad_bloques);