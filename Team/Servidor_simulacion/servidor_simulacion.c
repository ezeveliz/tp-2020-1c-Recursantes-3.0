#include "servidor_simulacion.h"

int port = 5002;
int socket_server;
bool server_initialize = false;
t_log* logger;

int main() {

    start_log();

    initialize_server();

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

    switch(headerStruct -> type){

        case SUB_APPEARED:
            printf("APPEARD_POKEMON");
            break;
        case SUB_LOCALIZED:
            printf("LOCALIZED_POKEMON");
            break;
        case SUB_CAUGHT:
            printf("SUB_CAUGHT");
            break;
        case GET_POK:
            printf("GET_POKEMON");
            break;
        case CATCH_POK:
            printf("CATCH_POKEMON");
            break;
        case APPEARED_POK:
            printf("APPEARED_POKEMON");
            break;
        case LOCALIZED_POK:
            printf("LOCALIZED_POKEMON");
            break;
        case CAUGHT_POK:
            printf("CAUGHT_POKEMON");
            break;
        default:
            printf("la estas cagando compa");
            break;
    }

}

void initialize_server(){

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

    server_initialize = true;
}