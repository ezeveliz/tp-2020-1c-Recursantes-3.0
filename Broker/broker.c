//
// Created by utnso on 07/04/20.
//

#include "broker.h"


int main(int argc, char **argv) {
    if (argc != 2) {
        cfg_path = strdup("broker.cfg");
    } else {
        cfg_path = strdup(argv[1]);
    }
    logger = log_create("broker.log", "BROKER", 1, LOG_LEVEL_TRACE);
    log_info(logger,"Log started.");
    set_config();
    log_info(logger,"Configuration succesfully setted.");

    pthread_t server_thread;
    pthread_create(&server_thread, NULL, server_function, NULL);

    tests_broker();

    pthread_join(server_thread, NULL);

    return EXIT_SUCCESS;
}

/*
 * Configuration starts
 */
void set_config(){
    cfg_file = config_create(cfg_path);

    if (!cfg_file) {
        log_error(logger, "No se encontró el archivo de configuración");
        return;
    }

    config.mem_size = config_get_int_value(cfg_file, "TAMANO_MEMORIA");
    config.min_partition_size = config_get_int_value(cfg_file, "TAMANO_MINIMO_PARTICION");
    config.mem_algorithm = config_get_string_value(cfg_file, "ALGORITMO_MEMORIA");
    config.mem_swap_algorithm = config_get_string_value(cfg_file, "ALGORITMO_REEMPLAZO");
    config.free_partition_algorithm = config_get_string_value(cfg_file, "ALGORITMO_PARTICION_LIBRE");
    config.broker_ip = config_get_string_value(cfg_file, "IP_BROKER");
    config.broker_port = config_get_int_value(cfg_file, "PUERTO_BROKER");
    config.compactation_freq = config_get_int_value(cfg_file, "FRECUENCIA_COMPACTACION");
    config.log_file= config_get_string_value(cfg_file, "LOG_FILE");
   }
/*
* Configuration ends
*/


void *server_function(void *arg) {

    int socket;

    if((socket = create_socket()) == -1) {
        log_error(logger, "Error al crear el socket");
    }

    if ((bind_socket(socket, config.broker_port)) == -1) {
        log_error(logger, "Error al bindear el socket");
    }

    //--Funcion que se ejecuta cuando se conecta un nuevo programa
    void new(int fd, char *ip, int port) {
        if(&fd != null && ip != null && &port != null) {
            log_info(logger, "Nueva conexión");
        }
    }

    //--Funcion que se ejecuta cuando se pierde la conexion con un cliente
    void lost(int fd, char *ip, int port) {
        if(&fd == null && ip == null && &port == null){
            log_info(logger, "Se perdió una conexión");
            //Cierro la conexión fallida
            log_info(logger, "Cerrando conexión");
            close(fd);
        }
    }

    //--funcion que se ejecuta cuando se recibe un nuevo mensaje de un cliente ya conectado
    void incoming(int fd, char *ip, int port, MessageHeader *headerStruct) {

        t_list *cosas = receive_package(fd, headerStruct);

        switch (headerStruct->type) {
            case ABC:;
                {
//                    chat_mensaje* mensaje = void_a_mensaje(list_get(cosas, 0));
//                    mostrar_mensaje(mensaje);
                    break;
                }

            default: {
                log_warning(logger, "Operacion desconocida. No quieras meter la pata\n");
                break;
            }
        }
    }
    log_info(logger, "Hilo de servidor iniciado...");
    start_multithread_server(socket, &new, &lost, &incoming);
}


void tests_broker(){
    //mem_assert recive mensaje de error y una condicion, si falla el test lo loggea
    #define test_assert(message, test) do { if (!(test)) { log_error(test_logger, message); tests_fail++; } tests_run++; } while (0)
    t_log* test_logger = log_create("memory_tests.log", "MEM", true, LOG_LEVEL_TRACE);
    int tests_run = 0;
    int tests_fail = 0;

  /*  t_new_pokemon* new_pokemon = create_new_pokemon("Pikachu", 5, 2, 3);
    t_new_pokemon* otro_new_pokemon = void_a_new_pokemon(new_pokemon_a_void(new_pokemon));*/

   /* t_get_pokemon* Pika = create_get_pokemon("Pikachu");
    void* streamPika = get_pokemon_a_void(Pika);
    t_get_pokemon* PikaAgain = void_a_get_pokemon(streamPika);
    log_info(logger, "%s encontrado!", PikaAgain->nombre_pokemon);*/

   /* t_localized_pokemon* new_localized_pokemon = create_localized_pokemon("Tu vieja", 3, 1,1, 2,2, 3,3);
    t_localized_pokemon* another_bitch = void_a_localized_pokemon(localized_pokemon_a_void(new_localized_pokemon));
    for(int i=0;i<6;i++){
        log_info(logger,"%d \n", another_bitch->coordenadas[i]);
    }*/

    log_warning(test_logger, "Pasaron %d de %d tests", tests_run-tests_fail, tests_run);
    log_destroy(test_logger);
}


// Pongo estas funciones aca, para debuggear facil, despues las ponemos en la libreria
/*
 *
 * NEW_POKEMON STARTS
 *
 * */
t_new_pokemon* create_new_pokemon(char* nombre_pokemon, uint32_t pos_x, uint32_t pos_y, uint32_t cantidad){
    t_new_pokemon* new_pokemon = malloc(sizeof(t_new_pokemon));
    new_pokemon->nombre_pokemon_length = strlen(nombre_pokemon) + 1;
    new_pokemon->nombre_pokemon = nombre_pokemon;
    new_pokemon->pos_x = pos_x;
    new_pokemon->pos_y = pos_y;
    new_pokemon->cantidad = cantidad;
}

void* new_pokemon_a_void(t_new_pokemon* new_pokemon){
    void* stream = malloc(sizeof(uint32_t)*4 + new_pokemon->nombre_pokemon_length);
    int offset = 0;

    memcpy(stream + offset, &new_pokemon->nombre_pokemon_length, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, new_pokemon->nombre_pokemon, new_pokemon->nombre_pokemon_length);
    offset += new_pokemon->nombre_pokemon_length;

    memcpy(stream + offset, &new_pokemon->pos_x, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, &new_pokemon->pos_y, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, &new_pokemon->cantidad, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    return stream;
}

t_new_pokemon* void_a_new_pokemon(void* stream){
    t_new_pokemon* new_pokemon = malloc(sizeof(t_new_pokemon));

    memcpy(&(new_pokemon->nombre_pokemon_length), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    new_pokemon->nombre_pokemon = malloc(new_pokemon->nombre_pokemon_length);
    memcpy(new_pokemon->nombre_pokemon, stream, new_pokemon->nombre_pokemon_length);
    stream += new_pokemon->nombre_pokemon_length;

    memcpy(&(new_pokemon->pos_x), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    memcpy(&(new_pokemon->pos_y), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    memcpy(&(new_pokemon->cantidad), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    return new_pokemon;
}
/*
 *
 * NEW_POKEMON ENDS
 *
 * */

/*
 *
 * GET_POKEMON STARTS
 *
 * */

t_get_pokemon* create_get_pokemon(char* nombre_pokemon){
    t_get_pokemon* get_pokemon = malloc(sizeof(t_get_pokemon));
    get_pokemon->nombre_pokemon_length = strlen(nombre_pokemon) + 1;
    get_pokemon->nombre_pokemon = nombre_pokemon;
    return get_pokemon;
}

void* get_pokemon_a_void(t_get_pokemon* get_pokemon){
    void* stream = malloc(sizeof(uint32_t) + get_pokemon->nombre_pokemon_length);
    int offset = 0;

    memcpy(stream + offset, &get_pokemon->nombre_pokemon_length, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, get_pokemon->nombre_pokemon, get_pokemon->nombre_pokemon_length);
    offset += get_pokemon->nombre_pokemon_length;

    return stream;
}

t_get_pokemon* void_a_get_pokemon(void* stream){
    t_get_pokemon* get_pokemon = malloc(sizeof(t_get_pokemon));

    memcpy(&(get_pokemon->nombre_pokemon_length), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    get_pokemon->nombre_pokemon = malloc(get_pokemon->nombre_pokemon_length);
    memcpy(get_pokemon->nombre_pokemon, stream, get_pokemon->nombre_pokemon_length);
    stream += get_pokemon->nombre_pokemon_length;

    return get_pokemon;
}


/*
 *
 * GET_POKEMON ENDS
 *
 * */

/*
 *
 * LOCALIZED_POKEMON STARTS
 *
 * */

t_localized_pokemon* create_localized_pokemon(char* nombre_pokemon, uint32_t cantidad_coordenadas, ...){
    t_localized_pokemon* localized_pokemon = malloc(sizeof(t_localized_pokemon));
    localized_pokemon->nombre_pokemon_length = strlen(nombre_pokemon) + 1;
    localized_pokemon->nombre_pokemon = nombre_pokemon;
    localized_pokemon->cantidad_coordenas = cantidad_coordenadas;
    localized_pokemon->coordenadas = malloc(cantidad_coordenadas*2*(sizeof(uint32_t)));

    va_list ap;
    va_start(ap, cantidad_coordenadas*2);
    for (int i = 0; i < cantidad_coordenadas*2; ++i) {
        localized_pokemon->coordenadas[i] = va_arg(ap, uint32_t);
    }
    va_end(ap);

    return localized_pokemon;
}

void* localized_pokemon_a_void(t_localized_pokemon* localized_pokemon){
    int len = sizeof(uint32_t) + localized_pokemon->nombre_pokemon_length + sizeof(uint32_t)*(localized_pokemon->cantidad_coordenas*2 + 1);
    void* stream = malloc(sizeof(uint32_t) + localized_pokemon->nombre_pokemon_length + sizeof(uint32_t)*(localized_pokemon->cantidad_coordenas*2 + 1));
    int offset = 0;

    memcpy(stream + offset, &localized_pokemon->nombre_pokemon_length, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, localized_pokemon->nombre_pokemon, localized_pokemon->nombre_pokemon_length);
    offset += localized_pokemon->nombre_pokemon_length;

    memcpy(stream + offset, &localized_pokemon->cantidad_coordenas, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, localized_pokemon->coordenadas, sizeof(uint32_t)*localized_pokemon->cantidad_coordenas*2);
    offset += sizeof(uint32_t)*localized_pokemon->cantidad_coordenas*2;

    return stream;
}

t_localized_pokemon* void_a_localized_pokemon(void* stream){
    t_localized_pokemon* localized_pokemon = malloc(sizeof(t_localized_pokemon));

    memcpy(&(localized_pokemon->nombre_pokemon_length), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    localized_pokemon->nombre_pokemon = malloc(localized_pokemon->nombre_pokemon_length);
    memcpy(localized_pokemon->nombre_pokemon, stream, localized_pokemon->nombre_pokemon_length);
    stream += localized_pokemon->nombre_pokemon_length;

    memcpy(&(localized_pokemon->cantidad_coordenas), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    localized_pokemon->coordenadas = malloc(sizeof(uint32_t)*localized_pokemon->cantidad_coordenas*2);
    memcpy(localized_pokemon->coordenadas, stream, sizeof(uint32_t)*localized_pokemon->cantidad_coordenas*2);
    stream += sizeof(uint32_t)*localized_pokemon->cantidad_coordenas*2;



    return localized_pokemon;
}

/*
 *
 * LOCALIZED_POKEMON ENDS
 *
 * */