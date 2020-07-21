//
// Created by emi on 5/6/20.
//

#include "funciones_aux.h"

/*
 * Funciones de bitmap
 */

t_bitarray* create_bitmap(int cantidad_bloques){

    int bitmap_size = tamanio_bitmap(cantidad_bloques);

    char* bitmap_string = malloc(bitmap_size);
    t_bitarray * bitarray = bitarray_create(bitmap_string, bitmap_size);

    //Inicializo el bitmap en 0s
    limpiar_bitmpa(bitarray);

    return bitarray;
}

int tamanio_bitmap(int cantidad_bloques){

    int bitmap_size = cantidad_bloques/8;

    //Se fija si sobran bloques en el bitmap
    if((cantidad_bloques%8)!=0){

        //Si sobran le suma uno para agregarlo
        bitmap_size++;
    }

    return bitmap_size;
}

void doom_bitmap(t_bitarray* bitarray){
    //Imprimo cada bit
    for(int i = 0; i < bitarray->size*8; i++){

        //cada vez que se escriben 8bits hago un salto
        if((i%8)==0){
            printf("\n");
        }
        printf("%d", bitarray_test_bit(bitarray, i));
    }
    printf("\n");
}

void limpiar_bitmpa(t_bitarray* bitarray){

    for(int i = 0; i < bitarray->size*8; i++){
        bitarray_clean_bit(bitarray, i);
    }
}


// Obtiene un string con el bitmap
char* obtener_bitmap(FILE* archivo_bitmap, int cant_bloques){
    //Pongo el puntero del archivo en 0
    rewind(archivo_bitmap);

    int bitmap_size = tamanio_bitmap(cant_bloques);
    char* buffer = malloc(bitmap_size + 1);

    fread( buffer, sizeof(char), bitmap_size, archivo_bitmap );

    return buffer;
}

int escribir_bitmap(t_bitarray* bitmap, FILE* archivo){
    rewind(archivo);
    return fwrite(bitmap->bitarray,sizeof(char),bitmap->size,archivo);
}

/* Crea una carpeta real
 * Param: El path donde queres que se cree
 * Param: El modo en el que lo queres crear
 */
int crear_carpeta(char* path, int modo){
    int resultado = mkdir(path , modo);

    if(resultado == -1){
        printf("Algo salio mal! %s\n", hstrerror(errno));
        return 1;
    }

    return 0;
};

/* Concatena dos string y devuelve otro sin necesidad de reservar
 * memoria de antemano
 * Parmas: strings a concatenar
 */
char* concatenar_strings(char*path1, char* path2){
    char* concatenado = malloc(strlen(path1) + strlen(path2) + 1);
    strcpy(concatenado, path1);
    strcat (concatenado, path2);

    return concatenado;
}