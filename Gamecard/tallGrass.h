//
// Created by emi on 31/5/20.
//

#ifndef GAMECARD_TALLGRASS_H
#define GAMECARD_TALLGRASS_H
#define BLOCK_SIZE 5
#define BLOCKS 64
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
void limpiar_unidades_antiguas(char* path);
int crear_metadata(char* path);
int crear_blocks(char* path);
int crear_file(char* path);
int crear_carpeta(char* path, int modo);
int remove_directory(const char *path);
char* concatenar_strings(char*path1, char* path2);
t_bitarray* create_bitmap(int cantidad_bloques);
void doom_bitmap(t_bitarray* bitarray);
void limpiar_bitmpa(t_bitarray* bitarray);
int tamanio_bitmap(int cantidad_bloques);
char* obtener_bitmap(FILE* archivo_bitmap);
void escribir_bitmap(t_bitarray* bitmap, FILE* archivo);