#include "team.h"

TEAMConfig config;
t_log* logger;
t_log* logger_server;
bool subscribed_to_appeared_pokemon = false;
bool subscribed_to_localized_pokemon = false;
bool subscribed_to_caught_pokemon = false;
t_config *config_file;

int main() {
    MessageType test = ABC;
    pthread_t queues_subscription_thread;
    pthread_t server_thread;

    // Leo archivo de configuracion
    read_config_options();

    // Inicializo el log
    start_log();

    //Creo el servidor para que el GameBoy y el Broker me manden mensajes
    pthread_create(&server_thread, NULL, server_function, NULL);
    
    //Intento conectarme una vez al broker y suscribirme a las listas
    attempt_subscription();

    // Si no me logre suscribir a todas las colas, levanto un hilo que lo reintente cada n segundos
    if(!subscribed_to_all_global_queues()) {

        pthread_create(&queues_subscription_thread, NULL, queues_subscription_function, NULL);
        pthread_detach(queues_subscription_thread);

    }

    initialize_structures();

    //Esta linea esta solo de prueba
    send_to_server(test);

    //Joineo el hilo main con el del servidor para el GameBoy, en realidad ninguno de los 2 tendria que terminar nunca
    pthread_join(server_thread, NULL);

    config_destroy(config_file);
    log_destroy(logger);
}

void read_config_options() {

    //TODO: verificar que el archivo exista y salir del proceso si no
    config_file = config_create("../team.config");
    config.posiciones_entrenadores = config_get_array_value(config_file, "POSICIONES_ENTRENADORES");
    config.pokemon_entrenadores = config_get_array_value(config_file, "POKEMON_ENTRENADORES");
    config.objetivos_entrenadores = config_get_array_value(config_file, "OBJETIVOS_ENTRENADORES");
    config.tiempo_reconexion = config_get_int_value(config_file, "TIEMPO_RECONEXION");
    config.retardo_ciclo_cpu = config_get_int_value(config_file, "RETARDO_CICLO_CPU");
    config.algoritmo_planificacion = config_get_string_value(config_file, "ALGORITMO_PLANIFICACION");
    config.quantum = config_get_int_value(config_file, "QUANTUM");
    config.estimacion_inicial = config_get_int_value(config_file, "ESTIMACION_INICIAL");
    config.ip_broker = config_get_string_value(config_file, "IP_BROKER");
    config.puerto_broker = config_get_int_value(config_file, "PUERTO_BROKER");
    config.ip_team = config_get_string_value(config_file, "IP_TEAM");
    config.puerto_team = config_get_int_value(config_file, "PUERTO_TEAM");
    config.log_file = config_get_string_value(config_file, "LOG_FILE");
}

//TODO: cambiar el 1 por un 0 para la entrega
void start_log() {

    logger = log_create(config.log_file, "team", 1, LOG_LEVEL_TRACE);
}

void attempt_subscription() {

    // Me intento conectar al Broker y suscribirme a la cola de appeared_pokemon
    int broker = connect_to_broker();
    if (broker != -1 && !subscribed_to_appeared_pokemon) {
        subscribed_to_appeared_pokemon = subscribe_to_queue(broker, SUB_APPEARED);
        disconnect_from_broker(broker);
    }

    // Me intento conectar al Broker y suscribirme a la cola de localized_pokemon
    broker = connect_to_broker();
    if (broker != -1 && !subscribed_to_localized_pokemon) {
        subscribed_to_localized_pokemon = subscribe_to_queue(broker, SUB_LOCALIZED);
        disconnect_from_broker(broker);
    }

    // Me intento conectar al Broker y suscribirme a la cola de caught_pokemon
    broker = connect_to_broker();
    if (broker != -1 && !subscribed_to_caught_pokemon) {
        subscribed_to_caught_pokemon = subscribe_to_queue(broker, SUB_CAUGHT);
        disconnect_from_broker(broker);
    }
}

int connect_to_broker(){

    int client_socket;
    if((client_socket = create_socket()) == -1) {
        log_error(logger, "Error al crear el socket de cliente");
        return -1;
    }
    if(connect_socket(client_socket, config.ip_broker, config.puerto_broker) == -1){
        log_error(logger, "Error al conectarse al Broker");
        return -1;
    }
    return client_socket;
}

void disconnect_from_broker(int broker_socket) {
    close_socket(broker_socket);
}

bool subscribe_to_queue(int broker, MessageType cola) {

    // Creo un paquete para la suscripcion a una cola, adjunto la ip y el puerto de mi server
    t_paquete* paquete = create_package(cola);
    add_to_package(paquete, (void*) config.ip_team, strlen(config.ip_team) + 1);
    add_to_package(paquete, (void*) &config.puerto_team, sizeof(int));

    // Envio el paquete, si no se puede enviar retorno false
    if(send_package(paquete, broker)  == -1){
        return false;
    }

    // Limpieza
    free_package(paquete);

    // Trato de recibir el encabezado de la respuesta
    MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
    if(receive_header(broker, buffer_header) <= 0) {
        return false;
    }

    // Recibo la confirmacion
    t_list* rta_list = receive_package(broker, buffer_header);
    int rta = *(int*) list_get(rta_list, 0);

    // Limpieza
    free(buffer_header);
    //TODO: verificar si hay que destruir la lista

    return rta == 1;
}

//TODO: agregar servidor de Rodri
void* server_function(void* arg) {


    start_log_server();

    int server_socket;

    // La creacion de nuestro socket servidor puede fallar, si falla duermo y vuelvo a intentar en n segundos
    while ((server_socket = initialize_server()) == -1) {

        sleep(config.tiempo_reconexion);
    }

    start_server(server_socket, &new, &lost, &incoming);

    return null;
}

void* queues_subscription_function(void* arg) {

    //Mientras que no este suscripto a todas las colas globales, sigo reintentando
    // indefinidamente cada n segundos
    while(!subscribed_to_all_global_queues()) {

        attempt_subscription();

        sleep(config.tiempo_reconexion);
    }

    return null;
}

//TODO: terminar de implementar
void initialize_structures(){

    //Itero la lista de entrenadores, y creo un hilo por cada uno
    char** ptr = config.posiciones_entrenadores;
    int pos = 0;
    //Itero el array de posiciones de entrenadores
    for (char* coordenada = *ptr; coordenada; coordenada=*++ptr) {
        // TODO: crear un array de pthreads para los entrenadores
        // Definir si el array de entrenadores tendria que ser global o si no importa
        log_info(logger, "entrenador: %d", pos);
        pos++;
    }
}

void start_log_server() {

    //Cambiar 1 por 0?
    logger_server=log_create("../servidor.log", "servidor", 1, LOG_LEVEL_TRACE);
}

int initialize_server(){

    int server_socket;
    int port = config.puerto_team;

    if((server_socket = create_socket()) == -1) {
        log_error(logger_server, "Error creating socket");
        return -1;
    }
    if((bind_socket(server_socket, port)) == -1) {
        log_error(logger_server, "Error binding socket");
        return -1;
    }

    return server_socket;
}

void new(int server_socket, char * ip, int port){
    log_info(logger_server,"Nueva conexion: Socket %d, Puerto: %d", server_socket, port);
}

void lost(int server_socket, char * ip, int port){
    log_error(logger_server, "Conexion perdida");
}

void incoming(int server_socket, char* ip, int port, MessageHeader * headerStruct){

    t_list* paquete_recibido = receive_package(server_socket, headerStruct);

    void* mensaje = list_get(paquete_recibido,0);

    switch(headerStruct -> type){

        case SUB_APPEARED:
            printf("APPEARD_POKEMON");
            break;
        case SUB_LOCALIZED:
            printf("LOCALIZED_POKEMON");
            break;
        case SUB_CAUGHT:
            printf("SUB_CAUGHT");
            break;
        case GET_POK:
            printf("GET_POKEMON");
            break;
        case CATCH_POK:
            printf("CATCH_POKEMON");
            break;
        case APPEARED_POK:
            printf("APPEARED_POKEMON");
            break;
        case LOCALIZED_POK:
            printf("LOCALIZED_POKEMON");
            break;
        case CAUGHT_POK:
            printf("CAUGHT_POKEMON");
            break;
        default:
            printf("la estas cagando compa");
            break;
    }

}

//----------------------------------------HELPERS----------------------------------------

bool subscribed_to_all_global_queues() {
    return subscribed_to_appeared_pokemon && subscribed_to_localized_pokemon && subscribed_to_caught_pokemon;
}

//Funcion de prueba
void send_to_server(MessageType mensaje){
    int broker = connect_to_broker();
    t_paquete* paquete = create_package(mensaje);


    char* enviar = malloc(50);
    strcpy(enviar, "test");

    add_to_package(paquete,(void*) enviar, strlen("test")+1);


    if(send_package(paquete, broker)  == -1){
        printf("No se pudo mandar");
    }
    disconnect_from_broker(broker);
}
