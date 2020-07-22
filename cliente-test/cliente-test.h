//
// Created by utnso on 29/05/20.
//

#ifndef BROKER_CLIENTE_TEST_H
#define BROKER_CLIENTE_TEST_H

#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commLib/connections.h>
#include <commLib/structures.h>

int CANTIDAD_MENSAJES_A_ENVIAR;

typedef struct msj {
    int id_mensaje;
} msj;
t_list* PENDIENTES;

typedef struct ClienteTestCFG {
    int id_cliente;
    char* broker_ip;
    int broker_port;
    char* cliente_test_ip;
    int cliente_test_port;

} ClienteTestCFG;

ClienteTestCFG config;
t_log * logger;
int broker_fd;

int connect_to_broker();
bool set_config();
int subscribir_cola(MessageType cola);
void server();
void terminar();
void mandar_mensaje();
msj* create_msj(int id);
void delete_msj(int id);


#endif //BROKER_CLIENTE_TEST_H
