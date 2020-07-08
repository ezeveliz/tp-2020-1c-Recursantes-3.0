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

//-----------------------Funciones ante la llegada de mensajes ----------------------------------------//

void mensaje_new_pokemon(t_new_pokemon* pokemon, uint32_t id){
    char* path_file = obtener_path_file();

    char* path_archvio = string_new();
    string_append(&path_archvio, path_file);
    string_append(&path_archvio, "/");
    string_append(&path_archvio, pokemon->nombre_pokemon);

    //Verificar si existe pokemon sino crearlo
    if(!find_tall_grass(pokemon->nombre_pokemon)){// --> paso 1
        create_tall_grass(path_archvio);
    }
    //Verificar si el archivo se puede abrir sino intentar en x tiempo
    t_file* archivo = open_tall_grass(path_archvio);
    //Hago una recursividad llamando a la funcion hasta que se pueda conectar
    if( archivo == NULL ){ // --> paso 4

        free(path_file);
        free(path_archvio);
        close_tall_grass(archivo);
        sleep(configuracion.tiempo_reoperacion);
        mensaje_new_pokemon(pokemon,id);

    }else{
        //Verificar si existe la entrada en el archivo y agregar uno a la cantidad sino agregarlo al final
        t_pos_pokemon* pos_pok = buscar_coordenadas(pokemon->pos_x, pokemon->pos_y, archivo);

        char* registro_agregar = string_new();
        string_append(&registro_agregar,string_itoa(pokemon->pos_x));
        string_append(&registro_agregar,"-");
        string_append(&registro_agregar,string_itoa(pokemon->pos_y));
        string_append(&registro_agregar,"=");

        //Verifico si esta el registro con esa posicion
        if( pos_pok == NULL ){

            //Le agrego la cantidad al string y lo escribo en el archivo
            string_append(&registro_agregar,string_itoa(pokemon->cantidad));
            string_append(&registro_agregar,"\n");
            write_tall_grass(archivo, registro_agregar, archivo->metadata->size - 1,string_length(registro_agregar));
        }else{

            //Elimino la entrada vieja
            delet_tall_grass(archivo,pos_pok->pos_archivo, pos_pok->tam);

            //Le agrego la cantidad que quiero al registro a agregar
            string_append(&registro_agregar,string_itoa(pos_pok->cant + pokemon->cantidad));
            string_append(&registro_agregar,"\n");

            //Escribo el registro
            write_tall_grass(archivo,registro_agregar,(archivo->metadata->size) - 1,string_length(registro_agregar));

        }
        //Enviar mensaje APPEARED_POKEMON
        t_paquete * paquete = create_package(APPEARED_POK);


        t_appeared_pokemon* appeared_pokemon = create_appeared_pokemon(pokemon->nombre_pokemon,pokemon->pos_x,pokemon->pos_y);
        void* mensaje_serializado = appeared_pokemon_a_void(appeared_pokemon);


        add_to_package(paquete, (void *) &id, sizeof(uint32_t));
        add_to_package(paquete, mensaje_serializado, sizeof_appeared_pokemon(appeared_pokemon));

        //Si no se puede conectar informar por log
        envio_mensaje(paquete,configuracion.ip_broker,configuracion.puerto_broker);

        //Libero
        free(mensaje_serializado);
        free(appeared_pokemon->nombre_pokemon);
        free(appeared_pokemon);
        free(path_archvio);
        free(path_file);

        //Cerrar el archivo
        close_tall_grass(archivo);
    }

}


int envio_mensaje(t_paquete *paquete, char *ip, uint32_t puerto) {
    int server_socket = create_socket();

    if (server_socket == -1) {
        printf("Error al crear el socket\n");
        return -1;
    }

    if (connect_socket(server_socket, ip, puerto) == -1) {
        log_error(logger, "Conexion fallida Broker ip:%s, puerto:%d", ip, puerto);
        close_socket(server_socket);
        return -1;
    }

    if (send_package(paquete, server_socket) == -1) {
        log_error(logger, "Error al enviar paquete al Broker ip:%s, puerto:%d", ip, puerto);
        close_socket(server_socket);
        return -1;
    }

    // Trato de recibir el encabezado de la respuesta
    MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
    if(receive_header(server_socket, buffer_header) <= 0) {
        log_info(logger, "No recibi un header");
        return false;
    }

    //Esto solo si es el proceso broker creo
    // Recibo la confirmacion
    t_list* rta_list = receive_package(server_socket, buffer_header);
    int rta = *(int*) list_get(rta_list, 0);
    printf("Llego mensaje de confirmacion exitoso: %d\n", rta);//TODO:Revisar si estoy hay que logearlo

    // Limpieza
    free(buffer_header);

    list_destroy_and_destroy_elements(rta_list, free);

    /////////////////////////////////////////////////////////
    //log_info(logger, "Se envio un mensaje a la ip: %s, puerto: %d\n", ip, puerto);

    close_socket(server_socket);
    return 1;

}


void mensaje_catch_pokemon(t_catch_pokemon* pokemon){
    char* path_file = obtener_path_file();

    char* path_archvio = string_new();
    string_append(&path_archvio, path_file);
    string_append(&path_archvio, "/");
    string_append(&path_archvio, pokemon->nombre_pokemon);
    //Verificar si existe el pokemon, sino informar el error
    if(!find_tall_grass(pokemon->nombre_pokemon)){

    }

    //Verificar si el archivo se puede abrir sino intentar en x tiempo
    t_file* archivo = open_tall_grass(path_archvio);
    if( archivo == NULL ){
        free(path_file);
        free(path_archvio);
        sleep(configuracion.tiempo_reoperacion);
        mensaje_new_pokemon(pokemon);
    }

    //En caso que la cantidad del Pokémon sea “1”, se debe eliminar la línea. En caso contrario se debe decrementar la cantidad en uno.
    //Cerrar archivo
    close_tall_grass(archivo);
    //Mandar un CAUGHT_POKEMON
}

void mensaje_get_pokemon(t_get_pokemon* pokemon){
    char* path_file = obtener_path_file();

    char* path_archvio = string_new();
    string_append(&path_archvio, path_file);
    string_append(&path_archvio, "/");
    string_append(&path_archvio, pokemon->nombre_pokemon);

    //Verificar si existe el pokemon, sino informar el error
    if(!find_tall_grass(pokemon->nombre_pokemon)){

    }

    //Verificar si el archivo se puede abrir sino intentar en x tiempo
    t_file* archivo = open_tall_grass(path_archvio);
    if( archivo == NULL ){
        free(path_file);
        free(path_archvio);
        sleep(configuracion.tiempo_reoperacion);
        mensaje_new_pokemon(pokemon);
    }
    //Obtener todas las posiciones y cantidades requeridas


    //Cerrar archivo
    close_tall_grass(archivo);
    // Enviar menaje LOCALIZED_POKEMON al broker
}

t_pos_pokemon* obtener_sig_coordenada(t_file* archivo){
    t_pos_pokemon* pos = malloc(sizeof (t_pos_pokemon));

    //String para acumular el registro(12-3=212\n)
    char* string_con_pos = string_new();

    // lee de a un char del archivo, porque no se el largo exacto
    char* char_leido = read_tall_grass(archivo,1,archivo->pos);
    archivo->pos++;

    //Comparo que no sea el final del archivo ni un salto de linea
    while(strcmp(char_leido,"\n") != 0 && char_leido[0] != EOF){
        //acumulo de a un char
        string_append(&string_con_pos, char_leido);

        //Leo el siguiente char
        char* char_leido = read_tall_grass(archivo,1,archivo->pos);
        archivo->pos++;
    }

    //Controlo que no haya llegado al final de archivo
    if(char_leido[0] != EOF){

        //Tiene las posiciones con un - entre ellas en pos 0
        //Tiene la cantidad en la pos 1
        char** string_pos_cantidad = string_split(string_con_pos,"=");

        //Tiene las posiciones para converitr en un int
        char** string_pos = string_split(string_con_pos[0],"-");

        //Seteo la pos en un struct
        pos->cant = atoi(string_pos_cantidad[1]);
        pos->x = atoi(string_pos[0]);
        pos->y = atoi(string_pos[1]);
        pos->tam = string_length(string_con_pos);
        pos->pos_archivo = (archivo->pos - atoi(string_con_pos));

        //Libero
        free(char_leido);
        free(string_con_pos);
        free(string_pos_cantidad[0]);
        free(string_pos_cantidad[1]);
        free(string_pos_cantidad);
        free(string_pos[0]);
        free(string_pos[1]);
        free(string_pos);
        return pos;
    }


    //Si no se encontro libero y devuelvo null
    free(char_leido);
    free(string_con_pos);
    return NULL;
}

//busca unas coordenadas dentro del archivo
t_pos_pokemon* buscar_coordenadas(uint32_t x, uint32_t  y, t_file* archivo){

    //Obtiene la primer registro del archivo
    t_pos_pokemon* pos_iteretor = obtener_sig_coordenada(archivo);

    //Recorro el archivo hasta encontrar la posicion indicada y sino es null
    while(pos_iteretor != NULL){

        if( x == pos_iteretor->x && y == pos_iteretor->y ){
            break;
        }

        //Obtengo el siguiente registro
        pos_iteretor = obtener_sig_coordenada(archivo);
    }

    //Retorno un archivo con el registro o null
    return pos_iteretor;
}
