#include "team.h"

/**
 * Declaracion de variables globales
 */

// Estructuras de configuracion de Team, la segunda es solo temporal
TEAMConfig config;
t_config *config_file;

// Logger de Team
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

pthread_t* thread_deadlock;

// Array de semaforos que bloquean a los hilos para pasar de new a ready
sem_t* new_ready_transition;

// Array de semaforos que bloquean a los hilos para pasar de ready a execute
sem_t* ready_exec_transition;

// Array de semaforos que bloquean a los hilos para pasar de block a ready
sem_t* block_ready_transition;

// Array de semaforos que bloquean a los hilos que estan esperando un catch
sem_t* block_catch_transition;

//Semaforo que se encarga de verificar que la lista de pokemons no este vacia
sem_t* s_cantidad_pokemons;

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

// TODO: falta una lista mas, esta lista solo va  tener los pokemones
//  recibidos(nombre, no estructura) por Localized, ya que segun el enunciado una vez
//  que se recibe un pokemon no hay que seguir aceptando pokemones de
//  la misma especie. https://docs.google.com/document/d/1be91Gn93O2Vp8frZoV1i5CmtOG0scE1PS8dMHsCP314/edit#heading=h.46r0co2

int main() {
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

    //Creo el servidor para que el GameBoy me mande mensajes
    pthread_create(&server_thread, NULL, server_function, NULL);

    //Creo 3 hilos para suscribirme a las colas globales
    subscribe_to_queues();

    // Inicializo estructuras, semaforos e hilos
    initialize_structures();

    // TODO: joinear hilos de entrenadores

    //Joineo el hilo main con el del servidor para el GameBoy
    pthread_join(server_thread, NULL);

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

int connect_to_broker(){

    int client_socket;
    if((client_socket = create_socket()) == -1) {
        log_error(logger, "Error al crear el socket de cliente");
        return -1;
    }
    if(connect_socket(client_socket, config.ip_broker, config.puerto_broker) == -1){
        log_error(logger, "Error al conectarse al Broker");
        return -1;
    }
    return client_socket;
}

void disconnect_from_broker(int broker_socket) {
    close_socket(broker_socket);
}

void* subscribe_to_queue_thread(void* arg) {
    MessageType cola = *(MessageType*)arg;

    // Me intento conectar y suscribir, la funcion no retorna hasta que no lo logre
    int broker = connect_and_subscribe(cola);
    log_info(logger, "Subscribed to queue");

    // Reservo cachito de memoria para confirmar los mensajes que le envio al Broker
    int* confirmacion = malloc(sizeof(int));
    *confirmacion = ACK;

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
            int idCorrelativo = *(int*) list_get(rta_list, 0);

            bool encontrador(void* _nombre) {
                return string_equals_ignore_case(pokName, (char*)_nombre);
            }

            // Switch case que seleccione que hacer con la respuesta segun el tipo de cola
            switch (cola) {

                case (APPEARED_POK):;

                    appeared_pokemon(rta_list);

                    break;

                case (LOCALIZED_POK):;

                    // Obtengo el paquetito de appeared
                    t_localized_pokemon* localizedPokemon = void_a_localized_pokemon(list_get(rta_list, 1));

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

                            // Hago signal en el semaforo que cuenta la cantidad de pokemones libres a entregar
                            sem_post(s_cantidad_pokemons);
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
                    t_caught_pokemon* caughtPokemon = void_a_caught_pokemon(list_get(rta_list, 1));

                    // Quito el mensaje que estaba en espera que tenga el mmismo idCorrelacional que el recibido
                    pthread_mutex_lock(&mutex_waiting_list);
                    bool hallar_entrenador(void* _mensaje) {
                        WaitingMessage* mensaje = (WaitingMessage*)_mensaje;
                        return mensaje->id_correlativo === idCorrelativo;
                    }
                    WaitingMessage* mensaje = (WaitingMessage*)list_remove_by_condition(waiting_list, hallar_entrenador);
                    pthread_mutex_unlock(&mutex_waiting_list);

                    // Llamo a la funcion que resuelve los caughts
                    caught_pokemon(mensaje->tid, caughtPokemon->atrapado);
                    break;

            }

            // Creo paquete para confirmar recepcion de mesaje al Broker
            t_paquete *package = create_package(cola);
            add_to_package(package, confirmacion, sizeof(int));

            // Envio confirmacion al Broker
            send_package(package, broker);

        // Si surgio algun error durante el receive header, me reconecto y vuelvo a iterar
        } else {
            log_info(logger, "Connection to queue lost\n");
            broker = connect_and_subscribe(cola);
            log_info(logger, "Resubscribed to queue\n");
        }

        // Limpieza
        free(buffer_header);
    }

    free(confirmacion);
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

void* server_function(void* arg) {

    start_log_server();

    int server_socket;

    // La creacion de nuestro socket servidor puede fallar, si falla duermo y vuelvo a intentar en n segundos
    while ((server_socket = initialize_server()) == -1) {

        sleep(config.tiempo_reconexion);
    }

    log_info(logger_server, "Server initiated");
    start_server(server_socket, &new, &lost, &incoming);

    return null;
}

void initialize_structures() {

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

    //Itero el array de posiciones de entrenadores
    for (char *coordenada = *ptr; coordenada; coordenada = *++ptr) {

        // Instancio un nuevo entrenador
        Entrenador *entrenador = (Entrenador *) malloc(sizeof(Entrenador));
        entrenador->estado = NEW;
        entrenador->objetivos_particular = dictionary_create();
        entrenador->stock_pokemons = dictionary_create();

        // Obtengo los objetivos y los pokemones que posee el entrenador actual
        char **objetivos_entrenador = string_split(config.objetivos_entrenadores[pos], "|");
        char **pokemon_entrenador = string_split(config.pokemon_entrenadores[pos], "|");
        char **posiciones = string_split(coordenada, "|");

        // Seteo los objetivos globales
        add_global_objectives(objetivos_entrenador, pokemon_entrenador);

        // Seteo los objetivos del entrenador y los que ya tiene
        add_to_dictionary(objetivos_entrenador, entrenador->objetivos_particular);
        add_to_dictionary(pokemon_entrenador, entrenador->stock_pokemons);

        // Buscamos y seteamos la cantidad de Pokemones maxima que puede tener el entrenador
        int contador_objetivos = 0;
        void iterador_objetivos(char* clave, void* contenido){
            contador_objetivos += *(int*) contenido;
        }
        dictionary_iterator(entrenador->objetivos_particular,iterador_objetivos);
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

    // Reservo espacio de memoria para el semaforo de la lista de Pokemones
    s_cantidad_pokemons = malloc(sizeof(sem_t));

    // Inicializo el semaforo en 0 porque no hay pokemons todavia
    sem_init(s_cantidad_pokemons,0,0);

    // Itero la lista de pokemons objetivos y realizo todos los gets correspondientes
    void iterador_pokemons(char* clave, void* contenido){

        // Creo la estructura de pokemon para mandarle al Broker
        t_get_pokemon* get_pok = create_get_pokemon(clave);

        // Paso la estructura a void
        void* pok_void = get_pokemon_a_void(get_pok);

        // Envio el get al Broker con un hilo
        send_message_thread(pok_void, sizeof_get_pokemon(pok_void), GET_POK, -1);
    }
    dictionary_iterator(objetivo_global, iterador_pokemons);

    pthread_create(&thread_deadlock,NULL, (void*) algoritmo_deadlock,NULL);
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

    // Bloqueo la transicion new - ready hasta que haya algun pokemon a capturar(proveniente de mensajes Appeared o Localized)
    sem_wait( &new_ready_transition[entrenador->tid] );

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

        // TODO: hay que ir acumulando el tiempo ejecutado en algun lugar para los calculos del SJF y del RR
        // Hallo la distancia hasta el destino, duermo al hilo durante x cantidad de segundos para simular ciclos de CPU

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

        // Esto es solo valido para FIFO y SJF-SD, en RR debo suspender el ciclo despues de cumplido el Q y en SJF-CD(verificar en cada iteracion?) no se
        while (distancia_a_viajar > 0) {
            sleep(config.retardo_ciclo_cpu);
            distancia_a_viajar--;
        }

        // Ya viaje a destino, ahora tengo que solicitar el catch o realizar el intercambio
        switch (entrenador->razon_movimiento) {

            // Estaba viajando para atrapar un Pokemon
            case (CATCH):;

                // TODO: Las listas que uso abajo deben estar protegidas por semaforos mutex

                // Me quito de la lista de ejecucion
                list_remove(estado_exec, 0);

                // Me asigno la razon de bloqueo ESPERANDO_CATCH y me agrego a la lista de bloqueo
                entrenador->razon_bloqueo = ESPERANDO_CATCH;
                list_add(estado_block, entrenador);

                // Creo la estructura t_catch_pokemon para enviarle al Broker
                Pokemon* pok = entrenador->pokemon_objetivo;
                t_catch_pokemon* pokemon_to_catch = create_catch_pokemon(pok->especie, pok->coordenada.pos_x, pok->coordenada.pos_y);

                // Llamo a la funcion para enviar un mensaje en un hilo y envio la estructura que cree antes
                send_message_thread(catch_pokemon_a_void(pokemon_to_catch), sizeof_catch_pokemon(pokemon_to_catch), CATCH_POK, entrenador->tid);

                // Me bloqueo esperando la rta del Broker
                sem_wait(&block_catch_transition[entrenador->tid]);

                break;

            // Estaba viajando para intercambiar un Pokemon
            case (RESOLUCION_DEADLOCK):;

                // TODO: fijarse que esto tambien deberia ser interrumpible por el planificador

                // El intercambio ocupa 5 ciclos de CPU
                int retardo_simulacion = 5;
                while (retardo_simulacion > 0) {
                    sleep(config.retardo_ciclo_cpu);
                    retardo_simulacion--;
                }

                // Terminar de implementar
                break;
        }

        // Verifico si el entrenador completo sus objetivos
        //bool objetivos_cumplidos = false;
        if (objetivos_cumplidos(entrenador)) {

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

                    list_remove(estado_exec, 0);
                    break;
            }

            // Le asigno el estado finish
            entrenador->estado = FINISH;

            // TODO: agregar a la lista de Finish?

        // En este caso no cumpli mis objetivos aun, debo quedarme en bloqueo
        } else {

            // Chequeo si venia de una resolucion de deadlock
            if (entrenador->razon_movimiento == RESOLUCION_DEADLOCK) {

                // Me quito de la lista de Ejecucion
                list_remove(estado_exec, 0);

                // Me agrego a la lista de Bloqueo
                list_add(estado_block, entrenador);
            }
            // Actualizo la razon de bloqueo
            entrenador->razon_bloqueo = ESPERANDO_POKEMON;

            // Me quedo bloqueado esperando a recibir un nuevo pokemon o algo para ejecutar
            sem_wait(&block_ready_transition[entrenador->tid]);
        }
    }

    // TODO: liberar la memoria reservada:
    //  - Tiempo de llegada
    return null;
}

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

    // Busco si no hay ningun entrenador en ejecucion
    if (list_size(estado_exec) == 0) {

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
        Entrenador * entrenador_elegido = (Entrenador*)list_get(estado_ready, 0);

        sem_post(&ready_exec_transition[entrenador_elegido->tid] );

    }

}

void sjf_sd_planner() {}

void sjf_cd_planner() {}

void rr_planner() {}

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
    log_info(logger_server,"Nueva conexion: Socket %d, Puerto: %d", server_socket, port);
}

void lost(int server_socket, char * ip, int port){
    log_info(logger_server, "Conexion perdida");
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
}

void appeared_pokemon(t_list* paquete){

    // TODO: implementar validaciones locas

    // Id del mensaje, nunca lo usamos
    int id = *(int*)list_get(paquete, 0); // Si es -1, el mensaje viene del GameBoy, sino del Broker

    // Contenido del appeared
    void* appeared_void = list_get(paquete, 1);

    // Paso el void* recibido a t_appeared_pokemon
    t_appeared_pokemon* appearedPokemon = void_a_appeared_pokemon(appeared_void);

    // Instancio la estructura pokemon y le seteo todos los parametros recibidos antes
    Pokemon *pokemon = (Pokemon*) malloc(sizeof(Pokemon));
    pokemon->especie = appearedPokemon->nombre_pokemon;
    pokemon->coordenada.pos_x = appearedPokemon->pos_x;
    pokemon->coordenada.pos_y = appearedPokemon->pos_y;

    // Agrego el pokemon a la lista de pokemones recibidos
    pthread_mutex_lock(&mutex_pokemon);
    list_add(pokemons, pokemon);
    pthread_mutex_unlock(&mutex_pokemon);

    // Hago un signal en el semaforo de pokemones para avisar que hay uno nuevo
    sem_post(s_cantidad_pokemons);

    // Destruyo el paquete recibido
    list_destroy(paquete);

    // LLamo al algoritmo de cercania
    algoritmo_de_cercania();
}

void algoritmo_de_cercania(){

    sem_wait(s_cantidad_pokemons);
    //Filtro los entrenadores que no pueden atrapar mas pokemons porque llegaron al limite
    bool puede_ir_ready(void* _entreador){
        Entrenador* entrenador = (Entrenador*) _entreador;
        return (entrenador->cant_stock < entrenador->cant_objetivos) && (entrenador->razon_bloqueo == ESPERANDO_POKEMON);
    }
    t_list* entrenadores_con_margen = list_filter(estado_block, puede_ir_ready);

    //Pongo todos en una lista asi es mas facil trabajar
    list_add_all(entrenadores_con_margen,estado_new);

    //Verifico que la lista de entrenadores no sea vacia
    if(list_size(entrenadores_con_margen) > 0){
        int cant_pok = list_size(pokemons);
        while(cant_pok > 0){
            Pokemon* pokemon = (Pokemon*) list_get(pokemons,0);

            //Verifico que la lista tenga mas de un entrenador porque sino rompe todo
            if(list_size(entrenadores_con_margen) > 1){

                //Ordeno a los entrenadores por cercania al pokemon
                bool entrenador_mas_cerca(void* _entrenador_actual, void* _entrenador_siguiente){
                    Entrenador* entrenador_actual = (Entrenador*) _entrenador_actual;
                    Entrenador* entrenador_siguiente = (Entrenador*) _entrenador_siguiente;

                    return distancia(entrenador_actual->pos_actual, pokemon->coordenada) < distancia(entrenador_siguiente->pos_actual, pokemon->coordenada);
                }
                list_sort(entrenadores_con_margen, entrenador_mas_cerca);
            }
            //Obtengo el primero que es el mas cercano
            Entrenador* entrenador_cercano = (Entrenador*) list_get(entrenadores_con_margen,0);

            //Removemos al entrenador de la lista entrenadores con margen ya que esta ordenada por cercania
            list_remove(entrenadores_con_margen, 0);

            bool remover(void* _entrenador){
                Entrenador* entrenador = (Entrenador*) _entrenador;
                return entrenador->tid == entrenador_cercano->tid;
            }
            //Removemeos al entrenador de la lista del estado correspondiente ya sea NEW o BLOCK
            switch(entrenador_cercano->estado){
                case (NEW):;

                    list_remove_by_condition(estado_new, remover);

                    entrenador_cercano->pokemon_objetivo = pokemon;
                    entrenador_cercano->razon_movimiento = CATCH;
                    list_add(estado_ready,entrenador_cercano);
                    entrenador_cercano->estado = READY;

                    sem_post( &new_ready_transition[entrenador_cercano->tid]);
                    break;

                case (BLOCK):;

                    list_remove_by_condition(estado_block, remover);

                    entrenador_cercano->pokemon_objetivo = pokemon;
                    entrenador_cercano->razon_movimiento = CATCH;
                    list_add(estado_ready,entrenador_cercano);
                    entrenador_cercano->estado = READY;

                    sem_post(&block_ready_transition[entrenador_cercano->tid]);
                    break;

                default:
                    printf("Andate a a concha de tu hermana");
                    break;
            }

            //Aca siempre es cero porque cuando hago un remove del elemento anterior todos se corren uno menos
            pthread_mutex_lock(&mutex_pokemon);
            list_remove(pokemons,0);
            pthread_mutex_unlock(&mutex_pokemon);
            cant_pok--;
        }
    }

}

void free_resources(){

    config_destroy(config_file);
    log_destroy(logger);
    pthread_mutex_destroy(&mutex_pokemon);
}


void algoritmo_deadlock(){


    sleep(config.tiempo_reconexion);

    bool _entrenadores_sin_margen(void* _entrenador){
        Entrenador* entrenador = (Entrenador*) _entrenador;
        return entrenador->cant_objetivos == entrenador->cant_stock;
    }
    t_list* entrenadores_sin_margen = list_filter(estado_block, _entrenadores_sin_margen);


}

//----------------------------------------HELPERS----------------------------------------

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

    // Me conecto al Broker
    int broker = connect_to_broker();

    // Chequeo si me pude conectar al Broker
    if (broker == -1) {

        // Ejecuto accion por default
        exec_default(header,tid);

    } else {

        // Creo y envio paquete
        t_paquete *package = create_package(header);
        add_to_package(package, message, size);

        // Chequeo si pude enviar la solicitud
        if (send_package(package, broker) == -1) {

            // Ejecuto accion por default
            exec_default(header, tid);

        } else {

            // Me quedo esperando hasta que me respondan el id correlacional o la confirmacion, si falla al recibir
            // el header significa que algo en el Broker se rompio, ejecuto la accion por default
            MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
            if(receive_header(broker, buffer_header) <= 0) {

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

                // TODO: loggear los mensajes enviados?
                // TODO: Limpieza
            }
        }

        // Me desconecto del Broker
        disconnect_from_broker(broker);
    }
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
            printf("Chupame la pija\n");
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

    // Chequeo si lo atrape
    if (atrapado == si) {

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

    // En este caso no pude atrapar el pokemon
    } else {

        // Obtengo el pokemon y libero la memoria?
        Pokemon* pok = entrenador->pokemon_objetivo;

    }

    // TODO: decidir que hacer con el pokemon aca

    // Ahora el entrenador vuelve al estado block normal
    sem_post(&block_catch_transition[entrenador->tid]);
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
