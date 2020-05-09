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

/**
 * Estructura encargada de representar un intervalo de tiempo
 * Esta formada a su vez por otras dos estructuras timespec, las cuales almacenan timestamps y con las cuales se puede
 * operar para obtener la diferencia entre ambas
 */
typedef struct _t_interval{
    struct timespec* start_time;
    struct timespec* end_time;
} t_interval;

typedef struct {
    int tid;
    int pos_x;
    int pos_y;
    t_dictionary* objetivos_particular;
    t_dictionary* stock_pokemons;
    struct timespec* tiempo_llegada;
}Entrenador;

/**
 * Estrucura encargada de representar una respuesta al cliente
 * Esta formada por el socket cliente al cual responder, un int que va a representar la respuesta y el cliente
 * se encargara de interpretar y un Header de respuesta
 */
typedef struct _t_new_response{
    int fd;
    int response;
    MessageType header;
} t_new_response;

#endif //TEAM_TEAMSTRUCTURES_H