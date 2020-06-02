//
// Created by utnso on 07/04/20.
//

#ifndef TEAM_TEAMSTRUCTURES_H
#define TEAM_TEAMSTRUCTURES_H

/**
 * Enum que representa los algoritmos de planificacion
 */
typedef enum planner_algorithm {
    FIFO,
    SJF_SD,
    SJF_CD,
    RR,
} planner_algorithm;

/**
 * Estructura encargada de almacenar las configuraciones que se pasan por archivo
 */
typedef struct {
    char** posiciones_entrenadores;
    char** pokemon_entrenadores;
    char** objetivos_entrenadores;
    int tiempo_reconexion;
    int retardo_ciclo_cpu;
    planner_algorithm algoritmo_planificacion;
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
typedef struct {
    struct timespec* start_time;
    struct timespec* end_time;
} t_interval;

typedef enum estado_entrenador{
    NEW,
    READY,
    EXEC,
    BLOCK,
    FINISH,
} estado_entrenador;

typedef struct {
    int tid;
    int pos_actual_x;
    int pos_actual_y;
    int pos_destino_x;
    int pos_destino_y;
    t_dictionary* objetivos_particular;
    t_dictionary* stock_pokemons;
    struct timespec* tiempo_llegada;
    estado_entrenador estado;
}Entrenador;

/**
 * Estrucura encargada de representar una respuesta al cliente
 * Esta formada por el socket cliente al cual responder, un int que va a representar la respuesta y el cliente
 * se encargara de interpretar y un Header de respuesta
 */
typedef struct {
    void* message;
    int size;
    MessageType header;
} t_new_message;

/**
 * Estructura de pokemon
 */

typedef struct {
    //Verificar si manejamos la estructura de entrenador como una lista o un dictionary y si la especie deberia ser un char*
    char* especie;
    int pos_x;
    int pos_y;
}Pokemon;


#endif //TEAM_TEAMSTRUCTURES_H