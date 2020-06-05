//
// Created by emi on 31/5/20.
//

#ifndef GAMECARD_TALLGRASS_H
#define GAMECARD_TALLGRASS_H
#define BLOCK_SIZE 5
#define BLOCKS 64
#define MAGIC_NUMBER "TALL_GRASS"


#include <commLib/connections.h>
#include <commLib/structures.h>
#include <commons/collections/list.h>
#include "funciones_aux.h"

#endif //GAMECARD_TALLGRASS_H

int montar(char* punto_montaje);
void limpiar_unidades_antiguas(char* path);
int crear_metadata(char* path);
int crear_blocks(char* path);
int crear_file(char* path);
int rmdir_tall_grass(const char *path);
int mkdir_tall_grass(char* path);
int create_tall_grass(char* path);

char* obtener_path_file();
char* obtener_path_blocks();
char* obtener_path_metadata();
t_list* ls_tall_grass(char* path);
bool find_tall_grass(char* nombre_archivo);