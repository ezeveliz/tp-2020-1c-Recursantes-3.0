//
// Created by emi on 31/5/20.
//

#include "tallGrass.h"

//TODO Armar test para mantener esto estable

char* carpeta_montaje;

t_list* archivos_abiertos;

//Bloquea el uso de la lista
sem_t* bloque_archivos_abiertos;

//Para logear lo que pasa en el file system
t_log *logger_tall_grass;

/* Crea la estrucutra de carpetas del file system siempre y cuando no exista
 * Los errores los muestra imprimiendo en consola
 * path: String con el punto donde vas a querer montarlo.
 * return: 0 exito, 1 error
 */
int montar(char* punto_montaje){
    //Creo el path del fileSystem
    char* path_tall_grass = concatenar_strings(punto_montaje,"/Tall_Grass");

    logger_tall_grass = log_create("tall_grass_logger", "Tall-Grass", 0, LOG_LEVEL_INFO);
    //Venga lo nuevo fuera lo viejo
    limpiar_unidades_antiguas(path_tall_grass);

    //Inicializo la lista de archivos abiertos
    archivos_abiertos = list_create();
    bloque_archivos_abiertos = malloc(sizeof(sem_t));
    sem_init (bloque_archivos_abiertos, 0, 1);

    //Creo la carpeta donde va a estar el fileSystem
    int resultado_tall_grass = crear_carpeta(path_tall_grass , ACCESSPERMS);

    //Verifico el resultado
    if(resultado_tall_grass == 0){
        //Creo las carpetas que lo componen y devuelvo el resultado
        int resultado_met = crear_metadata( path_tall_grass);
        int resultado_blok = crear_blocks(path_tall_grass);
        int resultado_file = crear_file(path_tall_grass);

        // Seteo en una variable global
        carpeta_montaje = path_tall_grass;
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
    log_info(logger_tall_grass,"Se crea la metadata del FS con Path:%s", path_metadata);

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

        //Libero
        free(bitmap->bitarray);
        bitarray_destroy(bitmap);
        fclose(archivo_bitmap);
        free(path_bitmap_bin);
        free(path_metadata);
        return 0;
    }
    free(path_metadata);
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

        log_info(logger_tall_grass,"Se crearon los bloques del FS en la carpeta:%s", path_blocks);
        free(path_blocks);
        return 0;
    }
    free(path_blocks);
    log_error(logger_tall_grass,"Error al crear bloques del FS");
    return 1;
}

//Crea la estructura file del filesystem ( Donde se van a guardar los archivos)
int crear_file(char* path){
    char* path_file = concatenar_strings(path,"/Files");

    mkdir_tall_grass(path_file);
    free(path_file);
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
    char* path_metadata = obtener_path_metadata();
    t_config* metadata = config_create( path_metadata );
    int blocks = config_get_int_value(metadata, "BLOCKS");
    free(path_metadata);
    config_destroy(metadata);

    return blocks;
}

int obtener_tamanio_bloques(){

    int blocks = BLOCK_SIZE;

    return blocks;
}

//Tipo 0 para directorio 1 para archivo
int crear_ficheto(char* path, int tipo){
    int resultado = crear_carpeta(path, ACCESSPERMS);

    //Verifico lo que tengo que crear si directorio o archivo
    if(resultado == 0){
        char* path_metadata = concatenar_strings(path, "/Metadata.bin");
        FILE * archivo_metadata = fopen(path_metadata,"w+");
        if(tipo == 0){
            resultado = fprintf(archivo_metadata,"DIRECTORY=Y");
            log_info(logger_tall_grass,"Se creo un directorio con Path:%s",path);

        }else{
            resultado = fprintf(archivo_metadata,"DIRECTORY=N\nSIZE=0\nBLOCKS=[]\nOPEN=N");
            log_info(logger_tall_grass,"Se creo un archivo con Path:%s",path);
        }
        fclose(archivo_metadata);
        free(path_metadata);
        return resultado > 0 ? 0:1;
    }

    log_error(logger_tall_grass,"Error al crear archivo con Path:%s",path);
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

        if(strcmp(nombre_archivo,(char*)list_get(archivos,i)) == 0){
            //Ante coincidencia pongo el resultado en true y rompo el ciclo
            resultado = true;
            break;
        }

    }

    char* elemento;

    for(int i = 0; i < archivos->elements_count; i++){
        elemento = list_get(archivos,i);
        free(elemento);
    }

    list_destroy(archivos);
    free(files);
    return resultado;
}

t_list* ls_tall_grass(char* path){

    //Declaro estructuras
    DIR *dp;
    struct dirent *ep;

    //Abro el directorio
    dp = opendir (path);

    //Verifico si lo pude abrir
    if (dp != NULL)
    {
        t_list * respuesta = list_create();

        //Recorro el directorio y voy cargando los archivos que contiene
        while (ep = readdir (dp)){

            //Los nombres de los direcotrios terminan en null
            //Recorro el strin en busca del null y ver el espacio que ocupa

            int i = 0;
            while(ep->d_name[i] != NULL){
                i++;
            }
            //Reservo el espacio para los caracteres y el '\0'
            char* elemento = malloc(i+1);
            //Copio los caracteres necesarios
            memcpy(elemento,ep->d_name,i);
            //Agrego el fin del string
            elemento[i] = '\0';
            //Lo sumo a la lista
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
    char* path_archivo_metadata = concatenar_strings(path,"/Metadata.bin");
    t_file * retorno = malloc(sizeof(t_file));

    int pos = buscar_archivo_abierto(path_archivo_metadata);

    if(pos == -1){
        agregar_archivo_abierto(path_archivo_metadata);
        //Genero el retorno
        retorno->pos = 0;
        retorno->path = malloc(strlen(path_archivo_metadata)+1);
        retorno->metadata = obtener_metadata_archivo(path_archivo_metadata);
        t_config* metadata = config_create(path_archivo_metadata);
        memcpy(retorno->path, path_archivo_metadata,strlen(path_archivo_metadata)+1);

        config_set_value(metadata,"OPEN", "Y");
        config_save(metadata);
        config_destroy(metadata);

        log_info(logger_tall_grass,"Se abrio el archivo: %s",path);
        free(path_archivo_metadata);
        return retorno;
    }else{

        log_error(logger_tall_grass,"Error al abrir archivo: %s",path);
        free(retorno);
        free(path_archivo_metadata);
        return NULL;
    }

}

int close_tall_grass(t_file * fd ){

    //Modifico el archivo de metadata
    t_config* metadata = config_create(fd->path);
    config_set_value(metadata,"OPEN", "N");
    config_save(metadata);
    config_destroy(metadata);

    int res = sacar_lista_archivos_abiertos(fd->path);

    log_info(logger_tall_grass,"Se cerro el archivo con Path:%s en el Hilo: %d ",fd->path, syscall(SYS_gettid));
    free(fd->path);
    free(fd->metadata->bloques);
    free(fd->metadata);
    free(fd);
    return res ;
}

int agregar_archivo_abierto(char* path){
    int resultado;
    char* path_a_agregar = malloc(string_length(path)+1);

    //Copio el path en su propia porcion de memoria
    strcpy(path_a_agregar,path);

    sem_wait(bloque_archivos_abiertos);

    resultado = list_add(archivos_abiertos,path_a_agregar);

    sem_post(bloque_archivos_abiertos);

    return 0;
}

int buscar_archivo_abierto(char* path){
    char* path_iteritor;
    int i = 0;
    bool resultado = false;
    sem_wait(bloque_archivos_abiertos);

    //Busco en la lista
    while( i < list_size(archivos_abiertos)){
        path_iteritor = (char *) list_get(archivos_abiertos,i);
        if(string_equals_ignore_case(path,path_iteritor)){
            resultado = true;
            break;
        }
        i++;
    }

    sem_post(bloque_archivos_abiertos);


    return resultado ? i:-1;
}

int sacar_lista_archivos_abiertos(char* path){
    int pos = buscar_archivo_abierto(path);

    //Reviso que pos no sea -1 por el error
    if(pos >= 0){
        char* path_a_eliminar;
        //Bloqueo por uso de la lista

        sem_wait(bloque_archivos_abiertos);

        path_a_eliminar = list_get(archivos_abiertos,pos);
        list_remove(archivos_abiertos,pos);

        sem_post(bloque_archivos_abiertos);

        free(path_a_eliminar);

        return pos;

    }else {

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

    //Controlo que el puntero este en los limites existentes del archivo
    //El igual es para que agregue en la ultima posicion
    if(exedente_byte >= 0){
        //Seteo la posicion en el archivo
        archivo->pos = posicion_dentro_archivo;

        //Calculo los bytes extra que voy a necesitar agregar
        int byte_bloque_extra = size_a_escribir - (archivo->metadata->size - posicion_dentro_archivo);
        FILE* file_bloque_uso;
        int nro_bloque_uso, nro_bloque_ant;
        int tamanio_bloque = obtener_tamanio_bloques();
        int off_set ;

        //Obtengo el primer bloque en el que voy a escribir
        nro_bloque_uso = nro_bloque_ant= next(archivo);
        //Obtengo su file descriptor
        file_bloque_uso = obtener_file_bloque(nro_bloque_uso,"r+");

        //Itero por cada byte a escribir
        for(int i = 0; i < size_a_escribir; i++){

            //Verifico que siga siendo el mismo bloque sino cambio su archivo
            if(nro_bloque_uso != nro_bloque_ant){
                fclose(file_bloque_uso);
                file_bloque_uso = obtener_file_bloque(nro_bloque_uso,"r+");
                nro_bloque_ant = nro_bloque_uso;
            }
            //Calculo la poiscion relativa dentro del archivo de bloque
            off_set = (archivo->pos -1) % tamanio_bloque;
            //Seteo el off_set
            fseek(file_bloque_uso,off_set,SEEK_SET);
            fwrite(&datos_escribir[i],1,1,file_bloque_uso);

            //Evito que el puntero se desplace una posicion mas
            if( i + 1 < size_a_escribir){
                nro_bloque_uso = next(archivo);
            }
        }
        //Cierro el ultimo file descriptor del ultimo bloque
        fclose(file_bloque_uso);

        //Controlo si hay que agregar bytes extra al tamanio del archivo
        if(byte_bloque_extra > 0){
            //Caso positivo los agrego
            agregar_byte_archivo(archivo, byte_bloque_extra);
        }


        //Retorno los bytes que escribi
        return size_a_escribir;

    }else{
        //Ante error devuelvo -1
        return -1;
    }
}

//Aumenta el puntero y te devuelve el bloque
// donde tiene que escribir
int next(t_file* archivo){
    //Calculo los bloques por los q paso la posicion
    int bloques_usados = bloque_relativo_archivo(archivo->pos);
    int nro_bloque_actual = obtener_bloque(archivo->metadata->bloques,bloques_usados);

    //Controlo que la pos siguiente este en otro bloque
    if(nro_bloque_actual == -1){

        //Pido un bloque libre
        t_list* bloques_libres = obtener_bloques_libres(1);

        //Controlo que haya bloques libres
        if(list_size(bloques_libres) != 0){
            nro_bloque_actual = *(int*)list_get(bloques_libres,0);
            agregar_bloque_archivo(archivo, nro_bloque_actual);
            archivo->pos++;
        }else{
            //Si no hay bloques libres devuelvo error
            nro_bloque_actual = -1;
        }

        list_destroy_and_destroy_elements(bloques_libres,free);

    }else{

        //Si no hay bloques para agregar muevo el puntero en uno
        archivo->pos++;
    }

    return nro_bloque_actual;
}

void agregar_bloque_archivo(t_file* archivo, uint32_t bloque){
    t_config* metadata = config_create(archivo->path);
    char* blocks = config_get_string_value(metadata,"BLOCKS");
    //Le agrego la coma adelante del numero

    char* bloque_string1;

    if(strlen(blocks)==2){
        bloque_string1 = string_itoa(bloque);
    }else{
        char* numero = string_itoa(bloque);
        bloque_string1 = concatenar_strings(",",numero);
        free(numero);
    }

    //Le agrego ] al final del numero
    char* bloque_string2 = concatenar_strings(bloque_string1,"]");
    //Le saco la ultima ] para agregar el numero al final del array con el formato correspondiente
    char* bloques_abiertos = string_substring(blocks,0,string_length(blocks)-1);
    //Concateno los demas string
    char* bloques_final = concatenar_strings(bloques_abiertos,bloque_string2);


    //Lo abro como config seteo el nuevo array, guardo en archivo y libero

    config_set_value(metadata,"BLOCKS", bloques_final);
    config_save(metadata);
    config_destroy(metadata);

    //Libero el puntero anterior a version vieja de bloques
    free(archivo->metadata->bloques);
    //Asigno version nueva ya guardada de bloques
    archivo->metadata->bloques = bloques_final;

    log_info(logger_tall_grass,"Se agrego un bloque al archivo: %s bloque: %d",archivo->path, bloque);
    //Libero
    free(bloque_string1);
    free(bloque_string2);
    free(bloques_abiertos);
}

void agregar_byte_archivo(t_file* archivo, int cantidad){
    //Abro el archivo de metadata como un t_config
    //Me aprobecho de eso para simplificar la cosa
    sleep(1); //todo acordate de sacarlo
    t_config* metadata = config_create(archivo->path);
    int size_anterior = config_get_int_value(metadata,"SIZE");
    char* size = string_itoa(size_anterior + cantidad);

    //Modifico el t_config con el nuevo valor
    config_set_value(metadata,"SIZE", size);

    //modifico la estructura t_file tambien
    archivo->metadata->size = config_get_int_value(metadata,"SIZE");

    //Salvo el archivo
    config_save(metadata);

    //Libero el t_config
    config_destroy(metadata);
    free(size);
}

void disminuir_byte_archivo(t_file* archivo, int cantidad){
    //Abro el archivo de metadata como un t_config
    //Me aprobecho de eso para simplificar la cosa
    t_config* metadata = config_create(archivo->path);
    int size_anterior = config_get_int_value(metadata,"SIZE");
    char* size = string_itoa(size_anterior - cantidad);

    //Modifico el t_config con el nuevo valor
    config_set_value(metadata,"SIZE", size);

    //modifico la estructura t_file tambien
    archivo->metadata->size = config_get_int_value(metadata,"SIZE");

    //Salvo el archivo
    config_save(metadata);

    //Libero el t_config
    config_destroy(metadata);
    free(size);
}

FILE* obtener_file_bloque(int numero_bloque,char* flag){
    char* path_bloques = obtener_path_blocks();
    char* numero_bloque_s = string_itoa(numero_bloque) ;

    //Genero el path del bloque
    char* nombre_archivo_bloque = string_from_format("%s/%s%s",path_bloques,numero_bloque_s,".bin");

    //Lo abro para escribir
    FILE * archivo_bloque = fopen(nombre_archivo_bloque,flag);

    //Libero
    free(path_bloques);
    free(numero_bloque_s);
    free(nombre_archivo_bloque);

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
            char** bloques_split = cortar_bloques_array(bloques_cortados);

            //Me fijo que el array tenga esa cantidad de posiciones
            if(contar_elementos_array(bloques_split) > posicion){
                numero_bloque = atoi(bloques_split[posicion]);
            }else {
                //si no devuelvo error
                numero_bloque = -1;
            }

            liberar_elementos_array(bloques_split);
        }
        free(bloques_cortados);
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

//Devuleve la posicion en el array de bloques dentro del archivo
int bloque_relativo_archivo(int posicion){
    //Calculo los bloques por los q paso la posicion
    int bloque_relativo = posicion/obtener_tamanio_bloques();

    return bloque_relativo;
}

char** cortar_bloques_array(char* array){
    char **bloques_split;

    if(strlen(array)>1){
        bloques_split = string_split(array, ",");
    }else if(strlen(array) == 1){
        bloques_split = malloc(sizeof(char*) * 2);
        bloques_split[0] = string_substring(array,1,1);
    }else{
        bloques_split = malloc(sizeof(char*));
        bloques_split = NULL;
    }
    return bloques_split;
}

char* read_tall_grass(t_file* archivo, uint32_t size_a_leer, uint32_t posicion){

    //Posiciono el puntero en el lugar correspondiente
    archivo->pos = posicion;

    FILE* file_bloque_uso;
    int nro_bloque_uso, nro_bloque_ant;
    int tamanio_bloque = obtener_tamanio_bloques();
    int off_set ;

    //Inicializo un string auxiliar
    char* aux = malloc(2);
    aux[1] = '\0';// le pongo un fin de string

    //Inicializo el buffer
    char* buffer = malloc(size_a_leer + 1);
    buffer[0] = '\0';

    //Busco el bloque correspondiente y abro el archivo de ese bloque
    nro_bloque_uso = nro_bloque_ant= next(archivo);
    file_bloque_uso = obtener_file_bloque(nro_bloque_uso,"r+");

    //Itero por cada byte a leer
    for(int i = 0; i < size_a_leer; i++){
        //Verifico que siga siendo el mismo bloque sino cambio su archivo
        if(nro_bloque_uso != nro_bloque_ant){
            fclose(file_bloque_uso);
            file_bloque_uso = obtener_file_bloque(nro_bloque_uso,"r+");
            nro_bloque_ant = nro_bloque_uso;
        }

        //Calculo el off_set relativo al bloque
        off_set = (archivo->pos -1) % tamanio_bloque; //Le resto 1 porque el next le suma 1

        //Posiciono el puntero en el archivo del bloque
        fseek(file_bloque_uso,off_set,SEEK_SET);
        //Leo de a un caracter
        fread(aux, sizeof(char), 1, file_bloque_uso);
        //Copio cada caracter a un buffer
        string_append(&buffer,aux);

        // Controlo que no pase al siguiente bloque si es la ultima lectura
        if( i + 1 < size_a_leer){
            nro_bloque_uso = next(archivo);
        }
    }

    //Cierro el ultimo archivo de bloque usado
    fclose(file_bloque_uso);
    free(aux);

    return buffer;
}

int truncate_tall_grass(t_file* archivo, uint32_t off_set){

    //Seteo la pos al archivo
    archivo->pos = off_set;
    //Calculo los bytes a disminuir
    int bytes_a_disminuir = (archivo->metadata->size - off_set);

    //Calculo los bloques por los q paso la posicion
    int bloque_usado_en_array = bloque_relativo_archivo(archivo->pos); //Es la posicion del array de bloques
    int nro_bloque_actual = obtener_bloque(archivo->metadata->bloques,bloque_usado_en_array);//Es el nro de bloque en el file system

    //Obtengo el file del archivo
    FILE * bloque = obtener_file_bloque(nro_bloque_actual,"r+");

    //Trunco el archivo del bloque en la pos que debo
    ftruncate(bloque->_fileno, (archivo->pos % obtener_tamanio_bloques()));

    //bloque usado en el array es el bloque que apunta el ultimo byte del archivo
    //tengo  que eliminar el siguiente por eso le sumo 1
    int i = bloque_usado_en_array +1;

    //Primer bloque que voy a eliminar
    int bloque_a_liberar = obtener_bloque(archivo->metadata->bloques, i);

    FILE* bloque_a_liberar_archivo;

    //Libero hasta que el proximo bloque que quiero obtener me de error
    while(bloque_a_liberar != -1){
        //Para eliminar la info los abro como si fueran archivos nuevos
        bloque_a_liberar_archivo= obtener_file_bloque(bloque_a_liberar,"w+");
        fclose(bloque_a_liberar_archivo);

        //Libero del bitmap
        liberar_bloque(bloque_a_liberar);
        //Incremento el i para liberar el siguiente
        i++;

        //Obtengo el siguiente bloque a liberar
        bloque_a_liberar = obtener_bloque(archivo->metadata->bloques, i);
    }

    //Controlo en caso de que este vaciando el archivo
    //Fue la unica forma que encontre
    if(((int)archivo->metadata->size - bytes_a_disminuir) == 0){
        //El bloque que voy a liberar es al que apunta el ultimo byte 0 bloque 0 del array
        bloque_a_liberar = obtener_bloque(archivo->metadata->bloques, bloque_usado_en_array);
        //Obtengo el archivo de ese bloque lo abro como si fuera nuevo
        bloque_a_liberar_archivo= obtener_file_bloque(bloque_a_liberar,"w+");
        fclose(bloque_a_liberar_archivo);

        //Libero el bloque del bitmap
        liberar_bloque(bloque_a_liberar);
    }

    sacar_bloques_metadata(archivo,off_set);

    //Disminuyo en la metadata
    disminuir_byte_archivo(archivo, bytes_a_disminuir);
    return bytes_a_disminuir;
}

//Funcion para sacar el ultimo bloque de la metadata del archivo
void sacar_bloques_metadata(t_file* archivo, uint32_t pos_final_archivo){
    log_info(logger_tall_grass,"Se va a sacar un bloque del archivo: %s", archivo->path);

    //Calculo los bloques por los q paso la posicion
    int bloque_usado_en_array = bloque_relativo_archivo(pos_final_archivo); //Es la posicion del array de bloques

    //Abro el archivo como
    t_config* metadata = config_create(archivo->path);
    char** blocks = config_get_array_value(metadata,"BLOCKS");

    char* bloques_que_van_archivo = string_new();

    //Verifico que no se haya borrado el archivo y que quede sin bloques
    if(pos_final_archivo == 0){
        string_append(&bloques_que_van_archivo,"[]");
    }else{
        //Armo el string de bloques
        string_append(&bloques_que_van_archivo,"[");// Lo incializo con el primer corchete

        for(int i = 0; i < bloque_usado_en_array && blocks[i] != NULL; i++){
            string_append(&bloques_que_van_archivo,blocks[i]);
            string_append(&bloques_que_van_archivo,",");
        }

        //Le agrego la ultima posicion y lo pongo afuera porque va sin coma
        string_append(&bloques_que_van_archivo,blocks[bloque_usado_en_array]);
        string_append(&bloques_que_van_archivo,"]");// Lo cierro con el ultimo chorchete
    }

    free(archivo->metadata->bloques);
    //Asigno version nueva ya guardada de bloques
    archivo->metadata->bloques = bloques_que_van_archivo;

    //Lo abro como config seteo el nuevo array, guardo en archivo y libero
    config_set_value(metadata,"BLOCKS", bloques_que_van_archivo);
    config_save(metadata);
    config_destroy(metadata);

    //Libero el puntero a puntero
    int i = 0;
    while(blocks[i] != NULL){
        free(blocks[i]);
        i++;
    }
    free(blocks);

}

int delet_tall_grass(t_file* archivo, uint32_t off_set, uint32_t cantidad_byte) {
    log_info(logger_tall_grass,"Se va a eliminar datos del archivo: %s pos: %d cant:%d",archivo->path, off_set, cantidad_byte);
    char *caracter_a_copiar;
//i < cantidad_byte &&
    for (int i = 0;  (off_set + cantidad_byte + i) < (archivo->metadata->size); i++) {
        if ((off_set + cantidad_byte + i) < archivo->metadata->size) {
            caracter_a_copiar = read_tall_grass(archivo, 1, (off_set + cantidad_byte + i));
            write_tall_grass(archivo, caracter_a_copiar, 1, off_set + i);
            free(caracter_a_copiar);
        }

    }
    //Si la cantidad de bytes que queres borrar rebasa al archivo trunca en una posicion menor
    //Para evitar eso pongo una validacion
    int pos_truncar;

    //Esta casteado por problemas con el tipo de dato
    if(((int)(archivo->metadata->size - cantidad_byte)) > (int) off_set){
        pos_truncar = archivo->metadata->size - cantidad_byte;
    }else{
        pos_truncar = off_set;
    }

    return truncate_tall_grass(archivo,pos_truncar);

}

//Te devuelve la lista con los que hay
t_list* obtener_bloques_libres( int cantidad_pedida ) {
    int cantidad_bloques = obtener_cantidad_bloques();
    char *path_bitmap = obtener_path_bitmap();
    t_list *bloques_libres = list_create();

    FILE *archivo_bitmap = fopen(path_bitmap, "r+");

    //Lo dejo esperando hasta que pueda entrar
    while (flock(archivo_bitmap->_fileno, LOCK_EX | LOCK_NB) != 0) {}


    //Bloque el archivo y si tengo exito entra el el while
    t_bitarray *bitmap = bitarray_create(obtener_bitmap(archivo_bitmap, cantidad_bloques),
                                         tamanio_bitmap(cantidad_bloques));
    int contador_bloques_obtenidos = 0;

    //Busco todos los bloques que necestio
    for (int i = 0; i < cantidad_bloques && contador_bloques_obtenidos < cantidad_pedida; i++) {

        //Si esta libre lo agrego
        if (bitarray_test_bit(bitmap, i) == 0) {
            uint32_t *bloque_libre = (int*) malloc(sizeof(uint32_t));
            *bloque_libre = i;
            list_add(bloques_libres, bloque_libre);
            bitarray_set_bit(bitmap, i);
            log_info(logger_tall_grass,"Se ocupa el bloque: %d del bitmap",i);
            contador_bloques_obtenidos++;
        }
    }

    //Escribo el nuevo bitmap bloqueado en el archivo
    escribir_bitmap(bitmap, archivo_bitmap);
    //Libero la estructura
    free(bitmap->bitarray);
    bitarray_destroy(bitmap);
    //Desbloqueo el archivo
    flock(archivo_bitmap->_fileno, LOCK_UN);


    fclose(archivo_bitmap);
    free(path_bitmap);
    return bloques_libres;
}

int liberar_bloque(uint32_t nro_bloque){
    char* path_bitmap = obtener_path_bitmap();
    FILE *archivo_bitmap = fopen(path_bitmap, "r+");

    if(archivo_bitmap != NULL){

        //Lo dejo esperando hasta que pueda entrar
        while (flock(archivo_bitmap->_fileno, LOCK_EX | LOCK_NB) != 0) {}

        //Obtengo la cantidad de bloques, para calcular tamanio del bitmap
        int cantidad_bloques = obtener_cantidad_bloques();
        char* string_bitmap = obtener_bitmap(archivo_bitmap, cantidad_bloques);

        t_bitarray* bitmap = bitarray_create(string_bitmap,tamanio_bitmap(cantidad_bloques));

        //Limpio el bitmap
        bitarray_clean_bit(bitmap,nro_bloque);

        //Escribo el bitmap modificado en el archivo
        escribir_bitmap(bitmap, archivo_bitmap);
        log_info(logger_tall_grass,"Se libera bloque: %d del bitmap", nro_bloque);

        //Desbloqueo el archivo
        flock(archivo_bitmap->_fileno, LOCK_UN);

        //Libero
        free(string_bitmap);

        //Cierro el archivo
        fclose(archivo_bitmap);
        free(path_bitmap);
        bitarray_destroy(bitmap);
        return 0;
    }else{
        free(path_bitmap);
        return -1;
    }
}

t_metadata* obtener_metadata_archivo(char* path){


    //Trato a la metadata del archivo como un config
    t_config* conf = config_create(path);
    t_metadata* metadata = malloc(sizeof(t_metadata));

    //sete el tamanio
    metadata->size= config_get_int_value(conf,"SIZE");

    //calculo el largo del string de bloques
    int  largo_string = strlen(config_get_string_value(conf,"BLOCKS"));

    //Pido memoria para los bloques
    char* bloques_viejos = malloc(largo_string + 1);

    //Copio la info del archivo en el puntero que agarre recien
    memcpy(bloques_viejos,config_get_string_value(conf,"BLOCKS"),largo_string+1);

    //Asigno los bloques a la metadata
    metadata->bloques = bloques_viejos;

    //Libero y retorno
    config_destroy(conf);
    return metadata;
}

int espacio_libre_archivo(t_file* archivo){
    int espacio_libre = (calcular_bloques_archivo(archivo->metadata) * obtener_tamanio_bloques()) - (archivo->metadata->size);
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

void metadata_destroy(t_metadata* metadata){
    //free(metadata->bloques);
    free(metadata);
}
