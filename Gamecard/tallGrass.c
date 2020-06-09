//
// Created by emi on 31/5/20.
//

#include "tallGrass.h"

char* carpeta_montaje;

int main(){
    montar("..");
    t_file * archivo = open_tall_grass("../Tall_Grass/Files/pikachu/Metadata.bin");
    char* palabra = "aaaaaaaaaaaaaaaaaaaaaaa";//Usar 5 bloques
    write_tall_grass(archivo, palabra , strlen(palabra), 0);
    palabra = "bbbbbbb";// No tiene que agregar ninguno
    write_tall_grass(archivo, palabra , strlen(palabra), 0);
    printf("%d\n %s\n",archivo->metadata->size,archivo->metadata->bloques);


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
        carpeta_montaje = path_tall_grass;// Seteo en una variable global
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
        for(int i = 0; i < BLOCKS; i++){
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
    return concatenar_strings(carpeta_montaje,"/Files");
}
char* obtener_path_blocks(){
    return concatenar_strings(carpeta_montaje,"/Blocks");
}
char* obtener_path_metadata(){
    return concatenar_strings(carpeta_montaje,"/Metadata/Metadata.bin");
}
char* obtener_path_bitmap(){
    return concatenar_strings(carpeta_montaje,"/Metadata/Bitmap.bin");
}

int obtener_cantidad_bloques(){
    t_config* metadata = config_create(obtener_path_metadata());
    int blocks = config_get_int_value(metadata, "BLOCKS");
    config_destroy(metadata);

    return blocks;
}
int obtener_tamanio_bloques(){
    t_config* metadata = config_create(obtener_path_metadata());
    int blocks = config_get_int_value(metadata, "BLOCK_SIZE");
    config_destroy(metadata);

    return blocks;
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

t_file* open_tall_grass(char* path){
    t_file * retorno = malloc(sizeof(t_file));
    retorno->pos = 0;
    retorno->path = malloc(strlen(path)+1);
    retorno->metadata = obtener_metadata_archivo(path);
    //r+ lectura-escritura || w+ archivo en blanco
    FILE* archivo = fopen(path,"r+");
    memcpy(retorno->path, path,strlen(path)+1);

//    LOCK_EX es para que sea bloque exclusivo
//    LOCK_NB es para que sea no bloqueante si esta bloqueado
    if( flock(archivo->_fileno, LOCK_EX | LOCK_NB) == 0){
        set_estado_archivo(archivo,'Y');
        fclose(archivo);
        return retorno;
    }else{
        fclose(archivo);
        return NULL;
    }
}

int close_tall_grass(t_file * fd ){
    FILE* archivo = fopen(fd->path,"r+");
    if(set_estado_archivo(archivo,'N') == -1){
        return -1;
    };

    if(flock(archivo->_fileno, LOCK_UN) != 0){
        return -1;
    }

    free(fd->path);
    return fclose(archivo) ;
}

//Setias el estado open del archivo
//Estados Y|N

int set_estado_archivo(FILE* archivo,char estado){
    //Pongo en 0 el puntero del archivo
    rewind(archivo);
    //Busco la posicion del '=' de open
    int posicion = buscar_caracter_archivo(archivo,'=',4) + 1;//No esta bien esto pero es la forma mas facil

    //Controlo posible error
    if(posicion != -1){
        fseek(archivo, posicion, SEEK_SET); //Posiciono el puntero del archivo ahi
        fputc(estado,archivo);//Pongo el estado pasado por parametro en lugar del existente
    }else{
        return -1;
    }
}

int buscar_caracter_archivo(FILE* archivo, char caracter_a_buscar , int numero_de_aparicion){
    int i =0;
    int contador_aparicion = 0;
    char c = fgetc(archivo);

    //Compara caracter
    while( c != EOF ){
        //Si es igual y es el numero de aparicion que estoy buscando
        if( c == caracter_a_buscar && ++contador_aparicion == numero_de_aparicion){
            return i;
        }
        i++;
        c = fgetc(archivo);
    }
    //sino devuelvo error
    return -1;
}



int write_tall_grass(t_file* archivo, char* datos_escribir, uint32_t size_a_escribir, uint32_t posicion_dentro_archivo){

    int exedente_byte = (archivo->metadata->size) - posicion_dentro_archivo;
    if(exedente_byte >= 0){
        archivo->pos = posicion_dentro_archivo;
        int byte_bloque_extra = size_a_escribir - (archivo->metadata->size - posicion_dentro_archivo);
        printf("\n---------------\n");
        FILE* file_bloque_uso;
        int nro_bloque_uso, nro_bloque_ant;
        int tamanio_bloque = obtener_tamanio_bloques();
        nro_bloque_uso = nro_bloque_ant= next(archivo);
        file_bloque_uso = obtener_file_bloque(nro_bloque_uso);
        int off_set ;
        for(int i = 0; i < size_a_escribir; i++){
            //Verifico que siga siendo el mismo bloque sino cambio su archivo
            if(nro_bloque_uso != nro_bloque_ant){
                fclose(file_bloque_uso);
                file_bloque_uso = obtener_file_bloque(nro_bloque_uso);
                nro_bloque_ant = nro_bloque_uso;
            }
            off_set = (archivo->pos -1) % tamanio_bloque; //Le resto 1 porque el next le suma 1
            fseek(file_bloque_uso,off_set,SEEK_SET);
            fwrite(&datos_escribir[i],1,1,file_bloque_uso);

            // No quiero que se corra a una posicion mas
            if( i + 1 < size_a_escribir){
                nro_bloque_uso = next(archivo);
                printf("%d\n",nro_bloque_uso);
            }
        }
        fclose(file_bloque_uso);

        if(byte_bloque_extra > 0){
            agregar_byte_archivo(archivo, byte_bloque_extra);
        }
        return size_a_escribir;
    }else{
        return -1;
    }

}

//Aumenta el puntero y te devuelve el bloque
// donde tiene que escribir
int next(t_file* archivo){
    //Calculo los bloques por los q paso la posicion
    int bloques_usados = bloque_relativo_archivo(archivo->pos);
    int nro_bloque_actual = obtener_bloque(archivo->metadata->bloques,bloques_usados);

    if(nro_bloque_actual == -1){
        t_list* bloques_libres = obtener_bloques_libres(1);
        if(list_size(bloques_libres) != 0){
            nro_bloque_actual = *(int*)list_get(bloques_libres,0);
            agregar_bloque_archivo(archivo, nro_bloque_actual);
            archivo->pos++;
        }else{
            nro_bloque_actual = -1;
        }
    }else{
        archivo->pos++;
    }
    return nro_bloque_actual;

}
int agregar_bloque_archivo(t_file* archivo, uint32_t bloque){
    t_config* metadata = config_create(archivo->path);
    char* blocks = config_get_string_value(metadata,"BLOCKS");
    //Le agrego la coma adelante del numero
    char* bloque_string1 = strlen(blocks)==2? string_itoa(bloque) :concatenar_strings(",",string_itoa(bloque));

    //Le agrego ] al final del numero
    char* bloque_string2 = concatenar_strings(bloque_string1,"]");
    //Le saco la ultima ] para agregar el numero al final del array con el formato correspondiente
    char* bloques_abiertos = string_substring(blocks,0,string_length(blocks)-1);
    //Concateno todo
    char* bloques_final = concatenar_strings(bloques_abiertos,bloque_string2);


    //Lo abro como config seteo el nuevo array, guardo en archivo y libero

    config_set_value(metadata,"BLOCKS", bloques_final);
    config_save(metadata);
    config_destroy(metadata);

    //Libero el puntero anterior a version vieja de bloques
    free(archivo->metadata->bloques);
    //Asigno version nueva ya guardada de bloques
    archivo->metadata->bloques = bloques_final;

    //Libero
    free(bloque_string2);
    free(bloque_string1);
    free(bloques_abiertos);
}

void agregar_byte_archivo(t_file* archivo, int cantidad){
    t_config* metadata = config_create(archivo->path);
    int size_anterior = config_get_int_value(metadata,"SIZE");
    config_set_value(metadata,"SIZE", string_itoa(size_anterior + cantidad));
    archivo->metadata->size = config_get_int_value(metadata,"SIZE");
    config_save(metadata);
    config_destroy(metadata);

}
FILE* obtener_file_bloque(int numero_bloque){
    char* path_bloques = obtener_path_blocks();
    char* numero_bloque_s = string_itoa(numero_bloque) ;
    //Genero el path del bloque
    char* nombre_archivo_bloque = string_from_format("%s/%s%s",path_bloques,numero_bloque_s,".bin");
    //Lo abro para escribir
    FILE * archivo_bloque = fopen(nombre_archivo_bloque,"r+");
    //Libero
    free(path_bloques);
    free(numero_bloque_s);
   // free(nombre_archivo_bloque);
    //Retorno el file descriptor
    return archivo_bloque;
}


//Obtengo el bloque de un array en una determinada posicion
int obtener_bloque(char* bloques,int posicion){
    int numero_bloque;

    if(strlen(bloques) > 2){
        //Le saco los corchetes
        char* bloques_cortados = string_substring(bloques,1,strlen(bloques)-2);

        //Controlo que haya un elemento
        if(strlen(bloques_cortados) == 1){
            //Controlo que sea el elemento pedido
            if(posicion == 0){
                //Si hay un solo elemento d
                numero_bloque = atoi(bloques_cortados);
            }else{
                numero_bloque = -1;
            }
        }else {
            //Si hay mas de un elemento los separo
            char** bloques_split = cortar_bloques_array(bloques);

            //Me fijo que el array tenga esa cantidad de posiciones
            if(contar_elementos_array(bloques_split) > posicion){
                numero_bloque = atoi(bloques_split[posicion]);
            }else {
                //si no devuelvo error
                numero_bloque = -1;
            }

            //liberar_elementos_array(bloques_split);
        }
    }else {
        numero_bloque = -1;
    }

    return numero_bloque;
}

int contar_elementos_array(char** array){
    int size_array = 0;

    while (array[size_array] != NULL) {
        size_array++;
    }
    return size_array;
}

int liberar_elementos_array(char** array){
    int i = 0;

    while (array[i] != NULL) {
        free(array[i]);
        i++;
    }
    free(array);
    return i;
}

int bloque_relativo_archivo(int posicion){
    //Calculo los bloques por los q paso la posicion
    int bloque_relativo = posicion/obtener_tamanio_bloques();

    return bloque_relativo;
}

char** cortar_bloques_array(char* array){
    char **bloques_split;

    if(strlen(array)>3){
        bloques_split = string_split(array, ",");
    }else if(strlen(array) == 3){
        bloques_split = malloc(sizeof(char*) * 2);
        bloques_split[0] = string_substring(array,1,1);
    }else{
        bloques_split = malloc(sizeof(char*));
        bloques_split = NULL;
    }
    return bloques_split;
}

int read_tall_grass(){

}

int rmfile_tall_grass(){

}

//Te devuelve la lista con los que hay
//TODO:Usar un lock o semaforo
t_list* obtener_bloques_libres(int cantidad_pedida){
    int cantidad_bloques = obtener_cantidad_bloques();
    char* path_bitmap = obtener_path_bitmap();
    t_list* bloques_libres = list_create();
    FILE* archivo_bitmap = fopen(path_bitmap,"r+");
    t_bitarray* bitmap = bitarray_create(obtener_bitmap(archivo_bitmap, cantidad_bloques),tamanio_bitmap(cantidad_bloques));
    int contador_bloques_obtenidos = 0;

    for(int i = 0; i < cantidad_bloques && contador_bloques_obtenidos < cantidad_pedida; i++){
        int estado_bloque= bitarray_test_bit(bitmap,i);
        if(estado_bloque == 0){
            int* bloque_libre = malloc(sizeof(uint32_t));
            *bloque_libre = i;
            list_add(bloques_libres,bloque_libre);
            bitarray_set_bit(bitmap,i);
            contador_bloques_obtenidos++;
        }
    }
    escribir_bitmap(bitmap, archivo_bitmap);
    fclose(archivo_bitmap);
    return bloques_libres;
}

int liberar_bloques(t_list* bloques_a_liberar){
    char* path_bitmap = obtener_path_bitmap();
    FILE* archivo_bitmap = fopen(path_bitmap,"r+");
    if(archivo_bitmap != NULL){
        int cantidad_bloques = obtener_cantidad_bloques();
        t_bitarray* bitmap = bitarray_create(obtener_bitmap(archivo_bitmap, cantidad_bloques),tamanio_bitmap(cantidad_bloques));

        for(int i = 0; i < list_size(bloques_a_liberar); i++){
            bitarray_set_bit(bitmap,*(int*)list_get(bloques_a_liberar,i));
        }
        //Escribo el bitmap modificado en el archivo
        escribir_bitmap(bitmap, archivo_bitmap);
        fclose(archivo_bitmap);
        return list_size(bloques_a_liberar);//Devuelvo los bloques que fueron liberados
    }else{
        return -1;
    }
}

//TODO: Modificar ese nose
t_metadata* obtener_metadata_archivo(char* path){
    t_config* nose = config_create(path);
    t_metadata* metadata = malloc(sizeof(t_metadata));
    metadata->size= config_get_int_value(nose,"SIZE");
    int  largo_string = strlen(config_get_string_value(nose,"BLOCKS"));
    char* bloques_viejos = malloc(largo_string);
    memcpy(bloques_viejos,config_get_string_value(nose,"BLOCKS"),largo_string+1);
    metadata->bloques = bloques_viejos;
    config_destroy(nose);
    return metadata;
}

int espacio_libre_archivo(t_file* archivo){
    t_metadata* metadata = obtener_metadata_archivo(archivo);
    int espacio_libre = (calcular_bloques_archivo(metadata) * obtener_tamanio_bloques()) - metadata->size;
    metadata_destroy(metadata);
    return espacio_libre;

}

int calcular_bloques_archivo(t_metadata* metadata){
    int tamanio_bloque = obtener_tamanio_bloques();
    int resultado = metadata->size / tamanio_bloque;

    if(metadata->size % tamanio_bloque != 0 ){
        resultado ++;
    }

    return resultado;
}

int calcular_bloques(int byts){
    int tamanio_bloque = obtener_tamanio_bloques();
    int resultado = byts/tamanio_bloque;
    if(byts % tamanio_bloque != 0 ){
        resultado ++;
    }
    return resultado;
}


void metadata_destroy(t_metadata* metadata){
    //free(metadata->bloques);
    free(metadata);
}
