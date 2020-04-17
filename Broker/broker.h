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


/*
 *
 * NEW_POKEMON STARTS
 *
 * */
typedef struct {
    uint32_t nombre_pokemon_length;
    char* nombre_pokemon;
    uint32_t pos_x;
    uint32_t pos_y;
    uint32_t cantidad;
} t_new_pokemon;

t_new_pokemon* create_new_pokemon(char* nombre_pokemon, uint32_t pos_x, uint32_t pos_y, uint32_t cantidad);
void* new_pokemon_a_void(t_new_pokemon* new_pokemon);
t_new_pokemon* void_a_new_pokemon(void* stream);

/*
 *
 * GET_POKEMON STARTS
 *
 * */


typedef struct {
    uint32_t nombre_pokemon_length;
    char* nombre_pokemon;

} t_get_pokemon;

t_get_pokemon* create_get_pokemon(char* nombre_pokemon);
void* get_pokemon_a_void(t_get_pokemon* get_pokemon);
t_get_pokemon* void_a_get_pokemon(void* stream);


/*
 *
 * LOCALIZED_POKEMON STARTS
 *
 * */


typedef struct {
    uint32_t nombre_pokemon_length;
    char* nombre_pokemon;
    uint32_t cantidad_coordenas;
    uint32_t* coordenadas;
} t_localized_pokemon;

t_localized_pokemon* create_localized_pokemon(char* nombre_pokemon, uint32_t cantidad_coordenadas, ...);
void* localized_pokemon_a_void(t_localized_pokemon* localized_pokemon);
t_localized_pokemon* void_a_localized_pokemon(void* stream);

/*
 *
 * CAUGHT_POKEMON STARTS
 *
 * */

typedef struct {
    uint32_t atrapado;
} t_caught_pokemon;

t_caught_pokemon* create_caught_pokemon(uint32_t atrapado);
void* caught_pokemon_a_void(t_caught_pokemon* caught_pokemon);
t_caught_pokemon* void_a_caught_pokemon(void* stream);


void tests_broker();
#endif //TEAM_BROKER_H

/*
 *
 * CATCH_POKEMON STARTS
 *
 * */

typedef struct {
    uint32_t nombre_pokemon_length;
    char* nombre_pokemon;
    uint32_t pos_x;
    uint32_t pos_y;
} t_catch_pokemon;

t_catch_pokemon* create_catch_pokemon(char* nombre_pokemon, uint32_t pos_x, uint32_t pos_y);
void* catch_pokemon_a_void(t_catch_pokemon* catch_pokemon);
t_catch_pokemon* void_a_catch_pokemon(void* stream);

/*
 *
 * APPEARED_POKEMON STARTS
 *
 * */

typedef struct {
    uint32_t nombre_pokemon_length;
    char* nombre_pokemon;
    uint32_t pos_x;
    uint32_t pos_y;
} t_appeared_pokemon;

t_appeared_pokemon* create_appeared_pokemon(char* nombre_pokemon, uint32_t pos_x, uint32_t pos_y);
void* appeared_pokemon_a_void(t_appeared_pokemon* appeared_pokemon);
t_appeared_pokemon* void_a_appeared_pokemon(void* stream);

