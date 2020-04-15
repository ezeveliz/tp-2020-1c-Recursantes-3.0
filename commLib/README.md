# commLib
Biblioteca de comunicación para TP de operativos

Primero, obviamente, importar la biblioteca a nuestros distintos proyectos

Cliente(libmuse, libsuse/hilolay)
Llamar a create_socket()
Llamar a connect_socket(), pasarle el socket anterior, la ip y el puerto de envio
Llamar a create_package() para crear un paquete vacio, pasarle el MessageType correspondiente al mensaje a enviar.
Agregar todos los campos necesarios al paquete mediante la funcion add_to_package(), pasarle el paquete al cual agregar, el elemento a agregar casteado a un void* y el largo del elemento.
Enviar datos: llamar a send_package(),pasarle el paquete y el socket al cual enviar.
Recibir datos:
Llamar a receive_header(), pasarle el socket de recepcion, y un puntero a MessageHeader para saber que header voy a recibir en el siguiente mensaje.
Llamar a receive_package(), pasarle el socket de recepcion y el puntero al header recibido en la operacion anterior.
Hacer lo que se tenga que hacer con los datos anteriores, repetir si es necesario.
Llamar a free_package() para liberar la memoria solicitada para almacenar el paquete.
Llamar a close_socket() para cerrar el descriptor de archivo.

Servidor(Muse, Suse, Fuse?)
Llamar a create_socket()
Llamar a bind_socket(), pasarle el socket anterior y el puerto de escucha.
Crear las funciones de nueva conexion, nuevo mensaje y conexion perdida, las mismas tienen las siguientes firmas:
void (*incoming_message)(int fd, char * ip, int port, MessageHeader * header))
void (*lost_connection)(int fd, char * ip, int port)
void (*new_connection)(int fd, char * ip, int port)
fd es el socket correspondiente a la conexion, ip es la IP del proceso que está del otro lado de la conexión(?), port es el puerto de la conexión anterior y MessageHeader, es la estructura que expliqué arriba.
Dentro de cada una de estas 3 funciones se puede usar, o no, las funciones para recibir o enviar datos explicadas en el cliente. La unica variacion es la funcion encargada de recibir un paquete, ya que como el servidor utiliza al header de los mensajes entrantes(funcion incoming) para hacer unas verificaciones, no es necesario pedirlo mediante receive_header(), directamente utilizar receive_package().
Crear un hilo nuevo con pthread o lo que sea y pasarle como funcion a start_server(), a la misma se le debe pasar el socket creado y bindeado, pasarle las tres funciones creadas en el punto anterior y el puntero a los MessageHeader a recibir. 
Finalmente, divertirse.
