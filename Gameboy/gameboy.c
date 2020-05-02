//
// Created by utnso on 07/04/20.
//

#include "gameboy.h"

t_config * archConfig;
t_log* 	logger;

int main(int argc, char *argv[]){

    archConfig = config_create("../gameboy_config");
    logger =  log_create("gameboy_log", "Gameboy", 0, LOG_LEVEL_INFO);

    if(argc > 2){
        switch (str2Proces(argv[1])){
            case BROKER:
                broker_distribuidor(argc, argv);
                break;
            case TEAM:
                team_distribuidor(argc,argv);
                break;
            case GAMECARD:
                gamecard_distribuidor(argc,argv);
                break;
            case SUSCRIPTOR:
                if(argc < PARAMETROS_SUSCRIPCION){
                    msj_error();
                    break;
                }
                suscribir(argv[2],argv[3]);
                break;
            default: msj_error();
        }
    } else
        msj_error();

    //config_destroy(archConfig);
    //log_destroy(logger);
}

void msj_error(){
    printf("Erro en argumentos");
}

/*Busca en un key-value con los nombres del proceso para pasarlo a un valor numerico de enum
 * @param Un string con el nombre del proceso
 * @return el int que corresponde al proceso
 */
int str2Proces (const char *str)
{
    int j;
    for (j = 0;  j < sizeof(conversionProces) / sizeof(conversionProces[0]);  ++j)
        if (!strcmp(str, conversionProces[j].str))
            return conversionProces[j].val;
    return -1;
}

/*Busca en un key-value con los nombres de los mensajes para pasarlo a un valor numerico de enum
 * @param Un string con el nombre del mensaje
 * @return el int que corresponde al mensaje
 */
int str2Msj (const char *str)
{
    int j;
    for (j = 0;  j < sizeof (conversionMsj) / sizeof (conversionMsj[0]);  ++j)
        if (!strcmp (str, conversionMsj[j].str))
            return conversionMsj[j].val;
    return -1;
}

int str2Queue (const char*str)
{
    int j;
    for (j = 0;  j < sizeof (conversionQueue) / sizeof (conversionQueue[0]);  ++j)
        if (!strcmp (str, conversionQueue[j].str))
            return conversionQueue[j].val;
    return -1;
}


/* Estas funciones se encargan de ver que tipo de mensaje es y tratarlo
 * @params el numero de argumentos con el que invocaron al main
 * @params un vector de char* que tiene todos los argumentos que le pasaron al main
 */
void broker_distribuidor(int argc, char* argv[])
{
    char* ip = config_get_string_value(archConfig, "IP_BROKER");
    char* puerto = config_get_string_value(archConfig, "PUERTO_BROKER");
    t_paquete* paquete;
    void* mensaje_serializado;
    int id;

    switch (str2Msj(argv[2])){
        case NEW_POKEMON:

            if(argc < PARAMETROS_BROKER_NEW){
                msj_error();
                break;
            }

            paquete = create_package(NEW_POK);
            t_new_pokemon* new_pokemon = create_new_pokemon( argv[3],atoi(argv[4]),atoi(argv[5]),atoi(argv[6]) );
            mensaje_serializado = new_pokemon_a_void(new_pokemon);
            add_to_package( paquete, mensaje_serializado , size_t_new_pokemon(new_pokemon));

            mensaje_proceso(BROKER, paquete);
            free(mensaje_serializado);
            free(new_pokemon);
            break;

        case APPEARED_POKEMON:

            if(argc < PARAMETROS_BROKER_APPEARED){
                msj_error();
                break;
            }
            paquete = create_package(APPEARED_POK);
            t_appeared_pokemon* appeared_pokemon = create_appeared_pokemon( argv[3],atoi(argv[4]),atoi(argv[5]) );
            mensaje_serializado = appeared_pokemon_a_void(appeared_pokemon);
            id = atoi(argv[6]);
            add_to_package( paquete, (void*) &id, sizeof(uint32_t));
            add_to_package( paquete, mensaje_serializado, size_t_appeared_pokemon(appeared_pokemon));

            mensaje_proceso(BROKER, paquete);

            free(appeared_pokemon);
            free(mensaje_serializado);
            break;

        case CATCH_POKEMON:
            if(argc < PARAMETROS_BROKER_CATCH){
                msj_error();
                break;
            }

            paquete = create_package(CATCH_POK);
            t_catch_pokemon* catch_pokemon = create_catch_pokemon( argv[3],atoi(argv[4]),atoi(argv[5]) );
            mensaje_serializado = catch_pokemon_a_void(catch_pokemon);
            add_to_package( paquete, mensaje_serializado, size_t_catch_pokemon(catch_pokemon));

            mensaje_proceso(BROKER, paquete);
            free(catch_pokemon);
            free(mensaje_serializado);
            break;

        case CAUGHT_POKEMON:
            if(argc < PARAMETROS_BROKER_CAUGHT) {
                msj_error();
                break;
            }
            paquete = create_package(CAUGHT_POK);
            t_caught_pokemon* caught_pokemon = create_caught_pokemon( okFailToInt(argv[4]) );
            mensaje_serializado = caught_pokemon_a_void(caught_pokemon);
            id = atoi(argv[3]);
            add_to_package( paquete, (void*) &id, sizeof(uint32_t));
            add_to_package( paquete, mensaje_serializado, size_t_caught_pokemon(caught_pokemon));

            mensaje_proceso(BROKER, paquete);
            free(caught_pokemon);
            free(mensaje_serializado);
            break;

        case GET_POKEMON:
            if(argc < PARAMETROS_BROKER_GET){
                msj_error();
                break;
            }
            paquete = create_package(GET_POK);
            t_get_pokemon* get_pokemon = create_get_pokemon( argv[3] );
            mensaje_serializado = get_pokemon_a_void(get_pokemon);
            add_to_package( paquete, mensaje_serializado, size_t_get_pokemon(get_pokemon));

            mensaje_proceso(BROKER, paquete);
            free(get_pokemon);
            free(mensaje_serializado);
            break;

        default: printf("Error ese mensaje no se puede mandar al broker");
    }

    free(ip);
    free_package(paquete);
}

void team_distribuidor(int argc, char* argv[])
{
    char* ip = config_get_string_value(archConfig, "IP_TEAM");
    char* puerto = config_get_string_value(archConfig, "PUERTO_TEAM");
    t_paquete* paquete;
    void* mensaje_serializado;

    switch (str2Msj(argv[2])){
        case APPEARED_POKEMON:
            if(argc < PARAMETROS_TEAM_APPEARED){
                msj_error();
                break;
            }

            paquete = create_package(APPEARED_POK);
            t_appeared_pokemon* appeared_pokemon = create_appeared_pokemon( argv[3],atoi(argv[4]),atoi(argv[5]));
            mensaje_serializado = appeared_pokemon_a_void(appeared_pokemon);
            add_to_package( paquete, mensaje_serializado, size_t_appeared_pokemon(appeared_pokemon));

            mensaje_proceso(TEAM, paquete);
            free(appeared_pokemon);
            free(mensaje_serializado);
            break;
        default: printf("Error ese mensaje no se puede mandar al broker");
    }
    free(ip);
    free(puerto);
}

void gamecard_distribuidor(int argc, char* argv[])
{
    char* ip = config_get_string_value(archConfig, "IP_GAMECARD");
    char* puerto = config_get_string_value(archConfig, "PUERTO_GAMECARD");
    t_paquete* paquete;
    void* mensaje_serializado;
    int id;
    switch (str2Msj(argv[2])){
        case NEW_POKEMON:
            if( argc < PARAMETROS_GAMECARD_NEW ){
                msj_error();
                break;
            }
            paquete = create_package(NEW_POK);
            t_new_pokemon* new_pokemon = create_new_pokemon( argv[3],atoi(argv[4]),atoi(argv[5]),atoi(argv[6]) );
            mensaje_serializado = new_pokemon_a_void(new_pokemon);
            id = atoi(argv[7]);
            add_to_package( paquete, (void*) &id, sizeof(uint32_t));
            add_to_package( paquete, mensaje_serializado, size_t_new_pokemon(new_pokemon));


            mensaje_proceso(GAMECARD, paquete);
            free(new_pokemon);
            free(mensaje_serializado);
            break;

        case CATCH_POKEMON:
            if( argc < PARAMETROS_GAMECARD_CATCH ){
                msj_error();
                break;
            }

            paquete = create_package(CATCH_POK);
            t_catch_pokemon* catch_pokemon = create_catch_pokemon( argv[3],atoi(argv[4]),atoi(argv[5]) );
            mensaje_serializado = catch_pokemon_a_void(catch_pokemon);
            id = atoi(argv[6]);
            add_to_package( paquete, (void*) &id, sizeof(uint32_t));
            add_to_package( paquete, mensaje_serializado, size_t_catch_pokemon(catch_pokemon));

            mensaje_proceso(GAMECARD, paquete);
            free(catch_pokemon);
            free(mensaje_serializado);
            break;

        case GET_POKEMON:
            if( argc < PARAMETROS_GAMECARD_GET ){
                msj_error();
                break;
            }
            paquete = create_package(GET_POK);
            t_get_pokemon* get_pokemon = create_get_pokemon( argv[3]);
            mensaje_serializado = get_pokemon_a_void(get_pokemon);
            add_to_package( paquete, mensaje_serializado, size_t_get_pokemon(get_pokemon));

            mensaje_proceso(GAMECARD, paquete);
            free(get_pokemon);
            free(mensaje_serializado);
            break;

        default: printf("Error ese mensaje no se puede mandar al broker");
    }
    free(ip);
    free(puerto);
}


/* Esta funcion es para suscribirte a una cola de mensajes del broker
 * @params el nombre de la cola a la que te queres suscribir
 * @params el tiempo por el que te queres suscribir
 */

void suscribir(char* cola_mensaje, char* tiempo)
{
    //Genera el socket para enviar al broker
    int broker = create_socket();

    char* ip_broker = config_get_string_value(archConfig, "IP_BROKER");
    int puerto_broker = config_get_string_value(archConfig, "PUERTO_BROKER");

    if (broker == -1) {
        printf("Error al crear el socket\n");
        return;
    }

    if (connect_socket(broker, ip_broker, puerto_broker) == -1) {
        log_error(logger, "Conexion fallida ip:%s, puerto:%d", ip_broker, puerto_broker);
        close_socket(broker);
        return;
    }

    log_info(logger,"Se logro conexion con ip: %s, puerto: %d\n", ip_broker, puerto_broker);

        //Se suscribe a la cola
    if(subscribe_to_queue(broker,str2Msj(cola_mensaje)) != -1){
        close_socket(broker);
        log_info(logger,"Te suscribiste a la cola: %s", cola_mensaje);

        //Se configura un timer
        timer(atoi(tiempo));

        //crear servidor
        serverFunction();
    }else{
        close_socket(broker);
        printf("Error al intentar Suscribir");
    }
}

/*
 * Funciones para manejar el tiempo de ejecucion
 * de suscribir
 */

/*
 * Inicia el Timer para parar la ejecucion y
 * setea todos los parametros necesarios
 * @param tiempo de ejecucion
 */

void timer(int tiempo)
{
    struct sigaction sa;
    memset (&sa, 0, sizeof (sa));
    sa.sa_handler = &timer_handler;
    sigaction (SIGALRM, &sa, 0);

    struct itimerval timer;
    timer.it_value.tv_sec = tiempo;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;


    setitimer (ITIMER_REAL, &timer, 0);
}

/*
 * Sirve para atrapar la senial de alarma
 * @params la señal
 */
void timer_handler (int signum)
{
    printf ("\nTermino el tiempo!\n");
    exit(-1);
}

/*
 * Esta funcion fue robada al señor Eze
 * Esta funcion esta escrita en spanglish
 */

int subscribe_to_queue(int broker, MessageType cola)
{
    // Creo un paquete para la suscripcion a una cola, adjunto la ip y el puerto de mi server
    t_paquete* paquete = create_package(cola);
    char* mi_ip = config_get_string_value(archConfig, "IP_GAMEBOY");
    int mi_puerto = config_get_int_value(archConfig, "PUERTO_GAMEBOY");
    add_to_package(paquete, (void*) mi_ip, strlen(mi_puerto) + 1);//Mi ip
    add_to_package(paquete, (void*) &mi_puerto, sizeof(int));//Mi puerto

    //Limpieza
    free(mi_ip);

    // Envio el paquete, si no se puede enviar retorno false
    if(send_package(paquete, broker)  == -1){
        return false;
    }

    // Limpieza
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
    list_destroy(rta_list);

    return rta == 1;
}

void serverFunction(){

    int socket;
    int port = config_get_int_value(archConfig, "PUERTO_GAMEBOY");

    if((socket = create_socket()) == -1) {
        log_error(logger,"Error al crear el socket");
        return;
    }
    if((bind_socket(socket, port)) == -1) {
        log_error(logger,"Error al bindear el socket");
        return;
    }


    void new(int fd, char * ip, int port){}

    void lost(int fd, char * ip, int port){}

    void incoming(int fd, char * ip, int port, MessageHeader * headerStruct){

        t_list *cosas = receive_package(fd, headerStruct);
        void* mensaje = list_get(cosas, 0);

        //Segun el mensaje que recibiste lo logea
        switch (headerStruct->type){
            case NEW_POK: {
                t_new_pokemon *newPokemon = void_a_new_pokemon(mensaje);
                log_info(logger, "NEW_POKEMON Nombre Pokemon: %s \nCantidad: %d \nPosicion: (%d,%d)\n",
                         newPokemon->nombre_pokemon, newPokemon->cantidad, newPokemon->pos_x, newPokemon->pos_y);
                free(newPokemon->nombre_pokemon);
                free(newPokemon);
                break;
            }
            case APPEARED_POK: {
                t_appeared_pokemon *appearedPokemon = void_a_appeared_pokemon(mensaje);
                log_info(logger, "APEPEARED_POKEMON Nombre Pokemon: %s \nPosicion: (%d,%d)\n",
                         appearedPokemon->nombre_pokemon, appearedPokemon->pos_x, appearedPokemon->pos_y);
                free(appearedPokemon->nombre_pokemon);
                free(appearedPokemon);
                break;
            }
            case CATCH_POK: {
                t_catch_pokemon *catchPokemon = void_a_catch_pokemon(mensaje);
                log_info(logger, "CATCH_POKEMON Nombre Pokemon: %s \nPosicion: (%d,%d)\n", catchPokemon->nombre_pokemon,
                         catchPokemon->pos_x, catchPokemon->pos_y);
                free(catchPokemon->nombre_pokemon);
                free(catchPokemon);
                break;
            }
            case CAUGHT_POK: {
                t_caught_pokemon *caughtPokemon = void_a_caught_pokemon(mensaje);
                log_info(logger, "CAUGHT_POKEMON Fue atrapado: %d\n", caughtPokemon->atrapado);
                free(caughtPokemon);
                break;
            }
            case GET_POK: {
                t_get_pokemon *getPokemon = void_a_get_pokemon(mensaje);
                log_info(logger, "GET_POKEMON Nombre; %s\n", getPokemon->nombre_pokemon);
                free(getPokemon->nombre_pokemon);
                free(getPokemon);
                break;
            }
            case LOCALIZED_POK: {
                t_localized_pokemon *localizedPokemon = void_a_localized_pokemon(mensaje);
                log_info(logger, "GET_POKEMON Nombre; %s Cantidad de coordenadas: %d\n",
                         localizedPokemon->nombre_pokemon, localizedPokemon->cantidad_coordenas);
                free(localizedPokemon->nombre_pokemon);
                free(localizedPokemon);
                break;
            }
            default:
                printf("Operacion desconocida. No quieras meter la pata\n");

                break;

        }

        //Limpieza
        list_destroy(cosas);
        free(mensaje);

    }

    //Inicio servidor
    start_server(socket, &new, &lost, &incoming);
}

/*
 * Funciones de envio de mensajes
 */

int envio_mensaje(t_paquete* paquete, char* ip, uint32_t puerto){
    int server_socket = create_socket();

    if (server_socket == -1) {
        printf("Error al crear el socket\n");
        return -1;
    }

    if (connect_socket(server_socket, ip, puerto) == -1) {
        log_error(logger, "Conexion fallida ip:%s, puerto:%d", ip, puerto);
        close_socket(server_socket);
        return -1;
    }
    log_info(logger,"Se logro conexion con ip: %s, puerto: %d\n", ip, puerto);

    if(send_package(paquete, server_socket) == -1){
        log_error(logger, "Error al enviar paquete ip:%s, puerto:%d", ip, puerto);
        close_socket(server_socket);
        return -1;
    }

    log_info(logger,"Se envio un mensaje a la ip: %s, puerto: %d\n", ip, puerto);
    close_socket(server_socket);
    return 1;

}


/*
 * sirve para mandar a la direccion del proceso correspondiente
 */
int mensaje_proceso(int proceso, t_paquete* paquete){
    int resultado = 0;

    switch (proceso){
        case BROKER:
            resultado = envio_mensaje(paquete, config_get_string_value(archConfig, "IP_BROKER"), config_get_int_value(archConfig, "PUERTO_BROKER"));
            break;
        case TEAM:
            resultado = envio_mensaje(paquete, config_get_string_value(archConfig, "IP_TEAM"), config_get_int_value(archConfig, "PUERTO_TEAM"));
            break;
        case GAMECARD:
            resultado = envio_mensaje(paquete, config_get_string_value(archConfig, "IP_GAMECARD"), config_get_int_value(archConfig, "PUERTO_GAMECARD"));
            break;
    }

    return resultado;
}


/*
 * Funciones para calcular tamanio de los pokemon
 * Todas las funciones estan armadas como sumas de los atributos de los mensajes
 */

int size_t_new_pokemon(t_new_pokemon*  new_pokemon){
    return sizeof(uint32_t) + new_pokemon->nombre_pokemon_length + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t);
}

int size_t_appeared_pokemon(t_appeared_pokemon*  appeared_pokemon){
    return sizeof(uint32_t) + appeared_pokemon->nombre_pokemon_length + sizeof(uint32_t)  + sizeof(uint32_t);
}

int size_t_catch_pokemon(t_catch_pokemon*  catch_pokemon){
    return sizeof(uint32_t) + catch_pokemon->nombre_pokemon_length + sizeof(uint32_t) + sizeof(uint32_t);
}

int size_t_caught_pokemon(t_caught_pokemon*  caught_pokemon){
    return sizeof(uint32_t);
}

int size_t_get_pokemon(t_get_pokemon*  get_pokemon){
    return sizeof(uint32_t) + get_pokemon->nombre_pokemon_length;
}

int okFailToInt(char* resultado){
    if(strcmp("OK",resultado)==0){
        return 1;
    }else{
        return (strcmp("FAIL",resultado)== 0) ? 0 : -1;
    }
}

