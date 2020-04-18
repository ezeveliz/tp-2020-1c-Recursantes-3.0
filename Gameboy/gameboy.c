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

// Busca en un key-value el proceso para pasarlo a un valor numerico de enum
int str2Proces (const char *str)
{
    int j;
    for (j = 0;  j < sizeof(conversionProces) / sizeof(conversionProces[0]);  ++j)
        if (!strcmp(str, conversionProces[j].str))
            return conversionProces[j].val;
    return -1;
}

int str2Msj (const char *str)
{
    int j;
    for (j = 0;  j < sizeof (conversionMsj) / sizeof (conversionMsj[0]);  ++j)
        if (!strcmp (str, conversionMsj[j].str))
            return conversionMsj[j].val;
    return -1;
}


// Estas funciones se encargan de ver que tipo de mensaje es y como tratarlo
void broker_distribuidor(int argc, char* argv[]){
    char* ip = config_get_string_value(archConfig, "IP_BROKER");
    char* puerto = config_get_string_value(archConfig, "PUERTO_BROKER");

    switch (str2Msj(argv[2])){
        case 1:
            //Metes los parametros en un paquete
            //aca envias el mensaje
            if(argc < 7){
                msj_error();
                break;
            }
            printf("./gameboy BROKER NEW_POKEMON [POKEMON] [POSX] [POSY] [CANTIDAD]");
            break;
        case 2:
            //Metes los parametros en un paquete
            //aca envias el mensaje
            if(argc < 7){
                msj_error();
                break;
            }
            printf("./gameboy BROKER APPEARED_POKEMON [POKEMON] [POSX] [POSY] [ID_MENSAJE]");
            break;
        case 3:
            if(argc < 6){
                msj_error();
                break;
            }
            //Metes los parametros en un paquete
            //aca envias el mensaje
            printf("./gameboy BROKER CATCH_POKEMON [POKEMON] [POSX] [POSY]");
            break;
        case 4:
            if(argc < 5) {
                msj_error();
                break;
            }
            //Metes los parametros en un paquete
            //aca envias el mensaje
            printf("./gameboy BROKER CAUGHT_POKEMON [ID_MENSAJE] [OK/FAIL]");
            break;
        case 5:
            if(argc < 4){
                msj_error();
                break;
            }
            //Metes los parametros en un paquete
            //aca envias el mensaje
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

void suscribir(char* cola_mensaje,char* tiempo){
    char* ip = config_get_string_value(archConfig, "IP_BROKER");
    char* puerto = config_get_string_value(archConfig, "PUERTO_BROKER");

    printf("Te queres suscribir a %s durante %s",cola_mensaje, tiempo);

    free(ip);
    free(puerto);
}