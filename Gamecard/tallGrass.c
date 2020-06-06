//
// Created by emi on 31/5/20.
//

#include "tallGrass.h"

char* punto_montaje;

int main(){
    montar("..");

    FILE* fd = open_tall_grass("../Tall_Grass/Metadata/Metadata.bin");
    if(fd != NULL){
        printf("%d\n",fd->_fileno);
    } else{
        printf("No anda \n");
    }
    close_tall_grass(fd);
    printf("%d\n",open_tall_grass("../Tall_Grass/Metadata/Metadata.bin") == NULL? -1: 0);
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
        punto_montaje = path_tall_grass;// Seteo en una variable global
        return (resultado_met || resultado_blok || resultado_file);
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
        int status = rmdir_tall_grass(path);

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

    mkdir_tall_grass(path_file);
    create_tall_grass(concatenar_strings(path_file,"/pikachu"));
    return 0;
}


/*
 * Operaciones del file system
 */

char* obtener_path_file(){
    return concatenar_strings(punto_montaje,"/Files");
}
char* obtener_path_blocks(){
    return concatenar_strings(punto_montaje,"/Blocks");
}
char* obtener_path_metadata(){
    return concatenar_strings(punto_montaje,"/Metadata/Metadata.bin");
}

//Tipo 0 para directorio 1 para archivo
int crear_ficheto(char* path, int tipo){
    int resultado = crear_carpeta(path, ACCESSPERMS);

    if(resultado == 0){
        char* path_metadata = concatenar_strings(path, "/Metadata.bin");
        FILE * archivo_metadata = fopen(path_metadata,"w+");
        if(tipo == 0){
            resultado = fprintf(archivo_metadata,"DIRECTORY=Y");
        }else{
            resultado = fprintf(archivo_metadata,"DIRECTORY=N\nSIZE=0\nBLOCKS=[]\nOPEN=N");
        }
        fclose(archivo_metadata);
        free(path_metadata);
        return resultado > 0 ? 0:1;
    }

    return 1;
}

int mkdir_tall_grass(char* path){
    return crear_ficheto(path,0);
}

/*Sacada de lo mas profundo de starckoverflow
 * Param: direccion de la carpeta
 * Return 0 por exito
 * Return -1 por error
 */
int rmdir_tall_grass(const char *path) {
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
                        r2 = rmdir_tall_grass(buf);
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

bool find_tall_grass(char* nombre_archivo){
    char* files = obtener_path_file();

    //Obtengo una lista con los archivos que contiene ese directorio
    t_list* archivos = ls_tall_grass(files);
    int numero_archivos = list_size(archivos);
    bool resultado = false;

    //Recorro la lista buscando coincidencia con el nombre que busco
    for(int i = 0; i < numero_archivos; i++){
        if(strcmp(nombre_archivo, list_get(archivos,i)) == 0){

            //Ante coincidencia pongo el resultado en true y rompo el ciclo
            resultado = true;
            break;
        }
    }

    list_clean_and_destroy_elements(archivos,free);
    return resultado;
}

t_list* ls_tall_grass(char* path){

    DIR *dp;
    struct dirent *ep;

    dp = opendir (path);
    if (dp != NULL)
    {
        t_list * respuesta = list_create();
        while (ep = readdir (dp)){
            char* elemento = malloc(strlen(ep->d_name));
            list_add(respuesta,elemento);
        }

        (void) closedir (dp);
        return respuesta;
    }
    else
       return NULL;
}

int create_tall_grass(char* path){
    return crear_ficheto(path,1);
}

FILE* open_tall_grass(char* path){
//    struct flock fl;
//    memset(&fl, 0, sizeof(fl));
//
//    fl.l_whence = SEEK_SET;
//    fl.l_start = 0;
//    fl.l_len = 0;
//    fl.l_pid = 0;

    //r+ lectura-escritura || w+ archivo en blanco
    FILE*  file = fopen(path,"r+");
    //LOCK_EX es para que sea bloque exclusivo
    //LOCK_NB es para que sea no bloqueante si esta bloqueado
    int resultado = flock(file->_fileno, LOCK_EX | LOCK_NB);
    if( resultado== 0){
//        int a1,a2;
//        char* a3;
//        fscanf(file,"%d %d %s",a1,a2,a3);
//        printf("BLOCK_SIZE=%d BLOCKS=%d MAGIC_NUMBER=%s",&a1,&a2,a3);
//        return file;
    }else{
        fclose(file);
        return NULL;
    }
}

int close_tall_grass( int fd ){
    return flock(fd, LOCK_UN) ;

}

int write_tall_grass(){

}

int read_tall_grass(){

}

int rmfile_tall_grass(){

}