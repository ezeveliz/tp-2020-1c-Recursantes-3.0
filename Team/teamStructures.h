//
// Created by utnso on 07/04/20.
//

#ifndef TEAM_TEAMSTRUCTURES_H
#define TEAM_TEAMSTRUCTURES_H

/**
 * Estructura encargada de almacenar las configuraciones que se pasan por archivo
 */
typedef struct _TEAMConfig {
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
    char* log_file;
} TEAMConfig;


#endif //TEAM_TEAMSTRUCTURES_H