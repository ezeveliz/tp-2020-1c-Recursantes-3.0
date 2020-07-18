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
    double alpha;
    int estimacion_inicial;
    char* ip_broker;
    int puerto_broker;
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

/**
 * Enum para identificar los estados de un entrenador dado
 */
typedef enum estado_entrenador{
    NEW,
    READY,
    EXEC,
    BLOCK,
    FINISH,
} estado_entrenador;

/**
 * Enum para identificar las distintas causas de bloqueo
 */
typedef enum razonBloqueo{
    SIN_ESPACIO,
    DEADLOCK, // Esta razon se le setea a los 2 entrenadores que fueron seleccionados para hacer el intercambio
    ESPERANDO_CATCH,
    ESPERANDO_POKEMON, // Estoy disponible pero no hay pokemones para darme
}RazonBloqueo;

/**
 * Estructura que representa un par de coordenadas
 */
typedef struct coordenada{
    int pos_x;
    int pos_y;
}Coordenada;

/**
 * Estructura de pokemon
 */
typedef struct {
    char* especie; // Nombre
    Coordenada coordenada; // Posicion en la que se encuentra
}Pokemon;

/**
 * Representa la razon por la que se esta moviendo el entrenador, para atrapar un pokemon o por un intercambio
 */
typedef enum razon_movimiento{
    CATCH,
    RESOLUCION_DEADLOCK
}RazonMovimiento;

/**
 * Estructura que representa al entrenador
 */
typedef struct tEntrenador{
    int tid;
    int acumulado_total; // Aca voy acumulando todos los ciclos de todas las rafagas del entrenador
    int acumulado_actual; // Aca voy acumulando los ciclos de la rafaga actual
    int ultima_ejecucion; // Aca tengo guardado solo el largo de la ultima rafaga
    double ultimo_estimado; // Aca guardo el ultimo estimado del planificador
    double estimado_actual; //Aca guardo el estimado actual del planificador SJF con desalojo
    Coordenada pos_actual; // Posicion en la que se encuentra actualmente
    RazonMovimiento razon_movimiento; // Razon por la que se esta ejecutando el hilo del entrenador
    t_dictionary* objetivos_particular; // Objetivos del entrenador
    t_dictionary* stock_pokemons; // Pokemones que ya posee
    struct timespec* tiempo_llegada; // Tiempo en que llega a Ready
    estado_entrenador estado; // Estado en el que se encientra
    int cant_stock; // Cant de pokemones que posee
    int cant_objetivos; // Cant de pokemones que debe atrapar
    RazonBloqueo razon_bloqueo;
    Pokemon* pokemon_objetivo; // Pokemon hacia el que me dirijo
    struct tEntrenador* entrenador_objetivo; // Entrenador hacia el que me dirijo
    bool vengo_de_ejecucion; // Se usa para los algoritmos con desalojo para saber si fui desalojado en la ejecucion anterior
    bool tengo_que_desalojar; // Se usa para el SJF con desalojo para verificar si tengo que desalojar el hilo o no
    bool calcular_estimado; // Se usa para saber cuando recalcular el estimado de un hilo
}Entrenador;

/**
 * Estrucura encargada de representar el mensaje que manda un entrenador y que se le va a pasar al hilo encargado de enviarlo
 */
typedef struct {
    void* message; // Mensaje a enviar
    uint32_t size; // Tamaño del mensaje a enviar
    MessageType header; // Tipo de mensaje a enviar
    int tid; // Tid del entrenador que envia el mensaje, si es -1, el mensaje no fue solicitado por un entrenador, p/ej: el get_pok
} t_new_message;

/**
 * Esta estructura relaciona al tid de un entrenador con el id correlacional de
 * un mensaje que este esperando(solo para los catches en realidad)
 */
typedef struct {
    int tid;
    int id_correlativo;
} WaitingMessage;

#endif //TEAM_TEAMSTRUCTURES_H