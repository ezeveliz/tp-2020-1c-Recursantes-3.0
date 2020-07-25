//
// Created by utnso on 07/04/20.
//

#include "broker.h"


int main(int argc, char **argv) {
    if (argc != 2) {
        cfg_path = strdup("./broker.cfg");
    } else {
        cfg_path = strdup(argv[1]);
    }

    pthread_mutex_init(&M_MEMORIA_PRINCIPAL, NULL);
    pthread_mutex_init(&M_LIST_NEW_POKEMON, NULL);
    pthread_mutex_init(&M_LIST_APPEARED_POKEMON, NULL);
    pthread_mutex_init(&M_LIST_GET_POKEMON, NULL);
    pthread_mutex_init(&M_LIST_LOCALIZED_POKEMON, NULL);
    pthread_mutex_init(&M_LIST_CATCH_POKEMON, NULL);
    pthread_mutex_init(&M_LIST_CAUGHT_POKEMON, NULL);
    pthread_mutex_init(&M_SUBSCRIPTORES, NULL);
    pthread_mutex_init(&M_MENSAJES, NULL);
    pthread_mutex_init(&M_MENSAJE_SUBSCRIPTORE, NULL);
    pthread_mutex_init(&M_IDENTIFICADOR_MENSAJE, NULL);
    pthread_mutex_init(&M_PARTICIONES, NULL);
    pthread_mutex_init(&M_PARTICIONES_QUEUE, NULL);
    pthread_mutex_init(&M_ARBOL_BUDDY, NULL);
    pthread_mutex_init(&M_INTENTOS, NULL);
    pthread_mutex_init(&M_PROCESO, NULL);
    pthread_mutex_init(&M_RECURSAR, NULL);

    signal(SIGUSR1, dump_cache);

    // Logs que piden en el TP
    FILE* s = fopen("broker.log", "w"); // Reinicio los logs
    fclose(s);
    tp_logger = log_create("broker.log", "BROKER", 1, LOG_LEVEL_TRACE);

    // Logs propios
    s = fopen(".broker-log-propio.log", "w");
    fclose(s);
    logger = log_create(".broker-log-propio.log", "BROKER", 0, LOG_LEVEL_TRACE);
    log_info(logger,"Log started.");

    set_config();
    log_info(logger,"Configuration succesfully setted.");

    pthread_t server_thread;
    pthread_create(&server_thread, NULL, server_function, NULL);

    // Inicializo
    FILE* archivo_dump = fopen("dump.txt", "w");
    fclose(archivo_dump);
    IDENTIFICADOR_MENSAJE = 1;
    INTENTOS = 0;
    MIN_PART_LEN = config.min_partition_size;
    MEMORIA_PRINCIPAL = malloc(config.mem_size);
    SUBSCRIPTORES = list_create();
    MENSAJES = list_create();
    MENSAJE_SUBSCRIPTORE = list_create();
    PARTICIONES = list_create();
    log_debug(logger, "Creo la particion inicial del tamanio total de la memoria");
    particion* principal = particion_create(0, config.mem_size, true);
    list_add(PARTICIONES, principal);
    ARBOL_BUDDY = malloc(sizeof(t_nodo));
    ARBOL_BUDDY->particion = principal;
    ARBOL_BUDDY->der = NULL;
    ARBOL_BUDDY->izq = NULL;
    ARBOL_BUDDY->padre = NULL;
    ARBOL_BUDDY->es_hoja = true;

    //printPartList();


    // Inicializamos las colas
    LIST_NEW_POKEMON = list_create();
    LIST_APPEARED_POKEMON = list_create();
    LIST_GET_POKEMON = list_create();
    LIST_LOCALIZED_POKEMON = list_create();
    LIST_CATCH_POKEMON = list_create();
    LIST_CAUGHT_POKEMON = list_create();
    if(strcmp(config.mem_swap_algorithm, "FIFO")==0){
        PARTICIONES_QUEUE = list_create();
        log_debug(logger, "Se crea la 'cola' para FIFO");
    }


//    tests_broker();

//    particion* A = asignar_particion_buddy(ARBOL_BUDDY, 100);
//    raise(SIGUSR1);
//    particion* B = asignar_particion_buddy(ARBOL_BUDDY, 200);
//    raise(SIGUSR1);
//    particion* C = asignar_particion_buddy(ARBOL_BUDDY, 50);
//    raise(SIGUSR1);
//    particion* D = asignar_particion_buddy(ARBOL_BUDDY, 500);
//    raise(SIGUSR1);
//    buddy_liberar_particion(C);
//    raise(SIGUSR1);
//    buddy_liberar_particion(D);
//    raise(SIGUSR1);
//    buddy_liberar_particion(A);
//    raise(SIGUSR1);
//    buddy_liberar_particion(B);
//    raise(SIGUSR1);

    pthread_join(server_thread, NULL);

    pthread_mutex_destroy(&M_MEMORIA_PRINCIPAL);
    pthread_mutex_destroy(&M_LIST_NEW_POKEMON);
    pthread_mutex_destroy(&M_LIST_APPEARED_POKEMON);
    pthread_mutex_destroy(&M_LIST_GET_POKEMON);
    pthread_mutex_destroy(&M_LIST_LOCALIZED_POKEMON);
    pthread_mutex_destroy(&M_LIST_CATCH_POKEMON);
    pthread_mutex_destroy(&M_LIST_CAUGHT_POKEMON);
    pthread_mutex_destroy(&M_SUBSCRIPTORES);
    pthread_mutex_destroy(&M_MENSAJES);
    pthread_mutex_destroy(&M_MENSAJE_SUBSCRIPTORE);
    pthread_mutex_destroy(&M_IDENTIFICADOR_MENSAJE);
    pthread_mutex_destroy(&M_PARTICIONES);
    pthread_mutex_destroy(&M_PROCESO);
    pthread_mutex_destroy(&M_RECURSAR);

    log_error(logger, "Salgo del Broker");
    return EXIT_SUCCESS;
}

/*
 * Configuration starts
 */
void set_config(){
    cfg_file = config_create(cfg_path);

    if (!cfg_file) {
        log_error(logger, "No se encontró el archivo de configuración");
        exit(EXIT_FAILURE);
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
            log_info(tp_logger, "Se conecta un nuevo proceso");
        }
    }

    //--Funcion que se ejecuta cuando se pierde la conexion con un cliente
    void lost(int fd, char *ip, int port) {
        //Cierro la conexión fallida
        log_info(logger, "Se perdió una conexión");
        disconnect_subscriptor_by_fd(fd);
        log_info(logger, "Se desconecto el fd: %d", fd);
        close(fd);
    }

    //--funcion que se ejecuta cuando se recibe un nuevo mensaje de un cliente ya conectado
    void incoming(int fd, char *ip, int port, MessageHeader *headerStruct) {

        t_list *cosas = receive_package(fd, headerStruct);

        switch (headerStruct->type) {

            case SUB_NEW:;
                {
                    log_info(tp_logger, "Nuevo subscriptor de NEW");
                    pthread_mutex_lock(&M_LIST_NEW_POKEMON);
                    subscribir_a_cola(cosas, ip, port, fd, LIST_NEW_POKEMON, SUB_NEW);
                    pthread_mutex_unlock(&M_LIST_NEW_POKEMON);
                    break;
                }

            case SUB_APPEARED:;
                {
                    log_info(tp_logger, "Nuevo subscriptor de APPEARED");
                    pthread_mutex_lock(&M_LIST_APPEARED_POKEMON);
                    subscribir_a_cola(cosas, ip, port, fd, LIST_APPEARED_POKEMON, SUB_APPEARED);
                    pthread_mutex_unlock(&M_LIST_APPEARED_POKEMON);
                    break;
                }

            case SUB_LOCALIZED:;
                {
                    log_info(tp_logger, "Nuevo subscriptor de LOCALIZED");
                    pthread_mutex_lock(&M_LIST_LOCALIZED_POKEMON);
                    subscribir_a_cola(cosas, ip, port, fd, LIST_LOCALIZED_POKEMON, SUB_LOCALIZED);
                    pthread_mutex_unlock(&M_LIST_LOCALIZED_POKEMON);
                    break;
                }

            case SUB_CAUGHT:;
                {
                    log_info(tp_logger, "Nuevo subscriptor de CAUGHT");
                    pthread_mutex_lock(&M_LIST_LOCALIZED_POKEMON);
                    subscribir_a_cola(cosas, ip, port, fd, LIST_CAUGHT_POKEMON, SUB_CAUGHT);
                    pthread_mutex_unlock(&M_LIST_LOCALIZED_POKEMON);
                    break;
                }

            case SUB_GET:;
                {
                    log_info(tp_logger, "Nuevo subscriptor de GET");
                    pthread_mutex_lock(&M_LIST_GET_POKEMON);
                    subscribir_a_cola(cosas, ip, port, fd, LIST_GET_POKEMON, SUB_GET);
                    pthread_mutex_unlock(&M_LIST_GET_POKEMON);
                    break;
                }

            case SUB_CATCH:;
                {
                    log_info(tp_logger, "Nuevo subscriptor de CATCH");
                    pthread_mutex_lock(&M_LIST_CATCH_POKEMON);
                    subscribir_a_cola(cosas, ip, port, fd, LIST_CATCH_POKEMON, SUB_CATCH);
                    pthread_mutex_unlock(&M_LIST_CATCH_POKEMON);
                    break;
                }

            case NEW_POK:;
                {
                    pthread_mutex_lock(&M_PROCESO);
                    log_info(tp_logger, "Llega un mensaje a la cola NEW_POK");
                    // Le llega un un_mensaje
                    t_new_pokemon* new_pokemon = void_a_new_pokemon(list_get(cosas,0));

                    // Cargamos el un_mensaje en nuestro sistema
                    mensaje* un_mensaje = mensaje_create(0, 0, NEW_POK, sizeof_new_pokemon(new_pokemon));


                    // Guardamos el contenido del mensaje en la memoria principal
                    memcpy(un_mensaje->puntero_a_memoria, list_get(cosas,0), sizeof_new_pokemon(new_pokemon));
                  
                    // Cargamos el un_mensaje a la lista de New_pokemon
                    cargar_mensaje(LIST_NEW_POKEMON, un_mensaje);

                    //Envio el ID de respuesta
                    int respuesta = un_mensaje->id;
                    t_paquete* paquete = create_package(NEW_POK);
                    add_to_package(paquete, (void*) &respuesta, sizeof(int));
                    send_package(paquete, fd);
                    free_package(paquete);

                    pthread_mutex_unlock(&M_PROCESO);

                    // Enviamos los mensajes pendientes
                    recursar_operativos();
                    free(new_pokemon->nombre_pokemon);
                    free(new_pokemon);
                    break;
                }

            case APPEARED_POK:;
                {
                    pthread_mutex_lock(&M_PROCESO);
                    log_info(tp_logger, "Llega un mensaje a la cola APPEARED_POK");
                    uint32_t mensaje_co_id = *((uint32_t *) list_get(cosas, 0));
                    t_appeared_pokemon* appeared_pokemon = void_a_appeared_pokemon(list_get(cosas,1));

                    // Cargamos el un_mensaje en nuestro sistema
                    mensaje* un_mensaje = mensaje_create(0, mensaje_co_id, APPEARED_POK, sizeof_appeared_pokemon(appeared_pokemon));

                    // Guardamos el contenido del mensaje en la memoria principal
                    memcpy(un_mensaje->puntero_a_memoria, list_get(cosas,1), sizeof_appeared_pokemon(appeared_pokemon));

                    // Cargamos el un_mensaje a la lista de Appeared_pokemon
                    cargar_mensaje(LIST_APPEARED_POKEMON, un_mensaje);

                    //Envio el ID de respuesta
                    int respuesta = un_mensaje->id;
                    t_paquete* paquete = create_package(APPEARED_POK);
                    add_to_package(paquete, (void*) &respuesta, sizeof(int));
                    send_package(paquete, fd);
                    free_package(paquete);

                    pthread_mutex_unlock(&M_PROCESO);

                    // Enviamos los mensajes pendientes
                    recursar_operativos();
                    free(appeared_pokemon->nombre_pokemon);
                    free(appeared_pokemon);
                    break;
                }

            case LOCALIZED_POK:;
                {
                    pthread_mutex_lock(&M_PROCESO);
                    log_info(tp_logger, "Llega un mensaje a la cola LOCALIZED_POK");
                    uint32_t mensaje_co_id = *((uint32_t *) list_get(cosas, 0));
                    t_localized_pokemon* localized_pokemon = void_a_localized_pokemon(list_get(cosas,1));

                    // Cargamos el un_mensaje en nuestro sistema
                    mensaje* un_mensaje = mensaje_create(0, mensaje_co_id, LOCALIZED_POK, sizeof_localized_pokemon(localized_pokemon));

                    // Guardamos el contenido del mensaje en la memoria principal
                    memcpy(un_mensaje->puntero_a_memoria, list_get(cosas,1), sizeof_localized_pokemon(localized_pokemon));

                    // Cargamos el un_mensaje a la lista de Localized_pokemon
                    cargar_mensaje(LIST_LOCALIZED_POKEMON, un_mensaje);

                    //Envio el ID de respuesta
                    int respuesta = un_mensaje->id;
                    t_paquete* paquete = create_package(LOCALIZED_POK);
                    add_to_package(paquete, (void*) &respuesta, sizeof(int));
                    send_package(paquete, fd);
                    free_package(paquete);

                    pthread_mutex_unlock(&M_PROCESO);

                    // Enviamos los mensajes pendientes
                    recursar_operativos();
                    free(localized_pokemon->nombre_pokemon);
                    free(localized_pokemon->coordenadas);
                    free(localized_pokemon);
                    break;
                }

            case CAUGHT_POK:;
                {
                    pthread_mutex_lock(&M_PROCESO);
                    log_info(tp_logger, "Llega un mensaje a la cola CAUGHT_POK");
                    uint32_t mensaje_co_id = *((uint32_t *) list_get(cosas, 0));
                    t_caught_pokemon* caught_pokemon = void_a_caught_pokemon(list_get(cosas,1));

                    // Cargamos el un_mensaje en nuestro sistema
                    mensaje* un_mensaje = mensaje_create(0, mensaje_co_id, CAUGHT_POK, sizeof_caught_pokemon(caught_pokemon));

                    // Guardamos el contenido del mensaje en la memoria principal
                    memcpy(un_mensaje->puntero_a_memoria, list_get(cosas,1), sizeof_caught_pokemon(caught_pokemon));

                    // Cargamos el un_mensaje a la lista de Caught_pokemon
                    cargar_mensaje(LIST_CAUGHT_POKEMON, un_mensaje);

                    //Envio el ID de respuesta
                    int respuesta = un_mensaje->id;
                    t_paquete* paquete = create_package(CAUGHT_POK);
                    add_to_package(paquete, (void*) &respuesta, sizeof(int));
                    send_package(paquete, fd);
                    free_package(paquete);

                    pthread_mutex_unlock(&M_PROCESO);

                    // Enviamos los mensajes pendientes
                    recursar_operativos();
                    free(caught_pokemon);
                    break;
                }

            case GET_POK:;
                {
                    pthread_mutex_lock(&M_PROCESO);
                    log_info(tp_logger, "Llega un mensaje a la cola GET_POK");
                    t_get_pokemon* get_pokemon = void_a_get_pokemon(list_get(cosas,0));

                    // Cargamos el un_mensaje en nuestro sistema
                    mensaje* un_mensaje = mensaje_create(0, 0, GET_POK, sizeof_get_pokemon(get_pokemon));

                    // Guardamos el contenido del mensaje en la memoria principal
                    memcpy(un_mensaje->puntero_a_memoria, list_get(cosas,0), sizeof_get_pokemon(get_pokemon));

                    // Cargamos el un_mensaje a la lista de Get_pokemon
                    cargar_mensaje(LIST_GET_POKEMON, un_mensaje);

                    //Envio el ID de respuesta
                    int respuesta = un_mensaje->id;
                    t_paquete* paquete = create_package(GET_POK);
                    add_to_package(paquete, (void*) &respuesta, sizeof(int));
                    send_package(paquete, fd);
                    free_package(paquete);

                    pthread_mutex_unlock(&M_PROCESO);

                    // Enviamos los mensajes pendientes
                    recursar_operativos();
                    free(get_pokemon->nombre_pokemon);
                    free(get_pokemon);
                    break;
                }

            case CATCH_POK:;
                {
                    pthread_mutex_lock(&M_PROCESO);
                    log_info(tp_logger, "Llega un mensaje a la cola CATCH_POK");
                    t_catch_pokemon* catch_pokemon = void_a_catch_pokemon(list_get(cosas,0));

                    // Cargamos el un_mensaje en nuestro sistema
                    mensaje* un_mensaje = mensaje_create(0, 0, CATCH_POK, sizeof_catch_pokemon(catch_pokemon));

                    // Guardamos el contenido del mensaje en la memoria principal
                    memcpy(un_mensaje->puntero_a_memoria, list_get(cosas,0), sizeof_catch_pokemon(catch_pokemon));

                    // Cargamos el un_mensaje a la lista de Catch_pokemon
                    cargar_mensaje(LIST_CATCH_POKEMON, un_mensaje);

                    //Envio el ID de respuesta
                    int respuesta = un_mensaje->id;
                    t_paquete* paquete = create_package(CATCH_POK);
                    add_to_package(paquete, (void*) &respuesta, sizeof(int));
                    send_package(paquete, fd);
                    free_package(paquete);

                    pthread_mutex_unlock(&M_PROCESO);

                    // Enviamos los mensajes pendientes
                    recursar_operativos();
                    free(catch_pokemon->nombre_pokemon);
                    free(catch_pokemon);
                    break;
                }

            case ACK:;
                {
                    pthread_mutex_lock(&M_RECURSAR);
                    int id_subscriptor = *(int*) list_get(cosas, 0);
                    int id_mensaje = *(int*) list_get(cosas, 1);
                    pthread_mutex_lock(&M_MENSAJE_SUBSCRIPTORE);
                    log_info(tp_logger, "Recibimos el ACK del mensaje %d del suscriptor %d",
                             id_mensaje, id_subscriptor);
                    flag_ack(id_subscriptor, id_mensaje);
                    pthread_mutex_unlock(&M_MENSAJE_SUBSCRIPTORE);
                    pthread_mutex_unlock(&M_RECURSAR);
                    break;
                }

            default: {
                log_warning(logger, "Operacion desconocida. No quieras meter la pata\n");
                break;
            }
        }
        void element_destroyer(void* element){
            free(element);
        }
        list_destroy_and_destroy_elements(cosas, element_destroyer);
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

    // 1+1 = 2
    int tmp = 2;
    test_assert("1+1=2", tmp == (1+1));

    //        particion* nueva_particion1 = particion_create(3, 4, false);
//    particion* nueva_particion2 = particion_create(7, 2, false);
//    particion* nueva_particion4 = particion_create(9, 2, false);
//    particion* nueva_particion3 = particion_create(11, 7, false);
//    particion* particion_inicial = particion_create(18, 2, true);
    log_debug(logger, "NEW_POKEMON");
    t_new_pokemon* new_pika = create_new_pokemon("Pikachu", 3, 4, 2);
    size_t partition_size = sizeof_new_pokemon(new_pika);
    particion* base = asignar_particion(partition_size);
    log_debug(logger, "Base: %d", base);
    printPartList();
    /*
     *
     *
     *
     */

    log_debug(logger, "GET_POKEMON");
    t_get_pokemon* get_pika = create_get_pokemon("Pikachu");
    size_t partition_size1 = sizeof_get_pokemon(get_pika);
    particion* base1 = asignar_particion(partition_size1);
    log_debug(logger, "Base: %d", base1);
    printPartList();
    /*
     *
     *
     *
     *
     */
    log_debug(logger, "LOCALIZED_POKEMON");
    uint32_t coordenadas_tmp[] = {3, 4, 5, 6};
    t_localized_pokemon* loc_pika = create_localized_pokemon("Pikachu", 2, coordenadas_tmp);
    size_t partition_size2 = sizeof_localized_pokemon(loc_pika);
    particion* base2 = asignar_particion(partition_size2);
    log_debug(logger, "Base: %d", base2);
    printPartList();

    /*
  *
  *
  *
  *
  */
    log_debug(logger, "LOCALIZED_POKEMON");
    t_localized_pokemon* loc_pika1 = create_localized_pokemon("Pikachu", 2, coordenadas_tmp);
    size_t partition_size3 = sizeof_localized_pokemon(loc_pika1);
    particion* base3 = asignar_particion(partition_size3);
    log_debug(logger, "Base: %d", base3);
    printPartList();
    /*
     *
     *
     *
     *
     */
    log_debug(logger, "LOCALIZED_POKEMON");
    t_localized_pokemon* loc_pika2 = create_localized_pokemon("Pikachu", 2, coordenadas_tmp);
    size_t partition_size4 = sizeof_localized_pokemon(loc_pika2);
    particion* base4 = asignar_particion(partition_size4);
    log_debug(logger, "Base: %d", base4);
    printPartList();
    /*
     *
     *
     *
     *
     */
    log_debug(logger, "LOCALIZED_POKEMON");
    t_localized_pokemon* loc_pika3 = create_localized_pokemon("Pikachu", 2, coordenadas_tmp);
    size_t partition_size5 = sizeof_localized_pokemon(loc_pika3);
    particion* base5= asignar_particion(partition_size5);
    printPartList();
    /*
     *
     *
     *
     *
     */
    log_debug(logger, "Eliminamos GET_POKEMON(base:%d)", 24);
    particion_delete(24);
    printPartList();
    particion_delete(56);
    printPartList();

    particion_destroy(base5);

//    log_debug(logger, "Eliminamos LOCALIZED_POKEMON(base:%d)", 36);
//    particion_delete(36);
//    printPartList();
//    pthread_join(server_thread, NULL);
    compactar_particiones();
    printPartList();



    log_warning(test_logger, "Pasaron %d de %d tests", tests_run-tests_fail, tests_run);
    log_destroy(test_logger);
}

mensaje* mensaje_create(int id, int id_correlacional, MessageType tipo, size_t tam){
    mensaje* nuevo_mensaje = malloc(sizeof(mensaje));

    if(id == 0){
        pthread_mutex_lock(&M_IDENTIFICADOR_MENSAJE);
        id = IDENTIFICADOR_MENSAJE;
        IDENTIFICADOR_MENSAJE++;
        pthread_mutex_unlock(&M_IDENTIFICADOR_MENSAJE);
    }

    nuevo_mensaje->id = id;
    nuevo_mensaje->id_correlacional = id_correlacional;
    nuevo_mensaje->tipo = tipo;
    nuevo_mensaje->tam = tam;

    // Asignamos la particion
    particion* particion_libre;
    if(strcmp(config.mem_algorithm, "PARTICIONES") == 0){
        particion_libre = asignar_particion(tam);
    } else if(strcmp(config.mem_algorithm, "BS") == 0){
        int _tam = tam >= MIN_PART_LEN ? tam : MIN_PART_LEN;
        particion_libre = asignar_particion_buddy(ARBOL_BUDDY, _tam);
    } else{
        log_error(logger, "Unexpected algorithm");
        exit(EXIT_FAILURE);
    }

    particion_libre->mensaje = nuevo_mensaje;
    nuevo_mensaje->puntero_a_memoria = MEMORIA_PRINCIPAL + particion_libre->base;

    list_add(MENSAJES, nuevo_mensaje);

    log_info(tp_logger, "Se almaceno %s en la posicion %d", cola_to_string(tipo), particion_libre->base);
    log_debug(logger, "Se almaceno %s en la posicion %d", cola_to_string(tipo), particion_libre->base);
    return nuevo_mensaje;
}

// Se elimina el Mensaje de la lista de mensajes
void mensaje_destroy(int id_mensaje){
    bool id_search(void* un_mensaje){
        mensaje* m = (mensaje*) un_mensaje;
        return m->id == id_mensaje;
    }
    void element_destroyer(void* element){
        free(element);
    }
    list_remove_and_destroy_by_condition(MENSAJES, id_search, element_destroyer);
}


subscriptor* subscriptor_create(int id, char* ip, int puerto, int socket){
    subscriptor* nuevo_subscriptor = malloc(sizeof(subscriptor));

    nuevo_subscriptor->id_subs = id;
    nuevo_subscriptor->ip_subs = ip;
    nuevo_subscriptor->puerto_subs = puerto;
    nuevo_subscriptor->socket = socket;

    list_add(SUBSCRIPTORES, nuevo_subscriptor);

    return nuevo_subscriptor;

}


bool existe_sub_en_lista(int id, t_list* cola){
   bool id_search(void* un_sub){
       subscriptor* sub = (subscriptor*) un_sub;
       return sub->id_subs == id;
   }
   return (subscriptor*)list_find(cola, id_search) != NULL;
}

void subscriptor_delete(int id, t_list* cola){
    bool id_search(void* un_sub){
        subscriptor* sub = (subscriptor*) un_sub;
        return sub->id_subs == id;
    }
    void element_destroyer(void* element){
        free(element);
    }
    list_remove_and_destroy_by_condition(cola, id_search, element_destroyer);
}

//void printSubList(){
//    int size = list_size(SUBSCRIPTORES);
//    for(int i=0; i<size; i++){
//        subscriptor* s = list_get(SUBSCRIPTORES, i);
//        printf("id: %d, ip: %s, port: %d, socket: %d \n", s->id_subs, s->ip_subs, s->puerto_subs, s->socket);
//    }
//}
void printMenSubList(){
    int size = list_size(MENSAJE_SUBSCRIPTORE);
    for(int i=0; i<size; i++){
        mensaje_subscriptor* s = list_get(MENSAJE_SUBSCRIPTORE, i);
        printf("id_mensaje: %d, id_sub: %d, enviado: %s, ack: %s \n", s->id_mensaje, s->id_subscriptor, s->enviado ? "true" : "false", s->ack ? "true" : "false");
    }
}

//void printMenList() {
//    int size = list_size(MENSAJES);
//    for (int i = 0; i < size; i++) {
//        mensaje *s = list_get(MENSAJES, i);
//        printf("id_mensaje: %d, id_co: %d \n", s->id, s->id_correlacional);
//    }
//}
void printPartList() {
    int size = list_size(PARTICIONES);
    printf("<--------------------------------------------\n");
    for(int i=0; i<size; i++) {
        particion *s = list_get(PARTICIONES, i);
        printf("base: %d, tam: %d, is_free: %s, ultimo_uso: % " PRIu64 "\n", s->base, s->tam,
               s->libre ? "true" : "false", s->ultimo_uso);
    }
    printf("-------------------------------------------->\n");
}

mensaje_subscriptor* mensaje_subscriptor_create(int id_mensaje, int id_sub){
    log_debug(logger, "Se crea una nueva relacion mensaje[%d]-subscriptor[%d]", id_mensaje, id_sub);
    mensaje_subscriptor* nuevo_mensaje_subscriptor = malloc(sizeof(mensaje_subscriptor));

    nuevo_mensaje_subscriptor->id_mensaje = id_mensaje;
    nuevo_mensaje_subscriptor->id_subscriptor = id_sub;
    nuevo_mensaje_subscriptor->enviado = false;
    nuevo_mensaje_subscriptor->ack = false;

    list_add(MENSAJE_SUBSCRIPTORE, nuevo_mensaje_subscriptor);

    return nuevo_mensaje_subscriptor;

}

void mensaje_subscriptor_delete(int id_mensaje, int id_sub){
    log_error(logger, "Se borra la relacion mensaje[%d]-subscriptor[%d]", id_mensaje, id_sub);
    bool multiple_id_search(void* un_men_sub){
        mensaje_subscriptor* men_sub = (mensaje_subscriptor*) un_men_sub;
        return men_sub->id_mensaje == id_mensaje && men_sub->id_subscriptor == id_sub;
    }
    void element_destroyer(void* element){
        free(element);
    }
    list_remove_and_destroy_by_condition(MENSAJE_SUBSCRIPTORE, multiple_id_search, element_destroyer);

    int cantidad_mensajes = list_size(MENSAJE_SUBSCRIPTORE);
    log_error(logger, "Hay %d mensajes pendientes", cantidad_mensajes);
}

void subscribir_a_cola(t_list* cosas, char* ip, int puerto, int fd, t_list* una_cola, MessageType tipo){
    pthread_mutex_lock(&M_PROCESO);
    pthread_mutex_lock(&M_SUBSCRIPTORES);
    pthread_mutex_lock(&M_MENSAJES);
    int id = *((int*) list_get(cosas, 0));
    log_warning(logger, "Nuevo SUB_%s id:%d fd:%d ip:%s port:%d", cola_to_string(sub_to_men(tipo)), id, fd, ip, puerto);

    // Si cambia el puerto o la ip lo borro y vuelvo a crearlo
    if(existe_sub_en_lista(id, SUBSCRIPTORES)){
        log_warning(logger, "Ya existia el subscriptor en el sistema");
        subscriptor_actualizar_fd(id, fd);

        if(!existe_sub_en_lista(id, una_cola)){
            log_warning(logger, "Pero no estaba en la cola %s, asi que lo agrego", cola_to_string(sub_to_men(tipo)));
            subscriptor* nuevo_subscriptor = find_subscriptor(id);
            list_add(una_cola, nuevo_subscriptor);
        }

    } else{
        log_debug(logger, "Se crea un nuevo subscriptor de %s - ID:%d FD:%d", cola_to_string(sub_to_men(tipo)), id, fd);
        subscriptor* nuevo_subscriptor = subscriptor_create(id, ip, puerto, fd);
        list_add(una_cola, nuevo_subscriptor);
        nuevo_subscriptor->conectado = true;
    }

    int respuesta = 1;
    t_paquete* paquete = create_package(tipo);
    add_to_package(paquete, (void*) &respuesta, sizeof(int));
    send_package(paquete, fd);
    free_package(paquete);

    // Busque los mensajes antiguos en memoria y se cree las estructuras mensaje_subscriptor
    for (int i = 0; i < list_size(MENSAJES); ++i) {
        mensaje* un_mensaje = list_get(MENSAJES, i);
        if (un_mensaje->tipo == sub_to_men(tipo)){
            mensaje_subscriptor_create(un_mensaje->id, id);
        }
    }


    pthread_mutex_unlock(&M_MENSAJES);
    pthread_mutex_unlock(&M_SUBSCRIPTORES);

    recursar_operativos();
    pthread_mutex_unlock(&M_PROCESO);
}

void subscriptor_actualizar_fd(int id, int fd){
    int cant = list_size(SUBSCRIPTORES);
    for (int i = 0; i < cant; ++i) {
        subscriptor* un_sub = list_get(SUBSCRIPTORES, i);
        if (un_sub->id_subs == id){
            log_debug(logger, "Actualizamos el subscriptor %d con el fd:%d", id, fd);
            un_sub->socket = fd;
            un_sub->conectado = true;
        }
    }
}

// Carga mensajes si no estaban antes
// Crea las relaciones Mensaje-Subscriptor
void cargar_mensaje(t_list* una_cola, mensaje* un_mensaje){
    pthread_mutex_lock(&M_MENSAJE_SUBSCRIPTORE);
    int cantidad_subs = list_size(una_cola);
    for (int i = 0; i < cantidad_subs; ++i) {
        subscriptor* un_subscriptor = list_get(una_cola, i);
        if(!existe_mensaje_subscriptor(un_mensaje->id, un_subscriptor->id_subs)){
            mensaje_subscriptor_create(un_mensaje->id, un_subscriptor->id_subs);
        }
        cantidad_subs = list_size(una_cola);
    }
    pthread_mutex_unlock(&M_MENSAJE_SUBSCRIPTORE);
}

// Elimina todos los Mensaje-Subscriptor de ese Mensaje
void descargar_mensaje(t_list* una_cola, int un_mensaje){
    pthread_mutex_lock(&M_MENSAJE_SUBSCRIPTORE);
    int cantidad_subs = list_size(una_cola);
    for (int i = 0; i < cantidad_subs; ++i) {
        subscriptor* un_subscriptor = list_get(una_cola, i);
        if(existe_mensaje_subscriptor(un_mensaje, un_subscriptor->id_subs)){
            mensaje_subscriptor_delete(un_mensaje, un_subscriptor->id_subs);
        }
        cantidad_subs = list_size(una_cola);
    }
    pthread_mutex_unlock(&M_MENSAJE_SUBSCRIPTORE);
}

bool existe_mensaje_subscriptor(int id_mensaje, int id_subs){
    for (int i = 0; i < list_size(MENSAJE_SUBSCRIPTORE); ++i) {
        mensaje_subscriptor* relacion = list_get(MENSAJE_SUBSCRIPTORE, i);
        if (relacion->id_mensaje == id_mensaje && relacion->id_subscriptor == id_subs){
            return true;
        }
    }
    return false;
}


mensaje* find_mensaje(int id){
    bool id_search(void* un_mensaje){
        mensaje* message = (mensaje*) un_mensaje;
        return message->id == id;
    }
    mensaje* encontrado = list_find(MENSAJES, id_search);
    return encontrado;
}

subscriptor* find_subscriptor(int id){
    bool id_search(void* un_sub){
        subscriptor* sub = (subscriptor*) un_sub;
        return sub->id_subs == id;
    }

    subscriptor* encontrado = list_find(SUBSCRIPTORES, id_search);
    return encontrado;
}

void disconnect_subscriptor_by_fd(int fd){
    bool id_search(void* un_sub){
        subscriptor* sub = (subscriptor*) un_sub;
        return sub->socket == fd;
    }

    subscriptor* encontrado = list_find(SUBSCRIPTORES, id_search);
    if(encontrado != null){encontrado->conectado = false;}
}


// Esta funcion recorre la lista MENSAJE_SUBSCRIPTORE mandando los mensajes pendientes
void recursar_operativos(){
    pthread_mutex_lock(&M_RECURSAR);

    int cantidad_mensajes = list_size(MENSAJE_SUBSCRIPTORE);
    log_debug(logger, "Hay %d mensajes pendientes", cantidad_mensajes);
    for (int i = 0; i < cantidad_mensajes; ++i) {
        mensaje_subscriptor* coso = list_get(MENSAJE_SUBSCRIPTORE, i);
        void* cosito = mensaje_subscriptor_a_void(coso);


        if(!coso->enviado){

            pthread_t mensaje_thread;
            pthread_create(&mensaje_thread, NULL, mandar_mensaje, cosito);
            pthread_join(mensaje_thread, NULL);
//            pthread_detach(mensaje_thread);
            /*
             * Si le pongo detach:
             * puede salir de aca antes de terminar de mandar los mensaje
             * llegar un mensaje nuevo
             * borrar el viejo que todavia no se mando
             * y cuando se quiera mandar no tener mensaje
             */

        } else{
            free(cosito);
        }
        // Vuelvo a actualizar el tamaño por si entró alguien en el medio
        cantidad_mensajes = list_size(MENSAJE_SUBSCRIPTORE);
    }
    pthread_mutex_unlock(&M_RECURSAR);
}
int send_message_test(t_paquete* paquete, int socket){
    return 1;
}
void* mandar_mensaje(void* cosito){
    log_info(logger, "Entro en mandar mensaje");
    mensaje_subscriptor* coso = void_a_mensaje_subscriptor(cosito);

    subscriptor* un_subscriptor = find_subscriptor(coso->id_subscriptor);
    mensaje* un_mensaje = find_mensaje(coso->id_mensaje);
    if (!un_mensaje){
        log_error(logger, "Mandar mensaje no encontro el MENSAJE con id %d", coso->id_mensaje);
        free(coso);
        return null;
    }
    if (!un_subscriptor){
        log_error(logger, "Mandar mensaje no encontro el SUBSCRIPTOR con id %d", coso->id_subscriptor);
        free(coso);
        return null;
    }

    t_paquete* paquete = create_package(un_mensaje->tipo);
    add_to_package(paquete, (void*) &un_mensaje->id, sizeof(int));
    add_to_package(paquete, (void*) &un_mensaje->id_correlacional, sizeof(int));
    add_to_package(paquete, un_mensaje->puntero_a_memoria, un_mensaje->tam);
    log_error(logger, "Estoy justo antes del send_package()");
    if(!un_subscriptor->conectado){log_warning(logger,"El subscriptor no se encuentra conectado");return null;}
    if (send_package(paquete, un_subscriptor->socket) > 0){
        log_info(tp_logger, "Se envia el mensaje %d al suscriptor %d", un_mensaje->id, un_subscriptor->id_subs);
        log_debug(logger, "Se envia el mensaje %d al suscriptor %d", un_mensaje->id, un_subscriptor->id_subs);
        flag_enviado(coso->id_subscriptor, coso->id_mensaje);
        // Actualizo el LRU
        particion* una_particion = find_particion_by_id_mensaje(un_mensaje->id);
        if (!una_particion){
            log_error(logger, "Mandar mensaje no encontro la particion del id mensaje %d", un_mensaje->id);
            free(coso);
            return null;
        }
        una_particion->ultimo_uso = unix_epoch();
    }
    log_error(logger, "Estoy justo despues del send_package()");
    free(coso);
    free(cosito);
    free_package(paquete);
    log_info(logger, "Salgo de mandar mensaje");
    return null;
}

particion* find_particion_by_id_mensaje(int id_mensaje){
    bool id_search(void* una_part){
        particion* part_encontrada = (particion*) una_part;
        if (part_encontrada->mensaje == null){return false;}
        return part_encontrada->mensaje->id == id_mensaje;
    }
    particion* encontrado = list_find(PARTICIONES, id_search);
    return encontrado;
}

void* mensaje_subscriptor_a_void(mensaje_subscriptor* un_men_sub){
    void* stream = malloc(sizeof(uint32_t)*2 + sizeof(bool)*2);
    int offset = 0;

    memcpy(stream + offset, &un_men_sub->id_mensaje, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, &un_men_sub->id_subscriptor, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, &un_men_sub->enviado, sizeof(bool));
    offset += sizeof(bool);

    memcpy(stream + offset, &un_men_sub->ack, sizeof(bool));
    offset += sizeof(bool);

    return stream;

}

mensaje_subscriptor* void_a_mensaje_subscriptor(void* stream){
    mensaje_subscriptor* un_men_sub = malloc(sizeof(mensaje_subscriptor));

    memcpy(&(un_men_sub->id_mensaje), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    memcpy(&(un_men_sub->id_subscriptor), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    memcpy(&(un_men_sub->enviado), stream, sizeof(bool));
    stream += sizeof(bool);

    memcpy(&(un_men_sub->ack), stream, sizeof(bool));
    stream += sizeof(bool);

    return un_men_sub;

}
void flag_enviado(uint32_t id_sub, uint32_t id_men){
    for(int i = 0; i<list_size(MENSAJE_SUBSCRIPTORE);i++){
        mensaje_subscriptor* x = list_get(MENSAJE_SUBSCRIPTORE, i);
        if(x->id_mensaje == id_men && x->id_subscriptor == id_sub){x->enviado = true;}
    }
}

// ATENCION AHORA EL flag_ack ELIMINA EL MENSAJE_SUBSCRIPTOR
void flag_ack(uint32_t id_sub, uint32_t id_men){
    for(int i = 0; i<list_size(MENSAJE_SUBSCRIPTORE);i++){
        mensaje_subscriptor* x = list_get(MENSAJE_SUBSCRIPTORE, i);
        if(x->id_mensaje == id_men && x->id_subscriptor == id_sub){x->ack = true;}
    }
    mensaje_subscriptor_delete(id_men, id_sub);
}

/*
███╗   ███╗███████╗███╗   ███╗ ██████╗ ██████╗ ██╗ █████╗
████╗ ████║██╔════╝████╗ ████║██╔═══██╗██╔══██╗██║██╔══██╗
██╔████╔██║█████╗  ██╔████╔██║██║   ██║██████╔╝██║███████║
██║╚██╔╝██║██╔══╝  ██║╚██╔╝██║██║   ██║██╔══██╗██║██╔══██║
██║ ╚═╝ ██║███████╗██║ ╚═╝ ██║╚██████╔╝██║  ██║██║██║  ██║
╚═╝     ╚═╝╚══════╝╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═╝╚═╝╚═╝  ╚═╝
*/


particion* particion_create(int base, int tam, bool is_free){
    particion* nueva_particion = malloc(sizeof(particion));

    nueva_particion->base = base;
    nueva_particion->tam = tam;
    nueva_particion->libre = is_free;
    nueva_particion->ultimo_uso = unix_epoch();
    nueva_particion->mensaje = null;

    return nueva_particion;
}

void particion_delete(int base){
    for(int i = 0; i<list_size(PARTICIONES);i++){
        particion* x = list_get(PARTICIONES, i);
        if(x->base == base){
            x->libre = true;
            if(strcmp(config.mem_swap_algorithm, "FIFO")==0){quitarVictimaFIFO(x->base);};
            log_info(tp_logger, "Se elimina la particion con base %d", x->base);
        }
    }
    ordenar_particiones();
}

particion* buscar_particion_libre(int tam){
    if (strcmp(config.free_partition_algorithm, "FF") == 0) {
        log_debug(logger, "First fit search starts...");
        return first_fit_search(tam);
    } else if (strcmp(config.free_partition_algorithm, "BF") == 0) {
        log_debug(logger, "Best fit search starts...");
        return best_fit_search(tam);
    } else {
        log_error(logger, "Unexpected algorithm");
        exit(EXIT_FAILURE);
    }
}

particion* first_fit_search(int tam){
    int size = list_size(PARTICIONES);
    for(int i=0; i<size; i++){
        particion* x = list_get(PARTICIONES, i);
        if(x->libre == true && tam <= x->tam && x->tam >= MIN_PART_LEN){
            log_info(logger, "Free partition with enough size found!(base: %d)", x->base);
            return x;
        }
    }
    log_warning(logger, "There are not availables partitions  :|");
    return NULL;
}

particion* best_fit_search(int tam){
    int size = list_size(PARTICIONES);
    t_list* candidatos = list_create();
    for(int i=0; i<size; i++){
        particion* x = list_get(PARTICIONES, i);
        if(x->libre == true && tam <= x->tam && x->tam >= MIN_PART_LEN){
            log_info(logger, "Free partition with enough size found!(base: %d)", x->base);
            if(tam == x->tam){log_info(logger, "Best fit partition found!(base:%d)", x->base);return x;}
            list_add(candidatos, x);
        }
    }
    log_debug(logger, "Looking for the best fit one..");
    int candidatos_size = list_size(candidatos);
    if(candidatos_size != 0){
        particion* best_fit;
        int best_fit_diff = 999999;
        for(int i=0; i<candidatos_size; i++){
            particion* y = list_get(candidatos, i);
            int diff = y->tam - tam;
            if(best_fit_diff > diff){
                best_fit_diff = diff;
                best_fit = y;
            }
        }
        log_info(logger, "Best fit partition found!(base:%d)", best_fit->base);
        return best_fit;
    }else{
        log_warning(logger, "There are not availables partitions :|");
        return NULL;
    }
}

void algoritmo_de_reemplazo(){
    // Antes de reemplazar, envio los mensajes pendientes
    recursar_operativos();

    pthread_mutex_lock(&M_RECURSAR);
    particion* una_particion;
    if(strcmp(config.mem_swap_algorithm, "FIFO") == 0){
        log_debug(logger, "FIFO victim search starts...");
        una_particion = get_fifo();
    }else if (strcmp(config.mem_swap_algorithm, "LRU") == 0){
        log_debug(logger, "LRU victim search starts...");
        una_particion = get_lru();
    }else{
        log_error(logger, "Unexpected algorithm");
        exit(EXIT_FAILURE);
    }
    log_error(logger, "Se borra la particion con base: %d", una_particion->base);
    log_info(tp_logger, "Se borra la particion con base: %d", una_particion->base);

    // Borro el mensaje de las estrucutras
    log_error(logger, "Borramos el mensaje: %d", una_particion->mensaje->id);
    t_list* cola = men_to_cola(una_particion->mensaje->tipo);
    descargar_mensaje(cola, una_particion->mensaje->id);
    mensaje_destroy(una_particion->mensaje->id);

    if(strcmp(config.mem_algorithm, "PARTICIONES") == 0){
        particion_delete(una_particion->base);
    } else if(strcmp(config.mem_algorithm, "BS") == 0){
        buddy_liberar_particion(una_particion);
        printPartList();
    } else{
        log_error(logger, "Unexpected algorithm");
        exit(EXIT_FAILURE);
    }
    pthread_mutex_unlock(&M_RECURSAR);
}

/*
 * Se buscará una partición libre que tenga suficiente memoria continua como para contener el valor.
 * En caso de no encontrarla, se pasará al paso siguiente (si corresponde, en caso contrario se pasará al paso 3 directamente).
 *
 * Se compactará la memoria y se realizará una nueva búsqueda.
 * En caso de no encontrarla, se pasará al paso siguiente.
 *
 * Se procederá a eliminar una partición de datos. Luego, si no se pudo encontrar una partición con suficiente memoria
 * como para contener el valor, se volverá al paso 2 o al 3 según corresponda.
 *
 * Se deberá poder configurar la frecuencia de compactación (en la unidad “cantidad de búsquedas fallidas”).
 * El valor -1 indicará compactar solamente cuando se hayan eliminado todas las particiones.
 */

particion* asignar_particion(size_t tam) {
    pthread_mutex_lock(&M_INTENTOS);
    pthread_mutex_lock(&M_MEMORIA_PRINCIPAL);
    pthread_mutex_lock(&M_PARTICIONES);
    pthread_mutex_lock(&M_PARTICIONES_QUEUE);
    particion *particion_libre = buscar_particion_libre(tam);
    if (particion_libre != NULL) {
        log_info(logger, "Doing the job..");
        //Si la particion libre encontrada es de igual tamanio a la particion a alojar no es necesario ordenar
        if (particion_libre->tam == tam) {

            //Cambio el estado de la libre a falso y actualiza su ultimo uso.
            particion_libre->libre = false;
            particion_libre->ultimo_uso = unix_epoch();
            if(strcmp(config.mem_swap_algorithm, "FIFO") == 0){list_add(PARTICIONES_QUEUE, particion_libre);}

            log_info(logger, "Partition assigned(base: %d)", particion_libre->base);
            pthread_mutex_unlock(&M_PARTICIONES_QUEUE);
            pthread_mutex_unlock(&M_PARTICIONES);
            pthread_mutex_unlock(&M_MEMORIA_PRINCIPAL);
            pthread_mutex_unlock(&M_INTENTOS);
            return particion_libre;
        }
        //Si no es de igual tamano, debo crear una nueva particion con base en la libre y reacomodar la base y tamanio de la libre.
        else {
            tam = tam >= MIN_PART_LEN ? tam : MIN_PART_LEN;
            particion* nueva_particion = particion_create(particion_libre->base, tam, false);
            list_add(PARTICIONES, nueva_particion);
            if(strcmp(config.mem_swap_algorithm, "FIFO") == 0){list_add(PARTICIONES_QUEUE, nueva_particion);}
            //actualizo base y tamanio de particion libre.
            particion_libre->base += tam;
            particion_libre->tam -= tam;


            //Finalmente, ordeno la lista PARTICIONES por base en orden ASC


            log_info(logger, "Partition assigned(base: %d)", nueva_particion->base);
            log_info(logger, "Rearranging partitions...");
            ordenar_particiones();
            mergear_particiones_libres();
            log_info(logger, "Ready");
            pthread_mutex_unlock(&M_PARTICIONES_QUEUE);
            pthread_mutex_unlock(&M_PARTICIONES);
            pthread_mutex_unlock(&M_MEMORIA_PRINCIPAL);
            pthread_mutex_unlock(&M_INTENTOS);
            return nueva_particion;
        }
    } else {
        log_warning(logger, "It was not possible to assign partition!");
        if((INTENTOS >= config.compactation_freq) && (config.compactation_freq!=-1)){
            compactar_particiones();
            INTENTOS = 0;
        } else{
            algoritmo_de_reemplazo();
            mergear_particiones_libres();
            INTENTOS++;
        }
        pthread_mutex_unlock(&M_PARTICIONES_QUEUE);
        pthread_mutex_unlock(&M_PARTICIONES);
        pthread_mutex_unlock(&M_MEMORIA_PRINCIPAL);
        pthread_mutex_unlock(&M_INTENTOS);
        // La recursivistica concha de tu hermana como nuestras cursadas de operativos
        return asignar_particion(tam);
    }
}

// Ordena las particiones y mergea las particiones libres
void ordenar_particiones(){
    bool particion_anterior(particion* particion_antes, particion* particion_despues) {
        return particion_antes->base < particion_despues->base;
    }

    list_sort(PARTICIONES, (void*) particion_anterior);

    return;
}

// Recorro la tabla, si encuentro dos particiones libres consecutivas las mergeo
void mergear_particiones_libres(){
    int size = list_size(PARTICIONES);
    for(int i=0; i<size-1; i++){

        particion* una_particion = list_get(PARTICIONES, i);
        particion* siguiente_particion = list_get(PARTICIONES, i + 1);

        if (una_particion->libre && siguiente_particion->libre){

            una_particion->tam += siguiente_particion->tam;
            list_remove(PARTICIONES, i+1);
            free(siguiente_particion);
            size = list_size(PARTICIONES);
            i = 0;
        }

    }
    return;
}

void dump_cache(int sig){
    // Loggeo y abro el archivo
    log_info(tp_logger, "Se solicito un Dump de cache");
    FILE* archivo_dump = fopen("dump.txt", "a");

    // Printeo la fecha y hora
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(archivo_dump, "Dump: %02d/%02d/%d %02d:%02d:%02d\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
    fflush(archivo_dump);

    // Printeo el contenido de la cache
    pthread_mutex_lock(&M_PARTICIONES);
    int size = list_size(PARTICIONES);
    for(int i=0; i<size; i++) {
        particion *s = list_get(PARTICIONES, i);
        //Todo: %06p
        fprintf(archivo_dump, "Particion %d: %06d-%06d\t"
                              "[%s]\t"
                              "Size: %db",
                              i+1, s->base, s->base+s->tam,
                              s->libre ? "L" : "X",
                              s->tam);
        if(!s->libre){
            fprintf(archivo_dump, "\tLRU: %" PRIu64 "\tCola: %s\tID: %d",
                    s->ultimo_uso,
                    cola_to_string(s->mensaje->tipo),
                    s->mensaje->id);
        }
        fprintf(archivo_dump, "\n");
    }
    pthread_mutex_unlock(&M_PARTICIONES);
    fprintf(archivo_dump, "\n");

    // Cierro el archivo y libero la memoria
    fclose(archivo_dump);
    return;
}

void compactar_particiones(){
    log_debug(logger, "COMPACTAMO LO COSO");
    log_info(tp_logger, "COMPACTAMOS");
    int size = list_size(PARTICIONES);
    for(int i=0; i<size;i++){
        particion* particion_libre = list_get(PARTICIONES, i);
        if(particion_libre->libre){
            for(int z=i+1;z<size;z++){
                particion* particion_ocupada = list_get(PARTICIONES, z);
                if(!particion_ocupada->libre){

                    // Movemos primero la memoria real
                    memcpy(MEMORIA_PRINCIPAL + particion_libre->base,
                           particion_ocupada->mensaje->puntero_a_memoria,
                           particion_ocupada->mensaje->tam);
                    particion_ocupada->mensaje->puntero_a_memoria = MEMORIA_PRINCIPAL + particion_libre->base;

                    // Despues acomodamos las estrucuras
                    particion_ocupada->base = particion_libre->base;
                    particion_libre->base += particion_ocupada->tam;

                    ordenar_particiones();
                    mergear_particiones_libres();
                    //  printPartList();
                    size = list_size(PARTICIONES);
                    break;
                }
            }
        }
    }
    return;
}

char* cola_to_string(MessageType cola) {
    switch (cola) {
        case NEW_POK:
            return "NEW_POK";
        case GET_POK:
            return "GET_POK";
        case CATCH_POK:
            return "CATCH_POK";
        case APPEARED_POK:
            return "APPEARED_POK";
        case LOCALIZED_POK:
            return "LOCALIZED_POK";
        case CAUGHT_POK:
            return "CAUGHT_POK";
        default:
            return "No es una cola";
    }
}

MessageType sub_to_men(MessageType cola) {
    switch (cola) {
        case SUB_NEW:
            return NEW_POK;
        case SUB_GET:
            return GET_POK;
        case SUB_CATCH:
            return CATCH_POK;
        case SUB_APPEARED:
            return APPEARED_POK;
        case SUB_LOCALIZED:
            return LOCALIZED_POK;
        case SUB_CAUGHT:
            return CAUGHT_POK;
        default:
            log_error(logger, "Error en sub_to_men");
            exit(EXIT_FAILURE);
    }
}

t_list* men_to_cola(MessageType tipo) {
    switch (tipo) {
        case NEW_POK:
            return LIST_NEW_POKEMON;
        case GET_POK:
            return LIST_GET_POKEMON;
        case CATCH_POK:
            return LIST_CATCH_POKEMON;
        case APPEARED_POK:
            return LIST_APPEARED_POKEMON;
        case LOCALIZED_POK:
            return LIST_LOCALIZED_POKEMON;
        case CAUGHT_POK:
            return LIST_CAUGHT_POKEMON;
        default:
            log_error(logger, "Error en men_to_cola");
            exit(EXIT_FAILURE);
    }
}

particion* get_fifo(){
    return list_get(PARTICIONES_QUEUE, 0);
}

void quitarVictimaFIFO(int base){
    int size = list_size(PARTICIONES_QUEUE);
    for(int i=0; i<size; i++){
        particion* x = list_get(PARTICIONES_QUEUE, i);
        if(x->base == base){
            list_remove(PARTICIONES_QUEUE, i);
            break;
        }
    }
}

particion* get_lru(){
    uint64_t lru_ts = unix_epoch();
    particion* lru_p;
    int size = list_size(PARTICIONES);
    for(int i=0;i<size;i++){
        particion* x = list_get(PARTICIONES, i);
        if(x->ultimo_uso < lru_ts && !x->libre){
            lru_ts = x->ultimo_uso;
            lru_p = x;
        }
    }
    return lru_p;
}

/*
██████╗ ██╗   ██╗██████╗ ██████╗ ██╗   ██╗
██╔══██╗██║   ██║██╔══██╗██╔══██╗╚██╗ ██╔╝
██████╔╝██║   ██║██║  ██║██║  ██║ ╚████╔╝
██╔══██╗██║   ██║██║  ██║██║  ██║  ╚██╔╝
██████╔╝╚██████╔╝██████╔╝██████╔╝   ██║
╚═════╝  ╚═════╝ ╚═════╝ ╚═════╝    ╚═╝
*/

// Devuelve el nodo que tiene una particion de IGUAL tamaño
t_nodo* buscar_nodo_tam(struct t_nodo* nodo, int tam){
    // Si es null devuelvo null
    if(nodo == NULL){
        return NULL;
    }
    // Si la particion esta libre y hay espacio devuelvo ese nodo
    particion* una_particion = nodo->particion;
    if(nodo->es_hoja && una_particion->tam == tam && una_particion->libre){
        return nodo;
    }
    // Si no busco a izquierda
    t_nodo* res_izq = buscar_nodo_tam(nodo->izq, tam);
    if(res_izq){
        return res_izq;
    }
    // Si no busco a derecha
    t_nodo* res_der = buscar_nodo_tam(nodo->der, tam);
    if(res_der){
        return res_der;
    }
}

particion* asignar_particion_buddy(t_nodo* raiz, size_t tam) {
    pthread_mutex_lock(&M_MEMORIA_PRINCIPAL);
    pthread_mutex_lock(&M_PARTICIONES);
    pthread_mutex_lock(&M_PARTICIONES_QUEUE);
    pthread_mutex_lock(&M_ARBOL_BUDDY);
    log_info(logger, "Empiezo con el Buddy system");
    int potencia = 0;
    while (tam > pow(2, potencia)){
        potencia++;
    }
    int tam_buscado = pow(2, potencia);
    log_info(logger, "La potencia mas chica que cumple el tamaño es: %d", tam_buscado);

    t_nodo* nodo_respuesta = buscar_nodo_tam(raiz, tam_buscado);
    if (nodo_respuesta != NULL){
        log_info(logger, "Caso lindo: encontramos una particion del tamaño %d", tam_buscado);

        //Cambio el estado de la libre a falso y actualiza su ultimo uso.
        nodo_respuesta->particion->libre = false;
        nodo_respuesta->particion->ultimo_uso = unix_epoch();
        if(strcmp(config.mem_swap_algorithm, "FIFO") == 0){list_add(PARTICIONES_QUEUE, nodo_respuesta->particion);}

        log_info(logger, "Particion asignada (base: %d)", nodo_respuesta->particion->base);
        pthread_mutex_unlock(&M_ARBOL_BUDDY);
        pthread_mutex_unlock(&M_PARTICIONES_QUEUE);
        pthread_mutex_unlock(&M_PARTICIONES);
        pthread_mutex_unlock(&M_MEMORIA_PRINCIPAL);
        return nodo_respuesta->particion;
    } else {
        log_info(logger, "Caso feo: No encontramos una particion del tamaño %d", tam_buscado);
        // No hay una particion del tamaño buscado
        // Vamos a buscar, entre las que tenemos, cual tiene tamaño mas cercano a lo que buscamos

        int nuevo_tam_buscado = tam_buscado * 2;
        t_nodo* nodo_a_dividir = buscar_nodo_tam(ARBOL_BUDDY, nuevo_tam_buscado);
        while (nodo_a_dividir == NULL) {
            // Si no encuentro una particion mas grande, aumento el tamaño y vuelvo a buscar
            nuevo_tam_buscado = nuevo_tam_buscado * 2;
            nodo_a_dividir = buscar_nodo_tam(ARBOL_BUDDY, nuevo_tam_buscado);
            if (nuevo_tam_buscado > config.mem_size){
                // SI LLEGO ACA ES QUE NO HAY SUFICIENTE TAMAÑO PARA ASIGNAR
                // llamar al algoritmo de reemplazo y vuelvo a correr el asignar particiones
                algoritmo_de_reemplazo();
                pthread_mutex_unlock(&M_ARBOL_BUDDY);
                pthread_mutex_unlock(&M_PARTICIONES_QUEUE);
                pthread_mutex_unlock(&M_PARTICIONES);
                pthread_mutex_unlock(&M_MEMORIA_PRINCIPAL);
                return asignar_particion_buddy(raiz, tam);
            }
        }

        // Dividimos la particion y probamos devuelta
        t_nodo* hijo_izq = buddy_dividir_raiz(nodo_a_dividir);
        log_info(logger, "Dividimos el nodo y nos metemos recursivamente en Asignar Buddy");

        pthread_mutex_unlock(&M_ARBOL_BUDDY);
        pthread_mutex_unlock(&M_PARTICIONES_QUEUE);
        pthread_mutex_unlock(&M_PARTICIONES);
        pthread_mutex_unlock(&M_MEMORIA_PRINCIPAL);
        return asignar_particion_buddy(hijo_izq, tam);
    }

}

// Divide el nodo y devuelve el hijo izquierdo
t_nodo* buddy_dividir_raiz(t_nodo* raiz){
    t_nodo* hijo_izq = crear_nodo(null, null, null, null, null, null);
    t_nodo* hijo_der = crear_nodo(null, null, null, null, null, null);

    raiz->izq = hijo_izq;
    raiz->der = hijo_der;
    hijo_izq->buddy = hijo_der;
    hijo_der->buddy = hijo_izq;
    hijo_izq->padre = raiz;
    hijo_der->padre = raiz;
    hijo_izq->es_hoja = true;
    hijo_der->es_hoja = true;
    raiz->es_hoja = false;

    int tam = raiz->particion->tam / 2;
    particion* particion_izq = particion_create(raiz->particion->base, tam, true);
    particion* particion_der = particion_create(raiz->particion->base+tam, tam, true);
    list_add(PARTICIONES, particion_izq);
    list_add(PARTICIONES, particion_der);
    particion_destroy(raiz->particion);
    raiz->particion = null;

    hijo_izq->particion = particion_izq;
    hijo_der->particion = particion_der;

    ordenar_particiones();
    return hijo_izq;
}

// Como como el inverso de la funcion de ariba
// Mergea dos particiones si ambos buddy estan libres
void buddy_mergear(t_nodo* nodo){
    // Si el buddy no esta libre o no es una hoja o es la raiz del arbol no hay que mergear
    if(nodo == ARBOL_BUDDY){
        return;
    }
    if (!nodo->buddy->es_hoja) {
        return;
    }
    if (!nodo->buddy->particion->libre){
        return;
    }


    t_nodo* papuchi = nodo->padre;
    t_nodo* nodo_izq = papuchi->izq;
    t_nodo* nodo_der = papuchi->der;
    papuchi->izq = null;
    papuchi->der = null;
    papuchi->es_hoja = true;
    log_info(tp_logger, "Buddy: Se asociaron los bloques con base %d y %d",
            nodo_izq->particion->base, nodo_der->particion->base);

    particion* particion_padre = particion_create(nodo_izq->particion->base, nodo_izq->particion->tam*2, true);
    list_add(PARTICIONES, particion_padre);
    papuchi->particion = particion_padre;
    particion_destroy(nodo_izq->particion);
    particion_destroy(nodo_der->particion);

    ordenar_particiones();

    free(nodo_izq);
    free(nodo_der);
    buddy_mergear(papuchi);
}


void particion_destroy(particion * unaparticion){
    bool id_search(void* p){
        particion* sub = (particion*) p;
        return sub == unaparticion;
    }
    list_remove_by_condition(PARTICIONES, id_search);
    free(unaparticion);
}

void buddy_liberar_particion(particion* particion_victima){
    // Libero la particion
    particion_victima->libre = true;
    if(strcmp(config.mem_swap_algorithm, "FIFO")==0){quitarVictimaFIFO(particion_victima->base);}

    // Busco el nodo que contenia la particion y mergeo si corresponde
    t_nodo* nodo_que_contenia_la_particion = buscar_nodo_particion(ARBOL_BUDDY, particion_victima);
    buddy_mergear(nodo_que_contenia_la_particion);
}

t_nodo* buscar_nodo_particion(struct t_nodo* nodo, particion* una_particion){
    // Si es null devuelvo null
    if(nodo == NULL){
        return NULL;
    }
    // Si es la particion que buscamos devuelvo ese nodo
    if(nodo->particion == una_particion){
        return nodo;
    }
    // Si no busco a izquierda
    t_nodo* res_izq = buscar_nodo_particion(nodo->izq, una_particion);
    if(res_izq){
        return res_izq;
    }
    // Si no busco a derecha
    t_nodo* res_der = buscar_nodo_particion(nodo->der, una_particion);
    if(res_der){
        return res_der;
    }
}

t_nodo* crear_nodo(particion* particion, struct t_nodo* izq, struct t_nodo* der, struct t_nodo* padre, struct t_nodo* buddy, bool es_hoja){
    t_nodo* nuevo_nodo = malloc(sizeof(t_nodo));
    nuevo_nodo->particion = particion;
    nuevo_nodo->izq = izq;
    nuevo_nodo->der = der;
    nuevo_nodo->padre = padre;
    nuevo_nodo->buddy = buddy;
    nuevo_nodo->es_hoja = es_hoja;
    return nuevo_nodo;
}
