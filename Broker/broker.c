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

    pthread_mutex_lock(&M_MEMORIA_PRINCIPAL);
    pthread_mutex_unlock(&M_MEMORIA_PRINCIPAL);

    signal(SIGUSR1, dump_cache);

    // Logs que piden en el TP
    tp_logger = log_create("broker.log", "BROKER", 1, LOG_LEVEL_TRACE);

    // Logs propios
    logger = log_create(".broker-log-propio.log", "BROKER", 1, LOG_LEVEL_TRACE);
    log_info(logger,"Log started.");

    set_config();
    log_info(logger,"Configuration succesfully setted.");

    pthread_t server_thread;
    pthread_create(&server_thread, NULL, server_function, NULL);
    // Inicializo
    IDENTIFICADOR_MENSAJE = 1;

    MEMORIA_PRINCIPAL = malloc(config.mem_size);
    SUBSCRIPTORES = list_create();
    MENSAJES = list_create();
    MENSAJE_SUBSCRIPTORE = list_create();
    PARTICIONES = list_create();
    log_debug(logger, "Creo la particin inicial del tamanio total de la memoria");
    particion* principal = particion_create(0, config.mem_size, true);
    list_add(PARTICIONES, principal);
    printPartList();



    // Inicializamos las colas
    LIST_NEW_POKEMON = list_create();
    LIST_APPEARED_POKEMON = list_create();
    LIST_GET_POKEMON = list_create();
    LIST_LOCALIZED_POKEMON = list_create();
    LIST_CATCH_POKEMON = list_create();
    LIST_CAUGHT_POKEMON = list_create();
    //tests_broker();
    if(strcmp(config.mem_swap_algorithm, "FIFO")==0){
        PARTICIONES_QUEUE = list_create();
        log_debug(logger, "Se crea la 'cola' para FIFO");
    }

//        particion* nueva_particion1 = particion_create(3, 4, false);
//    particion* nueva_particion2 = particion_create(7, 2, false);
//    particion* nueva_particion4 = particion_create(9, 2, false);
//    particion* nueva_particion3 = particion_create(11, 7, false);
//    particion* particion_inicial = particion_create(18, 2, true);
    log_debug(logger, "NEW_POKEMON");
    t_new_pokemon* new_pika = create_new_pokemon("Pikachu", 3, 4, 2);
    size_t partition_size = sizeof_new_pokemon(new_pika);
    int base = asignar_particion(partition_size);
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
    int base1 = asignar_particion(partition_size1);
    log_debug(logger, "Base: %d", base1);
    printPartList();
    /*
     *
     *
     *
     *
     */
    log_debug(logger, "LOCALIZED_POKEMON");
    t_localized_pokemon* loc_pika = create_localized_pokemon("Pikachu", 2, 3, 4, 5, 6);
    size_t partition_size2 = sizeof_localized_pokemon(loc_pika);
    int base2 = asignar_particion(partition_size2);
    log_debug(logger, "Base: %d", base2);
    printPartList();

    /*
  *
  *
  *
  *
  */
    log_debug(logger, "LOCALIZED_POKEMON");
    t_localized_pokemon* loc_pika1 = create_localized_pokemon("Pikachu", 2, 3, 4, 5, 6);
    size_t partition_size3 = sizeof_localized_pokemon(loc_pika1);
    int base3 = asignar_particion(partition_size3);
    log_debug(logger, "Base: %d", base3);
    printPartList();
    /*
     *
     *
     *
     *
     */
    log_debug(logger, "LOCALIZED_POKEMON");
    t_localized_pokemon* loc_pika2 = create_localized_pokemon("Pikachu", 2, 3, 4, 5, 6);
    size_t partition_size4 = sizeof_localized_pokemon(loc_pika2);
    int base4 = asignar_particion(partition_size4);
    log_debug(logger, "Base: %d", base4);
    printPartList();
    /*
     *
     *
     *
     *
     */
    log_debug(logger, "LOCALIZED_POKEMON");
    t_localized_pokemon* loc_pika3 = create_localized_pokemon("Pikachu", 2, 3, 4, 5, 6);
    size_t partition_size5 = sizeof_localized_pokemon(loc_pika3);
    int base5= asignar_particion(partition_size5);
    log_debug(logger, "Base: %d", base5);
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

//    log_debug(logger, "Eliminamos LOCALIZED_POKEMON(base:%d)", 36);
//    particion_delete(36);
//    printPartList();
//    pthread_join(server_thread, NULL);
    compactar_particiones();
    printPartList();

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
        close(fd);
    }

    //--funcion que se ejecuta cuando se recibe un nuevo mensaje de un cliente ya conectado
    void incoming(int fd, char *ip, int port, MessageHeader *headerStruct) {

        t_list *cosas = receive_package(fd, headerStruct);

        switch (headerStruct->type) {

            case SUB_NEW:;
                {
                    subscribir_a_cola(cosas, ip, port, fd, LIST_NEW_POKEMON, SUB_NEW);
                    log_info(tp_logger, "Nuevo subscriptor de NEW");
                    break;
                }

            case SUB_APPEARED:;
                {
                    subscribir_a_cola(cosas, ip, port, fd, LIST_APPEARED_POKEMON, SUB_APPEARED);
                    log_info(tp_logger, "Nuevo subscriptor de APPEARED");
                    break;
                }

            case SUB_LOCALIZED:;
                {
                    subscribir_a_cola(cosas, ip, port, fd, LIST_LOCALIZED_POKEMON, SUB_LOCALIZED);
                    log_info(tp_logger, "Nuevo subscriptor de LOCALIZED");
                    break;
                }

            case SUB_CAUGHT:;
                {
                    subscribir_a_cola(cosas, ip, port, fd, LIST_CAUGHT_POKEMON, SUB_CAUGHT);
                    log_info(tp_logger, "Nuevo subscriptor de CAUGHT");
                    break;
                }

            case SUB_GET:;
                {
                    subscribir_a_cola(cosas, ip, port, fd, LIST_GET_POKEMON, SUB_GET);
                    log_info(tp_logger, "Nuevo subscriptor de GET");
                    break;
                }

            case SUB_CATCH:;
                {
                    subscribir_a_cola(cosas, ip, port, fd, LIST_CATCH_POKEMON, SUB_CATCH);
                    log_info(tp_logger, "Nuevo subscriptor de CATCH");
                    break;
                }

            case NEW_POK:;
                {
                    log_info(tp_logger, "Llega un mensaje a la cola NEW_POK");
                    // Le llega un un_mensaje
                    t_new_pokemon* new_pokemon = void_a_new_pokemon(list_get(cosas,0));

                    // Cargamos el un_mensaje en nuestro sistema
                    mensaje* un_mensaje = mensaje_create(0, 0, NEW_POK, sizeof_new_pokemon(new_pokemon));
                    un_mensaje->puntero_a_memoria = new_pokemon_a_void(new_pokemon);

                    // Cargamos el un_mensaje a la lista de New_pokemon
                    cargar_mensaje(LIST_NEW_POKEMON, un_mensaje);

                    //Envio el ID de respuesta
                    int respuesta = un_mensaje->id;
                    t_paquete* paquete = create_package(NEW_POK);
                    add_to_package(paquete, (void*) &respuesta, sizeof(int));
                    send_package(paquete, fd);

                    // Enviamos los mensajes pendientes
                    recursar_operativos();
                    break;
                }

            case APPEARED_POK:;
                {
                    log_info(tp_logger, "Llega un mensaje a la cola APPEARED_POK");
                    uint32_t mensaje_co_id = *((uint32_t *) list_get(cosas, 0));
                    t_appeared_pokemon* appeared_pokemon = void_a_appeared_pokemon(list_get(cosas,1));

                    // Cargamos el un_mensaje en nuestro sistema
                    mensaje* un_mensaje = mensaje_create(0, mensaje_co_id, APPEARED_POK, sizeof_appeared_pokemon(appeared_pokemon));
                    un_mensaje->puntero_a_memoria = appeared_pokemon_a_void(appeared_pokemon);

                    // Cargamos el un_mensaje a la lista de Appeared_pokemon
                    cargar_mensaje(LIST_APPEARED_POKEMON, un_mensaje);

                    // Enviamos los mensajes pendientes
                    recursar_operativos();

                    //Envio el ID de respuesta
                    int respuesta = un_mensaje->id;
                    t_paquete* paquete = create_package(APPEARED_POK);
                    add_to_package(paquete, (void*) &respuesta, sizeof(int));
                    send_package(paquete, fd);
                    break;
                }

            case LOCALIZED_POK:;
                {
                    log_info(tp_logger, "Llega un mensaje a la cola LOCALIZED_POK");
                    uint32_t mensaje_co_id = *((uint32_t *) list_get(cosas, 0));
                    t_localized_pokemon* localized_pokemon = void_a_localized_pokemon(list_get(cosas,1));

                    // Cargamos el un_mensaje en nuestro sistema
                    mensaje* un_mensaje = mensaje_create(0, mensaje_co_id, LOCALIZED_POK, sizeof_localized_pokemon(localized_pokemon));
                    un_mensaje->puntero_a_memoria = localized_pokemon_a_void(localized_pokemon);

                    // Cargamos el un_mensaje a la lista de Localized_pokemon
                    cargar_mensaje(LIST_LOCALIZED_POKEMON, un_mensaje);

                    // Enviamos los mensajes pendientes
                    recursar_operativos();

                    //Envio el ID de respuesta
                    int respuesta = un_mensaje->id;
                    t_paquete* paquete = create_package(LOCALIZED_POK);
                    add_to_package(paquete, (void*) &respuesta, sizeof(int));
                    send_package(paquete, fd);
                    break;
                }

            case CAUGHT_POK:;
                {
                    log_info(tp_logger, "Llega un mensaje a la cola CAUGHT_POK");
                    uint32_t mensaje_co_id = *((uint32_t *) list_get(cosas, 0));
                    t_caught_pokemon* caught_pokemon = void_a_caught_pokemon(list_get(cosas,1));

                    // Cargamos el un_mensaje en nuestro sistema
                    mensaje* un_mensaje = mensaje_create(0, mensaje_co_id, CAUGHT_POK, sizeof_caught_pokemon(caught_pokemon));
                    un_mensaje->puntero_a_memoria = caught_pokemon_a_void(caught_pokemon);

                    // Cargamos el un_mensaje a la lista de Caught_pokemon
                    cargar_mensaje(LIST_CAUGHT_POKEMON, un_mensaje);

                    // Enviamos los mensajes pendientes
                    recursar_operativos();

                    //Envio el ID de respuesta
                    int respuesta = un_mensaje->id;
                    t_paquete* paquete = create_package(CAUGHT_POK);
                    add_to_package(paquete, (void*) &respuesta, sizeof(int));
                    send_package(paquete, fd);
                    break;
                }

            case GET_POK:;
                {
                    log_info(tp_logger, "Llega un mensaje a la cola GET_POK");
                    t_get_pokemon* get_pokemon = void_a_get_pokemon(list_get(cosas,0));

                    // Cargamos el un_mensaje en nuestro sistema
                    mensaje* un_mensaje = mensaje_create(0, 0, GET_POK, sizeof_get_pokemon(get_pokemon));
                    un_mensaje->puntero_a_memoria = get_pokemon_a_void(get_pokemon);

                    // Cargamos el un_mensaje a la lista de Get_pokemon
                    cargar_mensaje(LIST_GET_POKEMON, un_mensaje);

                    // Enviamos los mensajes pendientes
                    recursar_operativos();

                    //Envio el ID de respuesta
                    int respuesta = un_mensaje->id;
                    t_paquete* paquete = create_package(GET_POK);
                    add_to_package(paquete, (void*) &respuesta, sizeof(int));
                    send_package(paquete, fd);
                    break;
                }

            case CATCH_POK:;
                {
                    log_info(tp_logger, "Llega un mensaje a la cola CATCH_POK");
                    t_catch_pokemon* catch_pokemon = void_a_catch_pokemon(list_get(cosas,0));

                    // Cargamos el un_mensaje en nuestro sistema
                    mensaje* un_mensaje = mensaje_create(0, 0, CATCH_POK, sizeof_catch_pokemon(catch_pokemon));
                    un_mensaje->puntero_a_memoria = catch_pokemon_a_void(catch_pokemon);

                    // Cargamos el un_mensaje a la lista de Catch_pokemon
                    cargar_mensaje(LIST_CATCH_POKEMON, un_mensaje);

                    // Enviamos los mensajes pendientes
                    recursar_operativos();

                    //Envio el ID de respuesta
                    int respuesta = un_mensaje->id;
                    t_paquete* paquete = create_package(CATCH_POK);
                    add_to_package(paquete, (void*) &respuesta, sizeof(int));
                    send_package(paquete, fd);
                    break;
                }

            case ACK:;
                {
                    int id_subscriptor = *(int*) list_get(cosas, 0);
                    int id_mensaje = *(int*) list_get(cosas, 1);
                    flag_ack(id_subscriptor, id_mensaje);
                    log_info(tp_logger, "Recibimos el ACK del mensaje %d del suscriptor %d",
                             id_mensaje, id_subscriptor);
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
    nuevo_mensaje->tam = tam;
    nuevo_mensaje->puntero_a_memoria = asignar_puntero_a_memoria();
    nuevo_mensaje->lru = unix_epoch();

    list_add(MENSAJES, nuevo_mensaje);

    return nuevo_mensaje;
}


void* asignar_puntero_a_memoria(){
    // proximamente
    return NULL;
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


bool existe_sub(int id, t_list* cola){
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
    list_remove_by_condition(cola, id_search);
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
    mensaje_subscriptor* nuevo_mensaje_subscriptor = malloc(sizeof(mensaje_subscriptor));

    nuevo_mensaje_subscriptor->id_mensaje = id_mensaje;
    nuevo_mensaje_subscriptor->id_subscriptor = id_sub;
    nuevo_mensaje_subscriptor->enviado = false;
    nuevo_mensaje_subscriptor->ack = false;

    list_add(MENSAJE_SUBSCRIPTORE, nuevo_mensaje_subscriptor);

    return nuevo_mensaje_subscriptor;

}

void mensaje_subscriptor_delete(int id_mensaje, int id_sub){
    bool multiple_id_search(void* un_men_sub){
        mensaje_subscriptor* men_sub = (subscriptor*) un_men_sub;
        return men_sub->id_mensaje == id_mensaje && men_sub->id_subscriptor == id_sub;
    }
    list_remove_by_condition(MENSAJE_SUBSCRIPTORE, multiple_id_search);

}
void subscribir_a_cola(t_list* cosas, char* ip, int puerto, int fd, t_list* una_cola, MessageType tipo){
    int id = *((int*) list_get(cosas, 0));

    // Si cambia el puerto o la ip lo borro y vuelvo a crearlo
    if(existe_sub(id, una_cola)){
        subscriptor_delete(id, una_cola);
        log_info(logger, "Ya existia el subscriptor en el sistema");
    }

    subscriptor* nuevo_subscriptor = subscriptor_create(id, ip, puerto, fd);

    list_add(una_cola, nuevo_subscriptor);

    int respuesta = 1;
    t_paquete* paquete = create_package(tipo);
    add_to_package(paquete, (void*) &respuesta, sizeof(int));
    send_package(paquete, fd);
}

void cargar_mensaje(t_list* una_cola, mensaje* un_mensaje){
    int cantidad_subs = list_size(una_cola);
    for (int i = 0; i < cantidad_subs; ++i) {
        subscriptor* un_subscriptor = list_get(una_cola, i);
        mensaje_subscriptor_create(un_mensaje->id, un_subscriptor->id_subs);
        cantidad_subs = list_size(una_cola);
    }
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


// Esta funcion recorre la lista MENSAJE_SUBSCRIPTORE mandando los mensajes pendientes
void recursar_operativos(){
    int cantidad_mensajes = list_size(MENSAJE_SUBSCRIPTORE);
    for (int i = 0; i < cantidad_mensajes; ++i) {
        mensaje_subscriptor* coso = list_get(MENSAJE_SUBSCRIPTORE, i);
        void* cosito = mensaje_subscriptor_a_void(coso);


        if(!coso->enviado){

            pthread_t mensaje_thread;
            pthread_create(&mensaje_thread, NULL, mandar_mensaje, (void*)cosito);
            pthread_join(mensaje_thread, NULL);

        }
        // Vuelvo a actualizar el tamaño por si entró alguien en el medio
        cantidad_mensajes = list_size(MENSAJE_SUBSCRIPTORE);
    }
}
int send_message_test(t_paquete* paquete, int socket){
    return 1;
}
void mandar_mensaje(void* cosito){
    mensaje_subscriptor* coso = void_a_mensaje_subscriptor(cosito);
    subscriptor* un_subscriptor = find_subscriptor(coso->id_subscriptor);
    mensaje* un_mensaje = find_mensaje(coso->id_mensaje);

    t_paquete* paquete = create_package(un_mensaje->tipo);
    add_to_package(paquete, (void*) &un_mensaje->id, sizeof(int));
    add_to_package(paquete, (void*) &un_mensaje->id_correlacional, sizeof(int));
    add_to_package(paquete, un_mensaje->puntero_a_memoria, un_mensaje->tam);

    if (send_package(paquete, un_subscriptor->socket) > 0){
        log_info(tp_logger, "Se envia el mensaje %d al suscriptor %d", un_mensaje->id, un_subscriptor->id_subs);
        flag_enviado(coso->id_subscriptor, coso->id_mensaje);
    }
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
void* flag_enviado(uint32_t id_sub, uint32_t id_men){
    for(int i = 0; i<list_size(MENSAJE_SUBSCRIPTORE);i++){
        mensaje_subscriptor* x = list_get(MENSAJE_SUBSCRIPTORE, i);
        if(x->id_mensaje == id_men && x->id_subscriptor == id_sub){x->enviado = true;}
    }
}

void* flag_ack(uint32_t id_sub, uint32_t id_men){
    for(int i = 0; i<list_size(MENSAJE_SUBSCRIPTORE);i++){
        mensaje_subscriptor* x = list_get(MENSAJE_SUBSCRIPTORE, i);
        if(x->id_mensaje == id_men && x->id_subscriptor == id_sub){x->ack = true;}
    }
}

/* TODO
 * Cuando los subscriptores reciban todos los mensajes borrar el mensaje de la memoria (hay que borrarlo tambien de la estrutura ?)
 
 * Ver lo de kill -SIGUSR1 [proceso]

    El kill se envía desde consola. Ustedes deben implementar la función que maneje esa señal en el código.
    Estamos en tratativas de armar un vídeo explicando señales. 
    De base pueden usar esta presentación vieja que se entiende bastante el funcionamiento de la función.
    http://faq.utnso.com.ar/signals

 * Copiar y pegar funciones de la memoria principal de tps pasados
 */

/*
███╗   ███╗███████╗███╗   ███╗ ██████╗ ██████╗ ██╗ █████╗
████╗ ████║██╔════╝████╗ ████║██╔═══██╗██╔══██╗██║██╔══██╗
██╔████╔██║█████╗  ██╔████╔██║██║   ██║██████╔╝██║███████║
██║╚██╔╝██║██╔══╝  ██║╚██╔╝██║██║   ██║██╔══██╗██║██╔══██║
██║ ╚═╝ ██║███████╗██║ ╚═╝ ██║╚██████╔╝██║  ██║██║██║  ██║
╚═╝     ╚═╝╚══════╝╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═╝╚═╝╚═╝  ╚═╝
*/

// TODO: luego buscar como se usa el list_sort
// https://github.com/sisoputnfrba/so-commons-library/blob/master/tests/unit-tests/test_list.c

particion* particion_create(int base, int tam, bool is_free){
    particion* nueva_particion = malloc(sizeof(particion));

    nueva_particion->base = base;
    nueva_particion->tam = tam;
    nueva_particion->libre = is_free;
    nueva_particion->ultimo_uso = unix_epoch();

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
    if(strcmp(config.free_partition_algorithm, "FF") == 0){
        log_debug(logger, "First fit search starts...");
        return first_fit_search(tam);
    }else if (strcmp(config.free_partition_algorithm, "BF") == 0){
        log_debug(logger, "Best fit search starts...");
        return best_fit_search(tam);
    }else{
        log_error(logger, "Unexpected algorithm");
    }
}

particion* first_fit_search(tam){
    int size = list_size(PARTICIONES);
    for(int i=0; i<size; i++){
        particion* x = list_get(PARTICIONES, i);
        if(x->libre == true && tam <= x->tam ){
            log_info(logger, "Free partition with enough size found!(base: %d)", x->base);
            return x;
        }
    }
    log_warning(logger, "There are not availables partitions  :|");
    return NULL;
}

particion* best_fit_search(tam){
    int size = list_size(PARTICIONES);
    t_list* candidatos = list_create();
    for(int i=0; i<size; i++){
        particion* x = list_get(PARTICIONES, i);
        if(x->libre == true && tam <= x->tam){
            log_info(logger, "Free partition with enough size found!(base: %d)", x->base);
            if(tam == x->tam){log_info(logger, "Best fit partition found!(base:%d)", x->base);return x;}
            list_add(candidatos, x);
        }
    }
    log_debug(logger, "Looking for the best fit one..");
    int candidatos_size = list_size(candidatos);
    if(candidatos_size != 0){
        particion* best_fit;
        int best_fit_diff = 9999;
        for(int i=0; i<candidatos_size; i++){
            particion* y = list_get(candidatos, i);
            int diff = y->tam - tam;
            if(diff == 0){log_info(logger, "Best fit partition found!(base:%d)", y->base);return y;}
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

int asignar_particion(size_t tam) {
    particion *particion_libre = buscar_particion_libre(tam);
    if (particion_libre != NULL) {
        log_info(logger, "Doing the job..");
        //Si la particion libre encontrada es de igual tamanio a la particion a alojar no es necesario ordenar
        if (particion_libre->tam == tam) {

            //Cambio el estado de la libre a falso y actualiza su ultimo uso.
            particion_libre->libre = false;
            particion_libre->ultimo_uso = unix_epoch();


            log_info(logger, "Partition assigned(base: %d)", particion_libre->base);
            return particion_libre->base;
        }
            //Si no es de igual tamano, debo crear una nueva particion con base en la libre y reacomodar la base y tamanio de la libre.
        else {
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
            log_info(logger, "Ready");
            return nueva_particion->base;
        }
    } else {
        log_warning(logger, "It was not possible to assign partition!");
        return -1;
//        INTENTOS++;
    }

}
// Ordena las particiones y mergea las particiones libres
void ordenar_particiones(){
    bool particion_anterior(particion* particion_antes, particion* particion_despues) {
        return particion_antes->base < particion_despues->base;
    }

    list_sort(PARTICIONES, (void*) particion_anterior);

    mergear_particiones_libres();
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
    FILE* archivo_dump = fopen("dump.txt", "w");

    // Printeo la fecha y hora
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(archivo_dump, "Dump: %02d/%02d/%d %02d:%02d:%02d\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
    fflush(archivo_dump);

    // Printeo el contenido de la cache
    int size = list_size(PARTICIONES);
    for(int i=0; i<size; i++) {
        particion *s = list_get(PARTICIONES, i);
        fprintf(archivo_dump, "Particion %d: %06p-%06p\t"
                              "[%s]\t"
                              "Size: %db\t"
                              "LRU: %" PRIu64 "\t"
                              "Cola: %s\t"
                              "ID: %d\n",
                              i+1, s->base, s->base+s->tam,
                              s->libre ? "L" : "X",
                              s->tam,
                              s->ultimo_uso,
                              cola_to_string(NEW_POK),
                              1);
    }

    // Cierro el archivo y libero la memoria
    fclose(archivo_dump);
    return;
}

void compactar_particiones(){
    int size = list_size(PARTICIONES);
    for(int i=0; i<size;i++){
        particion* una_particion = list_get(PARTICIONES, i);
        if(una_particion->libre){
            for(int z=i;z<size;z++){
                particion* otra_particion = list_get(PARTICIONES, z);
                if(!otra_particion->libre){
                    uint64_t ult_uso_libre = una_particion->ultimo_uso;
                    una_particion->ultimo_uso = otra_particion->ultimo_uso;
                    otra_particion->ultimo_uso = ult_uso_libre;
                    otra_particion->base = una_particion->base;
                    una_particion->base += otra_particion->tam;
                    mergear_particiones_libres();
                    ordenar_particiones();
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