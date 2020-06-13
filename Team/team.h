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
#include <sys/time.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

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
 * Inicializo cosas:
 *  -   Listas de estados
 *  -   Diccionario de objetivos globales
 *  -   Semaforos para proteccion de listas
 *  -   Entrenadores, con su ubicacion, objetivos particulares, el estado inicial, un tid falso(el contador en un for)
 *  -   Semaforos para sincronizacion de entrenadores
 *  -   Mando los mensajes de get_pok segun las necesidades globales
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
void* trainer_thread(void* arg);

/**
 * Llamo al planificador correspondiente al pasado por configuracion
 */
void call_planner();

/**
 * Algoritmo de planificacion FIFO
 */
void fifo_planner();

/**
 * Algoritmo de planificaion SJF sin desalojo
 */
void sjf_sd_planner();

/**
 * Algoritmo de planificaion SJF con desalojo
 */
void sjf_cd_planner();

/**
 * Algoritmo de planificaion RR
 */
void rr_planner();

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

/**
 * Se llama a esta funcion cuando el servidor recibe un APPEARED
 * @param paquete: Se le pasa una lista para despues dividirlo en especie, pos_x y pos_y
 */
void appeared_pokemon(t_list* paquete );

/**
 * Algoritmo que encuentra el entrenador mas cerca de un pokemon, el pokemon encontrado es bloqueado para que
 * otro entrenador no pueda acceder a el. Esta funcion es llamada cuando hay un LOCALIZED, APPEARED y cuando termina
 * de ejecutar un entrenador.
 */
void algoritmo_de_cercania();

/**
 * Libero los recursos cuando todos los hilos terminaron de ejecutar
 */
void free_resources();

//----------------------------------------HELPERS----------------------------------------//

/**
 * Wrapper para liberar una lista, nombre mas corto
 * @param received
 * @param element_destroyer
 */
void free_list(t_list* received, void(*element_destroyer)(void*));

/**
 * Retorno una estructura que representa al tiempo en segundos y microsegundos
 * @return
 */
struct timespec get_time();

/**
 * Creo un nuevo intervalo con su memoria ya alocada
 * @return interval* interval
 */
t_interval* new_interval();

/**
 * Conversion de un timespec a microsegundos;
 * @param timespec
 * @return
 */
long timespec_to_us(struct timespec* timespec);

/**
 * Hallo la diferencia de tiempo entre dos timespec
 * @param start, timespec en el que inicio la medición
 * @param end, timespec en el que termino la medición
 * @param diff, timespec en el que se almacena la diferencia entre ambos
 */
void time_diff(struct timespec* start, struct timespec* end, struct timespec* diff);

/**
 * Creo un hilo para mandarle una solicitud al Broker
 *
 * @param message
 * @param size, tamanio del mensaje a enviar
 * @param header del mensaje a enviar
 * @param tid del entrenador, si me pasan -1, el tid no importa(en el caso de un GET por ejemplo), si no lo tomo en cuenta
 */
void send_message_thread(void* message, int size, MessageType header, int tid);

/**
 * Creo un paquetito para pasarle informacion al hilo que se va a encargar de mandarle una solicitud al Broker
 *
 * @param message
 * @param size, tamanio del mensaje a enviar
 * @param header del mensaje a enviar
 * @param tid del entrenador que envio el mensaje
 * @return paquete void* que le voy a pasar al hilo encargado de enviar el mensaje
 */
void* create_message_package(void* message, int size, MessageType header, int tid);

/**
 * Funcion encarga de mandarle una solicitud al Broker, responderle con una confirmacion y ejecutar el caso por default en caso de que no se pueda comunicar
 *
 * @param response_package
 * @return nada
 */
void* message_function(void* message_package);

/**
 * Funcion por default a ejecutarse segun el tipo de mensaje enviado
 *
 * @param header, tipo de solicitud enviada
 * @param tid, tid del entrenador que solicito el mensaje, si es -1 no utilizar
 */
void exec_default(MessageType header, int tid);

/**
 * Funcion que se ejecuta cuando recibo la respuesta de un catch, le avisa al entrenador si atrapo al pokemon o no
 * @param tid, tid del entrenador que mando originalmente el mensaje
 * @param atrapado, respuesta que me mando el Broker, un 0 es no atrapado y un 1 es atrapado
 */
void caught_pokemon(int tid, int atrapado);

/**
 * Hallo la distancia entre dos coordenadas, la actual y la de destino.
 * @param actual
 * @param siguiente
 * @return
 */
int distancia(Coordenada actual, Coordenada siguiente);

/**
 * Verifica si el entrenador cumplio todos sus objetivos respecto al stock de pokemons
 * @param entrenador
 * @return true o false
 */
bool objetivos_cumplidos(Entrenador* entrenador);

#endif //TEAM_TEAM_H
