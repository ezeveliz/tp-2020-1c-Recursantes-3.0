//
// Created by utnso on 07/04/20.
//

#ifndef TEAM_BROKER_H
#define TEAM_BROKER_H

#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>
#include <limits.h>
#include <signal.h>

#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/txt.h>
#include <commLib/connections.h>
#include <commLib/structures.h>



/*
 * Configuration starts
 */
typedef struct BrokerCFG {
    int mem_size;
    int min_partition_size;
    char* mem_algorithm;
    char* mem_swap_algorithm;
    char* free_partition_algorithm;
    char* broker_ip;
    int broker_port;
    int compactation_freq;
    char* log_file;

} BrokerCFG;

char * cfg_path;
t_config * cfg_file;
BrokerCFG config;

void set_config();
void *server_function(void *arg);


/*
 * Configuration ends
 */

void* MEMORIA_PRINCIPAL;
pthread_mutex_t M_MEMORIA_PRINCIPAL;

t_log * logger;
t_log * tp_logger;

t_list* LIST_NEW_POKEMON;
pthread_mutex_t M_LIST_NEW_POKEMON;
t_list* LIST_APPEARED_POKEMON;
pthread_mutex_t M_LIST_APPEARED_POKEMON;
t_list* LIST_GET_POKEMON;
pthread_mutex_t M_LIST_GET_POKEMON;
t_list* LIST_LOCALIZED_POKEMON;
pthread_mutex_t M_LIST_LOCALIZED_POKEMON;
t_list* LIST_CATCH_POKEMON;
pthread_mutex_t M_LIST_CATCH_POKEMON;
t_list* LIST_CAUGHT_POKEMON;
pthread_mutex_t M_LIST_CAUGHT_POKEMON;

typedef struct subscriptor {
    int id_subs;
    char* ip_subs;
    int puerto_subs;
    int socket;
} subscriptor;
t_list* SUBSCRIPTORES;
pthread_mutex_t M_SUBSCRIPTORES;

typedef struct mensaje {
    int id;
    int id_correlacional;
    MessageType tipo;
    size_t tam;
    void* puntero_a_memoria;
    unsigned long lru;
} mensaje;
t_list* MENSAJES;
pthread_mutex_t M_MENSAJES;

typedef struct mensaje_subscriptor {
    int id_mensaje;
    int id_subscriptor;
    bool enviado;
    bool ack;
} mensaje_subscriptor;
t_list* MENSAJE_SUBSCRIPTORE;
pthread_mutex_t M_MENSAJE_SUBSCRIPTORE;

int IDENTIFICADOR_MENSAJE;
pthread_mutex_t M_IDENTIFICADOR_MENSAJE;

int MIN_PART_LEN;
int INTENTOS;
pthread_mutex_t M_INTENTOS;

typedef struct particion {
    int base;
    int tam;
    bool libre;
    uint64_t ultimo_uso; // para LRU
    mensaje* mensaje; // esto es para el dump
} particion;
t_list* PARTICIONES;
pthread_mutex_t M_PARTICIONES;
t_list* PARTICIONES_QUEUE;
pthread_mutex_t M_PARTICIONES_QUEUE;

typedef struct t_nodo {
    particion* particion;
    struct t_nodo* izq;
    struct t_nodo* der;
    struct t_nodo* padre;
    struct t_nodo* buddy;
    bool es_hoja;
} t_nodo;
t_nodo* ARBOL_BUDDY;
pthread_mutex_t M_ARBOL_BUDDY;

void tests_broker();
mensaje* mensaje_create(int id, int id_correlacional, MessageType tipo, size_t tam);
mensaje_subscriptor* mensaje_subscriptor_create(int id_mensaje, int id_sub);
void* asignar_puntero_a_memoria();
subscriptor* subscriptor_create(int id, char* ip, int puerto, int socket);
bool existe_sub(int id, t_list* cola);
void subscriptor_delete(int id, t_list* cola);
void subscribir_a_cola(t_list* cosas, char* ip, int puerto, int fd, t_list* una_cola, MessageType tipo);
void mensaje_subscriptor_delete(int id_mensaje, int id_subscriptor);
mensaje* find_mensaje(int id);
subscriptor* find_subscriptor(int id);
void cargar_mensaje(t_list* una_cola, mensaje* un_mensaje);
void recursar_operativos();
void mandar_mensaje(void* coso);
void* mensaje_subscriptor_a_void(mensaje_subscriptor* un_men_sub);
mensaje_subscriptor* void_a_mensaje_subscriptor(void* stream);
void* flag_enviado(uint32_t id_sub, uint32_t id_men);
void* flag_ack(uint32_t id_sub, uint32_t id_men);
int send_message_test(t_paquete* paquete, int socket);
particion* particion_create(int base, int tam, bool is_free);
void particion_delete(int base);
particion* buscar_particion_libre(int tam);
particion* first_fit_search(int tam);
particion* best_fit_search(int tam);
void ordenar_particiones();
void mergear_particiones_libres();
void dump_cache(int sig);
void compactar_particiones();
char* cola_to_string(MessageType cola);
particion* get_fifo();
void quitarVictimaFIFO(int base);
particion* get_lru();
particion* asignar_particion(size_t tam);
void algoritmo_de_reemplazo();
particion* find_particion_by_id_mensaje(int id_mensaje);
bool existe_mensaje_subscriptor(int id_mensaje, int id_subs);
MessageType sub_to_men(MessageType cola);
#endif //TEAM_BROKER_H
