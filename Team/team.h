//
// Created by utnso on 07/04/20.
//

#ifndef TEAM_TEAM_H
#define TEAM_TEAM_H

#include <commons/config.h>
#include <commons/log.h>
#include <commLib/connections.h>
#include <commLib/structures.h>

#include <stdlib.h>
#include <pthread.h>
#include <string.h>

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

/**
 * Me suscribo a las colas del broker
 */
void subscribe_to_mq();

/**
 * Funcion para intentar la conexion con el broker
 */
void attempt_connection();

/**
 * Funcion que va a reintentar la conexion cada n segundos
 * @param arg
 * @return
 */
void* attempt_connection_thread(void* arg);

#endif //TEAM_TEAM_H
