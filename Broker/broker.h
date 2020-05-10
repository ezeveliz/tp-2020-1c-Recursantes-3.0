//
// Created by utnso on 07/04/20.
//

#ifndef TEAM_BROKER_H
#define TEAM_BROKER_H

#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
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


t_log * logger;

t_list* LIST_NEW_POKEMON;
t_list* LIST_APPEARED_POKEMON;
t_list* LIST_GET_POKEMON;
t_list* LIST_LOCALIZED_POKEMON;
t_list* LIST_CATCH_POKEMON;
t_list* LIST_CAUGHT_POKEMON;

typedef struct subscriptor {
    int id_subs;
    char* ip_subs;
    int puerto_subs;
    int socket;
} subscriptor;
t_list* SUBSCRIPTORES;

typedef struct mensaje {
    int id;
    int id_correlacional;
    MessageType tipo;
    size_t tam;
    void* puntero_a_memoria;
    unsigned long lru;
} mensaje;
t_list* MENSAJES;

typedef struct mensaje_subscriptor {
    int id_mensaje;
    int id_subscriptor;
    bool enviado;
    bool ack;
} mensaje_subscriptor;
t_list* MENSAJE_SUBSCRIPTORE;

int IDENTIFICADOR_MENSAJE;

void tests_broker();
mensaje* mensaje_create(int id, int id_correlacional, MessageType tipo, size_t tam);
mensaje_subscriptor* mensaje_subscriptor_create(int id_mensaje, int id_sub);
void* asignar_puntero_a_memoria();
size_t sizeof_new_pokemon(t_new_pokemon* estructura);
size_t sizeof_appeared_pokemon(t_appeared_pokemon* estructura);
size_t sizeof_get_pokemon(t_get_pokemon* estructura);
size_t sizeof_localized_pokemon(t_localized_pokemon* estructura);
size_t sizeof_catch_pokemon(t_catch_pokemon* estructura);
size_t sizeof_caught_pokemon(t_caught_pokemon* estructura);
subscriptor* subscriptor_create(int id, char* ip, int puerto, int socket);
bool existe_sub(int id, t_list* cola);
void subscriptor_delete(int id, t_list* cola);
void subscribir_a_cola(t_list* cosas, char* ip, int puerto, int fd, t_list* una_cola, MessageType tipo);
void mensaje_subscriptor_delete(int id_mensaje, int id_subscriptor);
mensaje* find_mensaje(int id);
subscriptor* find_subscriptor(int id);
void cargar_mensaje(t_list* una_cola, mensaje* un_mensaje);
void recursar_operativos();
#endif //TEAM_BROKER_H
