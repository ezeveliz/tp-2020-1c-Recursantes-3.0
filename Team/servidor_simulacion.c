#include "servidor_simulacion.h"


t_log* logger;

int main() {

    start_log();

    int PORT = 5002;
    int socket;
    if((socket = create_socket()) == -1) {
        log_error(logger, "Error creating socket");
        exit(1);
    }
    if((bind_socket(socket, PORT)) == -1) {
        log_error(logger, "Error binding socket");
        exit(2);
    }

    start_server(socket, &new, &lost, &incoming);

}

void start_log() {

    logger = log_create("../servidor_simulacion.log", "servidor_simulacion", 1, LOG_LEVEL_TRACE);
}

void new(int fd, char * ip, int port){
    printf("new");
}

void lost(int fd, char * ip, int port){
    printf("lost");
}


void incoming(int fd, char* ip, int port, MessageHeader * headerStruct){
    printf("incoming");
}