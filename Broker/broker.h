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
subscriptor* subscriptor_create(int id, char* ip, int puerto, int socket);
bool existe_sub(int id, t_list* cola);
void subscriptor_delete(int id, t_list* cola);
void subscribir_a_cola(t_list* cosas, char* ip, int puerto, int fd, t_list* una_cola, MessageType tipo);
#endif //TEAM_BROKER_H
