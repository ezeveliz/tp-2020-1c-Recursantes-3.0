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
 * Leo archivo de configuracion
 */
void read_config_options();

/**
 * Inicializo las estructuras necesarias
 */
void initialize_structures();

/**
 * Inicializo el log en la ruta especificada por archivo de configuracion
 */
void start_log();

/**
 * Funcion para suscribirme a las listas del Broker, solo se llama una vez despues de haberme podido conectar
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

/**
 * Funcion que va a correr en el hilo del servidor para escuchar los mensajes del gameboy
 * @param arg
 * @return
 */
void* server_function(void* arg);

/**
 * Envio un mensaje de prueba al servidor(Broker)
 * @param mensaje
 */
void send_to_server(MessageType mensaje);

#endif //TEAM_TEAM_H
