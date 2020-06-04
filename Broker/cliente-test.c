//
// Created by utnso on 29/05/20.
//
#include "../cliente-test/cliente-test.h"
int main(int argc, char **argv) {
    logger = log_create("cliente_test.log", "CLIENTE_TEST", 1, LOG_LEVEL_TRACE);
    log_info(logger,"Log started.");
    bool init_config = set_config();
    init_config ? log_info(logger,"Config setted up successfully. :)") : log_info(logger,"Error while setting config :|");
    int s = connect_to_broker();
    printf("%d\n", s);
    char x;
    scanf("%c", &x);


    x == '1' ? (close(s) == 0 ? log_info(logger, "chau") : log_info(logger, "error closing connection")) : log_info(logger, "keep connected");

}

bool set_config(){

    config.broker_ip = "127.0.0.1";
    config.broker_port = 5002;
    config.cliente_test_ip = "127.0.0.1";
    config.cliente_test_port = 5003;
    return (config.broker_ip && config.broker_port && config.cliente_test_ip && config.cliente_test_port) ? true : false;

}

int connect_to_broker(){

    int client_socket;
    if((client_socket = create_socket()) == -1) {
        log_error(logger, "Error creating client socket :|");
        return -1;
    }
    if(connect_socket(client_socket, config.broker_ip, config.broker_port) == -1){
        log_error(logger, "Error connecting to Broker :?");
        return -1;
    }
    log_info(logger, "Successfully connected to broker");
    return client_socket;
}
