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
void mandar_mensaje();


#endif //BROKER_CLIENTE_TEST_H
