#include "connections.h"

int create_socket() {
    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return -1;
    } else {
        int option = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
        return fd;
    }
}

int bind_socket(int socket, int port) {
    struct sockaddr_in server;
    int bindres;

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server.sin_zero), 8);

    bindres = bind(socket, (struct sockaddr *) &server, sizeof(struct sockaddr));
    return bindres;
}

int connect_socket(int socket, char *IP, int port) {
    struct hostent *he;
    struct sockaddr_in server;

    if ((he = gethostbyname(IP)) == NULL) {
        return -1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr = *((struct in_addr *) he->h_addr);
    bzero(&(server.sin_zero), 8);

    if (connect(socket, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1) {
        return -1;
    }
    return 0;
}

int close_socket(int socket) {
    close(socket);
    return 0;
}

int receive_header(int socket, MessageHeader *buffer) {
    int rec;
    rec = recv(socket, buffer, sizeof(MessageHeader), 0);
    return rec;
}

void* server_client(void* _params){

    //casteo los parametros porque a pthread hay que pasarle un puntero a void
    t_thread_client* params = (t_thread_client* ) _params;
    MessageHeader* header = malloc(sizeof(MessageHeader));

    int datos_recividos  = recv(params->socket, header, sizeof(MessageHeader),0);

    while (datos_recividos != -1) {

        if(datos_recividos == 0){
            //si devuelve 0 es porque se corto la conexion
            params->lost_connection(params->socket,params->client_ip, params->connection_port);
            close(params->socket);
            break;

        }else {

            params->incoming_message(params->socket, params->client_ip, params->connection_port, header);

        }

        datos_recividos  = recv(params->socket, header, sizeof(MessageHeader),0);
    }

    //Libero la memoria que pedi
    free(header);
    //free(params->client_ip);
    free(params);

}

int start_multithread_server(int socket,
    void (*new_connection)(int fd, char *ip, int port),
    void (*lost_connection)(int fd, char *ip, int port),
    void (*incoming_message)(int fd, char *ip, int port, MessageHeader *header)) {

    int addrlen, new_socket, i, bytesread, sd;
    struct sockaddr_in address;

    //Es header del mensaje del cliente
    MessageHeader *incoming;

    //Empieza a escuchar el socket y puede tener peticiones pendientes
    if (listen(socket, MAX_CONN) < 0) {

        return -1;
    }
    addrlen = sizeof(address);

    while (1) {

        //Se fija si hay nuevas conexion
        if ((new_socket = accept(socket, (struct sockaddr *) &address, (socklen_t *) &addrlen)) > 0) {

            //Acepta la nueva conexion
            new_connection(new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            getpeername(new_socket, (struct sockaddr *) &address, (socklen_t *) &addrlen);

            //Creo los parametros de la funcion que le voy a pasar al hilo
            t_thread_client *parametros_cliente = malloc(sizeof(t_thread_client));

            parametros_cliente->socket = new_socket;
            parametros_cliente->client_ip = inet_ntoa(address.sin_addr);
            parametros_cliente->connection_port = ntohs(address.sin_port);
            parametros_cliente->incoming_message = incoming_message;
            parametros_cliente->lost_connection = lost_connection;

            //Inicializo el hilo
            pthread_t thread_client;

            //Lo creo y pongo en ejecucion
            pthread_create(&thread_client, NULL, server_client, (void *) parametros_cliente);
            pthread_detach(thread_client);
        }

    }
}

int start_server(int socket,
                 void (*new_connection)(int fd, char * ip, int port),
                 void (*lost_connection)(int fd, char * ip, int port),
                 void (*incoming_message)(int fd, char * ip, int port, MessageHeader * header)) {

    int addrlen, new_socket ,client_socket_array[MAX_CONN], activity, i, bytesread, sd;
    int max_sd;
    struct sockaddr_in address;
    fd_set readfds;

    MessageHeader * incoming;

    for (i = 0; i < MAX_CONN; i++) {
        client_socket_array[i] = 0;
    }

    if (listen(socket, MAX_CONN) < 0) {
        return -1;
    }

    addrlen = sizeof(address);

    while(1) {
        FD_ZERO(&readfds);

        FD_SET(socket, &readfds);
        max_sd = socket;

        for (i = 0 ; i < MAX_CONN ; i++) {
            sd = client_socket_array[i];
            if (sd > 0){
                FD_SET( sd , &readfds);
            }
            if (sd > max_sd){
                max_sd = sd;
            }
        }
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(socket, &readfds)) {

            //TODO: revisar si en este caso deberia cortat o algo, ni me acuerdo que era
            if ((new_socket = accept(socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            }
            new_connection(new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            for (i = 0; i < MAX_CONN; i++) {
                if (client_socket_array[i] == 0) {
                    client_socket_array[i] = new_socket;
                    break;
                }
            }
        }

        for (i = 0; i < MAX_CONN; i++) {
            sd = client_socket_array[i];

            if (FD_ISSET(sd, &readfds)) {
                int client_socket = sd;

                incoming = malloc(sizeof(MessageHeader));

                if ((read(client_socket, incoming, sizeof(MessageHeader))) <= 0) {
                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);

                    lost_connection(client_socket, inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                    close(sd);
                    client_socket_array[i] = 0;
                } else {

                    incoming_message(client_socket, inet_ntoa(address.sin_addr) , ntohs(address.sin_port), incoming);

                }

                free(incoming);
            }
        }
    }
}

t_paquete * create_package(MessageType tipo) {
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->header = (MessageHeader *) malloc(sizeof(MessageHeader));
    paquete->header->type = tipo;
    paquete->header->data_size = 0;
    paquete->stream = NULL;
    return paquete;
}

void add_to_package(t_paquete *paquete, void *valor, int tamanio) {
    //valor = "hola"
    //tamanio = 5(4 del "hola" mas caracter terminador)
    //paquete->stream == NULL
    //paquete->stream length = 0
    //paquete->header->data_size == 0
    paquete->stream = realloc(paquete->stream, paquete->header->data_size + tamanio + sizeof(int));
    //paquete->stream length == 0 + 5 + 1

    memcpy(paquete->stream + paquete->header->data_size, &tamanio, sizeof(int));
    //paquete->stream == 5
    memcpy(paquete->stream + paquete->header->data_size + sizeof(int), valor, tamanio);
    //paquete->stream == 5hola

    paquete->header->data_size += tamanio + sizeof(int);
    //paquete->header->data_size == 6
}

int send_package(t_paquete *paquete, int socket_cliente) {
    int bytes = paquete->header->data_size + 2 * sizeof(int);
    void *a_enviar = serialize_package(paquete, bytes);

    int sent;

    if ((sent = send(socket_cliente, a_enviar, bytes, 0)) == -1) {
        printf("Error en el envio");
    }

    free(a_enviar);
    return sent;
}

void *serialize_package(t_paquete *paquete, int bytes) {
    void *serialized = malloc(bytes);
    int desplazamiento = 0;

    memcpy(serialized + desplazamiento, &(paquete->header->type), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(serialized + desplazamiento, &(paquete->header->data_size), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(serialized + desplazamiento, paquete->stream, paquete->header->data_size);

    return serialized;
}

void free_package(t_paquete *paquete) {
    free(paquete->header);
    free(paquete->stream);
    free(paquete);
}

t_list *receive_package(int socket_cliente, MessageHeader *header) {
    int size = header->data_size;
    int desplazamiento = 0;
    void *buffer = malloc(size);
    recv(socket_cliente, buffer, size, MSG_WAITALL);
    t_list *valores = list_create();
    int tamanio;

    while (desplazamiento < size) {
        memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
        desplazamiento += sizeof(int);
        char *valor = malloc(tamanio);
        memcpy(valor, buffer + desplazamiento, tamanio);
        desplazamiento += tamanio;
        list_add(valores, valor);
    }

    free(buffer);
    return valores;
}


// Pokemons

t_new_pokemon* create_new_pokemon(char* nombre_pokemon, uint32_t pos_x, uint32_t pos_y, uint32_t cantidad){
    t_new_pokemon* new_pokemon = malloc(sizeof(t_new_pokemon));
    new_pokemon->nombre_pokemon_length = strlen(nombre_pokemon) + 1;
    new_pokemon->nombre_pokemon = nombre_pokemon;
    new_pokemon->pos_x = pos_x;
    new_pokemon->pos_y = pos_y;
    new_pokemon->cantidad = cantidad;
}

void* new_pokemon_a_void(t_new_pokemon* new_pokemon){
    void* stream = malloc(sizeof(uint32_t)*4 + new_pokemon->nombre_pokemon_length);
    int offset = 0;

    memcpy(stream + offset, &new_pokemon->nombre_pokemon_length, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, new_pokemon->nombre_pokemon, new_pokemon->nombre_pokemon_length);
    offset += new_pokemon->nombre_pokemon_length;

    memcpy(stream + offset, &new_pokemon->pos_x, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, &new_pokemon->pos_y, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, &new_pokemon->cantidad, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    return stream;
}

t_new_pokemon* void_a_new_pokemon(void* stream){
    t_new_pokemon* new_pokemon = malloc(sizeof(t_new_pokemon));

    memcpy(&(new_pokemon->nombre_pokemon_length), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    new_pokemon->nombre_pokemon = malloc(new_pokemon->nombre_pokemon_length);
    memcpy(new_pokemon->nombre_pokemon, stream, new_pokemon->nombre_pokemon_length);
    stream += new_pokemon->nombre_pokemon_length;

    memcpy(&(new_pokemon->pos_x), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    memcpy(&(new_pokemon->pos_y), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    memcpy(&(new_pokemon->cantidad), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    return new_pokemon;
}


t_get_pokemon* create_get_pokemon(char* nombre_pokemon){
    t_get_pokemon* get_pokemon = malloc(sizeof(t_get_pokemon));
    get_pokemon->nombre_pokemon_length = strlen(nombre_pokemon) + 1;
    get_pokemon->nombre_pokemon = nombre_pokemon;
    return get_pokemon;
}

void* get_pokemon_a_void(t_get_pokemon* get_pokemon){
    void* stream = malloc(sizeof(uint32_t) + get_pokemon->nombre_pokemon_length);
    int offset = 0;

    memcpy(stream + offset, &get_pokemon->nombre_pokemon_length, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, get_pokemon->nombre_pokemon, get_pokemon->nombre_pokemon_length);
    offset += get_pokemon->nombre_pokemon_length;

    return stream;
}

t_get_pokemon* void_a_get_pokemon(void* stream){
    t_get_pokemon* get_pokemon = malloc(sizeof(t_get_pokemon));

    memcpy(&(get_pokemon->nombre_pokemon_length), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    get_pokemon->nombre_pokemon = malloc(get_pokemon->nombre_pokemon_length);
    memcpy(get_pokemon->nombre_pokemon, stream, get_pokemon->nombre_pokemon_length);
    stream += get_pokemon->nombre_pokemon_length;

    return get_pokemon;
}


t_localized_pokemon* create_localized_pokemon(char* nombre_pokemon, uint32_t cantidad_coordenadas, ...){
    t_localized_pokemon* localized_pokemon = malloc(sizeof(t_localized_pokemon));
    localized_pokemon->nombre_pokemon_length = strlen(nombre_pokemon) + 1;
    localized_pokemon->nombre_pokemon = nombre_pokemon;
    localized_pokemon->cantidad_coordenas = cantidad_coordenadas;
    localized_pokemon->coordenadas = malloc(cantidad_coordenadas*2*(sizeof(uint32_t)));

    va_list ap;
    va_start(ap, cantidad_coordenadas*2);
    for (int i = 0; i < cantidad_coordenadas*2; ++i) {
        localized_pokemon->coordenadas[i] = va_arg(ap, uint32_t);
    }
    va_end(ap);

    return localized_pokemon;
}

void* localized_pokemon_a_void(t_localized_pokemon* localized_pokemon){
    int len = sizeof(uint32_t) + localized_pokemon->nombre_pokemon_length + sizeof(uint32_t)*(localized_pokemon->cantidad_coordenas*2 + 1);
    void* stream = malloc(sizeof(uint32_t) + localized_pokemon->nombre_pokemon_length + sizeof(uint32_t)*(localized_pokemon->cantidad_coordenas*2 + 1));
    int offset = 0;

    memcpy(stream + offset, &localized_pokemon->nombre_pokemon_length, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, localized_pokemon->nombre_pokemon, localized_pokemon->nombre_pokemon_length);
    offset += localized_pokemon->nombre_pokemon_length;

    memcpy(stream + offset, &localized_pokemon->cantidad_coordenas, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, localized_pokemon->coordenadas, sizeof(uint32_t)*localized_pokemon->cantidad_coordenas*2);
    offset += sizeof(uint32_t)*localized_pokemon->cantidad_coordenas*2;

    return stream;
}

t_localized_pokemon* void_a_localized_pokemon(void* stream){
    t_localized_pokemon* localized_pokemon = malloc(sizeof(t_localized_pokemon));

    memcpy(&(localized_pokemon->nombre_pokemon_length), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    localized_pokemon->nombre_pokemon = malloc(localized_pokemon->nombre_pokemon_length);
    memcpy(localized_pokemon->nombre_pokemon, stream, localized_pokemon->nombre_pokemon_length);
    stream += localized_pokemon->nombre_pokemon_length;

    memcpy(&(localized_pokemon->cantidad_coordenas), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    localized_pokemon->coordenadas = malloc(sizeof(uint32_t)*localized_pokemon->cantidad_coordenas*2);
    memcpy(localized_pokemon->coordenadas, stream, sizeof(uint32_t)*localized_pokemon->cantidad_coordenas*2);
    stream += sizeof(uint32_t)*localized_pokemon->cantidad_coordenas*2;



    return localized_pokemon;
}



t_caught_pokemon* create_caught_pokemon(uint32_t atrapado){
    t_caught_pokemon* caught_pokemon = malloc(sizeof(t_caught_pokemon));
    caught_pokemon->atrapado = atrapado;
    return caught_pokemon;
}

void* caught_pokemon_a_void(t_caught_pokemon* caught_pokemon){
    void* stream = malloc(sizeof(uint32_t));
    int offset = 0;

    memcpy(stream + offset, &caught_pokemon->atrapado, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    return stream;
}

t_caught_pokemon* void_a_caught_pokemon(void* stream){
    t_caught_pokemon* caught_pokemon = malloc(sizeof(t_caught_pokemon));

    memcpy(&(caught_pokemon->atrapado), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    return caught_pokemon;
}


t_catch_pokemon* create_catch_pokemon(char* nombre_pokemon, uint32_t pos_x, uint32_t pos_y){
    t_catch_pokemon* catch_pokemon = malloc(sizeof(t_catch_pokemon));
    catch_pokemon->nombre_pokemon_length = strlen(nombre_pokemon) + 1;
    catch_pokemon->nombre_pokemon = nombre_pokemon;
    catch_pokemon->pos_x = pos_x;
    catch_pokemon->pos_y = pos_y;
}

void* catch_pokemon_a_void(t_catch_pokemon* catch_pokemon){
    void* stream = malloc(sizeof(uint32_t)*3 + catch_pokemon->nombre_pokemon_length);
    int offset = 0;

    memcpy(stream + offset, &catch_pokemon->nombre_pokemon_length, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, catch_pokemon->nombre_pokemon, catch_pokemon->nombre_pokemon_length);
    offset += catch_pokemon->nombre_pokemon_length;

    memcpy(stream + offset, &catch_pokemon->pos_x, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, &catch_pokemon->pos_y, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    return stream;
}

t_catch_pokemon* void_a_catch_pokemon(void* stream){
    t_catch_pokemon* catch_pokemon = malloc(sizeof(t_catch_pokemon));

    memcpy(&(catch_pokemon->nombre_pokemon_length), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    catch_pokemon->nombre_pokemon = malloc(catch_pokemon->nombre_pokemon_length);
    memcpy(catch_pokemon->nombre_pokemon, stream, catch_pokemon->nombre_pokemon_length);
    stream += catch_pokemon->nombre_pokemon_length;

    memcpy(&(catch_pokemon->pos_x), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    memcpy(&(catch_pokemon->pos_y), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    return catch_pokemon;
}



t_appeared_pokemon* create_appeared_pokemon(char* nombre_pokemon, uint32_t pos_x, uint32_t pos_y){
    t_appeared_pokemon* appeared_pokemon = malloc(sizeof(t_appeared_pokemon));
    appeared_pokemon->nombre_pokemon_length = strlen(nombre_pokemon) + 1;
    appeared_pokemon->nombre_pokemon = nombre_pokemon;
    appeared_pokemon->pos_x = pos_x;
    appeared_pokemon->pos_y = pos_y;
}

void* appeared_pokemon_a_void(t_appeared_pokemon* appeared_pokemon){
    void* stream = malloc(sizeof(uint32_t)*3 + appeared_pokemon->nombre_pokemon_length);
    int offset = 0;

    memcpy(stream + offset, &appeared_pokemon->nombre_pokemon_length, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, appeared_pokemon->nombre_pokemon, appeared_pokemon->nombre_pokemon_length);
    offset += appeared_pokemon->nombre_pokemon_length;

    memcpy(stream + offset, &appeared_pokemon->pos_x, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, &appeared_pokemon->pos_y, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    return stream;
}

t_appeared_pokemon* void_a_appeared_pokemon(void* stream){
    t_appeared_pokemon* appeared_pokemon = malloc(sizeof(t_appeared_pokemon));

    memcpy(&(appeared_pokemon->nombre_pokemon_length), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    appeared_pokemon->nombre_pokemon = malloc(appeared_pokemon->nombre_pokemon_length);
    memcpy(appeared_pokemon->nombre_pokemon, stream, appeared_pokemon->nombre_pokemon_length);
    stream += appeared_pokemon->nombre_pokemon_length;

    memcpy(&(appeared_pokemon->pos_x), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    memcpy(&(appeared_pokemon->pos_y), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    return appeared_pokemon;
}

unsigned long unix_epoch() {

    struct timeval tv;
    time_t seconds;
    gettimeofday(&tv, NULL);
    unsigned long long miliseconds_since_epoch = (unsigned long long)((unsigned long long)(tv.tv_sec) * (unsigned long long)1000 + (unsigned long long)(tv.tv_usec) / (unsigned long long)1000);

    seconds = time(NULL);
    int days_since_epoch = seconds/(60*60*24);
    unsigned long long midnight_miliseconds_since_epoch = (unsigned long long)days_since_epoch*(24*60*60*1000);

    unsigned long miliseconds_since_midnight = (unsigned long)(miliseconds_since_epoch - midnight_miliseconds_since_epoch);
    return miliseconds_since_midnight;
}
