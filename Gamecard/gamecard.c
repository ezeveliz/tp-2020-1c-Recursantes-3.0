//
// Created by utnso on 07/04/20.
//

#include "gamecard.h"

t_gamecard_config configuracion;
t_config* config_file;
t_log *logger;

// Hilos encargados de recibir los distintos mensajes del Broker
pthread_t appeared_thread;
pthread_t localized_thread;
pthread_t caught_thread;

int main() {
    pthread_t server_thread;

    //Leo la configuracion, si da error cierro el negocio
    if(leer_opciones_configuracion() == -1){
        printf("Error al leer configuracion\n");
        return -1;
    }

    // Inicializo el log, si no pude salgo del proceso
    logger = log_create("gameboy_log", "Gameboy", 1, LOG_LEVEL_INFO);//LOG_LEVEL_ERROR
    if (logger == NULL) {
        printf("No se pudo inicializar el log en la ruta especificada, saliendo.");
        return -1;
    }

    //Creo el servidor para que el GameBoy me mande mensajes
    pthread_create(&server_thread, NULL, server_function_gamecard, NULL);

    //Creo 3 hilos para suscribirme a las colas globales
    subscribe_to_queues();

    //Joineo el hilo main con el del servidor para el GameBoy
    pthread_join(server_thread, NULL);

    return 0;
}

int leer_opciones_configuracion() {

    config_file = config_create("../gamecard.config");
    if (!config_file) {
        return -1;
    }

    configuracion.ip_broker = config_get_string_value(config_file,"IP_BROKER");
    configuracion.puerto_broker = config_get_int_value(config_file,"PUERTO_BROKER");
    configuracion.punto_montaje = config_get_string_value(config_file,"PUNTO_MONTAJE_TALLGRASS");
    configuracion.tiempo_reconexion = config_get_int_value(config_file,"TIEMPO_DE_REINTENTO_CONEXION");
    configuracion.tiempo_reoperacion = config_get_int_value(config_file,"TIEMPO_DE_REINTENTO_OPERACION");
    configuracion.gamecard_id = config_get_int_value(config_file,"MAC");
    configuracion.puerto_gamecard = config_get_int_value(config_file,"PUERTO_GAMECARD");
    return 1;
}

void liberar_opciones_configuracion(){
    free(configuracion.ip_broker);
    free(configuracion.punto_montaje);
}

/*
 * Agreadecimiento especial al equipo team
 * Algunos dirian que la conexion fue plagio
 * yo digo que fue inspiracion creativa con
 * ctr+c ctr+v
 */

//----------------------------------------COLAS GLOBALES----------------------------------------//
//Crea un hilo para suscribirse a cada cola
void subscribe_to_queues() {

    // Levanto 3 hilos y en cada uno realizo una conexion al broker para cada una de las colas
    MessageType* appeared = malloc(sizeof(MessageType));
    *appeared = SUB_APPEARED;
    pthread_create(&appeared_thread, NULL, &subscribe_to_queue_thread, (void*)appeared);
    pthread_detach(appeared_thread);// Lo pone como un hilo separado para que libere los recursos despues

    MessageType* localized = malloc(sizeof(MessageType));
    *localized = SUB_LOCALIZED;
    pthread_create(&localized_thread, NULL, &subscribe_to_queue_thread, (void*)localized);
    pthread_detach(localized_thread);

    MessageType* caught = malloc(sizeof(MessageType));
    *caught = SUB_CAUGHT;
    pthread_create(&caught_thread, NULL, &subscribe_to_queue_thread, (void*)caught);
    pthread_detach(caught_thread);
}

//Es el hilo que generas para conectarte a una cola
void* subscribe_to_queue_thread(void* arg) {
    MessageType cola = *(MessageType *) arg;

    // Me intento conectar y suscribir, la funcion no retorna hasta que no lo logre
    int broker = connect_and_subscribe(cola);

    // Reservo cachito de memoria para confirmar los mensajes que le envio al Broker
    int *confirmacion = malloc(sizeof(int));
    *confirmacion = ACK;

    // Creo mensaje para loguear la perdida de conexion con el Broker
    char *conexionPerdida = string_new();
    string_append(&conexionPerdida, "La cola encargada de recibir los mensajes ");

    switch (cola) {
        case (NEW_POK):;
            string_append(&conexionPerdida, "Appeared ");
            break;
        case (CATCH_POK):;
            string_append(&conexionPerdida, "Localized ");
            break;
        case (GET_POK):;
            string_append(&conexionPerdida, "Caught ");
            break;
        default:;
            string_append(&conexionPerdida, "(mensaje no soportado) ");
            break;
    }
    string_append(&conexionPerdida,
                  "ha perdido la conexion con el Broker, a continuacion se intentara la resubscripcion.");

    // Creo mensaje para loguear la reconexion con el Broker
    char *conexionReestablecida = string_new();
    string_append(&conexionReestablecida, "La cola encargada de recibir los mensajes ");

    switch (cola) {
        case (NEW_POK):;
            string_append(&conexionReestablecida, "Appeared ");
            break;
        case (CATCH_POK):;
            string_append(&conexionReestablecida, "Localized ");
            break;
        case (GET_POK):;
            string_append(&conexionReestablecida, "Caught ");
            break;
        default:;
            string_append(&conexionReestablecida, "(mensaje no soportado) ");
            break;
    }
    string_append(&conexionReestablecida, "se ha podido reconectar con el Broker.");

    // Me quedo en un loop infinito esperando a recibir cosas
    bool a = true;// Uso esto solo para que desaparezca la sombra amarilla
    while (a) {

        MessageHeader *buffer_header = malloc(sizeof(MessageHeader));
        if (receive_header(broker, buffer_header) > 0) {


            // Recibo la respuesta del Broker
            t_list *rta_list = receive_package(broker, buffer_header);

            // La posicion 0 de la lista recibida es siempre el id correlacional correspondiente
            // El resto del mensajes usa las estructuras locas del Broker(detalladas en la commLib)

            // En la posicion 0 viene el id de mensaje correlativo
            int idMensaje = *(int *) list_get(rta_list, 0);

            int idCorrelativo = *(int *) list_get(rta_list, 1);


            // Switch case que seleccione que hacer con la respuesta segun el tipo de cola
            switch (cola) {

                case (NEW_POK):;
                    printf("NEW_POKEMON\n");
                    break;

                case (CATCH_POK):;
                    printf("CATCH_POKEMON\n");
                    break;

                case (GET_POK):;
                    printf("GET_POKEMON\n");
                    break;

            }

            // Creo paquete para confirmar recepcion de mesaje al Broker
            t_paquete *paquete = create_package(ACK);
            add_to_package(paquete, (void *) &(configuracion.gamecard_id), sizeof(int));
            add_to_package(paquete, (void *) &idMensaje, sizeof(int));

            // Envio confirmacion al Broker
            send_package(paquete, broker);

            // Si surgio algun error durante el receive header, me reconecto y vuelvo a iterar
        } else {
            log_info(logger, conexionPerdida);
            broker = connect_and_subscribe(cola);
            log_info(logger, conexionReestablecida);
        }

        // Limpieza
        free(buffer_header);
    }

    // Libero la memoria reservada con anterioridad
    free(confirmacion);
    free(conexionPerdida);
    free(conexionReestablecida);
    free(arg);

    return null;
}

//Por si se tiene que reconectar usas esta funcion
int connect_and_subscribe(MessageType cola) {
    int broker;
    bool connected = false;
    //Mientras que no este conectado itero
    while (!connected) {

        // Me conecto al Broker
        broker = connect_to_broker();

        // Si me pude conectar al Broker
        if (broker != -1) {

            // Me intento suscribir a la cola pasada por parametro
            connected = subscribe_to_queue(broker, cola);

            // Si no me pude conectar al Broker, me duermo y vuelvo a intentar en unos segundos
        } else {
            sleep(configuracion.tiempo_reconexion);
        }
    }

    return broker;
}

int connect_to_broker(){

    int client_socket;
    if((client_socket = create_socket()) == -1) {

        return -1;
    }
    if(connect_socket(client_socket, configuracion.ip_broker, configuracion.puerto_broker) == -1){

        return -1;
    }
    return client_socket;
}

void disconnect_from_broker(int broker_socket) {
    close_socket(broker_socket);
}

bool subscribe_to_queue(int broker, MessageType cola) {

    // Creo un paquete para la suscripcion a una cola
    t_paquete* paquete = create_package(cola);
    int* id = malloc(sizeof(int));
    *id = configuracion.gamecard_id;
    add_to_package(paquete, (void*) id, sizeof(int));

    // Envio el paquete, si no se puede enviar retorno false
    if(send_package(paquete, broker)  == -1){
        return false;
    }

    // Limpieza
    free(id);
    free_package(paquete);

    // Trato de recibir el encabezado de la respuesta
    MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
    if(receive_header(broker, buffer_header) <= 0) {
        return false;
    }

    // Recibo la confirmacion
    t_list* rta_list = receive_package(broker, buffer_header);
    int rta = *(int*) list_get(rta_list, 0);

    // Limpieza
    free(buffer_header);

    list_destroy_and_destroy_elements(rta_list, free);

    return rta == 1;
}

//----------------------------------------SERVIDOR----------------------------------------//


int initialize_server(){

    int server_socket;
    int port = configuracion.puerto_gamecard;

    if((server_socket = create_socket()) == -1) {
        log_error(logger, "Error creating server socket");
        return -1;
    }
    if((bind_socket(server_socket, port)) == -1) {
        log_error(logger, "Error binding server socket");
        return -1;
    }

    return server_socket;
}

void new(int server_socket, char * ip, int port){

}

void lost(int server_socket, char * ip, int port){

}

void incoming_gameboy(int server_socket, char* ip, int port, MessageHeader * headerStruct){

    t_list* paquete_recibido = receive_package(server_socket, headerStruct);

    // Si desaprobamos por esto es culpa de Emi
    int confirmacion = 1;

    // Creo paquete para responderle al GameBoy
    t_paquete *package = create_package(headerStruct->type);
    add_to_package(package, &confirmacion, sizeof(int));

    // Envio confirmacion al GameBoy
    send_package(package, server_socket);

    // Switch case que seleccione que hacer con la respuesta segun el tipo de cola
    switch(headerStruct -> type){

        case (NEW_POK):;
            printf("NEW_POKEMON\n");
            break;

        case (CATCH_POK):;
            printf("CATCH_POKEMON\n");
            break;

        case (GET_POK):;
            printf("GET_POKEMON\n");
            break;
    }

    // Confirmacion para responderle al Gameboy
    int* rta = malloc(sizeof(int));
    *rta = 1;

    t_paquete* paquete = create_package(ACK);
    add_to_package(paquete,  (void*)rta, sizeof(int));

    // Envio confirmacion al Gameboy
    send_package(paquete, server_socket);

    free(rta);
}

void* server_function_gamecard(void* arg) {

    int server_socket;

    // La creacion de nuestro socket servidor puede fallar, si falla duermo y vuelvo a intentar en n segundos
    while ((server_socket = initialize_server()) == -1) {

        sleep(configuracion.tiempo_reconexion);
    }

    start_server(server_socket, &new, &lost, &incoming_gameboy);

    return null;
}