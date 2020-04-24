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
                if(argc < 4){
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


/* Estas funciones se encargan de ver que tipo de mensaje es y tratarlo
 * @params el numero de argumentos con el que invocaron al main
 * @params un vector de char* que tiene todos los argumentos que le pasaron al main
 */
void broker_distribuidor(int argc, char* argv[]){
    char* ip = config_get_string_value(archConfig, "IP_BROKER");
    char* puerto = config_get_string_value(archConfig, "PUERTO_BROKER");
    t_paquete* paquete;
    void* mensaje_serializado;

    switch (str2Msj(argv[2])){
        case NEW_POKEMON:

            if(argc < 7){
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

            if(argc < 6){//TODO Cambiar esto por 7 cuando este solucionado el tema del id
                msj_error();
                break;
            }
            paquete = create_package(APPEARED_POK);
            t_appeared_pokemon* appeared_pokemon = create_appeared_pokemon( argv[3],atoi(argv[4]),atoi(argv[5]) ); //TODO ver tema id
            mensaje_serializado = appeared_pokemon_a_void(appeared_pokemon);
            add_to_package( paquete, mensaje_serializado, size_t_appeared_pokemon(appeared_pokemon));

            mensaje_proceso(BROKER, paquete);

            free(appeared_pokemon);
            free(mensaje_serializado);
            break;

        case CATCH_POKEMON:
            if(argc < 6){
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
            if(argc < 5) {
                msj_error();
                break;
            }
            paquete = create_package(CAUGHT_POK);
            t_caught_pokemon* caught_pokemon = create_caught_pokemon( okFailToInt(argv[4]) );// TODO ver tema del id
            mensaje_serializado = caught_pokemon_a_void(caught_pokemon);
            add_to_package( paquete, mensaje_serializado, size_t_caught_pokemon(caught_pokemon));

            mensaje_proceso(BROKER, paquete);
            free(caught_pokemon);
            free(mensaje_serializado);
            break;

        case GET_POKEMON:
            if(argc < 4){
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

void team_distribuidor(int argc, char* argv[]){
    char* ip = config_get_string_value(archConfig, "IP_TEAM");
    char* puerto = config_get_string_value(archConfig, "PUERTO_TEAM");
    t_paquete* paquete;
    void* mensaje_serializado;

    switch (str2Msj(argv[2])){
        case APPEARED_POKEMON:
            if(argc < 6){
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

void gamecard_distribuidor(int argc, char* argv[]){
    char* ip = config_get_string_value(archConfig, "IP_GAMECARD");
    char* puerto = config_get_string_value(archConfig, "PUERTO_GAMECARD");
    t_paquete* paquete;
    void* mensaje_serializado;

    switch (str2Msj(argv[2])){
        case NEW_POKEMON:
            if(argc < 7){
                msj_error();
                break;
            }
            paquete = create_package(NEW_POK);
            t_new_pokemon* new_pokemon = create_new_pokemon( argv[3],atoi(argv[4]),atoi(argv[5]),atoi(argv[6]) );
            mensaje_serializado = new_pokemon_a_void(new_pokemon);
            add_to_package( paquete, mensaje_serializado, size_t_new_pokemon(new_pokemon));

            mensaje_proceso(GAMECARD, paquete);
            free(new_pokemon);
            free(mensaje_serializado);
            break;

        case CATCH_POKEMON:
            if(argc < 6){
                msj_error();
                break;
            }

            paquete = create_package(CATCH_POK);
            t_catch_pokemon* catch_pokemon = create_catch_pokemon( argv[3],atoi(argv[4]),atoi(argv[5]) );
            mensaje_serializado = catch_pokemon_a_void(catch_pokemon);
            add_to_package( paquete, mensaje_serializado, size_t_catch_pokemon(catch_pokemon));

            mensaje_proceso(GAMECARD, paquete);
            free(catch_pokemon);
            free(mensaje_serializado);
            break;

        case GET_POKEMON:
            if(argc < 4){
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

void suscribir(char* cola_mensaje,char* tiempo){
    char* ip = config_get_string_value(archConfig, "IP_BROKER");
    char* puerto = config_get_string_value(archConfig, "PUERTO_BROKER");

    printf("Te queres suscribir a %s durante %s",cola_mensaje, tiempo);

    free(ip);
    free(puerto);
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


void mensaje_proceso(int proceso, t_paquete* paquete){
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

    resultado < 0 ? printf("error") : printf("exito");

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
