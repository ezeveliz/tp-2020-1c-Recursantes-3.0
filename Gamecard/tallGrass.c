//
// Created by emi on 31/5/20.
//

#include "tallGrass.h"

int main(){
    printf("salio todo : %d \n",montar(".."));
}

int montar(char* punto_montaje){
    crear_estructura_carpetas(punto_montaje);

    return 0;
}


/*Verifica si existe la carpeta tall Grass
 * return: -1 si existe
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

/* Crea la estrucutra de carpetas del file system siempre y cuando no exista
 * Los errores los muestra imprimiendo en consola
 * path: String con el punto donde vas a querer montarlo.
 * return: 0 exito, 1 error
 */
int crear_estructura_carpetas(char* punto_montaje){
    //Creo la carpeta principal

    char* path_tall_grass = concatenar_strings(punto_montaje,"/Tall_Grass");

    limpiar_unidades_antiguas(path_tall_grass);

    int resultado_tall_grass = crear_carpeta(path_tall_grass , ACCESSPERMS);

    char* path_file = concatenar_strings(path_tall_grass,"/Files");

    int resultado_file = crear_carpeta(path_file,ACCESSPERMS);

    crear_metadata( path_tall_grass);

    char* path_blocks = concatenar_strings(path_tall_grass,"/Blocks");

    int resultado_bloques = crear_carpeta(path_blocks, ACCESSPERMS);

    return resultado_tall_grass && resultado_bloques &&resultado_file;
}

int crear_metadata(char* path){
    char* path_metadata = concatenar_strings(path,"/Metadata");

    int resultado = crear_carpeta(path_metadata, ACCESSPERMS);

    if(resultado==0){
        char* path_metadata_bin = concatenar_strings(path_metadata,"/Metadata.bin");
        FILE * archivo_metadata = fopen(path_metadata_bin,"w+");
        fprintf(archivo_metadata,"BLOCK_SIZE=%d\nBLOCKS=%d\nMAGIC_NUMBER=%s",BLOCK_SIZE, BLOCKS, MAGIC_NUMBER);
        fclose(archivo_metadata);
        free(path_metadata_bin);

        char* path_bitmap_bin = concatenar_strings(path_metadata,"/Bitmap.bin");
        FILE * archivo_bitmap = fopen(path_bitmap_bin,"w+");
        t_bitarray* bitmap = create_bitmap(BLOCKS);
        fprintf(archivo_bitmap,"%s",bitmap->bitarray);
        fclose(archivo_bitmap);
        free(path_bitmap_bin);
    }
}


/*Sacada de lo mas profundo de starckoverflow
 *path: direccion de la carpeta
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

char* concatenar_strings(char*path1, char* path2){
    char* concatenado = malloc(strlen(path1) + strlen(path2) + 1);
    strcpy(concatenado, path1);
    strcat (concatenado, path2);

    return concatenado;
}

int crear_carpeta(char* path, int modo){
    int resultado = mkdir(path , modo);

    if(resultado == -1){
        printf("Algo salio mal! %s\n", hstrerror(errno));
    }
}

t_bitarray* create_bitmap(int cantidad_bloques){

    int bitmap_size = cantidad_bloques/8;
    char* bitmap_string = malloc(bitmap_size);

    return bitarray_create(bitmap_string, bitmap_size);
}
