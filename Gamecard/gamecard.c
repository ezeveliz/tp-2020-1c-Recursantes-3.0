//
// Created by utnso on 07/04/20.
//

#include "gamecard.h"

t_gameboy_config configuracion;
t_config* config_file;
t_log *logger;


int main() {
    logger = log_create("gameboy_log", "Gameboy", 1, LOG_LEVEL_INFO);//LOG_LEVEL_ERROR
    leer_opciones_configuracion();
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
    configuracion.gamecard_id =config_get_int_value(config_file,"ID");
    return 1;
}

void liberar_opciones_configuracion(){
    free(configuracion.ip_broker);
    free(configuracion.punto_montaje);
}

int conectar_broker(){

    int client_socket;
    if((client_socket = create_socket()) == -1) {
        log_error(logger, "Error al crear el socket de cliente");
        return -1;
    }
    if(connect_socket(client_socket, configuracion.ip_broker, configuracion.puerto_broker) == -1){
        log_error(logger, "Error al conectarse al Broker");
        return -1;
    }
    return client_socket;
}

void desconectar_broker(int broker_socket) {
    close_socket(broker_socket);
}

void suscribir_colas() {
    pthread_t new_thread;
    pthread_t get_thread;
    pthread_t catch_thread;

    // Levanto 3 hilos y en cada uno realizo una conexion al broker para cada una de las colas
    MessageType* new = malloc(sizeof(MessageType));
    *new = SUB_NEW;
    pthread_create(&new_thread, NULL, &suscribir_cola_thread, (void*)new);
    pthread_detach(new_thread);

    MessageType* get = malloc(sizeof(MessageType));
    *get = SUB_GET;
    pthread_create(&get_thread, NULL, &suscribir_cola_thread, (void*)get);
    pthread_detach(get_thread);

    MessageType* catch = malloc(sizeof(MessageType));
    *catch = SUB_CATCH;
    pthread_create(&catch_thread, NULL, &suscribir_cola_thread, (void*)catch);
    pthread_detach(catch_thread);
}

int suscribir_cola(int broker, MessageType cola) {

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
    void element_destroyer(void* element){
        free(element);
    }
    list_destroy_and_destroy_elements(rta_list,free);

    return rta == 1;
}

void* suscribir_cola_thread(void* arg) {
    MessageType cola = *(MessageType*)arg;

    // Me intento conectar y suscribir, la funcion no retorna hasta que no lo logre
    int broker = conectarse_y_suscribir(cola);
    log_info(logger, "Subscribed to queue\n");

    //TODO: PROBAR ESTO
    // Me quedo en un loop infinito esperando a recibir cosas
    while (1) {

        MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
        if(receive_header(broker, buffer_header) > 0) {

            // Recibo la confirmacion
            t_list* rta_list = receive_package(broker, buffer_header);
            int rta = *(int*) list_get(rta_list, 0);

            // TODO: Mandar el ACK

            switch (buffer_header->type){
                case NEW_POK:
                    mensaje_new_pokemon();
                    break;
                case GET_POK:
                    mensaje_get_pokemon();
                    break;
                case CATCH_POK:
                    mensaje_catch_pokemon();
                    break;
                default:
                    break;
            }
            // Limpieza
            free(buffer_header);
            //TODO: eliminar la lista

            // Si surgio algun error durante el receive header, me reconecto y vuelvo a iterar
        } else {
            log_info(logger, "Connection to queue lost\n");
            broker = conectarse_y_suscribir(cola);
            log_info(logger, "Resubscribed to queue\n");
        }
    }
    // TODO: si eventualmente se sale del while, hacerle free al arg recibido por parametro
    return null;
}

int conectarse_y_suscribir(MessageType cola) {
    int broker;
    bool connected = false;

    //Mientras que no este conectado itero
    while (!connected) {

        // Me conecto al Broker
        broker = conectar_broker();

        // Si me pude conectar al Broker
        if (broker != -1) {

            // Me intento suscribir a la cola pasada por parametro
            connected = suscribir_cola(broker, cola);

            // Si no me pude conectar al Broker, me duermo y vuelvo a intentar en unos segundos
        } else {
            sleep(configuracion.tiempo_reconexion);
        }
    }

    return broker;
}


void mensaje_new_pokemon(){
    printf("New_pokemon");
}
void mensaje_get_pokemon(){
    printf("Get_pokemon");
}
void mensaje_catch_pokemon(){
    printf("Catch_pokemon");
}

