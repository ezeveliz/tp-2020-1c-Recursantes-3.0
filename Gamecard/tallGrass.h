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
#include <string.h>
#include "funciones_aux.h"
#include <sys/file.h>
#include <commons/config.h>

#endif //GAMECARD_TALLGRASS_H

typedef struct{
    uint32_t size;
    char* bloques;
}t_metadata;

typedef struct{
    char* path;
} t_file;

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
char* obtener_path_bitmap();
t_list* ls_tall_grass(char* path);
bool find_tall_grass(char* nombre_archivo);

t_file* open_tall_grass(char* path);
int close_tall_grass( t_file * fd );
int set_estado_archivo(FILE* archivo,char estado);
int buscar_caracter_archivo(FILE* archivo, char caracter_a_buscar , int numero_de_aparicion);

int obtener_cantidad_bloques();
int obtener_tamanio_bloques();
t_list* obtener_bloques_libres(int cantidad_pedida);

int calcular_espacio_libre_archivo(t_metadata* metadata);

int espacio_libre_archivo();

int calcular_bloques_archivo(t_metadata* metadata);

t_metadata* obtener_metadata_archivo(t_file * archivo);
void metadata_destroy(t_metadata* metadata);

FILE* obtener_file_bloque(int numero_bloque);
int obtener_bloque(char* bloques,int posicion);
int agregar_bloque_archivo(t_file* archivo, uint32_t bloque);

write_tall_grass(t_file* archivo, char* datos_escribir, uint32_t size_a_escribir, uint32_t posicion_dentro_archivo);

int calcular_bloques(int byts);
void agregar_byte_archivo(t_file* archivo, int cantidad);