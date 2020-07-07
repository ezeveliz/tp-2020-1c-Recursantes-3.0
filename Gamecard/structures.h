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

typedef struct{
    uint32_t x;
    uint32_t y;
    uint32_t cant;
    uint32_t tam;
    uint32_t pos_archivo; //Es el tamaño que ocupa ese registro en el archivo
} t_pos_pokemon;