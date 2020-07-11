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
    montar("..");
//    mensaje_new_pokemon(create_new_pokemon("Charmander",1,1,100), 12);
//    mensaje_new_pokemon(create_new_pokemon("Charmander",1,1,100), 12);
//    mensaje_new_pokemon(create_new_pokemon("Charmander",2,1,100), 12);
//    mensaje_new_pokemon(create_new_pokemon("Charmander",2,1,100), 12);
//    mensaje_new_pokemon(create_new_pokemon("Charmander",2,1,1000000), 12);
    //
//    pthread_t server_thread;
//
//    //Leo la configuracion, si da error cierro el negocio
//    if(leer_opciones_configuracion() == -1){
//        printf("Error al leer configuracion\n");
//        return -1;
//    }
//
//    // Inicializo el log, si no pude salgo del proceso
//    logger = log_create("gameboy_log", "Gameboy", 1, LOG_LEVEL_INFO);//LOG_LEVEL_ERROR
//    if (logger == NULL) {
//        printf("No se pudo inicializar el log en la ruta especificada, saliendo.");
//        return -1;
//    }
//
//    //Creo el servidor para que el GameBoy me mande mensajes
//    pthread_create(&server_thread, NULL, server_function_gamecard, NULL);
//
//    //Creo 3 hilos para suscribirme a las colas globales
//    subscribe_to_queues();
//
//    //Joineo el hilo main con el del servidor para el GameBoy
//    pthread_join(server_thread, NULL);
//
    int cantidad_bloques = obtener_cantidad_bloques();
    char *path_bitmap = obtener_path_bitmap();
    t_list *bloques_libres = list_create();

    FILE *archivo_bitmap = fopen(path_bitmap, "r+");

    t_bitarray *bitmap = bitarray_create(obtener_bitmap(archivo_bitmap, cantidad_bloques),
                                         tamanio_bitmap(cantidad_bloques));


    doom_bitmap(bitmap);
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
    configuracion.tiempo_retardo_operacion = config_get_int_value(config_file,"TIEMPO_RETARDO_OPERACION");
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

            pthread_t funcion_thread;
            estructura_para_hilo* parametros_hilo = malloc(sizeof(estructura_para_hilo));
            parametros_hilo->id = idMensaje;
            parametros_hilo->estructura_pokemon = list_get(rta_list,2);

            // Switch case que seleccione que hacer con la respuesta segun el tipo de cola
            switch (cola) {

                //Tengo que crear un hilo para tratar cada caso
                case (NEW_POK):;
                    pthread_create(&funcion_thread, NULL, mensaje_new_pokemon, parametros_hilo);
                    pthread_detach(funcion_thread);
                    break;

                case (CATCH_POK):;
                    pthread_create(&funcion_thread, NULL, mensaje_catch_pokemon, parametros_hilo);
                    pthread_detach(funcion_thread);
                    break;

                case (GET_POK):;
                    pthread_create(&funcion_thread, NULL, mensaje_get_pokemon, parametros_hilo);
                    pthread_detach(funcion_thread);
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

void* mensaje_new_pokemon(void* parametros ){

    estructura_para_hilo* datos_param = (estructura_para_hilo*) parametros;

    t_new_pokemon* pokemon = void_a_new_pokemon(datos_param->estructura_pokemon) ;
    uint32_t id = datos_param->id;




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
        sleep(configuracion.tiempo_reoperacion);
        mensaje_new_pokemon(parametros);

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

            //Verifico si el archivo esta vacio por tema de posicion a escribir
            if(archivo->metadata->size > 0){
                write_tall_grass(archivo, registro_agregar,string_length(registro_agregar), archivo->metadata->size - 1);
            }else{
                write_tall_grass(archivo, registro_agregar,string_length(registro_agregar), archivo->metadata->size);
            }


        }else{

            //Elimino la entrada vieja
            delet_tall_grass(archivo, pos_pok->pos_archivo, pos_pok->tam +1); // Mas uno por el salto de linea

            //Le agrego la cantidad que quiero al registro a agregar
            string_append(&registro_agregar,string_itoa(pos_pok->cant + pokemon->cantidad));
            string_append(&registro_agregar,"\n");

            //Escribo el registro
            write_tall_grass(archivo,registro_agregar, string_length(registro_agregar),(archivo->metadata->size) - 1);

        }
//        //Enviar mensaje APPEARED_POKEMON
//        t_paquete * paquete = create_package(APPEARED_POK);
//
//
//        t_appeared_pokemon* appeared_pokemon = create_appeared_pokemon(pokemon->nombre_pokemon,pokemon->pos_x,pokemon->pos_y);
//        void* mensaje_serializado = appeared_pokemon_a_void(appeared_pokemon);
//
//
//        add_to_package(paquete, (void *) &id, sizeof(uint32_t));
//        add_to_package(paquete, mensaje_serializado, sizeof_appeared_pokemon(appeared_pokemon));
//
//        //Si no se puede conectar informar por log
//        envio_mensaje(paquete,configuracion.ip_broker,configuracion.puerto_broker);

        //Libero
//        free(mensaje_serializado);
//        free(appeared_pokemon->nombre_pokemon);
//        free(appeared_pokemon);
        free(path_archvio);
        free(path_file);

        //Cerrar el archivo
        close_tall_grass(archivo);

        //Libero el parametro poque ya no lo uso
        free(datos_param);
    }

    //Libero el pokemon antes de terminar el hilo
    free(pokemon->nombre_pokemon);
    free(pokemon);

    return null;

}


void* mensaje_catch_pokemon(void* parametros){

    estructura_para_hilo* datos_param = (estructura_para_hilo*) parametros;

    t_catch_pokemon* pokemon = void_a_catch_pokemon(datos_param->estructura_pokemon) ;
    uint32_t id = datos_param->id;



    char* path_file = obtener_path_file();

    char* path_archvio = string_new();
    string_append(&path_archvio, path_file);
    string_append(&path_archvio, "/");
    string_append(&path_archvio, pokemon->nombre_pokemon);

    //Verificar si existe el pokemon, sino informar el error
    if(!find_tall_grass(pokemon->nombre_pokemon)){
        //Informar del error
        printf("No existe directorio con ese nombre\n");
    }

    //Verificar si el archivo se puede abrir sino intentar en x tiempo
    t_file* archivo = open_tall_grass(path_archvio);
    if( archivo == NULL ){
        free(path_file);
        free(path_archvio);
        sleep(configuracion.tiempo_reoperacion);
        catch_pokemon_a_void(parametros);

    }else{
        //Busco la posicion en el archivo
        t_pos_pokemon* pos_pok = buscar_coordenadas(pokemon->pos_x, pokemon->pos_y, archivo);
        t_paquete * paquete = create_package(CAUGHT_POK);
        t_caught_pokemon* caught_pokemon;

        //verifico si no existe la posicion
        if(pos_pok == NULL){

            //Fijar que hago con el error
            printf("Error no existen esas coordenadas\n");
            //Genero el mensaje de error para mandarle al broker
            caught_pokemon = create_caught_pokemon(0);
        }else{

            //Creo un string con el registro en esa posicion
            char* registro_agregar = string_new();
            string_append(&registro_agregar,string_itoa(pokemon->pos_x));
            string_append(&registro_agregar,"-");
            string_append(&registro_agregar,string_itoa(pokemon->pos_y));
            string_append(&registro_agregar,"=");

            //Elimino la entrada vieja
            delet_tall_grass(archivo, pos_pok->pos_archivo, pos_pok->tam +1); // Mas uno por el salto de linea

            //Verifico que la posicion no quede sin entrada vacia
            if((pos_pok->cant -1) > 0){

                //Le agrego la cantidad que quiero al registro a agregar
                string_append(&registro_agregar,string_itoa(pos_pok->cant - 1));
                string_append(&registro_agregar,"\n");

                //Escribo el registro
                write_tall_grass(archivo,registro_agregar, string_length(registro_agregar),(archivo->metadata->size) - 1);
            }

            //Espero como dice enunciado paso 5
            sleep(configuracion.tiempo_retardo_operacion);

            //Cierro el archivo
            close_tall_grass(archivo);

            //Genero el mensaje de exito
            caught_pokemon = create_caught_pokemon(1);

            free(registro_agregar);
        }

//        //Le respondo al broker
//        void* mensaje_serializado = caught_pokemon_a_void(caught_pokemon);
//
//        //Agrego los datos al paquete
//        add_to_package(paquete, (void *) &id, sizeof(uint32_t));
//        add_to_package(paquete, mensaje_serializado, sizeof_caught_pokemon(caught_pokemon));
//
//        //Si no se puede conectar informar por log
//        envio_mensaje(paquete,configuracion.ip_broker,configuracion.puerto_broker);

        //Libero
        free(path_archvio);
        free(path_file);

        //Libero el parametro poque ya no lo uso
        free(datos_param);
    }

    //Libero los datos del pokemon
    free(pokemon->nombre_pokemon);
    free(pokemon);

    return null;

}

void* mensaje_get_pokemon(void* parametros){

    estructura_para_hilo* datos_param = (estructura_para_hilo*) parametros;

    t_get_pokemon* pokemon = void_a_get_pokemon(datos_param->estructura_pokemon) ;
    uint32_t id = datos_param->id;



    //Obtengo el path donde estan alojados los archivos
    char* path_file = obtener_path_file();

    //Genero el path del archvio
    char* path_archvio = string_new();
    string_append(&path_archvio, path_file);
    string_append(&path_archvio, "/");
    string_append(&path_archvio, pokemon->nombre_pokemon);

    t_localized_pokemon* localized_pokemon;

    //Verificar si existe el pokemon, sino devolver el mensaje sin posiciones ni cantidades
    if(!find_tall_grass(pokemon->nombre_pokemon)){
        //Generlo el mensaje de error
        localized_pokemon = create_localized_pokemon(pokemon->nombre_pokemon,0);
    }else{
        //Abro el archivo
        t_file* archivo = open_tall_grass(path_archvio);

        //Verificar si el archivo se puede abrir sino intentar en x tiempo
        if( archivo == NULL ){

            //Libero lo pedido
            free(path_file);
            free(path_archvio);

            //Espero x segundos antes de reintentar
            sleep(configuracion.tiempo_reoperacion);

            //Vuelvo a intentar llamando devuelta ala funcion
            mensaje_get_pokemon(parametros);

        } else{
            //Obtengo todas las coordenadas
            t_list* lista_pos = obtener_todas_coordenadas(archivo);

            //TODO hablar con fran que cambie esto
            //localized_pokemon = create_localized_pokemon(pokemon->nombre_pokemon, lista_pos->elements_count, );

            //Espero x segundos antes de cerrar el archivo
            sleep(configuracion.tiempo_retardo_operacion);
            close_tall_grass(archivo);

            //Libero el parametro poque ya no lo uso
            free(datos_param);
        }
    }

//    //Le respondo al broker
//    t_paquete* paquete = create_package(LOCALIZED_POK);
//    void* mensaje_serializado = localized_pokemon_a_void(localized_pokemon);
//
//    //Agrego los datos al paquete
//    add_to_package(paquete, (void *) &id, sizeof(uint32_t));
//    add_to_package(paquete, mensaje_serializado, sizeof_caught_pokemon(localized_pokemon));
//
//    //Si no se puede conectar informar por log
//    envio_mensaje(paquete,configuracion.ip_broker,configuracion.puerto_broker);

    //Libero los datos del pokemon
    free(pokemon->nombre_pokemon);
    free(pokemon);

    return null;
}

//-----------------------Funciones para el manejo de coordenadas --------------------------------------//

//Obtiene la sig coordenadas del poquemon segun el puntero
t_pos_pokemon* obtener_sig_coordenada(t_file* archivo){

    if(archivo->metadata->size != 0) {

        t_pos_pokemon *pos = malloc(sizeof(t_pos_pokemon));

        //String para acumular el registro(12-3=212\n)
        char *string_con_pos = string_new();

        // lee de a un char del archivo, porque no se el largo exacto
        char *char_leido = read_tall_grass(archivo, 1, archivo->pos);

        //Comparo que no sea el final del archivo ni un salto de linea
        while (strcmp(char_leido, "\n") != 0 && char_leido[0] != '\t' && archivo->metadata->size != 0) {
            //acumulo de a un char
            string_append(&string_con_pos, char_leido);

            //Leo el siguiente char
            char_leido = read_tall_grass(archivo, 1, archivo->pos);
        }

        //Controlo que no haya llegado al final de archivo
        if (char_leido[0] != '\t') {

            //Tiene las posiciones con un - entre ellas en pos 0
            //Tiene la cantidad en la pos 1
            char **string_pos_cantidad = string_split(string_con_pos, "=");

            //Tiene las posiciones para converitr en un int
            //string_pos_cantidad[0] x-y
            //string_pos_cantidad[1] cant ->
            char **string_pos = string_split(string_pos_cantidad[0], "-");

            //Seteo la pos en un struct
            pos->cant = atoi(string_pos_cantidad[1]);
            pos->x = atoi(string_pos[0]);
            pos->y = atoi(string_pos[1]);
            pos->tam = string_length(string_con_pos);
            pos->pos_archivo = (archivo->pos - string_length(string_con_pos));

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

    }else{

        return NULL;
    }
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

//Devuelve todas las posiciones de un pokemon en un t_list*
t_list* obtener_todas_coordenadas(t_file* archivo){

    //Seteo la pos del archivo en 0
    archivo->pos = 0;
    t_list* lista = list_create();

    //Voy traigo cada posicion
    t_pos_pokemon* pos_pok = obtener_sig_coordenada(archivo);
    while(pos_pok = NULL){
        //Los agrego a la lista
        list_add(lista,pos_pok);
        pos_pok = obtener_sig_coordenada(archivo);
    }

    //Envio la lista
    return lista;

}

//------------------------Funciones para el envio de mensajes ---------------------------------------//

int envio_mensaje(t_paquete *paquete, char *ip, uint32_t puerto) {
    int server_socket = create_socket();

    //Creo el socket
    if (server_socket == -1) {
        printf("Error al crear el socket\n");
        return -1;
    }

    //Conecto el socket al broker
    if (connect_socket(server_socket, ip, puerto) == -1) {
        log_error(logger, "Conexion fallida Broker ip:%s, puerto:%d", ip, puerto);
        close_socket(server_socket);
        return -1;
    }

    //Envio el mensaje
    if (send_package(paquete, server_socket) == -1) {
        log_error(logger, "Error al enviar paquete al Broker ip:%s, puerto:%d", ip, puerto);
        close_socket(server_socket);
        return -1;
    }

    // Trato de recibir el encabezado de la respuesta
    MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
    if(receive_header(server_socket, buffer_header) <= 0) {
        log_info(logger, "No recibi un header");
        return -1;
    }

    //Esto solo si es el proceso broker creo
    // Recibo la confirmacion
    t_list* rta_list = receive_package(server_socket, buffer_header);
    int rta = *(int*) list_get(rta_list, 0);

    // Limpieza
    free(buffer_header);

    list_destroy_and_destroy_elements(rta_list, free);

    close_socket(server_socket);
    return 1;

}