#include "team.h"

TEAMConfig* config;

int main() {

    initialize_structures();
    read_config_options();

}

void initialize_structures(){

    config = malloc(sizeof(TEAMConfig));
}

void read_config_options() {

    t_config *config_file = config_create("../team.config");
    config -> posiciones_entrenadores = config_get_array_value(config_file,"POSICIONES_ENTRENADORES");

}