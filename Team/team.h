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
 * Inicializo el log en la ruta especificada por archivo de configuracion
 */
void start_log();

/**
 * Esta funcion intenta suscribirse a las 3 colas globales: appeared_pokemon, localized_pokemon y caught_pokemon, en
 * caso de que no este conectado ya y setea 3 booleanos globales uno correspondiente a cada cola
 */
void attempt_subscription();

/**
 * Me conecto al servidor del Broker
 * @return -1 en caso de error o el socket del servidor
 */
int connect_to_broker();

/**
 * Me desconecto del Broker
 * @param broker_socket
 */
void disconnect_from_broker(int broker_socket);

/**
 * Intento suscribirme a la cola dada del Broker dado
 * TODO: falta agregar el parametro del tipo de cola
 * @param int broker
 * @return  True si me pude subscribir a la cola indicada, False si no
 */
bool subscribe_to_queue(int broker);

/**
 * Funcion que va a correr en el hilo del servidor para escuchar los mensajes del gameboy
 * @param arg
 * @return
 */
void* server_function(void* arg);

/**
 * Funcion que va a reintentar la subscripcion a las distintas colas de mensajes globales cada n segundos
 * @param arg
 * @return
 */
void* queues_subscription_function(void* arg);

/**
 * Inicializo las estructuras necesarias
 */
void initialize_structures();

//----------------------------------------HELPERS

/**
 * Verifico si estoy o no suscripto a TODAS las colas globales
 * @return True si estoy suscripto a todas, False si no estoy suscripto a todas
 */
bool subscribed_to_all_global_queues();

/**
 * Envio un mensaje de prueba al servidor(Broker)
 * @param mensaje
 */
void send_to_server(MessageType mensaje);

#endif //TEAM_TEAM_H
