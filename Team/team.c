#include "team.h"

TEAMConfig* config;
t_log* logger;

int main() {

    initialize_structures();
    read_config_options();
    start_log();

}

void initialize_structures(){

    config = malloc(sizeof(TEAMConfig));
}

void read_config_options() {

    t_config *config_file = config_create("../team.config");
    config -> posiciones_entrenadores = config_get_array_value(config_file, "POSICIONES_ENTRENADORES");
    config -> pokemon_entrenadores = config_get_array_value(config_file, "POKEMON_ENTRENADORES");
    config -> objetivos_entrenadores = config_get_array_value(config_file, "OBJETIVOS_ENTRENADORES");
    config -> tiempo_reconexion = config_get_int_value(config_file, "TIEMPO_RECONEXION");
    config -> retardo_ciclo_cpu = config_get_int_value(config_file, "RETARDO_CICLO_CPU");
    config -> algoritmo_planificacion = config_get_string_value(config_file, "ALGORITMO_PLANIFICACION");
    config -> quantum = config_get_int_value(config_file, "QUANTUM");
    config -> estimacion_inicial = config_get_int_value(config_file, "ESTIMACION_INICIAL");
    config -> ip_broker = config_get_string_value(config_file, "IP_BROKER");
    config -> puerto_broker = config_get_int_value(config_file, "PUERTO_BROKER");
    config -> log_file = config_get_string_value(config_file, "LOG_FILE");
}

void start_log() {
    //TODO: cambiar el 1 por un 0 para la entrega
    logger = log_create(config -> log_file, "team", 1, LOG_LEVEL_TRACE);
}