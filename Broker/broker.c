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

    // Inicializo
    IDENTIFICADOR_MENSAJE = 1;
    IDENTIFICADOR_SUBSCRIPTOR = 1;

    // Inicializamos las colas
    list_new_pokemon = list_create();
    list_appeared_pokemon = list_create();
    list_get_pokemon = list_create();
    list_localized_pokemon = list_create();
    list_catch_pokemon = list_create();
    list_caught_pokemon = list_create();

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

            case SUB_NEW:;
                {
                    char* ip = (char*) list_get(cosas, 0);
                    int puerto = *((int*) list_get(cosas, 1));

                    //Luego hacer algo con ip y puerto

                    int respuesta = 1;
                    t_paquete* paquete = create_package(SUB_NEW);
                    add_to_package(paquete, (void*) &respuesta, sizeof(int));
                    send_package(paquete, fd);
                    break;
                }

            case SUB_APPEARED:;
                {
                    char* ip = (char*) list_get(cosas, 0);
                    int puerto = *((int*) list_get(cosas, 1));

                    //Luego hacer algo con ip y puerto

                    int respuesta = 1;
                    t_paquete* paquete = create_package(SUB_APPEARED);
                    add_to_package(paquete, (void*) &respuesta, sizeof(int));
                    send_package(paquete, fd);
                    break;
                }

            case SUB_LOCALIZED:;
                {
                    char* ip = (char*) list_get(cosas, 0);
                    int puerto = *((int*) list_get(cosas, 1));

                    //Luego hacer algo con ip y puerto

                    int respuesta = 1;
                    t_paquete* paquete = create_package(SUB_LOCALIZED);
                    add_to_package(paquete, (void*) &respuesta, sizeof(int));
                    send_package(paquete, fd);
                    break;
                }

            case SUB_CAUGHT:;
                {
                    char* ip = (char*) list_get(cosas, 0);
                    int puerto = *((int*) list_get(cosas, 1));

                    //Luego hacer algo con ip y puerto

                    int respuesta = 1;
                    t_paquete* paquete = create_package(SUB_CAUGHT);
                    add_to_package(paquete, (void*) &respuesta, sizeof(int));
                    send_package(paquete, fd);
                    break;
                }

            case SUB_GET:;
                {
                    char* ip = (char*) list_get(cosas, 0);
                    int puerto = *((int*) list_get(cosas, 1));

                    //Luego hacer algo con ip y puerto

                    int respuesta = 1;
                    t_paquete* paquete = create_package(SUB_GET);
                    add_to_package(paquete, (void*) &respuesta, sizeof(int));
                    send_package(paquete, fd);
                    break;
                }

            case SUB_CATCH:;
                {
                    char* ip = (char*) list_get(cosas, 0);
                    int puerto = *((int*) list_get(cosas, 1));

                    //Luego hacer algo con ip y puerto

                    int respuesta = 1;
                    t_paquete* paquete = create_package(SUB_CATCH);
                    add_to_package(paquete, (void*) &respuesta, sizeof(int));
                    send_package(paquete, fd);
                    break;
                }

            case NEW_POK:;
                {
                    uint32_t mensaje_co_id = *((uint32_t *) list_get(cosas, 0));
                    t_new_pokemon* new_pokemon = void_a_new_pokemon(list_get(cosas,1));

                    //mensaje* mensaje = mensaje_create(mensaje_id, mensaje_co_id, NEW_POK, sizeof_pokemon(new_pokemon));

                    //create_package(NEW_POK);

                    //cargar_mensaje(list_new_pokemon, mensaje);

                    //Envio el ID de respuesta
                    int respuesta = 1;
                    t_paquete* paquete = create_package(NEW_POK);
                    add_to_package(paquete, (void*) &respuesta, sizeof(int));
                    send_package(paquete, fd);
                    break;
                }

            case APPEARED_POK:;
                {
                    uint32_t mensaje_co_id = *((uint32_t *) list_get(cosas, 0));
                    t_appeared_pokemon* appeared_pokemon = void_a_appeared_pokemon(list_get(cosas,1));

                    //mensaje* mensaje = mensaje_create(mensaje_id, mensaje_co_id, APPEARED_POK, sizeof_pokemon());

                    //create_package(APPEARED_POK);

                    //cargar_mensaje(list_new_pokemon, mensaje);

                    //Envio el ID de respuesta
                    int respuesta = 1;
                    t_paquete* paquete = create_package(APPEARED_POK);
                    add_to_package(paquete, (void*) &respuesta, sizeof(int));
                    send_package(paquete, fd);
                    break;
                }

            case LOCALIZED_POK:;
                {
                    uint32_t mensaje_co_id = *((uint32_t *) list_get(cosas, 0));
                    t_localized_pokemon* localized_pokemon = void_a_localized_pokemon(list_get(cosas,1));

                    //mensaje* mensaje = mensaje_create(mensaje_id, mensaje_co_id, LOCALIZED_POK, sizeof_pokemon(new_pokemon));

                    //create_package(LOCALIZED_POK);

                    //cargar_mensaje(list_new_pokemon, mensaje);

                    //Envio el ID de respuesta
                    int respuesta = 1;
                    t_paquete* paquete = create_package(LOCALIZED_POK);
                    add_to_package(paquete, (void*) &respuesta, sizeof(int));
                    send_package(paquete, fd);
                    break;
                }

            case CAUGHT_POK:;
                {
                    uint32_t mensaje_co_id = *((uint32_t *) list_get(cosas, 0));
                    t_caught_pokemon* caught_pokemon = void_a_caught_pokemon(list_get(cosas,1));

                    //mensaje* mensaje = mensaje_create(mensaje_id, mensaje_co_id, CAUGHT_POK, sizeof_pokemon(new_pokemon));

                    //create_package(CAUGHT_POK);

                    //cargar_mensaje(list_new_pokemon, mensaje);

                    //Envio el ID de respuesta
                    int respuesta = 1;
                    t_paquete* paquete = create_package(CAUGHT_POK);
                    add_to_package(paquete, (void*) &respuesta, sizeof(int));
                    send_package(paquete, fd);
                    break;
                }

            case GET_POK:;
                {
                    uint32_t mensaje_co_id = *((uint32_t *) list_get(cosas, 0));
                    t_get_pokemon* get_pokemon = void_a_get_pokemon(list_get(cosas,1));

                    //mensaje* mensaje = mensaje_create(mensaje_id, mensaje_co_id, GET_POK, sizeof_pokemon(new_pokemon));

                    //create_package(GET_POK);

                    //cargar_mensaje(list_new_pokemon, mensaje);

                    //Envio el ID de respuesta
                    int respuesta = 1;
                    t_paquete* paquete = create_package(GET_POK);
                    add_to_package(paquete, (void*) &respuesta, sizeof(int));
                    send_package(paquete, fd);
                    break;
                }

            case CATCH_POK:;
                {
                    uint32_t mensaje_co_id = *((uint32_t *) list_get(cosas, 0));
                    t_catch_pokemon* catch_pokemon = void_a_catch_pokemon(list_get(cosas,1));

                    //mensaje* mensaje = mensaje_create(mensaje_id, mensaje_co_id, CATCH_POK, sizeof_pokemon(new_pokemon));

                    //create_package(CATCH_POK);

                    //cargar_mensaje(list_new_pokemon, mensaje);

                    //Envio el ID de respuesta
                    int respuesta = 1;
                    t_paquete* paquete = create_package(CATCH_POK);
                    add_to_package(paquete, (void*) &respuesta, sizeof(int));
                    send_package(paquete, fd);
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
    


    log_warning(test_logger, "Pasaron %d de %d tests", tests_run-tests_fail, tests_run);
    log_destroy(test_logger);
}


mensaje* mensaje_create(int id, int id_correlacional, MessageType tipo, size_t tam){
    mensaje* nuevo_mensaje = malloc(sizeof(mensaje));

    if(id == 0){
        id = IDENTIFICADOR_MENSAJE;
        IDENTIFICADOR_MENSAJE++;
    }

    nuevo_mensaje->id = id;
    nuevo_mensaje->id_correlacional = id_correlacional;
    nuevo_mensaje->tipo = tipo;
    nuevo_mensaje->enviados = list_create();
    nuevo_mensaje->confirmados = list_create();
    nuevo_mensaje->tam = tam;
    nuevo_mensaje->puntero_a_memoria = asignar_puntero_a_memoria();
    nuevo_mensaje->lru = unix_epoch();

    return nuevo_mensaje;
}


void* asignar_puntero_a_memoria(){
    // proximamente
    return NULL;
}

subscriptor* subscriptor_create(int id, char* ip, int puerto){
    subscriptor* nuevo_subscriptor = malloc(sizeof(subscriptor));

    if(id == 0){
        id = IDENTIFICADOR_SUBSCRIPTOR;
        IDENTIFICADOR_SUBSCRIPTOR++;
    }

    nuevo_subscriptor->id_subs = id;
    nuevo_subscriptor->ip_subs = ip;
    nuevo_subscriptor->puerto_subs = puerto;

    return nuevo_subscriptor;

}

//TODO: Hacer los otros
size_t sizeof_pokemon(t_new_pokemon* estructura){
    size_t tam = sizeof(uint32_t)*4;
    tam += estructura->nombre_pokemon_length;
    return tam;
}