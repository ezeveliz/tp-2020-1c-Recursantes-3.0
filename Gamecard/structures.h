//
// Created by utnso on 09/05/20.
//

#ifndef GAMEBOY_STRUCTURES_H
#define GAMEBOY_STRUCTURES_H

#endif //GAMEBOY_STRUCTURES_H

typedef struct {
    char* punto_montaje;
    int tiempo_reconexion;
    int tiempo_reoperacion;
    char* ip_broker;
    int puerto_broker;
    int gamecard_id;
    int puerto_gamecard;
} t_gamecard_config;