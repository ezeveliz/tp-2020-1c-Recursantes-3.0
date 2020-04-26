//
// Created by utnso on 07/04/20.
//

#ifndef TEAM_GAMEBOY_H
#define TEAM_GAMEBOY_H

#include <string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <stdlib.h>
#include <stdio.h>
#include <commLib/connections.h>
#include <commLib/structures.h>
#include <sys/time.h>
#include <signal.h>

#include "structures.h"
#endif //TEAM_GAMEBOY_H


#define PARAMETROS_BROKER_NEW 7
#define PARAMETROS_BROKER_APPEARED 7
#define PARAMETROS_BROKER_CATCH 6
#define PARAMETROS_BROKER_CAUGHT 5
#define PARAMETROS_BROKER_GET 4
#define PARAMETROS_TEAM_APPEARED 6
#define PARAMETROS_GAMECARD_NEW 8
#define PARAMETROS_GAMECARD_CATCH 7
#define PARAMETROS_GAMECARD_GET 4
#define PARAMETROS_SUSCRIPCION 3

/*
 * Funciones de funcionamiento
 */
void broker_distribuidor(int , char** );

void team_distribuidor(int , char** );

void gamecard_distribuidor(int , char** );

void suscribir(char* ,char* );

int envio_mensaje(t_paquete*, char* , uint32_t);

void mensaje_proceso(int, t_paquete* );

/*
 * Funciones auxiliares
 */
int str2Proces (const char*);

int str2Msj (const char* );

int okFailToInt(char* );

void msj_error();

/*
 * Funciones tamanio
 */

int size_t_new_pokemon(t_new_pokemon* );

int size_t_appeared_pokemon(t_appeared_pokemon*);

int size_t_catch_pokemon(t_catch_pokemon*);

int size_t_caught_pokemon(t_caught_pokemon*);

int size_t_get_pokemon(t_get_pokemon*);

void timer_handler (int);

void suscripcion_timer(int);
