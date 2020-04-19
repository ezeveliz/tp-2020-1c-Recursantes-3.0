//
// Created by utnso on 18/04/20.
//

#ifndef TEAM_SERVIDOR_SIMULACION_H
#define TEAM_SERVIDOR_SIMULACION_H

#include <commons/config.h>
#include <commons/log.h>
#include <commLib/connections.h>
#include <commLib/structures.h>

#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "teamStructures.h"

void start_log();

void new(int fd, char * ip, int port);

void lost(int fd, char * ip, int port);

void incoming(int fd, char* ip, int port, MessageHeader * headerStruct);




#endif //TEAM_SERVIDOR_SIMULACION_H
