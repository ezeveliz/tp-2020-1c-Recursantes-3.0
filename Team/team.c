#include "team.h"

TEAMConfig config;
t_log* logger;
int server_socket;
bool server_socket_initialized = false;
t_config *config_file;

int main() {
    MessageType test = ABC;
    pthread_t connect_thread;
    pthread_t server_thread;

    read_config_options();
    initialize_structures();
    start_log();

    //Intento conectarme una vez al broker y suscribirme a las listas
    attempt_connection();
    //Creo el servidor para que el GameBoy me mande mensajes
    pthread_create(&server_thread, NULL, server_function, NULL);

    //Si no me pude conectar al Broker, levanto un hilo que siga intentando conectarse cada n segundos
    if(!server_socket_initialized) {

        pthread_create(&connect_thread, NULL, attempt_connection_thread, NULL);
        pthread_detach(connect_thread);

    }

    //Itero la lista de entrenadores, y creo un hilo por cada uno
    char** ptr = config.posiciones_entrenadores;
    int pos = 0;
    //Itero el array de posiciones de entrenadores
    for (char* coordenada = *ptr; coordenada; coordenada=*++ptr) {
        log_info(logger, "entrenador: %d", pos);
        pos++;
    }
     config_destroy(config_file);

     //Esta linea esta solo de prueba
     send_to_server(test);

     //Joineo el hilo main con el del servidor para el GameBoy, en realidad ninguno de los 2 tendria que terminar nunca
     pthread_join(server_thread, NULL);
}

void read_config_options() {

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

//TODO: implementar
void initialize_structures(){
    //Creo las estructuras de los entrenadores, pokemones y listas, etc
}

void start_log() {
    //TODO: cambiar el 1 por un 0 para la entrega
    logger = log_create(config.log_file, "team", 1, LOG_LEVEL_TRACE);
}

void attempt_connection() {

    if((server_socket = create_socket()) == -1) {
        log_error(logger, "Error al crear el socket de cliente");
        printf("No se pudo crear");
        return;
    }
    if(-1 == connect_socket(server_socket, config.ip_broker, config.puerto_broker)){
        log_error(logger, "Error al conectarse al Broker");
        printf("No se pudo conectar");
        return;
    }

    server_socket_initialized = true;

    subscribe_to_mq();
}

//TODO: desarrollar esto
void subscribe_to_mq() {}

void* attempt_connection_thread(void* arg) {

    while(!server_socket_initialized) {

        attempt_connection();
        sleep(config.tiempo_reconexion);
    }
    subscribe_to_mq();
}

void* server_function(void* arg) {
    //TODO: agregar servidor de Rodri
}

void send_to_server(MessageType mensaje){

    t_paquete* paquete = create_package(mensaje);

   add_to_package(paquete,(void*) &mensaje, sizeof(int));

   if(send_package(paquete, server_socket)  == -1){
       printf("No se pudo mandar");
   }

}