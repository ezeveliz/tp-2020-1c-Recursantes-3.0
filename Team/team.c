#include "team.h"

TEAMConfig config;
t_log* logger;
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

    start_log();

    //Intento conectarme una vez al broker y suscribirme a las listas
    attempt_subscription();

    //Creo el servidor para que el GameBoy y el Broker me manden mensajes
    pthread_create(&server_thread, NULL, server_function, NULL);

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
        subscribed_to_appeared_pokemon = subscribe_to_queue(broker);//appeared_pokemon
        disconnect_from_broker(broker);
    }

    // Me intento conectar al Broker y suscribirme a la cola de localized_pokemon
    broker = connect_to_broker();
    if (broker != -1 && !subscribed_to_localized_pokemon) {
        subscribed_to_localized_pokemon = subscribe_to_queue(broker);//localized_pokemon
        disconnect_from_broker(broker);
    }

    // Me intento conectar al Broker y suscribirme a la cola de caught_pokemon
    broker = connect_to_broker();
    if (broker != -1 && !subscribed_to_caught_pokemon) {
        subscribed_to_caught_pokemon = subscribe_to_queue(broker);//caught_pokemon
        disconnect_from_broker(broker);
    }
}

int connect_to_broker(){

    int server_socket;
    if((server_socket = create_socket()) == -1) {
        log_error(logger, "Error al crear el socket de cliente");
        return -1;
    }
    if(connect_socket(server_socket, config.ip_broker, config.puerto_broker) == -1){
        log_error(logger, "Error al conectarse al Broker");
        return -1;
    }
    return server_socket;
}

void disconnect_from_broker(int broker_socket) {
    close_socket(broker_socket);
}

//FIXME implementar una vez que este definido el metodo de suscripcion
// - a las colas del broker, falta el paramentro del tipo de cola
bool subscribe_to_queue(int broker) {
    //Aca iria un send y un receive, en caso de que salga tod.o bien, devolveria true
    return true;
}

//TODO: agregar servidor de Rodri
void* server_function(void* arg) {

}

void* queues_subscription_function(void* arg) {

    //Mientras que no este suscripto a todas las colas globales, sigo reintentando
    // indefinidamente cada n segundos
    while(!subscribed_to_all_global_queues()) {

        attempt_subscription();

        sleep(config.tiempo_reconexion);
    }
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

//----------------------------------------HELPERS----------------------------------------

bool subscribed_to_all_global_queues() {
    return subscribed_to_appeared_pokemon && subscribed_to_localized_pokemon && subscribed_to_caught_pokemon;
}

//Funcion de prueba
void send_to_server(MessageType mensaje){
    int broker = connect_to_broker();
    t_paquete* paquete = create_package(mensaje);

    add_to_package(paquete,(void*) &mensaje, sizeof(int));

    if(send_package(paquete, broker)  == -1){
        printf("No se pudo mandar");
    }
    disconnect_from_broker(broker);
}