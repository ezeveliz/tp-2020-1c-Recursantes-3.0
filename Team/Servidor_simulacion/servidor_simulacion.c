#include "servidor_simulacion.h"

int port = 5002;
int socket_server;
t_log* logger;

int main() {

    start_log();

    if((socket_server = create_socket()) == -1) {
        log_error(logger, "Error creating socket");
        printf("No se creo el socket");
        return;
    }
    if((bind_socket(socket_server, port)) == -1) {
        log_error(logger, "Error binding socket");
        printf("No se bindio el socket");
        return;
    }

    start_server(socket_server, &new, &lost, &incoming);

}

void start_log() {

    logger = log_create("../servidor_simulacion.log", "servidor_simulacion", 1, LOG_LEVEL_TRACE);
}

void new(int socket_server, char * ip, int port){
    printf("new \n");
}

void lost(int socket_server, char * ip, int port){
    printf("lost \n");
}


void incoming(int socket_server, char* ip, int port, MessageHeader * headerStruct){

    t_list *cosas = receive_package(socket_server, headerStruct);

    char* test = (char*) list_get(cosas,0);

    puts(test);

}