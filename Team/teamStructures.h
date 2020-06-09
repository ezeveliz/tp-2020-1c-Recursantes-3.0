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

typedef enum razonBloqueo{
    DEADLOCK,
    ESPERANDO_CATCH,
    ESPERANDO_POKEMON,
}RazonBloqueo;

typedef struct coordenada{
    int pos_x;
    int pos_y;
}Coordenada;

/**
 * Estructura de pokemon
 */

typedef struct {
    char* especie;
    Coordenada coordenada;
}Pokemon;

typedef enum razon_movimiento{
    CATCH,
    RESOLUCION_DEADLOCK
}RazonMovimiento;

/**
 * Estructura que representa al entrenador
 */
typedef struct tEntrenador{
    int tid;
    Coordenada pos_actual;
    RazonMovimiento razon_movimiento;
    t_dictionary* objetivos_particular;
    t_dictionary* stock_pokemons;
    struct timespec* tiempo_llegada;
    estado_entrenador estado;
    int cant_stock;
    int cant_objetivos;
    RazonBloqueo razon_bloqueo;
    Pokemon* pokemon_objetivo;
    struct tEntrenador* entrenador_objetivo;
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



#endif //TEAM_TEAMSTRUCTURES_H