#include "team.h"

/**
 * Declaracion de vbles globales
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

// Array de semaforos que bloquean a los hilos para pasar de new a ready
sem_t* new_ready_transition;

// Array de semaforos que bloquean a los hilos para pasar de ready a execute
sem_t* ready_exec_transition;

// Array de semaforos que bloquean a los hilos para pasar de block a ready
sem_t* block_ready_transition;

//Semaforo que se encarga de verificar que la lista de pokemons no este vacia
sem_t* s_cantidad_pokemons;

// Listas de entrenadores que representan los distintos estados
t_list* estado_new;
t_list* estado_ready;
t_list* estado_exec;
t_list* estado_block;
t_list* estado_finish;

// Lista de los pokemons(Los recibidos por Appeared o Localized)
t_list* pokemons;

// Semaforo Mutex para proteger la lista de pokemones
pthread_mutex_t mutex_pokemon;

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

    // Me quedo en un loop infinito esperando a recibir cosas
    while (true) {

        MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
        if(receive_header(broker, buffer_header) > 0) {

            // Recibo la res
            t_list* rta_list = receive_package(broker, buffer_header);
            // TODO: hay que ver de filtrar los ids de los mensajes que nosotros hayamos solicitado
            // Switch case que seleccione que hacer con la respuesta segun el tipo de cola
            switch (cola) {
                case (LOCALIZED_POK):;
                    int cant = *(int*) list_get(rta_list, 1);
                    while(cant > 0) {
                        Pokemon *pokemon = (Pokemon*) malloc(sizeof(Pokemon));

                        pokemon->especie = (char*) list_get(rta_list,0);
                        pokemon->coordenada.pos_x = *(int*) list_get(rta_list,cant*2);// Suponiendo que la informacion viene: nombre, cant, [coordenadas], esto funciona
                        pokemon->coordenada.pos_y = *(int*) list_get(rta_list,cant*2 + 1);

                        pthread_mutex_lock(&mutex_pokemon);
                        list_add(pokemons, pokemon);
                        pthread_mutex_unlock(&mutex_pokemon);
                        sem_post(&s_cantidad_pokemons);
                        cant --;
                    }

                    algoritmo_de_cercania(NULL);
                    break;
                case (CAUGHT_POK):
                    break;

            }
            // TODO: confirmar la recepcion con un send que mande un 1 o un ACK o algo de eso

            // Limpieza
            free(buffer_header);

        // Si surgio algun error durante el receive header, me reconecto y vuelvo a iterar
        } else {
            log_info(logger, "Connection to queue lost\n");
            broker = connect_and_subscribe(cola);
            log_info(logger, "Resubscribed to queue\n");
        }
    }
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

    //Inicializo el semaforo mutex y la lista de pokemons para conocer instancias de pokemons en el mapa
    pthread_mutex_init(&mutex_pokemon, NULL);
    pokemons = list_create();

    // Inicializo las listas de estados y otras cosas
    char **ptr = config.posiciones_entrenadores;
    int pos = 0, tamanio_entrenadores = 0;
    objetivo_global = dictionary_create();
    estado_new = list_create();
    estado_ready = list_create();
    estado_block = list_create();
    estado_finish = list_create();
    estado_exec = list_create();

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
        dictionary_iterator(entrenador->stock_pokemons,iterador_stock);
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

    // Itero los entrenadores, inicializamos los semaforos y el hilo correspondiente a cada uno
    for (int count = 0; count < tamanio_entrenadores; count++) {

        // Inicializo el semaforo correspondiente al entrenado en 0 para que quede bloqueado
        sem_init(&new_ready_transition[count], 0, 0);
        // Inicializo el semaforo correspondiente al entrenado en 0 para que quede bloqueado
        sem_init(&ready_exec_transition[count], 0, 0);
        // Inicializo el semaforo correspondiente al entrenado en 0 para que quede bloqueado
        sem_init(&block_ready_transition[count], 0, 0);
        Entrenador* entrenador_actual = (Entrenador*) list_get(estado_new, count);
        pthread_create(&threads_trainer[count], NULL, (void *) trainer_thread, (void *) entrenador_actual);
    }
    //Inicializo el semaforo en 0 porque no hay pokemons todavia
    sem_init(&s_cantidad_pokemons,0,0);

    // Itero la lista de pokemons objetivos y realizo todos los gets correspondientes
    void iterador_pokemons(char* clave, void* contenido){
        send_message_thread((void*) clave, string_length(clave), GET_POK, -1);
    }
    dictionary_iterator(objetivo_global, iterador_pokemons);
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

    entrenador->tiempo_llegada = malloc(sizeof(struct timespec));

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

        if(entrenador->razon_movimiento == CATCH){
            distancia_a_viajar = distancia(entrenador->pos_actual, entrenador->pokemon_objetivo->coordenada);
        }
        else{
            distancia_a_viajar = distancia(entrenador->pos_actual, entrenador->entrenador_objetivo->pos_actual);
        }

        // Esto es solo valido para FIFO y SJF-SD, en RR debo suspender el ciclo despues de cumplido el Q y en SJF-CD no se
        while (distancia_a_viajar > 0) {
            sleep(config.retardo_ciclo_cpu);
            distancia_a_viajar--;
        }

        //FIXME: Pasos a seguir:
        // 1. Verificar cual es la razon del movimiento (CATCH / Resolucion de deadlock)
        // 2. Si es CATCH, enviar el mensaje correspondiente y bloquearse
        // 2.1 Una vez que me hacen signal sobre el semaforo (CAUGHT), verificar si lo atrape o no y actualizar
        // la lista de objetivos globales y stock de pokemons.
        // 3. Si es resolucion de deadlock, simular 5 ciclos de CPU y realizar el intercambio entre dos entrenadores.
        // 4. Como sabemos con que entrenador tiene que hacer el intercambio?
        // 5. Si ya cumplimos con los objetivos, asignar el estado FINISH, nos borramos de todas las listas y llamar al
        // algoritmo de cercania para ver si hay otro hilo que se puede liberar.
        // 2.2 (Continuacion de 2.1) Una vez que respondieron el CAUGHT llamar al algoritmo de cercania.

        sem_wait(&block_ready_transition[entrenador->tid]);
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

    Pokemon *pokemon = (Pokemon*) malloc(sizeof(Pokemon));

   //TODO: ver de pasar los parametro con un memcpy y despues liberar y destruir todo el paquete
    pokemon->especie = (char*) list_get(paquete,0);
    pokemon->coordenada.pos_x = *(int*) list_get(paquete,1);
    pokemon->coordenada.pos_y = *(int*) list_get(paquete,2);

    pthread_mutex_lock(&mutex_pokemon);
    list_add(pokemons, pokemon);
    pthread_mutex_unlock(&mutex_pokemon);
    sem_post(&s_cantidad_pokemons);

    list_destroy(paquete);
    algoritmo_de_cercania(NULL);
}

void algoritmo_de_cercania(Entrenador* entrenador_exec){

    if(entrenador_exec->tid != NULL) {
        sem_wait(&s_cantidad_pokemons);
        //TODO: Esto iria aca? Porque en realidad el entrenador se bloquea cuando manda el catch por lo que solo habria que cambiar la razon de bloqueo
        //list_remove(estado_exec,0);

        int cant_pok = list_size(pokemons);
        //Verifico que haya pokemons para atrapar
        if(cant_pok > 0) {

            //Verifico que el entrenador tiene tiene espacio para seguir capturando pokemons
            if ((entrenador_exec->cant_stock < entrenador_exec->cant_objetivos) &&
                (entrenador_exec->razon_bloqueo == ESPERANDO_POKEMON)) {
                pthread_mutex_lock(&mutex_pokemon);
                bool mas_cercano(void *_pokemon_actual, void *_pokemon_siguiente) {
                    Pokemon *pokemon_actual = (Pokemon *) _pokemon_actual;
                    Pokemon *pokemon_siguiente = (Pokemon *) _pokemon_siguiente;

                    return distancia(entrenador_exec->pos_actual, pokemon_actual->coordenada) <
                           distancia(entrenador_exec->pos_actual, pokemon_siguiente->coordenada);
                }
                list_sort(pokemons, mas_cercano);
                pthread_mutex_unlock(&mutex_pokemon);

                Pokemon* pokemon = (Pokemon*) list_get(pokemons,0);

                entrenador_exec -> estado = READY;
                entrenador_exec-> razon_bloqueo = SIN_BLOQUEO;
                entrenador_exec->pokemon_objetivo = pokemon;
                list_add(estado_ready,entrenador_exec);

                pthread_mutex_lock(&mutex_pokemon);
                list_remove(pokemons,0);
                pthread_mutex_unlock(&mutex_pokemon);

            }
        } else {
            //No hay pokemons para atrapar
            entrenador_exec-> estado = BLOCK;
            entrenador_exec->razon_bloqueo = ESPERANDO_POKEMON;

            //TODO: Esto hay que hacerlo aca? Porque en realidad el entrenador ya esta bloqueado cuando mandamos el catch
            list_add(estado_block, entrenador_exec);
        }
    }
    else{
        //Quiere decir que voy a desbloquear un entrenador de NEW o BLOCK
        //Filtro los entrenadores que no pueden atrapar mas pokemons porque llegaron al limite
        bool puede_ir_ready(void* _entreador){
            Entrenador* entrenador = (Entrenador*) _entreador;
            return (entrenador->cant_stock < entrenador->cant_objetivos) && (entrenador->razon_bloqueo == ESPERANDO_POKEMON);
        }
        t_list* entrenadores_con_margen = list_filter(estado_block,puede_ir_ready);

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

    // Problema: que pasaria si entre que filtro y ordeno la lista de entrenadores,
    //  cambia el estado de algun entrenador(porque se ejecuto esta misma funcion en otro hilo)
    // Habria que poner un super Mutex para evitar que cambie el estado de los entrenadores

    // TODO: Iterar la lista de Pokemones, no olvidar el mutex para proteger la lista
}

void free_resources(){

    config_destroy(config_file);
    log_destroy(logger);
    pthread_mutex_destroy(&mutex_pokemon);
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
                int id = *(int*) list_get(rta_list, 0);


                // TODO: Enviar confirmacion con un ACK como header y con el id correlacional como contenido
            }
        }

        // Me desconecto del Broker
        disconnect_from_broker(broker);
    }
}

// TODO: verificar si esta cosa necesitaria mas parametros o no
void exec_default(MessageType header, int tid) {
    switch (header) {
        case GET_POK:
            // Accion a realizar por default cuando hago un GET_POK y no funciona la comunicacion con el Broker
            break;
        case CATCH_POK:
            // Accion a realizar por default cuando hago un CATCH_POK y no funciona la comunicacion con el Broker
            break;
        default:
            printf("Chupame la pija\n");
            break;
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