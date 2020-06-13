//
// Created by emi on 5/6/20.
//

#ifndef TALLGRASS_FUNCIONES_AUX_H
#define TALLGRASS_FUNCIONES_AUX_H

#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <commons/bitarray.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#endif //TALLGRASS_FUNCIONES_AUX_H

t_bitarray* create_bitmap(int cantidad_bloques);
int tamanio_bitmap(int cantidad_bloques);
void doom_bitmap(t_bitarray* bitarray);
void limpiar_bitmpa(t_bitarray* bitarray);
char* obtener_bitmap(FILE* archivo_bitmap, int cant_bloques);
int escribir_bitmap(t_bitarray* bitmap, FILE* archivo);
int crear_carpeta(char* path, int modo);
char* concatenar_strings(char*path1, char* path2);