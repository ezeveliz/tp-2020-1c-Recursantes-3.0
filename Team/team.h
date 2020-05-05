//
// Created by utnso on 07/04/20.
//

#ifndef TEAM_TEAM_H
#define TEAM_TEAM_H

#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <commLib/connections.h>
#include <commLib/structures.h>

#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "teamStructures.h"

/**
 * Leo archivo de configuracion y cargo los datos en nuestra estructura
 * @return si no se encontro el archivo de configuracion retorno el cod de error -1, 1 si se pudo leer archivo
 */
int read_config_options();

/**
 * Inicializo el log en la ruta especificada por archivo de configuracion
 * @return Si no pude crear el archivo de configuracion en la ruta especificada retorno -1, sino 1;
 */
int start_log();

/**
 * Esta funcion intenta suscribirse a las 3 colas globales: appeared_pokemon, localized_pokemon y caught_pokemon,
 * creando 3 hilos los cuales se van a ocupar de conectarse al Broker y mantener la conexion abierta
 */
void subscribe_to_queues();

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
 * Hilo en el que manejo la conexion/reconexion al servidor y la suscripcion a una cola dada
 * @param arg
 * @return
 */
void* subscribe_to_queue_thread(void* arg);

/**
 * Me intento conectar al Broker y suscribir a una cola dada
 * @param cola, cola a la cual suscribirme
 * @return retorno el socket al que me conecte
 */
int connect_and_subscribe(MessageType cola);

/**
 * Intento suscribirme a la cola dada del Broker dado
 * @param int broker
 * @param MessageType cola
 * @return True si me pude subscribir a la cola indicada, False si no
 */
bool subscribe_to_queue(int broker, MessageType cola);

/**
 * Funcion que va a correr en el hilo del servidor para escuchar los mensajes del gameboy
 * @param arg
 * @return
 */
void* server_function(void* arg);

/**
 * Inicializo las estructuras necesarias TODO: detallar
 */
void initialize_structures();

/**
 * Agrego elementos a un diccionario
 * @param cosas_agregar
 * @param diccionario
 * @return diccionario con los elementos agregados
 */
void add_to_dictionary(char** cosas_agregar, t_dictionary* diccionario);

/**
 * Agrego objetivos a los objetivos globales
 * @param objetivos_entrenador, los objetivos de un entrenador dado, se agregan a los ya existentes
 * @param pokemon_entrenador, pokemones que poseen los entrenadores actualmente, restan a los objetivos globales
 */

void add_global_objectives(char** objetivos_entrenador, char** pokemon_entrenador);

/**
 * Funcion en la que van a ir los hilos inicializados de los entrenadores
 * @param arg
 * @return
 */
void* scheduling(void* arg);

/**
 * Inicializo el log del servidor para pruebas
 */
void start_log_server();

/**
 * Inicializo el servidor. Creo el socket y hago el bind correspondiente
 */
int initialize_server();

/**
 * Cada vez que el servidor detecte que se inicio una nueva conexion va a llamar a esta
 * funcion
 * @param socket_server
 * @param ip
 * @param port
 */
void new(int socket_server, char * ip, int port);

/**
 * Cada vez que el servidor detecte que se perdio una conexion va a llamar a esta
 * funcion
 * @param socket_server
 * @param ip
 * @param port
 */
void lost(int socket_server, char * ip, int port);

/**
 * Cada vez que llegue una nueva conexion al servidor va a llamar a esta funcion
 * @param socket_server
 * @param ip
 * @param port
 * @param headerStruct
 */
void incoming(int socket_server, char* ip, int port, MessageHeader * headerStruct);

//----------------------------------------HELPERS----------------------------------------//

/**
 * Envio un mensaje de prueba al servidor(Broker)
 * @param mensaje
 */
void send_to_server(MessageType mensaje);

#endif //TEAM_TEAM_H
