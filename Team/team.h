//
// Created by utnso on 07/04/20.
//

#ifndef TEAM_TEAM_H
#define TEAM_TEAM_H

#include <commons/config.h>
#include <commons/log.h>
#include <stdlib.h>

#include "teamStructures.h"

/**
 * Inicializo las estructuras necesarias
 */
void initialize_structures();

/**
 * Leo archivo de configuracion
 */
void read_config_options();

/**
 * Inicializo el log en la ruta especificada por archivo de configuracion
 */
void start_log();

#endif //TEAM_TEAM_H
