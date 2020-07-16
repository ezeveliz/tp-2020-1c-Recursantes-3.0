//
// Created by emi on 31/5/20.
//

#ifndef GAMECARD_TALLGRASS_H
#define GAMECARD_TALLGRASS_H
#define BLOCK_SIZE 64
#define BLOCKS 1024
#define MAGIC_NUMBER "TALL_GRASS"

#include <commLib/connections.h>
#include <commLib/structures.h>
#include <commons/collections/list.h>
#include <string.h>
#include "funciones_aux.h"
#include <sys/file.h>
#include <commons/config.h>
#include <pthread.h>
#include <semaphore.h>

#endif //GAMECARD_TALLGRASS_H

typedef struct{
    uint32_t size;
    char* bloques;
}t_metadata;

typedef struct{
    char* path;
    t_metadata* metadata;
    int pos;
} t_file;


//Para obtenr path
char* obtener_path_file();
char* obtener_path_blocks();
char* obtener_path_metadata();
char* obtener_path_bitmap();

//Para montar
int montar(char* punto_montaje);
void limpiar_unidades_antiguas(char* path);
int crear_metadata(char* path);
int crear_blocks(char* path);
int crear_file(char* path);

//Funciones
int rmdir_tall_grass(const char *path);
int mkdir_tall_grass(char* path);
int create_tall_grass(char* path);
t_list* ls_tall_grass(char* path);
bool find_tall_grass(char* nombre_archivo);
t_file* open_tall_grass(char* path);
int close_tall_grass( t_file * fd );
char* read_tall_grass(t_file* archivo, uint32_t size_a_leer, uint32_t posicion);
int write_tall_grass(t_file* archivo, char* datos_escribir, uint32_t size_a_escribir, uint32_t posicion_dentro_archivo);
int truncate_tall_grass(t_file* archivo, uint32_t off_set);
int delet_tall_grass(t_file* archivo, uint32_t off_set, uint32_t cantidad_byte);

//Laburan sobre metadata archivo
int next(t_file* archivo);
int set_estado_archivo(FILE* archivo,char estado);
void agregar_byte_archivo(t_file* archivo, int cantidad);
void disminuir_byte_archivo(t_file* archivo, int cantidad);
void agregar_bloque_archivo(t_file* archivo, uint32_t bloque);
void sacar_bloques_metadata(t_file* archivo,  uint32_t pos_final_archivo);

//Funciones para obtener distintos elementos del fileSystem
int obtener_cantidad_bloques();
int obtener_tamanio_bloques();
t_list* obtener_bloques_libres(int cantidad_pedida);
int liberar_bloque(uint32_t nro_bloque);
FILE* obtener_file_bloque(int numero_bloque,char* flag);
int obtener_bloque(char* bloques,int posicion);

//Funciones sobre metadata
t_metadata* obtener_metadata_archivo(char* file);
void metadata_destroy(t_metadata* metadata);

//Quedo en desuso cuando descubri que podia usat t_config
//busca la posicion de un caracter en un archivo
int buscar_caracter_archivo(FILE* archivo, char caracter_a_buscar , int numero_de_aparicion);


int calcular_bloques_archivo(t_metadata* metadata);

//Funciones array
int contar_elementos_array(char** array);
int liberar_elementos_array(char** array);
int bloque_relativo_archivo(int posicion);
char** cortar_bloques_array(char* array);


//Manejo lista de archivos abiertos
int agregar_archivo_abierto(char* path);
int buscar_archivo_abierto(char* path);
int sacar_lista_archivos_abiertos(char* path);