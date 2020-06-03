//
// Created by emi on 31/5/20.
//

#include "tallGrass.h"


int main(){
    montar("..");
}

/* Crea la estrucutra de carpetas del file system siempre y cuando no exista
 * Los errores los muestra imprimiendo en consola
 * path: String con el punto donde vas a querer montarlo.
 * return: 0 exito, 1 error
 */
int montar(char* punto_montaje){
    //Creo el path del fileSystem
    char* path_tall_grass = concatenar_strings(punto_montaje,"/Tall_Grass");

    limpiar_unidades_antiguas(path_tall_grass);

    //Creo la carpeta donde va a estar el fileSystem
    int resultado_tall_grass = crear_carpeta(path_tall_grass , ACCESSPERMS);

    if(resultado_tall_grass == 0){
        //Creo las carpetas que lo componen y devuelvo el resultado
        int resultado_met = crear_metadata( path_tall_grass);
        int resultado_blok = crear_blocks(path_tall_grass);
        int resultado_file = crear_file(path_tall_grass);
        return (resultado_met && resultado_blok && resultado_file);
    }

    return 1;
}


/*Verifica si existe la carpeta tall Grass
 * return: 1 si existe
 * return: 0 si no existe
 */
void limpiar_unidades_antiguas(char* path){
    struct stat datosFichero;

    if (lstat (path, &datosFichero) == -1){
        return;
    }
    /* Se comprueba si es un link simbólico y lo elimina*/
    if (S_ISLNK(datosFichero.st_mode)){
        unlink(path);
    }

    /* Se comprueba si es un fichero normal o un link y lo elimina */
    if (S_ISREG(datosFichero.st_mode)){
        remove(path);

    }

    /* Se comprueba si es un directorio y lo elimina*/
    if (S_ISDIR(datosFichero.st_mode)) {
        int status = remove_directory(path);

        if(status != 0){
            printf("hubo un problema al eliminar el directorio!\n");
        }
    }
}

//Crea la estructura de metadata del fileSystem
int crear_metadata(char* path){
    char* path_metadata = concatenar_strings(path,"/Metadata");

    //Creo la carpeta
    int resultado = crear_carpeta(path_metadata, ACCESSPERMS);

    //Verifico error
    if(resultado==0){
        //Creo el archivo metadata.bin
        char* path_metadata_bin = concatenar_strings(path_metadata,"/Metadata.bin");
        FILE * archivo_metadata = fopen(path_metadata_bin,"w+");
        fprintf(archivo_metadata,"BLOCK_SIZE=%d\nBLOCKS=%d\nMAGIC_NUMBER=%s",BLOCK_SIZE, BLOCKS, MAGIC_NUMBER);
        fclose(archivo_metadata);
        free(path_metadata_bin);

        //Creo el archivo bitmap.bin
        char* path_bitmap_bin = concatenar_strings(path_metadata,"/Bitmap.bin");
        FILE * archivo_bitmap = fopen(path_bitmap_bin,"w+");
        t_bitarray* bitmap = create_bitmap(BLOCKS);
        escribir_bitmap(bitmap, archivo_bitmap);
        bitarray_destroy(bitmap);
        fclose(archivo_bitmap);
        free(path_bitmap_bin);

        return 0;
    }
    return 1;
}

//Crea la estructura blocks del fileSyste
int crear_blocks(char* path){
    char* path_blocks = concatenar_strings(path,"/Blocks");

    int resultado = crear_carpeta(path_blocks, ACCESSPERMS);

    if(resultado == 0){
        //Creo los bloques del file system
        for(int i = 1; i <= BLOCKS; i++){
            //Genero path para el bloque que voy a crear
            char* nombre_bloque = string_from_format("/%d.bin", i);
            char* path_bloque =  concatenar_strings(path_blocks,nombre_bloque);

            FILE * archivo_metadata = fopen(path_bloque,"w+");
            fclose(archivo_metadata);

            free(nombre_bloque);
            free(path_bloque);
        }
        return 0;
    }
    return 1;
}

//Crea la estructura file del filesystem
int crear_file(char* path){

    char* path_file = concatenar_strings(path,"/Files");

    return crear_carpeta(path_file,ACCESSPERMS);
}

/*Sacada de lo mas profundo de starckoverflow
 * Param: direccion de la carpeta
 * Return 0 por exito
 * Return -1 por error
 */
int remove_directory(const char *path) {
    DIR *d = opendir(path);
    size_t path_len = strlen(path);
    int r = -1;

    if (d) {
        struct dirent *p;

        r = 0;
        while (!r && (p=readdir(d))) {
            int r2 = -1;
            char *buf;
            size_t len;

            /* Skip the names "." and ".." as we don't want to recurse on them. */
            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
                continue;

            len = path_len + strlen(p->d_name) + 2;
            buf = malloc(len);

            if (buf) {
                struct stat statbuf;

                snprintf(buf, len, "%s/%s", path, p->d_name);
                if (!stat(buf, &statbuf)) {
                    if (S_ISDIR(statbuf.st_mode))
                        r2 = remove_directory(buf);
                    else
                        r2 = unlink(buf);
                }
                free(buf);
            }
            r = r2;
        }
        closedir(d);
    }

    if (!r)
        r = rmdir(path);

    return r;
}

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
}

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

char* obtener_bitmap(FILE* archivo_bitmap){
    //Pongo el puntero del archivo en 0
    rewind(archivo_bitmap);

    int bitmap_size = tamanio_bitmap(BLOCKS);
    char* buffer = malloc(bitmap_size + 1);

    //Copio cada byte del archivo a un buffer
    for(int i = 0; i < bitmap_size;i++){
        buffer[i] = fgetc(archivo_bitmap);
    }

    //Agrego el caracter de final de string
    buffer[bitmap_size] = '\0';

    return buffer;
}

void escribir_bitmap(t_bitarray* bitmap, FILE* archivo){

    char* buffer = bitmap->bitarray;
    for(int i = 0; i < bitmap->size;i++){
        //copio cada byte al archivo
        fputc(buffer[i],archivo);
    }
}