#include "team.h"

TEAMConfig config;
t_log* logger;
t_log* logger_server;
pthread_t appeared_thread;
pthread_t localized_thread;
pthread_t caught_thread;
t_config *config_file;
// Estructura clave-valor para manejar los objetivos globales, la clave es el nombre y el valor es la cantidad necesitada
t_dictionary* objetivo_global;
// Array de hilos de entrenador
pthread_t* threads_trainer;
sem_t* new_ready_transition;
sem_t* ready_exec_transition;
// Lista de los entrenadores con sus objetivos, posicion y demas cositas
t_list* entrenadores;

int main() {
    MessageType test = ABC;
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

    initialize_structures();

    //Creo 3 hilos para suscribirme a las colas globales
    subscribe_to_queues();

    //initialize_structures();

    //Esta linea esta solo de prueba
    //send_to_server(test);

    //Joineo el hilo main con el del servidor para el GameBoy, en realidad ninguno de los 2 tendria que terminar nunca
    pthread_join(server_thread, NULL);

    config_destroy(config_file);
    log_destroy(logger);
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

    //TODO: PROBAR ESTO
    // Me quedo en un loop infinito esperando a recibir cosas
    while (true) {

        MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
        if(receive_header(broker, buffer_header) > 0) {

            // Recibo la confirmacion
            t_list* rta_list = receive_package(broker, buffer_header);
            int rta = *(int*) list_get(rta_list, 0);

            // Switch case que seleccione que hacer con la respuesta segun el tipo de cola
            // TODO: confirmar la recepcion con un send que mande un 1 o un ACK o algo de eso

            // Limpieza
            free(buffer_header);
            //TODO: eliminar la lista

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

// Rodri puto
//TODO: terminar de implementar
void initialize_structures() {
    //Itero la lista de entrenadores, y creo un hilo por cada uno

    char **ptr = config.posiciones_entrenadores;
    int pos = 0, tamanio_entrenadores = 0;
    objetivo_global = dictionary_create();
    entrenadores = list_create();

    //Itero el array de posiciones de entrenadores
    for (char *coordenada = *ptr; coordenada; coordenada = *++ptr) {

        Entrenador *entrenador = (Entrenador *) malloc(sizeof(Entrenador));
        entrenador->estado = NEW;
        entrenador->objetivos_particular = dictionary_create();
        entrenador->stock_pokemons = dictionary_create();

        // Obtengo los objetivos y los pokemones que posee el entrenador actual
        char **objetivos_entrenador = string_split(config.objetivos_entrenadores[pos], "|");
        char **pokemon_entrenador = string_split(config.pokemon_entrenadores[pos], "|");
        char **posiciones = string_split(coordenada, "|");
        add_global_objectives(objetivos_entrenador, pokemon_entrenador);

        //Instancio la estructura entrenador con los datos recogidos del archivo de configuracion

        add_to_dictionary(objetivos_entrenador, entrenador->objetivos_particular);
        add_to_dictionary(pokemon_entrenador, entrenador->stock_pokemons);
        // Le asigno al entrenador sus coordenadas
        sscanf(posiciones[0], "%d", &entrenador->pos_x);
        sscanf(posiciones[1], "%d", &entrenador->pos_y);
        // Le asigno un tid falso al entrenador
        entrenador->tid = pos;

        list_add(entrenadores, (void *) entrenador);
        pos++;
    }

    //Obtengo la cantidad de entrenadores
    tamanio_entrenadores = list_size(entrenadores);

    // Creo un array de tantos hilos como entrenadores haya
    threads_trainer = (pthread_t *) malloc(tamanio_entrenadores * sizeof(pthread_t));
    // Creo un array de semaforos para bloquear la transicion new - ready
    new_ready_transition = (sem_t*) malloc(tamanio_entrenadores * sizeof(sem_t));
    // Creo un array de semaforos para bloquear la transicion ready - exec
    ready_exec_transition = (sem_t*) malloc(tamanio_entrenadores * sizeof(sem_t));
    for (int count = 0; count < tamanio_entrenadores; count++) {

        // Inicializo el semaforo correspondiente al entrenado en 0 para que quede bloqueado
        sem_init(&new_ready_transition[count], 0, 0);
        // Inicializo el semaforo correspondiente al entrenado en 0 para que quede bloqueado
        sem_init(&ready_exec_transition[count], 0, 0);
        Entrenador* entrenador_actual = (Entrenador*) list_get(entrenadores, count);
        pthread_create(&threads_trainer[count], NULL, (void *) trainer_thread, (void *) entrenador_actual);
    }

    // Itero la lista de pokemons objetivos y realizo todos los gets correspondientes
    void iterador_pokemons(char* clave, void* contenido){
        //send_message_thread(); TODO: Completar esta funcion
        //create_response_thread(broker, (void*) clave, GET_POK); TODO: Borrar esta
    }
    dictionary_iterator(objetivo_global, iterador_pokemons);

    // Iterar lista de hilos y joinear, esto habria que hacerlo en main?
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
    *(entrenador->tiempo_llegada) = get_time();

    // Bloqueo y llamo al planificador para que decida quien continua?
    while(entrenador->estado != FINISH){

        // El estado del entrenador pasa a ser ready
        entrenador->estado = READY;

        // Llamo al planificador
        call_planner();

        // Bloqueo esperando a que el planificador decida que ejecute
        sem_wait( &ready_exec_transition[entrenador->tid] );
    }
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
    bool esta_en_ejecucion(void* _entrenador) {
        Entrenador* entrenador = (Entrenador*) _entrenador;
        return entrenador->estado == EXEC;
    }
    if (!list_any_satisfy(entrenadores, esta_en_ejecucion)) {

        // Obtengo la lista de entrenadores en Ready
        bool esta_en_ready(void* _entrenador) {
            Entrenador* entrenador = (Entrenador*) _entrenador;
            return entrenador->estado == READY;
        }
        t_list* entrenadores_en_ready = list_filter(entrenadores, esta_en_ready);

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
        list_sort(entrenadores_en_ready, ordenar_por_llegada);

        // Obtengo el primer entrenador de la lista ordenada
        Entrenador * entrenador_elegido = (Entrenador*)list_get(entrenadores_en_ready, 0);

        sem_post( &ready_exec_transition[entrenador_elegido->tid] );
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

    void* mensaje = list_get(paquete_recibido,0);

    switch(headerStruct -> type){

        case APPEARED_POK:
            printf("APPEARED_POKEMON\n");
            break;
        default:
            printf("la estas cagando compa\n");
            break;
    }
    void element_destroyer(void* element){
        free(element);
    }
    free_list(paquete_recibido, element_destroyer);
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

void send_message_thread(void* message, int size, MessageType header) {

    // Creo un paquete con el mensaje, su tamaño y el header para poderselo pasar al thread
    void* message_package = create_message_package(message, size, header);

    // Levanto un hilo detacheable para poder enviar el mensaje
    pthread_t message_thread;
    pthread_create(&message_thread, NULL, message_function, message_package);
    pthread_detach(message_thread);
}

void* create_message_package(void* message, int size, MessageType header) {
    t_new_message* message_package = malloc(sizeof(t_new_message));
    message_package->message = message;
    message_package->size = size;
    message_package->header = header;

    return (void*)message_package;
}

void* message_function(void* message_package){

    // Desarmo la estructura que me pasaron
    t_new_message* new_message_package = (t_new_message*)message_package;
    void* message = new_message_package->message;
    int size = new_message_package->size;
    MessageType header = new_message_package->header;

    // Me conecto al Broker
    int broker = connect_to_broker();

    // Chequeo si me pude conectar al Broker
    if (broker == -1) {

        // Ejecuto accion por default
        exec_default(header);
        // TODO: Limpieza

    } else {

        // Creo y envio paquete
        t_paquete *package = create_package(header);
        add_to_package(package, message, size);

        // Chequeo si pude enviar la solicitud
        if (send_package(package, broker) == -1) {

            // Ejecuto accion por default
            exec_default(header);

        } else {

            MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
            if(receive_header(broker, buffer_header) <= 0) {
                return false; // TODO: ver que hacer aca
            }

            // Recibo el id correlacional o lo que sea
            t_list* rta_list = receive_package(broker, buffer_header);
            int rta = *(int*) list_get(rta_list, 0);

            // TODO: Enviar confirmacion con un ACK como header y con el id correlacional como contenido

            // Limpieza de estos ultimos 2 recv y send
        }

        // TODO: Limpieza gral
        // Me desconecto del Broker
        disconnect_from_broker(broker);
    }
}

// TODO: verificar si esta cosa necesitaria mas parametros o no
void exec_default(MessageType header) {
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

//Funcion de prueba
void send_to_server(MessageType mensaje){
    int broker = connect_to_broker();
    t_paquete* paquete = create_package(mensaje);


    char* enviar = malloc(50);
    strcpy(enviar, "test");

    add_to_package(paquete,(void*) enviar, strlen("test")+1);


    if(send_package(paquete, broker)  == -1){
        printf("No se pudo mandar");
    }
    disconnect_from_broker(broker);
}
