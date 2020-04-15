//
// Created by utnso on 07/04/20.
//

#ifndef TEAM_BROKER_H
#define TEAM_BROKER_H

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

void read_config();

/*
 * Configuration ends
 */

t_log * logger;


#endif //TEAM_BROKER_H
