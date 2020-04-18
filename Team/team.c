#include "team.h"

TEAMConfig* config;

int main() {

    read_config_options();

}

void read_config_options() {
    t_config *config_file = config_create("../team.config");

}