//
// Created by utnso on 07/04/20.
//

#ifndef TEAM_GAMEBOY_H
#define TEAM_GAMEBOY_H

#include <commons/config.h>
#include <commons/log.h>
#include "structures.h"
#include <string.h>
#include <stdlib.h>
#include <commLib/connections.h>
#include <commLib/structures.h>

#endif //TEAM_GAMEBOY_H

int str2Proces (const char*);

int str2Msj (const char* );

void broker_distribuidor(int , char** );

void team_distribuidor(int , char** );

void gamecard_distribuidor(int , char** );

void suscribir(char* ,char* );

void msj_error();

void mensaje_proceso(int, t_paquete* );

//void mensaje_broker_new_pokemon ( char* , uint32_t, uint32_t, uint32_t);

//void mensaje_broker_appeared_pokemon(char* , uint32_t, uint32_t, uint32_t);