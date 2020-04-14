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

#endif