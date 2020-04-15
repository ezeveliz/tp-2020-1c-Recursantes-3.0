//
// Created by utnso on 07/04/20.
//

#include "broker.h"


int main(int argc, char **argv) {
    if (argc != 2) {
        cfg_path = strdup("broker.cfg");
    } else {
        cfg_path = strdup(argv[1]);
    }
    logger = log_create("broker.log", "BROKER", 1, LOG_LEVEL_TRACE);
    log_info(logger,"Log started.");
    set_config();
    log_info(logger,"Configuration succesfully setted.");
    return EXIT_SUCCESS;
}

/*
 * Configuration starts
 */
void set_config(){
    cfg_file = config_create(cfg_path);

    if (!cfg_file) {
        log_error(logger, "No se encontró el archivo de configuración");
        return;
    }

    config.mem_size = config_get_int_value(cfg_file, "TAMANO_MEMORIA");
    config.min_partition_size = config_get_int_value(cfg_file, "TAMANO_MINIMO_PARTICION");
    config.mem_algorithm = config_get_string_value(cfg_file, "ALGORITMO_MEMORIA");
    config.mem_swap_algorithm = config_get_string_value(cfg_file, "ALGORITMO_REEMPLAZO");
    config.free_partition_algorithm = config_get_string_value(cfg_file, "ALGORITMO_PARTICION_LIBRE");
    config.broker_ip = config_get_string_value(cfg_file, "IP_BROKER");
    config.broker_port = config_get_int_value(cfg_file, "PUERTO_BROKER");
    config.compactation_freq = config_get_int_value(cfg_file, "FRECUENCIA_COMPACTACION");
    config.log_file= config_get_string_value(cfg_file, "LOG_FILE");
   }
/*
* Configuration ends
*/