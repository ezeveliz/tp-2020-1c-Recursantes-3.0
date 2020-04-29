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
    int broker_ip;
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

t_list* list_new_pokemon;
t_list* list_appeared_pokemon;
t_list* list_get_pokemon;
t_list* list_localized_pokemon;
t_list* list_catch_pokemon;
t_list* list_caught_pokemon;

typedef struct subscriptor {
    int id_subs;
    char* ip_subs;
    int puerto_subs;
} subscriptor;

typedef struct mensaje {
    int id;
    int id_correlacional;
    MessageType tipo;
    t_list* enviados;
    t_list* confirmados;
    size_t tam;
    void* puntero_a_memoria;
    unsigned long lru;
} mensaje;

int IDENTIFICADOR_MENSAJE;

void tests_broker();
mensaje* mensaje_create(int id, int id_correlacional, MessageType tipo, size_t tam);
void* asignar_puntero_a_memoria();
size_t sizeof_pokemon(t_new_pokemon* estructura);
#endif //TEAM_BROKER_H
