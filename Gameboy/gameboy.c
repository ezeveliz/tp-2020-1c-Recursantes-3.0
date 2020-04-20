//
// Created by utnso on 07/04/20.
//

#include "gameboy.h"
#include <string.h>
t_config * archConfig;

int main(int argc, char *argv[]){

    archConfig = config_create("../gameboy_config");

    printf("%s\n",config_get_string_value(archConfig, "IP_TEAM"));
    if(argc > 2){
        switch (str2Proces(argv[1])){
            case 1:
                broker_distribuidor(argc, argv);
                break;
            case 2:
                team_distribuidor(argc,argv);
                break;
            case 3:
                gamecard_distribuidor(argc,argv);
                break;
            case 4:
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


/* Estas funciones se encargan de ver que tipo de mensaje es y como tratarlo
 * @params el numero de argumentos con el que invocaron al main
 * @params un vector de char* que tiene todos los argumentos que le pasaron al main
 */
void broker_distribuidor(int argc, char* argv[]){
    char* ip = config_get_string_value(archConfig, "IP_BROKER");
    char* puerto = config_get_string_value(archConfig, "PUERTO_BROKER");
    t_paquete* paquete;
    void* mensaje_serializado;

    switch (str2Msj(argv[2])){
        case 1:

            if(argc < 7){
                msj_error();
                break;
            }

            paquete = create_package(NEW_POKEMON);
            t_new_pokemon* new_pokemon = create_new_pokemon( argv[3],atoi(argv[4]),atoi(argv[5]),atoi(argv[6]) );
            mensaje_serializado = new_pokemon_a_void(new_pokemon);
            add_to_package( paquete, mensaje_serializado, sizeof(t_new_pokemon));

            t_new_pokemon* newPokemon = void_a_new_pokemon(mensaje_serializado);
            printf("Nombre Pokemon: %s \nCantidad: %d \nPosicion: (%d,%d)\n", newPokemon->nombre_pokemon, newPokemon->cantidad, newPokemon->pos_x,newPokemon->pos_y);
            mensaje_proceso(BROKER, paquete);

            printf("./gameboy BROKER NEW_POKEMON [POKEMON] [POSX] [POSY] [CANTIDAD]");
            break;
        case 2:

            if(argc < 6){//TODO Cambiar esto por 7 cuando este solucionado el tema del id
                msj_error();
                break;
            }
            paquete = create_package(APPEARED_POKEMON);
            t_appeared_pokemon* appeared_pokemon = create_appeared_pokemon( argv[3],atoi(argv[4]),atoi(argv[5]) ); //TODO ver tema id
            mensaje_serializado = appeared_pokemon_a_void(appeared_pokemon);
            add_to_package( paquete, mensaje_serializado, sizeof(t_appeared_pokemon));

            mensaje_proceso(BROKER, paquete);
            printf("./gameboy BROKER APPEARED_POKEMON [POKEMON] [POSX] [POSY] [ID_MENSAJE]");
            break;
        case 3:
            if(argc < 6){
                msj_error();
                break;
            }

            paquete = create_package(CATCH_POKEMON);
            t_catch_pokemon* catch_pokemon = create_catch_pokemon( argv[3],atoi(argv[4]),atoi(argv[5]) );
            mensaje_serializado = catch_pokemon_a_void(catch_pokemon);
            add_to_package( paquete, mensaje_serializado, sizeof(t_catch_pokemon));

            mensaje_proceso(BROKER, paquete);
            //mensaje_broker_catch_pokemon(argv[3],atoi(argv[4]),atoi(argv[5]));
            printf("./gameboy BROKER CATCH_POKEMON [POKEMON] [POSX] [POSY]");
            break;
        case 4:
            if(argc < 5) {
                msj_error();
                break;
            }
            paquete = create_package(CAUGHT_POKEMON);
            t_caught_pokemon* caught_pokemon = create_caught_pokemon( atoi(argv[4]) );// TODO ver tema del id
            mensaje_serializado = caught_pokemon_a_void(caught_pokemon);
            add_to_package( paquete, mensaje_serializado, sizeof(t_caught_pokemon));

            mensaje_proceso(BROKER, paquete);
            printf("./gameboy BROKER CAUGHT_POKEMON [ID_MENSAJE] [OK/FAIL]");
            break;
        case 5:
            if(argc < 4){
                msj_error();
                break;
            }
            paquete = create_package(GET_POKEMON);
            t_get_pokemon* get_pokemon = create_get_pokemon( argv[3] );
            mensaje_serializado = new_pokemon_a_void(get_pokemon);
            add_to_package( paquete, mensaje_serializado, sizeof(t_get_pokemon));

            mensaje_proceso(BROKER, paquete);
            printf("./gameboy BROKER GET_POKEMON [POKEMON]");
            break;
        default: printf("Error ese mensaje no se puede mandar al broker");
    }
    free(ip);
    free(puerto);
}

void team_distribuidor(int argc, char* argv[]){
    char* ip = config_get_string_value(archConfig, "IP_TEAM");
    char* puerto = config_get_string_value(archConfig, "PUERTO_TEAM");
    switch (str2Msj(argv[2])){
        case 2:
            if(argc < 6){
                msj_error();
                break;
            }
            //Metes los parametros en un paquete
            //aca envias el mensaje
            printf("./gameboy TEAM APPEARED_POKEMON [POKEMON]");
            break;
        default: printf("Error ese mensaje no se puede mandar al broker");
    }
    free(ip);
    free(puerto);
}

void gamecard_distribuidor(int argc, char* argv[]){
    char* ip = config_get_string_value(archConfig, "IP_GAMECARD");
    char* puerto = config_get_string_value(archConfig, "PUERTO_GAMECARD");

    switch (str2Msj(argv[2])){
        case 1:
            if(argc < 7){
                msj_error();
                break;
            }
            //Metes los parametros en un paquete
            //aca envias el mensaje
            printf("./gameboy GAMECARD NEW_POKEMON [POKEMON] [POSX] [POSY] [CANTIDAD]");
            break;
        case 3:
            if(argc < 6){
                msj_error();
                break;
            }
            //Metes los parametros en un paquete
            //aca envias el mensaje
            printf("./gameboy GAMECARD CATCH_POKEMON [POKEMON] [POSX] [POSY]");
            break;
        case 5:
            if(argc < 4){
                msj_error();
                break;
            }
            //Metes los parametros en un paquete
            //aca envias el mensaje
            printf("./gameboy GAMECARD GET_POKEMON [POKEMON]");
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
        printf("Error al conectarse al servidor\n");
        close_socket(server_socket);
        return -1;
    }

    if(send_package(paquete, server_socket) == -1){
        //log_error(logger, "Error al enviar el mensaje.");
        close_socket(server_socket);
        return -1;
    }

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

void mensaje_broker_new_pokemon ( char* nombre_pokemon, uint32_t pos_x, uint32_t pos_y, uint32_t cantidad){

    t_paquete* paquete = create_package(NEW_POKEMON);
    t_new_pokemon* new_pokemon = create_new_pokemon( nombre_pokemon, pos_x, pos_y, cantidad);
    void* mensaje_serializado = new_pokemon_a_void(new_pokemon);

    add_to_package( paquete, mensaje_serializado, sizeof(t_new_pokemon));

    int resultado = envio_mensaje(paquete, config_get_string_value(archConfig, "IP_BROKER"), config_get_int_value(archConfig, "PUERTO_BROKER"));
    if(resultado < 0){
        printf("error");
        //error
    }else{
        printf("exito");
        //log()
    }
    //free_t_pokemon(..);
    free_package(paquete);
    free(mensaje_serializado);
}

/*
 * En proceso appeared_pokemon y caought pokemon las tengo que mandar con un id mas que es el del mensaje
 * Ver TODO Problemas con id
 */
/*
//./gameboy BROKER APPEARED_POKEMON [POKEMON] [POSX] [POSY] [ID_MENSAJE]
void mensaje_broker_appeared_pokemon( char* nombre_pokemon, uint32_t pos_x, uint32_t pos_y, uint32_t id_mensaje){

    t_paquete* paquete = create_package(APPEARED_POKEMON);
    t_appeared_pokemon* appeared_pokemon = create_appeared_pokemon( nombre_pokemon, pos_x, pos_y); //TODO agregar id_mensaje
    void* mensaje_serializado = appeared_pokemon_a_void(appeared_pokemon);

    add_to_package( paquete, mensaje_serializado, sizeof(t_appeared_pokemon));

    int resultado = envio_mensaje(paquete, config_get_string_value(archConfig, "IP_BROKER"), config_get_int_value(archConfig, "PUERTO_BROKER"));
    if(resultado < 0){
        printf("error");
        //error
    }else{
        printf("exito");
        //log()
    }
    //free_t_pokemon(..);
    free_package(paquete);
    free(mensaje_serializado);
}

//./gameboy BROKER CATCH_POKEMON [POKEMON] [POSX] [POSY]
void mensaje_broker_catch_pokemon(uint32_t atrapado){
    t_paquete* paquete = create_package(CATCH_POKEMON);
    t_catch_pokemon* catch_pokemon = create_catch_pokemon( nombre_pokemon, pos_x, pos_y, cantidad);
    void* mensaje_serializado = catch_pokemon_a_void(new_pokemon);

    add_to_package( paquete, mensaje_serializado, sizeof(t_catch_pokemon));

    int resultado = envio_mensaje(paquete, config_get_string_value(archConfig, "IP_BROKER"), config_get_int_value(archConfig, "PUERTO_BROKER"));
    if(resultado < 0){
        printf("error");
        //error
    }else{
        printf("exito");
        //log()
    }
    //free_t_pokemon(..);
    free_package(paquete);
    free(mensaje_serializado);
}

//./gameboy BROKER CAUGHT_POKEMON [ID_MENSAJE] [OK/FAIL]
void mensaje_broker_caught_pokemon(uint32_t atrapado){
    t_paquete* paquete = create_package(CAUGHT_POKEMON);
    t_ caught_pokemon* caught_pokemon = create_caught_pokemon(atrapado); //TODO agregar id_mensaje
    void* mensaje_serializado = caught_pokemon_a_void(caught_pokemon);

    add_to_package( paquete, mensaje_serializado, sizeof(t_caught_pokemon));

    int resultado = envio_mensaje(paquete, config_get_string_value(archConfig, "IP_BROKER"), config_get_int_value(archConfig, "PUERTO_BROKER"));
    if(resultado < 0){
        printf("error");
        //error
    }else{
        printf("exito");
        //log()
    }
    //free_t_pokemon(..);
    free_package(paquete);
    free(mensaje_serializado);
}

//./gameboy BROKER GET_POKEMON [POKEMON]
void mensaje_broker_get_pokemon(char* nombre_pokemon){
    t_paquete* paquete = create_package(GET_POKEMON);
    t_ caught_pokemon* caught_pokemon = create_caught_pokemon(atrapado);
    void* mensaje_serializado = caught_pokemon_a_void(caught_pokemon);

    add_to_package( paquete, mensaje_serializado, sizeof(t_caught_pokemon));

    int resultado = envio_mensaje(paquete, config_get_string_value(archConfig, "IP_BROKER"), config_get_int_value(archConfig, "PUERTO_BROKER"));
    if(resultado < 0){
        printf("error");
        //error
    }else{
        printf("exito");
        //log()
    }
    //free_t_pokemon(..);
    free_package(paquete);
    free(mensaje_serializado);
}
*/