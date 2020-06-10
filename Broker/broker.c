//
// Created by utnso on 07/04/20.
//

#include "broker.h"


int main(int argc, char **argv) {
    if (argc != 2) {
        cfg_path = strdup("../broker.cfg");
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

    SUBSCRIPTORES = list_create();
    MENSAJES = list_create();
    MENSAJE_SUBSCRIPTORE = list_create();

    // Inicializamos las colas
    LIST_NEW_POKEMON = list_create();
    LIST_APPEARED_POKEMON = list_create();
    LIST_GET_POKEMON = list_create();
    LIST_LOCALIZED_POKEMON = list_create();
    LIST_CATCH_POKEMON = list_create();
    LIST_CAUGHT_POKEMON = list_create();
    //tests_broker();

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
                    subscribir_a_cola(cosas, ip, port, fd, LIST_NEW_POKEMON, SUB_NEW);
                    log_info(logger, "Nuevo subscriptor de NEW");
                    break;
                }

            case SUB_APPEARED:;
                {
                    subscribir_a_cola(cosas, ip, port, fd, LIST_APPEARED_POKEMON, SUB_APPEARED);
                    log_info(logger, "Nuevo subscriptor de APPEARED");
                    break;
                }

            case SUB_LOCALIZED:;
                {
                    subscribir_a_cola(cosas, ip, port, fd, LIST_LOCALIZED_POKEMON, SUB_LOCALIZED);
                    log_info(logger, "Nuevo subscriptor de LOCALIZED");
                    break;
                }

            case SUB_CAUGHT:;
                {
                    subscribir_a_cola(cosas, ip, port, fd, LIST_CAUGHT_POKEMON, SUB_CAUGHT);
                    log_info(logger, "Nuevo subscriptor de CAUGHT");
                    break;
                }

            case SUB_GET:;
                {
                    subscribir_a_cola(cosas, ip, port, fd, LIST_GET_POKEMON, SUB_GET);
                    log_info(logger, "Nuevo subscriptor de GET");
                    break;
                }

            case SUB_CATCH:;
                {
                    subscribir_a_cola(cosas, ip, port, fd, LIST_CATCH_POKEMON, SUB_CATCH);
                    log_info(logger, "Nuevo subscriptor de CATCH");
                    break;
                }

            case NEW_POK:;
                {
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
    add_to_package(paquete, un_mensaje->puntero_a_memoria, un_mensaje->tam);

    if (send_package(paquete, un_subscriptor->socket) > 0){
        flag_enviado(coso->id_subscriptor, coso->id_mensaje);
    }

    // Trato de recibir el ACK
    MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
    if(receive_header(un_subscriptor->socket, buffer_header) <= 0) {
        log_error(logger, "No recibimos el ACK");
        return;
    }

    if (buffer_header->type == ACK){
        flag_ack(coso->id_subscriptor, coso->id_mensaje);
        log_info(logger, "Llego el ACK");
    } else{
        log_info(logger, "NO AMIGO LPM TE LLEGO ALGO QUE NO ERA UN ACK LISTO CERRAMO ACA");
        exit(-1);
    }

    printMenSubList();

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

//void recibir_ack(){
//TODO
//}

//        mandar_mensaje_thread(un_subscriptor, un_mensaje);

/* TODO
 * Cuando los subscriptores reciban todos los mensajes borrar el mensaje de la memoria (hay que borrarlo tambien de la estrutura ?)
 
 * Ver lo de kill -SIGUSR1 [proceso]

    El kill se envía desde consola. Ustedes deben implementar la función que maneje esa señal en el código.
    Estamos en tratativas de armar un vídeo explicando señales. 
    De base pueden usar esta presentación vieja que se entiende bastante el funcionamiento de la función.
    http://faq.utnso.com.ar/signals

 * Copiar y pegar funciones de la memoria principal de tps pasados
 */

