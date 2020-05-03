//
// Created by utnso on 07/04/20.
//

#ifndef TEAM_TEAMSTRUCTURES_H
#define TEAM_TEAMSTRUCTURES_H

/**
 * Estructura encargada de almacenar las configuraciones que se pasan por archivo
 */
typedef struct {
    char** posiciones_entrenadores;
    char** pokemon_entrenadores;
    char** objetivos_entrenadores;
    int tiempo_reconexion;
    int retardo_ciclo_cpu;
    char* algoritmo_planificacion;
    int quantum;
    int estimacion_inicial;
    char* ip_broker;
    int puerto_broker;
    char* ip_team;
    int puerto_team;
    char* log_file;
    int team_id;
} TEAMConfig;

typedef struct {
    int tid;
    int pos_x;
    int pos_y;
    t_dictionary* objetivos_particular;
    t_dictionary* stock_pokemons;
//    char* objetivos_particular;
//    char* stock_pokemons;
}Entrenador;

#endif //TEAM_TEAMSTRUCTURES_H