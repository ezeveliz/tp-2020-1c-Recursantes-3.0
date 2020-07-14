#include "team.h"

/**
 * Declaracion de variables globales
 */

// Estructuras de configuracion de Team, la segunda es solo temporal
TEAMConfig config;
t_config *config_file;

// Logger de Team y su mutex
t_log* logger;

// Logger del server de Team
t_log* logger_server;

// Hilos encargados de recibir los distintos mensajes del Broker
pthread_t appeared_thread;
pthread_t localized_thread;
pthread_t caught_thread;

// Estructura clave-valor para manejar los objetivos globales, la clave es el nombre y el valor es la cantidad necesitada
t_dictionary* objetivo_global;

// Array de hilos de entrenador
pthread_t* threads_trainer;

// Array de semaforos que bloquean a los hilos para pasar de new a ready
sem_t* new_ready_transition;

// Array de semaforos que bloquean a los hilos para pasar de ready a execute
sem_t* ready_exec_transition;

// Array de semaforos que bloquean a los hilos para pasar de block a ready
sem_t* block_ready_transition;

// Array de semaforos que bloquean a los hilos que estan esperando un catch
sem_t* block_catch_transition;

// Listas de entrenadores que representan los distintos estados
t_list* estado_new;
t_list* estado_ready;
t_list* estado_exec;
t_list* estado_block;
t_list* estado_finish;

// Lista en la que voy a encolar mensajes de los que este esperando respuesta(catches)
t_list* waiting_list;

// Lista de los pokemons(Los recibidos por Appeared o Localized)
t_list* pokemons;

// Lista de pokemons para filtrar los que ya el broker nos envio
t_list* pokemons_received;

// Semaforo Mutex para proteger la lista de pokemones
pthread_mutex_t mutex_pokemon;

// Semaforo mutex para proteger la lista de espera de respuestas
pthread_mutex_t mutex_waiting_list;

// Semaforo mutex para proteger la lista de pokemones recibidos(solo nombre)
pthread_mutex_t mutex_pokemons_received;

//Semaforo mutex para algoritmo de cercania
pthread_mutex_t mutex_algoritmo_cercania;

//Semaforo mutex deadlock
pthread_mutex_t mutex_deadlock;

int main() {
    int i = 0;
    pthread_t server_thread;

    // Leo archivo de configuracion, si no lo encuentro salgo del proceso
    if (read_config_options() == -1) {
        printf("No se encontro archivo de configuracion, saliendo.");
        return -1;
    }

    // Inicializo el log, si no pude salgo del proceso
    if (start_log() == -1) {
        printf("No se pudo inicializar el log en la ruta especificada, saliendo.");
        return -1;
    }

    // Inicializo estructuras, semaforos e hilos
int tamanio_entrenadores = initialize_structures();

    //Creo el servidor para que el GameBoy me mande mensajes
    pthread_create(&server_thread, NULL, server_function, NULL);

    //Joineo el hilo main con el del servidor para el GameBoy
    pthread_detach(server_thread);

    //Creo 3 hilos para suscribirme a las colas globales
    subscribe_to_queues();

    //Hacemos los gets aca para que no haya problemas de comunicacion y ya esten todas las estructuras inicializadas,
    // el servidor levantado y estamos suscriptos a las colas de mensajes correspondientes

    // Itero la lista de pokemons objetivos y realizo todos los gets
    void iterador_pokemons(char* clave, void* contenido){

        // Creo la estructura de pokemon para mandarle al Broker
        t_get_pokemon* get_pok = create_get_pokemon(clave);

        // Paso la estructura a void
        void* pok_void = get_pokemon_a_void(get_pok);
        free(get_pok);

        // Envio el get al Broker con un hilo
        send_message_thread(pok_void, sizeof_get_pokemon(pok_void), GET_POK, -1);
    }
    dictionary_iterator(objetivo_global, iterador_pokemons);

    // TODO: joinear hilos de entrenadores
    while(i < tamanio_entrenadores){
        pthread_join(threads_trainer[i], NULL);
        i++;
    }

    printf("Todos los entrenadores cumplieron sus objetivos \n");

    log_info(logger, "Todos los entrenadores cumplieron sus objetivos ");

    //Cuando termina la ejecucion de todos los hilos libero los recursos
    free_resources();
}

int read_config_options() {

    config_file = config_create("../team.config");
    if (!config_file) {
        return -1;
    }
    config.posiciones_entrenadores = config_get_array_value(config_file, "POSICIONES_ENTRENADORES");
    config.pokemon_entrenadores = config_get_array_value(config_file, "POKEMON_ENTRENADORES");
    config.objetivos_entrenadores = config_get_array_value(config_file, "OBJETIVOS_ENTRENADORES");
    config.tiempo_reconexion = config_get_int_value(config_file, "TIEMPO_RECONEXION");
    config.retardo_ciclo_cpu = config_get_int_value(config_file, "RETARDO_CICLO_CPU");
    config.quantum = config_get_int_value(config_file, "QUANTUM");
    config.estimacion_inicial = config_get_int_value(config_file, "ESTIMACION_INICIAL");
    config.ip_broker = config_get_string_value(config_file, "IP_BROKER");
    config.puerto_broker = config_get_int_value(config_file, "PUERTO_BROKER");
    config.ip_team = config_get_string_value(config_file, "IP_TEAM");
    config.puerto_team = config_get_int_value(config_file, "PUERTO_TEAM");
    config.log_file = config_get_string_value(config_file, "LOG_FILE");
    config.team_id = config_get_int_value(config_file, "TEAM_ID");

    // Transformo el algoritmo en un enum para simplificar el manejo en un futuro
    char* algoritmo = config_get_string_value(config_file, "ALGORITMO_PLANIFICACION");

    if (string_equals_ignore_case(algoritmo, "FIFO")) {
        config.algoritmo_planificacion = FIFO;
    } else if (string_equals_ignore_case(algoritmo, "SJF-SD")) {
        config.algoritmo_planificacion = SJF_SD;
    } else if (string_equals_ignore_case(algoritmo, "SJF-CD")) {
        config.algoritmo_planificacion = SJF_CD;
    } else if (string_equals_ignore_case(algoritmo, "RR")) {
        config.algoritmo_planificacion = RR;
    }

    return 1;
}

//TODO: cambiar el 1 por un 0 para la entrega
int start_log() {

    logger = log_create(config.log_file, "team", 1, LOG_LEVEL_TRACE);
    if (!logger) {
        return -1;
    }
    return 1;
}

//----------------------------------------COLAS GLOBALES----------------------------------------//

void subscribe_to_queues() {

    // Levanto 3 hilos y en cada uno realizo una conexion al broker para cada una de las colas
    MessageType* appeared = malloc(sizeof(MessageType));
    *appeared = SUB_APPEARED;
    pthread_create(&appeared_thread, NULL, &subscribe_to_queue_thread, (void*)appeared);
    pthread_detach(appeared_thread);

    MessageType* localized = malloc(sizeof(MessageType));
    *localized = SUB_LOCALIZED;
    pthread_create(&localized_thread, NULL, &subscribe_to_queue_thread, (void*)localized);
    pthread_detach(localized_thread);

    MessageType* caught = malloc(sizeof(MessageType));
    *caught = SUB_CAUGHT;
    pthread_create(&caught_thread, NULL, &subscribe_to_queue_thread, (void*)caught);
    pthread_detach(caught_thread);
}

void* subscribe_to_queue_thread(void* arg) {
    MessageType cola = *(MessageType*)arg;

    // Me intento conectar y suscribir, la funcion no retorna hasta que no lo logre
    int broker = connect_and_subscribe(cola);

    // Reservo cachito de memoria para confirmar los mensajes que le envio al Broker
    int* confirmacion = malloc(sizeof(int));
    *confirmacion = ACK;

    // Creo mensaje para loguear la perdida de conexion con el Broker
    char* conexionPerdida = string_new();
    string_append(&conexionPerdida, "La cola encargada de recibir los mensajes ");

    switch (cola) {
        case (APPEARED_POK):;
            string_append(&conexionPerdida, "Appeared ");
            break;
        case (LOCALIZED_POK):;
            string_append(&conexionPerdida, "Localized ");
            break;
        case (CAUGHT_POK):;
            string_append(&conexionPerdida, "Caught ");
            break;
        default:;
            string_append(&conexionPerdida, "(mensaje no soportado) ");
            break;
    }
    string_append(&conexionPerdida, "ha perdido la conexion con el Broker, a continuacion se intentara la resubscripcion.");

    // Creo mensaje para loguear la reconexion con el Broker
    char* conexionReestablecida = string_new();
    string_append(&conexionReestablecida, "La cola encargada de recibir los mensajes ");

    switch (cola) {
        case (APPEARED_POK):;
            string_append(&conexionReestablecida, "Appeared ");
            break;
        case (LOCALIZED_POK):;
            string_append(&conexionReestablecida, "Localized ");
            break;
        case (CAUGHT_POK):;
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

        MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
        if(receive_header(broker, buffer_header) > 0) {

            char* pokName;

            // Recibo la respuesta del Broker
            t_list* rta_list = receive_package(broker, buffer_header);

            // La posicion 0 de la lista recibida es siempre el id correlacional correspondiente
            // El resto del mensajes usa las estructuras locas del Broker(detalladas en la commLib)

            // En la posicion 0 viene el id de mensaje correlativo
            int idMensaje = *(int*) list_get(rta_list, 0);

            int idCorrelativo = *(int*) list_get(rta_list, 1);

            bool encontrador(void* _nombre) {
                return string_equals_ignore_case(pokName, (char*)_nombre);
            }

            // Switch case que seleccione que hacer con la respuesta segun el tipo de cola
            switch (cola) {

                case (APPEARED_POK):;

                    appeared_pokemon(rta_list); // El logueo del nuevo mensaje se realiza adentro asi puedo diferenciar el broker del gameboy

                    break;

                case (LOCALIZED_POK):;

                    // Obtengo el paquetito de appeared
                    t_localized_pokemon* localizedPokemon = void_a_localized_pokemon(list_get(rta_list, 2));

                    // Logueo la llegada del localized
                    char* localized = string_new();

                    string_append(&localized, "Ha llegado un nuevo mensaje Localized indicando que ");
                    if (localizedPokemon->cantidad_coordenas > 1) {

                        string_append(&localized, "llegaron ");
                        char* cant = string_itoa(localizedPokemon->cantidad_coordenas);
                        string_append(&localized, cant);
                        free(cant);
                        string_append(&localized, " instancias del pokemon ");
                    } else {

                        string_append(&localized, "llego una instancia del pokemon ");
                    }
                    string_append(&localized, localizedPokemon->nombre_pokemon);
                    string_append(&localized, " en: [");

                    int i = localizedPokemon->cantidad_coordenas;

                    int* coords = localizedPokemon->coordenadas;

                    while (i > 0) {

                        string_append(&localized, "(");
                        char* pos_x = string_itoa(coords[i*2]);
                        string_append(&localized, pos_x);
                        free(pos_x);
                        string_append(&localized, ", ");
                        char* pos_y = string_itoa(coords[(i*2) + 1]);
                        string_append(&localized, pos_y);
                        free(pos_y);
                        string_append(&localized, ")");

                        i--;
                    }
                    string_append(&localized, "]");
                    log_info(logger, localized);
                    free(localized);

                    // Obtengo el nombre de pokemon
                    pokName = localizedPokemon->nombre_pokemon;

                    // Verifico que no haya recibido el pokemon ya, si ya lo recibi no lo utilizo
                    if (!list_any_satisfy(pokemons_received, encontrador)) {

                        // Hallo la cantidad de pokemones recibidos
                        int cant = localizedPokemon->cantidad_coordenas;

                        // Hallo el array de coordenadas recibidas
                        int* coordenadas = localizedPokemon->coordenadas;

                        // Itero sobre los pokemones recibidos
                        while(cant > 0) {

                            // Instancio un nuevo pokemon
                            Pokemon *pokemon = (Pokemon*) malloc(sizeof(Pokemon));

                            // Seteo los parametros de la estructura Pokemon
                            pokemon->especie = pokName;
                            pokemon->coordenada.pos_x = coordenadas[cant * 2];
                            pokemon->coordenada.pos_y = coordenadas[(cant * 2) + 1];

                            // Agrego al pokemon a la lista de pokemones que voy a asignar a los entrenadores
                            pthread_mutex_lock(&mutex_pokemon);
                            list_add(pokemons, pokemon);
                            pthread_mutex_unlock(&mutex_pokemon);

                            cant --;
                        }

                        // Agrego el pokemon a la lista de pokemones recibidos(solo nombre)
                        pthread_mutex_lock(&mutex_pokemons_received);
                        list_add(pokemons_received, (void*) pokName);
                        pthread_mutex_unlock(&mutex_pokemons_received);

                        algoritmo_de_cercania();

                        // En este caso ya lo habia recibido, libero la memoria del paquete
                    } else {

                        // TODO: liberar memoria del paquete
                    }
                    break;

                case (CAUGHT_POK):;

                    // Obtengo el paquetito de caught
                    t_caught_pokemon* caughtPokemon = void_a_caught_pokemon(list_get(rta_list, 2));

                    // Quito el mensaje que estaba en espera que tenga el mmismo idCorrelacional que el recibido
                    pthread_mutex_lock(&mutex_waiting_list);
                    bool hallar_entrenador(void* _mensaje) {
                        WaitingMessage* mensaje = (WaitingMessage*)_mensaje;
                        return mensaje->id_correlativo == idCorrelativo;
                    }
                    WaitingMessage* mensaje = (WaitingMessage*)list_remove_by_condition(waiting_list, hallar_entrenador);
                    pthread_mutex_unlock(&mutex_waiting_list);

                    // Verifico que exista el mensaje en la lista ya que el id que me trae el Broker podria no existir en la lista
                    if (mensaje != null) {

                        // Llamo a la funcion que resuelve los caughts
                        caught_pokemon(mensaje->tid, caughtPokemon->atrapado);
                    } else {

                        // Logueo el mensaje catch que no era para nosotros
                        char* caught = string_new();
                        string_append(&caught, "Ha llegado un mensaje Caught que no correspondia con ninguno de los Catch enviados por nosotros.");
                        log_info(logger, caught);
                        free(caught);
                    }
                    break;

            }

            // Creo paquete para confirmar recepcion de mesaje al Broker
            t_paquete* paquete = create_package(ACK);
            add_to_package(paquete,  (void*)&(config.team_id), sizeof(int));
            add_to_package(paquete, (void*) &idMensaje, sizeof(int));

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
    // TODO: si eventualmente se sale del while, hacerle free al arg recibido por parametro
    return null;
}

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
            sleep(config.tiempo_reconexion);
        }
    }

    return broker;
}

int connect_to_broker(){

    int client_socket;
    if((client_socket = create_socket()) == -1) {

        return -1;
    }
    if(connect_socket(client_socket, config.ip_broker, config.puerto_broker) == -1){

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
    *id = config.team_id;
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
    free_list(rta_list, element_destroyer);

    return rta == 1;
}

//----------------------------------------SERVIDOR----------------------------------------//

void* server_function(void* arg) {

    start_log_server();

    int server_socket;

    // La creacion de nuestro socket servidor puede fallar, si falla duermo y vuelvo a intentar en n segundos
    while ((server_socket = initialize_server()) == -1) {

        sleep(config.tiempo_reconexion);
    }

    start_server(server_socket, &new, &lost, &incoming);

    return null;
}

void start_log_server() {

    //Cambiar 1 por 0?
    logger_server=log_create("../servidor.log", "servidor", 1, LOG_LEVEL_TRACE);
}

int initialize_server(){

    int server_socket;
    int port = config.puerto_team;

    if((server_socket = create_socket()) == -1) {
        log_error(logger_server, "Error creating server socket");
        return -1;
    }
    if((bind_socket(server_socket, port)) == -1) {
        log_error(logger_server, "Error binding server socket");
        return -1;
    }

    return server_socket;
}

void new(int server_socket, char * ip, int port){

}

void lost(int server_socket, char * ip, int port){

}

void incoming(int server_socket, char* ip, int port, MessageHeader * headerStruct){

    t_list* paquete_recibido = receive_package(server_socket, headerStruct);

    // Si desaprobamos por esto es culpa de Emi
    int confirmacion = 1;

    // Creo paquete para responderle al GameBoy
    t_paquete *package = create_package(headerStruct->type);
    add_to_package(package, &confirmacion, sizeof(int));

    // Envio confirmacion al GameBoy
    send_package(package, server_socket);

    switch(headerStruct -> type){

        case APPEARED_POK:
            appeared_pokemon(paquete_recibido);
            break;
        default:
            printf("la estas cagando compa\n");
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

//----------------------------------------ENTRENADORES----------------------------------------//

int initialize_structures() {

    //Inicializo el semaforo mutex y la lista de pokemons para conocer instancias de pokemons en el mapa y los que el broker ya nos envio
    pthread_mutex_init(&mutex_pokemon, NULL);
    pokemons = list_create();
    pokemons_received = list_create();

    // Inicializo las listas de estados y otras cosas
    char **ptr = config.posiciones_entrenadores;
    int pos = 0, tamanio_entrenadores = 0;
    objetivo_global = dictionary_create();
    estado_new = list_create();
    estado_ready = list_create();
    estado_block = list_create();
    estado_finish = list_create();
    estado_exec = list_create();
    waiting_list = list_create();

    // Inicializo semaforo mutex que protege la lista de espera
    pthread_mutex_init(&mutex_waiting_list, NULL);

    // Inicializo semaforo mutex que protege la lista de pokemones recibidos(solo nombre)
    pthread_mutex_init(&mutex_pokemons_received, NULL);

    //TODO: Despues ver si esto se queda o se borra
    pthread_mutex_init(&mutex_algoritmo_cercania, NULL);
    pthread_mutex_init(&mutex_deadlock, NULL);

    // Hallo la cantidad de listas de stock de pokemones hay
    int cant_listas_pokemones_stock = funcion_de_mierda(config.pokemon_entrenadores);

    //Itero el array de posiciones de entrenadores
    for (char *coordenada = *ptr; coordenada; coordenada = *++ptr) {

        // Instancio un nuevo entrenador
        Entrenador *entrenador = (Entrenador *) malloc(sizeof(Entrenador));
        entrenador->estado = NEW;
        entrenador->objetivos_particular = dictionary_create();
        entrenador->stock_pokemons = dictionary_create();
        entrenador->acumulado_total = 0;
        entrenador->acumulado_actual = 0;
        entrenador->ultima_ejecucion = 0;
        entrenador->ultimo_estimado = 0;

        // Obtengo los objetivos y los pokemones que posee el entrenador actual
        char **objetivos_entrenador = string_split(config.objetivos_entrenadores[pos], "|");
        //Verifico que el entrenador tenga stock de pokemons
        char **pokemon_entrenador;

        // Chequeo si el entrenador tiene pokemones en stock
        if(pos < cant_listas_pokemones_stock) {

            // Desarmo la lista de pokemons en stock
            pokemon_entrenador = string_split(config.pokemon_entrenadores[pos], "|");

            // Seteo los objetivos globales, teniendo en cuenta los objetivos y el stock
            add_global_objectives(objetivos_entrenador, pokemon_entrenador);
            //Seteo el stock de pokemons
            add_to_dictionary(pokemon_entrenador, entrenador->stock_pokemons);

            // En este caso no tenia pokemones en stock
        } else {

            // Seteo los objetivos globales, teniendo en cuenta SOLO los objetivos, ya que no posee stock
            add_to_dictionary(objetivos_entrenador, objetivo_global);
        }

        char **posiciones = string_split(coordenada, "|");

        // Seteo el objetivo particular del entrenador
        add_to_dictionary(objetivos_entrenador, entrenador->objetivos_particular);

        // Buscamos y seteamos la cantidad de Pokemones maxima que puede tener el entrenador
        int contador_objetivos = 0;
        void iterador_objetivos(char* clave, void* contenido){
            contador_objetivos += *(int*) contenido;
        }
        dictionary_iterator(entrenador->objetivos_particular, iterador_objetivos);
        entrenador->cant_objetivos = contador_objetivos;

        // Buscamos y seteamos la cantidad actual de pokemons que tiene el entrenador
        int contador_stock = 0;
        void iterador_stock(char* clave, void* contenido){
            contador_stock += *(int*) contenido;
        }
        dictionary_iterator(entrenador->stock_pokemons, iterador_stock);
        entrenador->cant_stock = contador_stock;

        // Le asigno al entrenador sus coordenadas
        sscanf(posiciones[0], "%d", &entrenador->pos_actual.pos_x);
        sscanf(posiciones[1], "%d", &entrenador->pos_actual.pos_y);

        // Le asigno un tid falso al entrenador
        entrenador->tid = pos;

        // Agrego al entrenador a New
        list_add(estado_new, (void *) entrenador);

        // Limpieza
        free_splitted_arrays(objetivos_entrenador, contador_objetivos);
        if(pos < cant_listas_pokemones_stock) {
            free_splitted_arrays(pokemon_entrenador, contador_stock);
        }
        free_splitted_arrays(posiciones, 2);
        pos++;
    }

    //Obtengo la cantidad de entrenadores nuevos
    tamanio_entrenadores = list_size(estado_new);

    // Creo un array de tantos hilos como entrenadores haya
    threads_trainer = (pthread_t *) malloc(tamanio_entrenadores * sizeof(pthread_t));
    // Creo un array de semaforos para bloquear la transicion new - ready
    new_ready_transition = (sem_t*) malloc(tamanio_entrenadores * sizeof(sem_t));
    // Creo un array de semaforos para bloquear la transicion ready - exec
    ready_exec_transition = (sem_t*) malloc(tamanio_entrenadores * sizeof(sem_t));
    // Creo un array de semaforos para bloquear la transicion block - ready
    block_ready_transition = (sem_t*) malloc(tamanio_entrenadores * sizeof(sem_t));
    // Creo un array de semaforos para bloquear los entrenadores que esten esperando un catch
    block_catch_transition = (sem_t*) malloc(tamanio_entrenadores * sizeof(sem_t));

    // Itero los entrenadores, inicializamos los semaforos y el hilo correspondiente a cada uno
    for (int count = 0; count < tamanio_entrenadores; count++) {

        // Inicializo los semaforos correspondientes al entrenador en 0 para que quede bloqueado
        sem_init(&new_ready_transition[count], 0, 0);
        sem_init(&ready_exec_transition[count], 0, 0);
        sem_init(&block_ready_transition[count], 0, 0);
        sem_init(&block_catch_transition[count], 0, 0);

        Entrenador* entrenador_actual = (Entrenador*) list_get(estado_new, count);
        pthread_create(&threads_trainer[count], NULL, (void *) trainer_thread, (void *) entrenador_actual);
    }

    return tamanio_entrenadores;

}

void add_to_dictionary(char** cosas_agregar, t_dictionary* diccionario){

    // Itero la lista de pokemones objetivo de un entrenador dado
    for (char* pokemon = *cosas_agregar; pokemon ; pokemon = *++cosas_agregar) {

        // Verifico si ya existia la necesidad de este pokemon, si existe le sumo uno
        if (dictionary_has_key(diccionario, pokemon)) {

            //Por alguna razon que no pude descifrar, no funcionaba el put
            *(int*)dictionary_get(diccionario, pokemon) += 1;

            // Si no existia la necesidad la creo
        } else {

            int* necesidad = (int*)malloc(sizeof(int));
            *necesidad = 1;
            dictionary_put(diccionario, pokemon, (void*) necesidad);
        }
    }
}

void add_global_objectives(char** objetivos_entrenador, char** pokemon_entrenador) {

    // Itero la lista de pokemones objetivo de un entrenador dado
    for (char* pokemon = *objetivos_entrenador; pokemon ; pokemon = *++objetivos_entrenador) {

        // Verifico si ya existia la necesidad de este pokemon, si existe le sumo uno
        if (dictionary_has_key(objetivo_global, pokemon)) {

            //Por alguna razon que no pude descifrar, no funcionaba el put
            *(int*)dictionary_get(objetivo_global, pokemon) += 1;

            // Si no existia la necesidad la creo
        } else {

            int* necesidad = (int*)malloc(sizeof(int));
            *necesidad = 1;
            dictionary_put(objetivo_global, pokemon, (void*) necesidad);
        }
    }

    // Itero la lista de pokemones que posee un entrenador dado, para restarle al objetivo global
    for (char* pokemon = *pokemon_entrenador; pokemon ; pokemon = *++pokemon_entrenador) {

        // Verifico si ya existia la necesidad de este pokemon, si existe le resto uno
        if (dictionary_has_key(objetivo_global, pokemon)) {

            *(int*)dictionary_get(objetivo_global, pokemon) -= 1;

            //TODO: verificar que no sean tan forros de poner un pokemon que nadie va a utilizar
            // Si no existia la necesidad la creo(con valor de -1)
        } else {

            int* necesidad = (int*)malloc(sizeof(int));
            *necesidad = -1;
            dictionary_put(objetivo_global, pokemon, (void*) necesidad);
        }
    }
}

void* trainer_thread(void* arg){
    Entrenador* entrenador = (Entrenador*) arg;

    // Logueo la entrada del entrenador en la cola New
    char* new = string_new();

    string_append(&new, "El entrenador ");
    char* trainer = string_itoa(entrenador->tid);
    string_append(&new, trainer);
    free(trainer);
    string_append(&new, " ha entrado en New");

    log_info(logger, new);

    free(new);

    // Bloqueo la transicion new - ready hasta que haya algun pokemon a capturar(proveniente de mensajes Appeared o Localized)
    sem_wait( &new_ready_transition[entrenador->tid] );

    char* ready = string_new();
    string_append(&ready,"El entrenador ");
    char* entrenador_tid_ready = string_itoa(entrenador->tid);
    string_append(&ready, entrenador_tid_ready);
    string_append(&ready, " ha entrado a Ready");

    log_info(logger, ready);

    free(entrenador_tid_ready);
    free(ready);

    // Reservo memoria, para el tiempo de llegada, aca guardo cuando entra a Ready
    entrenador->tiempo_llegada = malloc(sizeof(struct timespec));

    // Me quedo iterando hasta que este en estado Finish
    while(entrenador->estado != FINISH){

        // Marco el momento en que el entrenador llego a Ready
        *(entrenador->tiempo_llegada) = get_time();

        // Llamo al planificador
        call_planner();

        // Bloqueo esperando a que el planificador decida que ejecute
        sem_wait( &ready_exec_transition[entrenador->tid] );

        char* exec = string_new();
        string_append(&exec,"El entrenador ");
        char* entrenador_tid_exec = string_itoa(entrenador->tid);
        string_append(&exec, entrenador_tid_exec);
        string_append(&exec, " ha entrado a Exec");

        log_info(logger, exec);

        free(entrenador_tid_exec);
        free(exec);

        int distancia_a_viajar = 0;

        // Segun la razon de movimiento voy a calcular la distancia al objetivo de una manera diferente
        switch(entrenador->razon_movimiento) {
            case (CATCH):;

                distancia_a_viajar = distancia(entrenador->pos_actual, entrenador->pokemon_objetivo->coordenada);
                break;
            case (RESOLUCION_DEADLOCK):;

                distancia_a_viajar = distancia(entrenador->pos_actual, entrenador->entrenador_objetivo->pos_actual);
                break;
        }

        // Itero tantas veces como unidades se tenga que mover el entrenador a su posicion destino
        while (distancia_a_viajar > 0) {

            // Duermo el entrenador durante un ciclo de CPU
            sleep(config.retardo_ciclo_cpu);

            // Acumulo un ciclo de ejecucion
            entrenador->acumulado_actual += 1;

            distancia_a_viajar--;
        }

        // Ya viaje, seteo la posicion actual
        switch(entrenador->razon_movimiento) {
            case (CATCH):;

                entrenador->pos_actual.pos_x = entrenador->pokemon_objetivo->coordenada.pos_x;
                entrenador->pos_actual.pos_y = entrenador->pokemon_objetivo->coordenada.pos_y;
                break;
            case (RESOLUCION_DEADLOCK):;

                entrenador->pos_actual.pos_x = entrenador->entrenador_objetivo->pos_actual.pos_x;
                entrenador->pos_actual.pos_y = entrenador->entrenador_objetivo->pos_actual.pos_y;
                break;
        }

        // Logueo la posicion del entrenador luego del viaje
        char* viaje = string_new();

        string_append(&viaje, "El entrenador: ");
        trainer = string_itoa(entrenador->tid);
        string_append(&viaje, trainer);
        string_append(&viaje, ", ha viajado hasta: (");
        char* pos_x = string_itoa(entrenador->pos_actual.pos_x);
        string_append(&viaje, pos_x);
        string_append(&viaje, ", ");
        char* pos_y = string_itoa(entrenador->pos_actual.pos_y);
        string_append(&viaje, pos_y);
        string_append(&viaje, ").");

        log_info(logger, viaje);

        free(trainer);
        free(pos_y);
        free(pos_x);
        free(viaje);

        // Ya viaje a destino, ahora tengo que solicitar el catch o realizar el intercambio
        switch (entrenador->razon_movimiento) {

            // Estaba viajando para atrapar un Pokemon
            case (CATCH):;

                // TODO: Las listas que uso abajo deben estar protegidas por semaforos mutex

                // Actualizo el acumulado total con los nuevos ciclos, seteo la ultima ejecucion y reinicio el contador de ejecucion actual
                entrenador->acumulado_total += entrenador->acumulado_actual;
                entrenador->ultima_ejecucion = entrenador->acumulado_actual;
                entrenador->acumulado_actual = 0;

                // Me quito de la lista de ejecucion
                list_remove(estado_exec, 0);

                // Me asigno la razon de bloqueo ESPERANDO_CATCH y me agrego a la lista de bloqueo
                entrenador->estado = BLOCK;
                entrenador->razon_bloqueo = ESPERANDO_CATCH;
                list_add(estado_block, entrenador);

                // Duermo durante el tiempo que ocupa un ciclo de ejecucion correspondiente al envio de mensaje al broker
                sleep(config.retardo_ciclo_cpu);

                // Acumulo un ciclo de ejecucion
                entrenador->acumulado_actual += 1;

                // Creo la estructura t_catch_pokemon para enviarle al Broker
                Pokemon* pok = entrenador->pokemon_objetivo;
                t_catch_pokemon* pokemon_to_catch = create_catch_pokemon(pok->especie, pok->coordenada.pos_x, pok->coordenada.pos_y);

                // Llamo a la funcion para enviar un mensaje en un hilo y envio la estructura que cree antes
                send_message_thread(catch_pokemon_a_void(pokemon_to_catch), sizeof_catch_pokemon(pokemon_to_catch), CATCH_POK, entrenador->tid);

                // Logueo el catch
                char* catch = string_new();

                string_append(&catch, "El entrenador: ");
                trainer = string_itoa(entrenador->tid);
                string_append(&catch, trainer);
                free(trainer);
                string_append(&catch, ", ha solicitado atrapar al pokemon: ");
                string_append(&catch, entrenador->pokemon_objetivo->especie);
                string_append(&catch, " en la posicion: (");
                pos_x = string_itoa(entrenador->pos_actual.pos_x);
                string_append(&catch, pos_x);
                free(pos_x);
                string_append(&catch, ", ");
                pos_y = string_itoa(entrenador->pos_actual.pos_y);
                string_append(&catch, pos_y);
                free(pos_y);
                string_append(&catch, ").");

                log_info(logger, catch);

                free(catch);

                // Me bloqueo esperando la rta del Broker
                sem_wait(&block_catch_transition[entrenador->tid]);

                break;

                // Estaba viajando para intercambiar un Pokemon
            case (RESOLUCION_DEADLOCK):;

                // Simulo la ejecucion de 5 ciclos de CPU para el intercambio
                int retardo_simulacion = 5;
                while (retardo_simulacion > 0) {

                    // Duermo durante el tiepo que ocupa un ciclo de ejecucion
                    sleep(config.retardo_ciclo_cpu);

                    // Acumulo uno de los ciclos del intercambio
                    entrenador->acumulado_actual += 1;

                    retardo_simulacion--;
                }

                // Actualizo el acumulado total con los nuevos ciclos, seteo la ultima ejecucion y reinicio el contador de ejecucion actual
                entrenador->acumulado_total += entrenador->acumulado_actual;
                entrenador->ultima_ejecucion = entrenador->acumulado_actual;
                entrenador->acumulado_actual = 0;

                //Hago un vector porque add_to_dictionary recibe un char**
                char** pokemon_first_trainer = (char**) malloc(sizeof(char*) * 2);
                pokemon_first_trainer[0] = entrenador->pokemon_objetivo->especie;
                pokemon_first_trainer[1] = NULL;

                //Agrego al stock del entrenador el pokemon, si existe la key le sumo uno al value, si no existe la creo
                add_to_dictionary(pokemon_first_trainer,entrenador->stock_pokemons);

                bool prueba5 = dictionary_has_key(entrenador->stock_pokemons,*pokemon_first_trainer);
                bool prueba8 = dictionary_has_key(entrenador->entrenador_objetivo->stock_pokemons,*pokemon_first_trainer);

                char** pokemon_second_trainer = (char**) malloc(sizeof(char*) * 2);
                pokemon_second_trainer[0] = entrenador->entrenador_objetivo->pokemon_objetivo->especie;
                pokemon_second_trainer[1] = NULL;

                //Agrego al stock del entrenador objetivo el pokemon
                add_to_dictionary(pokemon_second_trainer,entrenador->entrenador_objetivo->stock_pokemons);

                bool prueba6 = dictionary_has_key(entrenador->entrenador_objetivo->stock_pokemons,*pokemon_second_trainer);
                bool prueba7 = dictionary_has_key(entrenador->stock_pokemons,*pokemon_second_trainer);

                remover_de_stock(entrenador, *pokemon_first_trainer, *pokemon_second_trainer);

                bool prueba1 = dictionary_has_key(entrenador->stock_pokemons,*pokemon_first_trainer);
                bool prueba2 = dictionary_has_key(entrenador->entrenador_objetivo->stock_pokemons,*pokemon_second_trainer);
                bool prueba3 = dictionary_has_key(entrenador->stock_pokemons,*pokemon_second_trainer);
                bool prueba4 = dictionary_has_key(entrenador->entrenador_objetivo->stock_pokemons,*pokemon_first_trainer);

                (*(int*) dictionary_get(entrenador->objetivos_particular,entrenador->pokemon_objetivo->especie))-1;
                (*(int*) dictionary_get(entrenador->entrenador_objetivo->objetivos_particular,entrenador->entrenador_objetivo->pokemon_objetivo->especie))-1;

                entrenador->razon_bloqueo = SIN_ESPACIO;
                entrenador->entrenador_objetivo-> razon_bloqueo = SIN_ESPACIO;

                char *deadlock = string_new();

                string_append(&deadlock, "El entrenador ");
                char *first_trainer = string_itoa(entrenador->tid);
                string_append(&deadlock, first_trainer);
                free(first_trainer);
                string_append(&deadlock, " le dio un ");
                string_append(&deadlock, entrenador->entrenador_objetivo->pokemon_objetivo->especie);
                string_append(&deadlock, " al entrenador ");
                char *second_trainer = string_itoa(entrenador->entrenador_objetivo->tid);
                string_append(&deadlock, second_trainer);
                free(second_trainer);
                string_append(&deadlock, " y, este le dio un ");
                string_append(&deadlock, entrenador->pokemon_objetivo->especie);

                log_info(logger, deadlock);
                free(deadlock);

                break;
        }

        // Verifico si el entrenador completo sus objetivos
        if (objetivos_cumplidos(entrenador)) {
            char* objetivos_cumplidos_entrenador = string_new();
            string_append(&objetivos_cumplidos_entrenador, "El entrenador ");
            char* entrenador_tid_objetivos_cumplidos = string_itoa(entrenador->tid);
            string_append(&objetivos_cumplidos_entrenador, entrenador_tid_objetivos_cumplidos);
            string_append(&objetivos_cumplidos_entrenador, " ha cumplido con los objetivos ");
            log_info(logger,objetivos_cumplidos_entrenador);
            free(objetivos_cumplidos_entrenador);
            free(entrenador_tid_objetivos_cumplidos);

            // Dependiendo de la razon del movimiento me tengo que quitar de una lista distinta
            switch (entrenador->razon_movimiento) {

                // Si esperaba un catch me tengo que quitar de la lista de Bloqueados
                case (CATCH):;

                    bool removedor(void* _trainer) {
                        Entrenador* trainer = (Entrenador*)_trainer;
                        return trainer->tid == entrenador->tid;
                    }
                    list_remove_by_condition(estado_block, removedor);
                    break;

                    // Si estaba resolviendo un DeadLock, me tengo que quita de la lista de ejecucion
                case (RESOLUCION_DEADLOCK):;
                    //TODO: Verificar si el otro entrenador que vino de una resolucion de deadlock termino sus objetivos y sacarlo de la lista correspondiente
                    list_remove(estado_exec, 0);

                    if(objetivos_cumplidos(entrenador->entrenador_objetivo)){
                        char* objetivos_cumplidos_entrenador = string_new();
                        string_append(&objetivos_cumplidos_entrenador, "El entrenador ");
                        char* entrenador_tid_objetivos_cumplidos = string_itoa(entrenador->entrenador_objetivo->tid);
                        string_append(&objetivos_cumplidos_entrenador, entrenador_tid_objetivos_cumplidos);
                        string_append(&objetivos_cumplidos_entrenador, " ha cumplido con los objetivos ");
                        log_info(logger,objetivos_cumplidos_entrenador);
                        free(objetivos_cumplidos_entrenador);
                        free(entrenador_tid_objetivos_cumplidos);

                        bool remover(void *_entrenador) {
                            Entrenador *entrenador_iteracion = (Entrenador *) _entrenador;
                            return entrenador->entrenador_objetivo->tid == entrenador_iteracion->tid;
                        }
                        list_remove_by_condition(estado_block, remover);

                        entrenador->entrenador_objetivo->estado = FINISH;
                        list_add(estado_finish, entrenador->entrenador_objetivo);
                    }

                    break;
            }

            // Le asigno el estado finish
            entrenador->estado = FINISH;
            list_add(estado_finish, entrenador);
            // TODO: agregar a la lista de Finish?

            // En este caso no cumpli mis objetivos aun, debo quedarme en bloqueo
        } else {

            char* objetivos_nocumplidos_entrenador = string_new();
            string_append(&objetivos_nocumplidos_entrenador, "El entrenador ");
            char* entrenador_tid_objetivos_nocumplidos = string_itoa(entrenador->tid);
            string_append(&objetivos_nocumplidos_entrenador, entrenador_tid_objetivos_nocumplidos);
            string_append(&objetivos_nocumplidos_entrenador, " todavia no ha cumplido con los objetivos ");
            log_info(logger,objetivos_nocumplidos_entrenador);
            free(objetivos_nocumplidos_entrenador);
            free(entrenador_tid_objetivos_nocumplidos);

            // Chequeo si tengo espacio para recibir mas pokemones
            if(entrenador->cant_stock < entrenador->cant_objetivos){

                char* entrenador_sin_lugar = string_new();
                string_append(&entrenador_sin_lugar, "El entrenador ");
                char* entrenador_tid_sin_lugar = string_itoa(entrenador->tid);
                string_append(&entrenador_sin_lugar, entrenador_tid_sin_lugar);
                string_append(&entrenador_sin_lugar, " tiene lugar para atrapar mas pokemons, procede a llamar al algoritmo de cercania ");
                log_info(logger,entrenador_sin_lugar);
                free(entrenador_sin_lugar);
                free(entrenador_tid_sin_lugar);

                entrenador->razon_bloqueo = ESPERANDO_POKEMON;
                // Llamo al algoritmo de cercania ya que puede haber pokemones sin asignar
                algoritmo_de_cercania();

                // No tengo mas espacio para recibir pokemones
            }else{

                //TODO: Verificar si el otro entrenador que vino de una resolucion de deadlock termino sus objetivos y sacarlo de la lista correspondiente
                list_remove(estado_exec, 0);
                entrenador->razon_bloqueo = SIN_ESPACIO; // En realidad esto significa que no tengo mas lugar para atrapar
                entrenador->estado = BLOCK;

                // LLamo al algoritmo de deadlock para ver si hay otro entrenador con cosillas que yo necesite
                algoritmo_deadlock();
            }

            call_planner();

            //Me quedo bloqueado esperando a recibir un nuevo pokemon o algo para ejecutar
            sem_wait(&block_ready_transition[entrenador->tid]);
        }
    }

    int bloqueado_estado = list_size(estado_block);
    int ready_estado = list_size(estado_ready);

    if(list_size(estado_ready) > 0){
        call_planner();
    }else{
        algoritmo_de_cercania();
        if(list_size(estado_ready) > 0) {
            call_planner();
        } else{
            algoritmo_deadlock();
            call_planner();
        }
    }

    // TODO: liberar la memoria reservada:
    //  - Tiempo de llegada
    pthread_exit(NULL);

}

bool objetivos_cumplidos(Entrenador* entrenador){

    bool todosCumplen = true;

    // Itero el diccionario de objetivos particular
    void iterador(char* key, void* value){

        // Por cada iteracion verifico que se cumplan las condiciones, si alguna vez no se cumplen, va a dar false para siempre jua jua jua(risa maligna)
        todosCumplen &= (dictionary_has_key(entrenador->stock_pokemons, key) && (*(int*)dictionary_get(entrenador->stock_pokemons,key) == *(uint32_t*) value));
    }
    dictionary_iterator(entrenador->objetivos_particular, iterador);

    return todosCumplen;
}

//----------------------------------------COMUNICACION ENTRENADORES----------------------------------------//

void send_message_thread(void* message, int size, MessageType header, int tid) {

    // Creo un paquete con el mensaje, su tamaño y el header para poderselo pasar al thread
    void* message_package = create_message_package(message, size, header, tid);

    // Levanto un hilo detacheable para poder enviar el mensaje
    pthread_t message_thread;
    pthread_create(&message_thread, NULL, message_function, message_package);
    pthread_detach(message_thread);
}

void* create_message_package(void* message, int size, MessageType header, int tid) {
    t_new_message* message_package = malloc(sizeof(t_new_message));
    message_package->message = message;
    message_package->size = size;
    message_package->header = header;
    message_package->tid = tid;

    return (void*)message_package;
}

void* message_function(void* message_package){

    // Desarmo la estructura que me pasaron
    t_new_message* new_message_package = (t_new_message*)message_package;
    void* message = new_message_package->message;
    int size = new_message_package->size;
    MessageType header = new_message_package->header;
    int tid = new_message_package->tid;

    // Construyo mensaje a loguear en caso de que falle la comunicacion con el Broker
    char* mensajeError = string_new();
    string_append(&mensajeError, "Ha fallado la conexion con el Broker al intentar un ");
    switch (header) {
        case (GET_POK):;
            string_append(&mensajeError, "Get, ");
            break;
        case (CATCH_POK):;
            string_append(&mensajeError, "Catch, ");
            break;
        default:;
            string_append(&mensajeError, "(mensaje no soportado), ");
            break;
    }
    string_append(&mensajeError, "se realizara la opcion por default.");

    // Me conecto al Broker
    int broker = connect_to_broker();

    // Chequeo si me pude conectar al Broker
    if (broker == -1) {

        // Logueo mensaje de error
        log_info(logger, mensajeError);

        // Ejecuto accion por default
        exec_default(header,tid);

    } else {

        // Creo y envio paquete
        t_paquete *package = create_package(header);
        add_to_package(package, message, size);

        // Chequeo si pude enviar la solicitud
        if (send_package(package, broker) == -1) {

            // Logueo mensaje de error
            log_info(logger, mensajeError);

            // Ejecuto accion por default
            exec_default(header, tid);

        } else {

            // Me quedo esperando hasta que me respondan el id correlacional o la confirmacion, si falla al recibir
            // el header significa que algo en el Broker se rompio, ejecuto la accion por default
            MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
            if(receive_header(broker, buffer_header) <= 0) {

                // Logueo mensaje de error
                log_info(logger, mensajeError);

                // Ejecuto accion por default
                exec_default(header, tid);

                // Pude recibir el header, intento recibir el resto de la respuesta
            } else {

                // Recibo el id correlacional/ confirmacion
                t_list* rta_list = receive_package(broker, buffer_header); // Si llegas a fallar aca despues de todas las comprobaciones, matate

                // Id del mensaje enviado
                int id = *(int *) list_get(rta_list, 0);

                // Solo voy a utilizar el id si es un catch para filtrar los mensajes recibidos
                if (header == CATCH_POK) {

                    // Instancio una cosa y la agrego a la lista de mensajes en espera por una rta
                    WaitingMessage cosa;
                    cosa.tid = tid;
                    cosa.id_correlativo = id;

                    pthread_mutex_lock(&mutex_waiting_list);
                    list_add(waiting_list, &cosa);
                    pthread_mutex_unlock(&mutex_waiting_list);
                }

                // TODO: Limpieza
            }
        }

        // Me desconecto del Broker
        disconnect_from_broker(broker);
    }

    free(mensajeError);
    free(message);
    free(message_package);
}

void exec_default(MessageType header, int tid) {
    switch (header) {

        // Accion a realizar por default cuando hago un GET_POK y no funciona la comunicacion con el Broker
        case GET_POK:;

            // No existen locaciones para el pokemon solicitado, no tendria que hacer nada
            break;

            // Accion a realizar por default cuando hago un CATCH_POK y no funciona la comunicacion con el Broker
        case CATCH_POK:;

            // El Pokemon ha sido atrapado
            caught_pokemon(tid, 1);
            break;
        default:
            break;
    }
}

void caught_pokemon(int tid, int atrapado) {

    // Declaro si y no para hacer las compraraciones
    int si = 1;

    // Busco al entrenador que envio el catch
    bool encontrador(void* _trainer) {
        Entrenador* trainer = (Entrenador*) _trainer;
        return trainer->tid == tid;
    }
    Entrenador* entrenador = list_find(estado_block, encontrador);

    // Logueo el mensaje caught recibido
    char* caught = string_new();

    string_append(&caught, "Ha llegado un mensaje Caught indicando que un pokemon ");
    string_append(&caught, entrenador->pokemon_objetivo->especie);

    // Chequeo si lo atrape
    if (atrapado == si) {

        string_append(&caught, " ha sido atrapado por el entrenador ");

        // Le aumento el stock en uno al entrenador
        entrenador->cant_stock += 1;

        char* pok_name = entrenador->pokemon_objetivo->especie;

        // Verifico si existe el pokemon en el stock del entrenador
        if (dictionary_has_key(entrenador->stock_pokemons, pok_name)) {

            // En este caso existia, le sumo
            *(int*)dictionary_get(entrenador->stock_pokemons, pok_name) += 1;

            // Si el pokemon no existia en el diccionario, lo agrego
        } else {

            int* necesidad = (int*)malloc(sizeof(int));
            *necesidad = 1;
            dictionary_put(entrenador->stock_pokemons, pok_name, (void*) necesidad);
        }

        // TODO: falta restarlo de los pokemons globales

        // En este caso no pude atrapar el pokemon
    } else {

        string_append(&caught, " no ha sido atrapado por el entrenador ");
        // Obtengo el pokemon y libero la memoria?
        Pokemon* pok = entrenador->pokemon_objetivo;

    }
    char* trainer = string_itoa(entrenador->tid);
    string_append(&caught, trainer);
    free(trainer);
    log_info(logger, caught);

    free(caught);

    // TODO: decidir que hacer con el pokemon aca

    // Ahora el entrenador vuelve al estado block normal
    sem_post(&block_catch_transition[entrenador->tid]);
}

//----------------------------------------PLANIFICACION Y DEADLOCK----------------------------------------//

void call_planner() {

    switch (config.algoritmo_planificacion) {
        case (FIFO):
            fifo_planner();
            break;
        case (SJF_SD):
            sjf_sd_planner();
            break;
        case (SJF_CD):
            sjf_cd_planner();
            break;
        case (RR):
            rr_planner();
            break;
        default:
            printf("Este no es un algoritmo valido");
            break;

    }
}

void fifo_planner() {

    log_info(logger, "Se llamo al algoritmo FIFO ");
    // Busco si no hay ningun entrenador en ejecucion
    if (list_size(estado_exec) == 0 && list_size(estado_ready) > 0) {

        log_info(logger, "Entro a FIFO planner y no hay nadie en ejecucion ");
        // Ordeno la lista de entrenadores en ready segun el tiempo de llegada
        bool ordenar_por_llegada(void* _entrenador1, void* _entrenador2) {
            Entrenador* entrenador1 = (Entrenador*) _entrenador1;
            Entrenador* entrenador2 = (Entrenador*) _entrenador2;

            if ((entrenador1->tiempo_llegada)->tv_sec == (entrenador2->tiempo_llegada)->tv_sec) {
                return (entrenador1->tiempo_llegada)->tv_nsec < (entrenador2->tiempo_llegada)->tv_nsec;
            } else {
                return (entrenador1->tiempo_llegada)->tv_sec < (entrenador2->tiempo_llegada)->tv_sec;
            }
        }
        list_sort(estado_ready, ordenar_por_llegada);

        // Obtengo el primer entrenador de la lista ordenada
        Entrenador * entrenador_elegido = (Entrenador*)list_remove(estado_ready, 0);
        list_add(estado_exec, (void*)entrenador_elegido);

        sem_post(&ready_exec_transition[entrenador_elegido->tid] );
    }

}

void sjf_sd_planner() {}

void sjf_cd_planner() {}

void rr_planner() {}

void appeared_pokemon(t_list* paquete){

    // Id del mensaje, nunca lo usamos
    int id = *(int*)list_get(paquete, 0); // Si es -1, el mensaje viene del GameBoy, sino del Broker

    // Contenido del appeared
    void* appeared_void = list_get(paquete, 2);

    // Paso el void* recibido a t_appeared_pokemon
    t_appeared_pokemon* appearedPokemon = void_a_appeared_pokemon(appeared_void);

    // Logueo la llegada de un mensaje appeared
    char* appeared = string_new();

    string_append(&appeared, "Ha llegado un mensaje Appeared del ");
    if (id == -1) {
        string_append(&appeared, "Gameboy ");
    } else {
        string_append(&appeared, "Broker ");
    }
    string_append(&appeared, "indicando que llego un ");
    string_append(&appeared, appearedPokemon->nombre_pokemon);
    string_append(&appeared, " en la posicion (");
    char* pos_x = string_itoa(appearedPokemon->pos_x);
    string_append(&appeared, pos_x);
    free(pos_x);
    string_append(&appeared, ", ");
    char* pos_y = string_itoa(appearedPokemon->pos_y);
    string_append(&appeared, pos_y);
    free(pos_y);
    string_append(&appeared, ").");

    log_info(logger, appeared);

    free(appeared);
    //TODO: Librerar paquete_recibido

    // Validaciones para verificar si puedo meter al pokemon en la lista de pokemons

    // Chequeo si el pokemon se encuentra en el diccionario de objetivos
    if (dictionary_has_key(objetivo_global, appearedPokemon->nombre_pokemon)) {

        // Chequeo si la necesidad de pokemones de la especie recibida es mayor a 0
        if (*((int*)dictionary_get(objetivo_global, appearedPokemon->nombre_pokemon)) > 0) {

            // Se cumplio tod.o

            // Instancio la estructura pokemon y le seteo todos los parametros recibidos antes
            Pokemon *pokemon = (Pokemon*) malloc(sizeof(Pokemon));
            pokemon->especie = appearedPokemon->nombre_pokemon;
            pokemon->coordenada.pos_x = appearedPokemon->pos_x;
            pokemon->coordenada.pos_y = appearedPokemon->pos_y;

            // Agrego el pokemon a la lista de pokemones recibidos
            pthread_mutex_lock(&mutex_pokemon);
            list_add(pokemons, pokemon);
            pthread_mutex_unlock(&mutex_pokemon);

            // Llamo al algoritmo de cercania
            algoritmo_de_cercania();
        }
    }

    // Destruyo el paquete recibido
    //list_destroy(paquete); Si lo destruyo despues no voy a tener el id para confirmarle la recepcion al Broker
}

void algoritmo_de_cercania() {

    if (list_size(pokemons) > 0) {
        pthread_mutex_lock(&mutex_algoritmo_cercania);
        log_info(logger, "Se ha iniciado el algoritmo de cercania ");

        //Filtro los entrenadores que no pueden atrapar mas pokemons porque llegaron al limite
        bool puede_ir_ready(void *_entreador) {
            Entrenador *entrenador = (Entrenador *) _entreador;
            return (entrenador->cant_stock < entrenador->cant_objetivos) &&
                   (entrenador->razon_bloqueo == ESPERANDO_POKEMON);
        }
        t_list *entrenadores_con_margen = list_filter(estado_block, puede_ir_ready);

        //Pongo todos en una lista asi es mas facil trabajar
        list_add_all(entrenadores_con_margen, estado_new);

        //Verifico que la lista de entrenadores no sea vacia
        if (list_size(entrenadores_con_margen) > 0) {
            int cant_pok = list_size(pokemons);
            while (cant_pok > 0) {
                Pokemon *pokemon = (Pokemon *) list_get(pokemons, 0);

                //Verifico que la lista tenga mas de un entrenador porque sino rompe todo
                if (list_size(entrenadores_con_margen) > 1) {

                    //Ordeno a los entrenadores por cercania al pokemon
                    bool entrenador_mas_cerca(void *_entrenador_actual, void *_entrenador_siguiente) {
                        Entrenador *entrenador_actual = (Entrenador *) _entrenador_actual;
                        Entrenador *entrenador_siguiente = (Entrenador *) _entrenador_siguiente;

                        return distancia(entrenador_actual->pos_actual, pokemon->coordenada) <
                               distancia(entrenador_siguiente->pos_actual, pokemon->coordenada);
                    }
                    list_sort(entrenadores_con_margen, entrenador_mas_cerca);
                }
                //Obtengo el primero que es el mas cercano
                Entrenador *entrenador_cercano = (Entrenador *) list_get(entrenadores_con_margen, 0);

                //Removemos al entrenador de la lista entrenadores con margen ya que esta ordenada por cercania
                list_remove(entrenadores_con_margen, 0);

                bool remover(void *_entrenador) {
                    Entrenador *entrenador = (Entrenador *) _entrenador;
                    return entrenador->tid == entrenador_cercano->tid;
                }
                //Removemeos al entrenador de la lista del estado correspondiente ya sea NEW o BLOCK
                switch (entrenador_cercano->estado) {
                    case (NEW):;

                        list_remove_by_condition(estado_new, remover);

                        entrenador_cercano->pokemon_objetivo = pokemon;
                        entrenador_cercano->razon_movimiento = CATCH;
                        list_add(estado_ready, entrenador_cercano);
                        entrenador_cercano->estado = READY;

                        sem_post(&new_ready_transition[entrenador_cercano->tid]);
                        break;

                    case (BLOCK):;

                        list_remove_by_condition(estado_block, remover);

                        entrenador_cercano->pokemon_objetivo = pokemon;
                        entrenador_cercano->razon_movimiento = CATCH;
                        list_add(estado_ready, entrenador_cercano);
                        entrenador_cercano->estado = READY;

                        sem_post(&block_ready_transition[entrenador_cercano->tid]);
                        break;

                    default:
                        break;
                }

                //Aca siempre es cero porque cuando hago un remove del elemento anterior todos se corren uno menos
                pthread_mutex_lock(&mutex_pokemon);
                list_remove(pokemons, 0);
                pthread_mutex_unlock(&mutex_pokemon);
                cant_pok--;

            }
            log_info(logger, "Ha terminado el algoritmo de cercania");
        }else{
            log_info(logger, "No hay entrenadores disponibles para atrapar un pokemon");
        }
        list_destroy(entrenadores_con_margen);
        pthread_mutex_unlock(&mutex_algoritmo_cercania);
    }
}

void algoritmo_deadlock(){

    pthread_mutex_lock(&mutex_deadlock);
    // Logueo el inicio del algoritmo de deteccion de deadlock
    log_info(logger,"Se ha iniciado el algoritmo de deteccion de deadlock");

    //Filtro la lista de entrenadores que no pueden atrapar mas pokemons
    bool _entrenadores_sin_margen(void* _entrenador){
        Entrenador* entrenador = (Entrenador*) _entrenador;
        return (entrenador->cant_objetivos == entrenador->cant_stock) && (entrenador->razon_bloqueo != DEADLOCK);
    }
    t_list* entrenadores_sin_margen = list_filter(estado_block, _entrenadores_sin_margen);

    int tamanio_ent = list_size(entrenadores_sin_margen);

    // Hay mas de un entrenador en deadlock, si hay menos de dos no hago nada y salgo
    if(tamanio_ent > 1 ) {

        log_info(logger,"Hay mas de un entrenador en deadlock");

        int cont_primero = 0;
        int cont_segundo = 1;
        bool cumplio_condiciones = false;

        while(cont_primero < tamanio_ent && !cumplio_condiciones) {

            //Agarro el primer entrenador bloqueado
            Entrenador *entrenador_primero = (Entrenador *) list_get(entrenadores_sin_margen, cont_primero);


            while (cont_segundo < tamanio_ent) {
                Entrenador *entrenador_segundo = (Entrenador *) list_get(entrenadores_sin_margen, cont_segundo);
                //Me devuelve un listado de pokemons que se repiten entre lo que necesita el primer entrenador y lo que tiene el segundo entrenador en stock
                t_list* repeat_pokemon = dictionary_contains(entrenador_segundo->stock_pokemons, entrenador_primero->objetivos_particular);

                log_info(logger, "Se calculo los pokemons que tiene el entrenador segundo y necesita el primero");

                if (list_size(repeat_pokemon) > 0) {
                    //Devuelve listado de pokemons que no los tiene como objetivo y los necesita el primer entrenador
                    t_list *unnecesary_pokemon = trainer_dont_need(entrenador_segundo, repeat_pokemon);
                    log_info(logger, "Se calculo la lista que necesita el primer entrenador y no necesita el segundo entrenador (lista definitiva)");

                    if (list_size(unnecesary_pokemon) > 0) {

                        //Calculo que pokemon no le sirve al primer entrenador
                        char* pokemon_primer_entrenador_no_necesita = (char*) malloc(sizeof(char*) * 50);
                        int i = 0;
                        void no_necesita(char* key, void*value){
                            //Busco el pokemon que no necesita el entrenador, es decir el pokemon que tiene en stock pero no esta en sus objetivos. La variable i es para agarrar al primero que encuentre
                            if(!dictionary_has_key(entrenador_primero->objetivos_particular, key) && (i == 0)){
                                pokemon_primer_entrenador_no_necesita = key;
                                i++;
                            }
                            else{
                                //Si lo tiene como objetivo quiere decir que tiene en stock uno mas del que deberia
                                if((dictionary_has_key(entrenador_primero->objetivos_particular, key)) && (i == 0)){
                                    int cantidad_objetivo = *(int*) dictionary_get(entrenador_primero->objetivos_particular,key);
                                    int cantidad_stock = *(int*) dictionary_get(entrenador_primero->stock_pokemons,key);
                                    if(cantidad_stock > cantidad_objetivo){
                                    pokemon_primer_entrenador_no_necesita = key;
                                    i++;
                                    }
                                }
                            }
                        }
                        dictionary_iterator(entrenador_primero->stock_pokemons,no_necesita);

                        char* deadlock_entrenadores = string_new();
                        string_append(&deadlock_entrenadores,"El entrenador ");
                        char* entrenador_primero_tid = string_itoa(entrenador_primero->tid);
                        string_append(&deadlock_entrenadores, entrenador_primero_tid);
                        string_append(&deadlock_entrenadores, " y el entrenador ");
                        char* entrenador_segundo_tid = string_itoa(entrenador_segundo->tid);
                        string_append(&deadlock_entrenadores, entrenador_segundo_tid);
                        string_append(&deadlock_entrenadores, " estan en deadlock ");
                        log_info(logger, deadlock_entrenadores);

                        free(deadlock_entrenadores);
                        free(entrenador_primero_tid);
                        free(entrenador_segundo_tid);

                        entrenador_primero->entrenador_objetivo = entrenador_segundo;
                        entrenador_primero->pokemon_objetivo->especie = (char*) list_get(unnecesary_pokemon,0) ;
                        entrenador_primero->razon_movimiento = RESOLUCION_DEADLOCK;
                        entrenador_primero->razon_bloqueo = DEADLOCK;
                        entrenador_segundo->razon_bloqueo = DEADLOCK;
                        //strcpy(entrenador_segundo->pokemon_objetivo->especie, pokemon_primer_entrenador_no_necesita);
                        entrenador_segundo->pokemon_objetivo->especie = pokemon_primer_entrenador_no_necesita;

                        bool remover(void *_entrenador) {
                            Entrenador *entrenador = (Entrenador *) _entrenador;
                            return entrenador->tid == entrenador_primero->tid;
                        }
                        list_remove_by_condition(estado_block, remover);

                        list_add(estado_ready, entrenador_primero);
                        entrenador_primero->estado = READY;
                        cumplio_condiciones = true;
                        //TODO: Liberar listas falopa que hicimos
                        sem_post(&block_ready_transition[entrenador_primero->tid]);
                        break;
                    }
                }
                //No hay ningun pokemon ni se cumplieron las condiciones para que se realice el intercambio
                cont_segundo++;
            }

            cont_primero++;
            cont_segundo = cont_primero + 1;
        }
    }else {
        log_info(logger, "No hay dos entrenadores en deadlock");
    }
    list_destroy(entrenadores_sin_margen);
    pthread_mutex_unlock(&mutex_deadlock);
}

void remover_de_stock(Entrenador* entrenador, char* pokemon_first_trainer, char* pokemon_second_trainer){

    //Remuevo del stock el pokemon que voy a dar para el primer entrenador
    if(*(int*)dictionary_get(entrenador->stock_pokemons, pokemon_second_trainer) > 1){
        *(int*) dictionary_get(entrenador->stock_pokemons, pokemon_second_trainer)-=1;
    }
    else{
        dictionary_remove(entrenador->stock_pokemons, pokemon_second_trainer);
    }

    //Remuevo del stock el pokemon que voy a dar para el segundo entrenador
    if(*(int*)dictionary_get(entrenador->entrenador_objetivo->stock_pokemons, pokemon_first_trainer) > 1){
        *(int*)dictionary_get(entrenador->entrenador_objetivo->stock_pokemons, pokemon_first_trainer)-=1;
    }
    else{
        dictionary_remove(entrenador->entrenador_objetivo->stock_pokemons, pokemon_first_trainer);
    }

}

t_list* trainer_dont_need(Entrenador* entrenador, t_list* pokemon_array){

    t_list* pokemons_innecesarios;
    bool filtrar_pokemons(void* _nombre_pokemon){
        char* nombre_pokemon = (char*) _nombre_pokemon;
        return !dictionary_has_key(entrenador->objetivos_particular,nombre_pokemon);
    }
    pokemons_innecesarios = list_filter(pokemon_array,filtrar_pokemons);

    return pokemons_innecesarios;
}


t_list* dictionary_contains(t_dictionary* stock_pokemons, t_dictionary* objetivos_pokemons){

    t_list* pokemons_estimado = list_create();

    //TODO: Sacar la clave cuando el value es 0
    void iterador(char* key, void* _value){
        if(dictionary_has_key(stock_pokemons,key) && (*(int*) _value > 0)){
            list_add(pokemons_estimado,key);
        }
    }
    dictionary_iterator(objetivos_pokemons,iterador);

    return pokemons_estimado;
}

//----------------------------------------HELPERS----------------------------------------//

void free_list(t_list* received, void(*element_destroyer)(void*)){
    list_destroy_and_destroy_elements(received, element_destroyer);
}

struct timespec get_time(){
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    return start;
}

t_interval* new_interval(){
    t_interval* iteration = malloc(sizeof(t_interval));
    iteration->start_time = malloc(sizeof(struct timespec));
    iteration->end_time = malloc(sizeof(struct timespec));
    return iteration;
}

long timespec_to_us(struct timespec* timespec){
    return (timespec->tv_sec * 1000000) + (timespec->tv_nsec / 1000);
}

void time_diff(struct timespec* start, struct timespec* end, struct timespec* diff){

    //Verifico si la resta entre final y principio da negativa
    if ((end->tv_nsec - start->tv_nsec) < 0)
    {
        //Verifico si la suma entre el acumulado y el nuevo tiempo es mayor a 999999999ns (casi un segundo), me excedi del limite
        if((diff->tv_nsec + (end->tv_nsec - start->tv_nsec + 1000000000)) > 999999999){

            diff->tv_sec += (end->tv_sec - start->tv_sec);
            diff->tv_nsec += (end->tv_nsec - start->tv_nsec);
        } else {

            diff->tv_sec += (end->tv_sec - start->tv_sec - 1);
            diff->tv_nsec += (end->tv_nsec - start->tv_nsec + 1000000000);
        }
    }
    else
    {
        //Verifico si la suma entre el acumulado y el nuevo tiempo es mayor a 999999999ns (casi un segundo), me excedi del limite
        if((diff->tv_nsec + (end->tv_nsec - start->tv_nsec)) > 999999999){

            diff->tv_sec += (end->tv_sec - start->tv_sec + 1);
            diff->tv_nsec += (end->tv_nsec - start->tv_nsec - 1000000000);
        } else {

            diff->tv_sec += (end->tv_sec - start->tv_sec);
            diff->tv_nsec += (end->tv_nsec - start->tv_nsec);
        }
    }
}

int distancia(Coordenada actual, Coordenada siguiente) {

    int pos_actual_x = actual.pos_x;
    int pos_actual_y = actual.pos_y;
    int pos_destino_x = siguiente.pos_x;
    int pos_destino_y = siguiente.pos_y;

    int dist_en_x = abs(pos_actual_x - pos_destino_x);
    int dist_en_y = abs(pos_actual_y - pos_destino_y);
    return dist_en_x + dist_en_y;
}

void free_resources(){

    config_destroy(config_file);
    log_destroy(logger);
    pthread_mutex_destroy(&mutex_pokemon);
}

void free_splitted_arrays(char ** elements, int cant) {
    for(int i = 0; i < cant; i++)
        free(elements[i]);
    free(elements);
}

int funcion_de_mierda(char** mierda) {
    int i = 0;
    // Itero el array de mierda hasta hallar el ultimo elemento(un NULL)
    for (char *mierda2 = *mierda; mierda2; mierda2 = *++mierda) {
        i++;
    }
    return i;
}