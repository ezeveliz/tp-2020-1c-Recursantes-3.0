//
// Created by utnso on 07/04/20.
//

#ifndef TEAM_GAMECARD_H
#define TEAM_GAMECARD_H

#include <commons/config.h>
#include <commons/log.h>
#include <commLib/connections.h>
#include <commLib/structures.h>
#include "structures.h"


#endif //TEAM_GAMECARD_H

int leer_opciones_configuracion();

void liberar_opciones_configuracion();

int conectar_broker();

void desconectar_broker(int broker_socket);

void suscribir_colas();

int suscribir_cola(int broker, MessageType cola);

void* suscribir_cola_thread(void* arg);

int conectarse_y_suscribir(MessageType cola);

void mensaje_new_pokemon();

void mensaje_get_pokemon();

void mensaje_catch_pokemon();