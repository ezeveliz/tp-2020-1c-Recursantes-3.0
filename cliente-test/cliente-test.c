//
// Created by utnso on 29/05/20.
//

/*
gcc cliente-test.c -o cliente-test -Wall -lcommons -lcommLib -lpthread; ./cliente-test
 */

#include "cliente-test.h"

int main(int argc, char **argv) {
    logger = log_create("cliente_test.log", "CLIENTE_TEST", 1, LOG_LEVEL_TRACE);
    log_info(logger,"Log started.");
    bool init_config = set_config();
    init_config ? log_info(logger,"Config setted up successfully. :)") : log_info(logger,"Error while setting config :|");
    broker_fd = connect_to_broker();
    printf("%d\n", broker_fd);

    if(subscribir_cola(SUB_NEW)){
        log_info(logger, "Se subscribio bien a NEW");
    }
    if(subscribir_cola(SUB_APPEARED)){
        log_info(logger, "Se subscribio bien a LOCALIZED");
    }

    // Mando un NEW_POK
    t_new_pokemon* new_pok = malloc(sizeof(t_new_pokemon));
    new_pok->nombre_pokemon = "Mamame las bolas";
    new_pok->nombre_pokemon_length = string_length("Mamame las bolas");
    new_pok->cantidad = 1;
    new_pok->pos_x = 2;
    new_pok->pos_y = 3;
    t_paquete* paquete = create_package(NEW_POK);
    size_t tam = sizeof(uint32_t)*4 + new_pok->nombre_pokemon_length;
    add_to_package(paquete, new_pokemon_a_void(new_pok), tam);
    send_package(paquete, broker_fd);
    log_info(logger, "Mando el mensaje de New Pokemon");

    // Trato de recibir el encabezado de la respuesta
    MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
    if(receive_header(broker_fd, buffer_header) <= 0) {
        log_info(logger, "No recibi un header");
        return false;
    }
    if(buffer_header->type == NEW_POK){
        log_info(logger, "Estoy por recibir el id del mensaje, todo piola");
    } else {
        exit(-1);
    }

    // Recibo el id mensaje
    t_list* rta_list = receive_package(broker_fd, buffer_header);
    int id_mensaje_new_pokemon = *(int*) list_get(rta_list, 0);
    log_info(logger, "Tengo el id de mensaje: %d", id_mensaje_new_pokemon);


    bool xd = true;
    while(xd){
        MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
        log_info(logger, "Llego aca");
        if(receive_header(broker_fd, buffer_header) > 0) {
            t_list *rta_list = receive_package(broker_fd, buffer_header);
            log_info(logger, "Recibi algo");

            switch (buffer_header->type) {
                case (NEW_POK):
                    log_info(logger, "Me llega un New");

                    // Mando el ACK
                    t_paquete* paquete = create_package(ACK);
                    send_package(paquete, broker_fd);

                    break;

                case (APPEARED_POK):
                    break;

                default:
                    log_warning(logger, "Operacion desconocida. No quieras meter la pata\n");
            }

            free(buffer_header);
        }
        else {
            log_info(logger, "El if me dio falso");
        }
    }

    char x;
    scanf("%c", &x);
    x == '1' ? (close(broker_fd) == 0 ? log_info(logger, "chau") : log_info(logger, "error closing connection")) : log_info(logger, "keep connected");
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
        exit(-1);
    }
    if(connect_socket(client_socket, config.broker_ip, config.broker_port) == -1){
        log_error(logger, "Error connecting to Broker :?");
        exit(-1);
    }
    log_info(logger, "Successfully connected to broker");
    return client_socket;
}

int subscribir_cola(MessageType cola){
    // Creo un paquete para la suscripcion a una cola
    t_paquete* paquete = create_package(cola);

    int* id = malloc(sizeof(int));
    *id = 69; // Numero cualquiera
    add_to_package(paquete, (void*) id, sizeof(int));

//    t_localized_pokemon* localized_pok = malloc(sizeof(t_localized_pokemon));
//    localized_pok->nombre_pokemon = "A";
//    localized_pok->nombre_pokemon_length = strlen(localized_pok->nombre_pokemon+1);
//    localized_pok->cantidad_coordenas = 1;
//    uint32_t* coor[2] = {1,1};
//    localized_pok->coordenadas-> = coor;
//    add_to_package(paquete, localized_pokemon_a_void(localized_pok), sizeof())

//    if(cola == SUB_APPEARED){
//        t_appeared_pokemon* app_pok = malloc(sizeof(t_appeared_pokemon));
//        app_pok->nombre_pokemon = "A";
//        app_pok->nombre_pokemon_length = strlen(app_pok->nombre_pokemon+1);
//        app_pok->pos_x = 0;
//        app_pok->pos_y = 1;
//        add_to_package(paquete, appeared_pokemon_a_void(app_pok), sizeof(uint32_t)*3 + app_pok->nombre_pokemon_length);
//    }

    // Envio el paquete, si no se puede enviar retorno false
    if(send_package(paquete, broker_fd)  == -1){
        return false;
    }

    // Limpieza
    free_package(paquete);

    // Trato de recibir el encabezado de la respuesta
    MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
    if(receive_header(broker_fd, buffer_header) <= 0) {
        log_info(logger, "No recibi un header");
        return false;
    }

    // Recibo la confirmacion
    t_list* rta_list = receive_package(broker_fd, buffer_header);
    int rta = *(int*) list_get(rta_list, 0);

    // Limpieza
    free(buffer_header);
    void element_destroyer(void* element){
        free(element);
    }
    list_destroy_and_destroy_elements(rta_list, element_destroyer);

    return rta == 1;
}
