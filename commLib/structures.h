#ifndef COMMLIB_STRUCTURES_H_
#define COMMLIB_STRUCTURES_H_
#define IP_LENGTH 20

#include "connections.h"

/**
 *  En este enum se agregan todos los distintos tipos de mensajes que se van a enviar entre los clientes y los
 *  servidores, cada vez que se agregue un nuevo enum, se debera volver a compilar y reinstalar la biblioteca
 */
typedef enum _MessageType {
    // Enum de prueba
    ABC,
    // PARA RESPONDER UTILIZAR EL MISMO ENCABEZADO
    // Suscripcion a colas globales Team-Broker
    SUB_NEW,
    SUB_APPEARED,
    SUB_LOCALIZED,
    SUB_CAUGHT,
    SUB_GET,
    SUB_CATCH,
    // Mensajes Team-Broker
    GET_POK,
    CATCH_POK,
    // Mensajes Broker-Team
    APPEARED_POK, // Tambien aplica a GameBoy-Team
    LOCALIZED_POK,
    CAUGHT_POK,
    NEW_POK
} MessageType;

/**
 *  Estructura que contiene el nombre del header de la comunicación y el tamaño del paquete en sí
 */
typedef struct _MessageHeader {
    MessageType type;
    int data_size;
} MessageHeader;

/**
 *  Estructura que contiene a un MessageHeader y a un void*, esta es la estructura que luego se serializara y se
 *  enviara por sockets
 */
 //TODO revisar si no se puede colocar todo directo en el paquete en vez de usar 2 estructuras
typedef struct {
    MessageHeader *header;
    void* stream;
} t_paquete;

/**
 * Estrucutra utilizada en el servidor multihilos
 */
typedef struct t_thread_client {
    int socket;
    char * client_ip;
    int connection_port;
    void (*lost_connection)(int, char*, int);
    void (*incoming_message)(int, char*, int, MessageHeader*);
}t_thread_client;

/**
 * Estructura utilizada para informar? aparicion de pokemon
 */
typedef struct {
    uint32_t nombre_pokemon_length;
    char* nombre_pokemon;
    uint32_t pos_x;
    uint32_t pos_y;
    uint32_t cantidad;
} t_new_pokemon;

/**
 * Estructura utlizada para solicitar? un pokemon con GET_POK
 */
typedef struct {
    uint32_t nombre_pokemon_length;
    char* nombre_pokemon;
} t_get_pokemon;

/**
 * Estructura utilizada para responder al mensaje GET_POK
 */
typedef struct {
    uint32_t nombre_pokemon_length;
    char* nombre_pokemon;
    uint32_t cantidad_coordenas;
    uint32_t* coordenadas;
} t_localized_pokemon;

/**
 * Estructura utilizada para responder al mensaje CATCH_POK
 */
typedef struct {
    uint32_t atrapado;
} t_caught_pokemon;

/**
 * Estructura utilizada para solicitar el atrapado de un pokemon con CATCH_POK
 */
typedef struct {
    uint32_t nombre_pokemon_length;
    char* nombre_pokemon;
    uint32_t pos_x;
    uint32_t pos_y;
} t_catch_pokemon;

/**
 * Estuctura utilizada para informar? la aparicion de un nuevo pokemon
 */
typedef struct {
    uint32_t nombre_pokemon_length;
    char* nombre_pokemon;
    uint32_t pos_x;
    uint32_t pos_y;
} t_appeared_pokemon;

#endif