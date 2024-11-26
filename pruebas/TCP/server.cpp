#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8080
// Define el puerto en el que el servidor escuchará conexiones. Aquí hemos elegido el puerto 8080.
#define BUFFER_SIZE 1024
//Define el tamaño máximo del búfer que se usará para recibir mensajes.

int main() {
    int server_fd, new_socket;
	//server_fd será el descriptor de archivo del socket del servidor.
	//server_fd será el descriptor de archivo del socket del servidor.
    struct sockaddr_in address; //Estructura que almacena la dirección IP y el puerto del servidor.
    int opt = 1; // Se utiliza para configurar opciones del socket (en este caso, reutilizar direcciones).
    int addrlen = sizeof(address); //  Almacena el tamaño de la estructura address
    char buffer[BUFFER_SIZE] = {0}; //  Almacena el tamaño de la estructura address

    // Crear socket
	/* AF_INET: Indica que se usará el protocolo IPv4.
	   SOCK_STREAM: Indica que se usará un socket orientado a conexión (TCP).
	   0: Permite al sistema elegir automáticamente el protocolo (TCP para SOCK_STREAM).
	*/
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    // Configurar el socket para reutilizar la dirección
	/* setsockopt: Configura opciones para el socket. 
	   Aquí se activa la reutilización de la dirección (SO_REUSEADDR) y el puerto (SO_REUSEPORT) para evitar problemas si el servidor se reinicia rápidamente.
	*/
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Error en setsockopt");
        exit(EXIT_FAILURE);
    }

    // Configurar la estructura sockaddr_in
    address.sin_family = AF_INET; //  Indica que se usará IPv4.
    address.sin_addr.s_addr = INADDR_ANY; // Permite que el servidor acepte conexiones desde cualquier interfaz de red disponible.
    address.sin_port = htons(PORT); // Convierte el número de puerto a formato de red (big-endian) usando htons

    // Asociar el socket con el puerto
	/*Asocia el socket del servidor (server_fd) con la dirección y puerto definidos en address.*/
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Error en bind");
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones
	/* Pone el socket en modo escucha, permitiendo que acepte conexiones.
	   El segundo parámetro (3) indica el número máximo de conexiones pendientes en la cola.
	*/
    if (listen(server_fd, 3) < 0) {
        perror("Error en listen");
        exit(EXIT_FAILURE);
    }


//	std::cout << "Servidor escuchando en el puerto " << PORT << std::endl;

    // Aceptar una 
	/* Acepta una conexión entrante.
	   El descriptor del socket del cliente se almacena en new_socket.
	   Si falla, muestra un mensaje de error y cierra el programa.
	*/
//    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
//        perror("Error en accept");
//        exit(EXIT_FAILURE);
//    }

    // Leer el mensaje del cliente
//    int valread = read(new_socket, buffer, BUFFER_SIZE); // Lee el mensaje enviado por el cliente y lo almacena en el buffer.
 //   if (valread > 0) { //Si se reciben datos se imprime el mensaje.
//        std::cout << "Mensaje recibido del cliente: " << buffer << std::endl;
//	  } else {
 //       std::cerr << "Error al leer el mensaje" << std::endl;
 //   }

    // Cerrar la conexión
 //   close(new_socket); //  Cierra la conexión con el cliente.
 //   close(server_fd); // Cierra el socket del servidor, liberando los recursos.

	while (true) { // Bucle infinito para aceptar conexiones continuamente
        std::cout << "Esperando una conexión..." << std::endl;

        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Error en accept");
            exit(EXIT_FAILURE);
        }

        std::cout << "Cliente conectado." << std::endl;

        while (true) { // Bucle para recibir múltiples mensajes de un cliente
            memset(buffer, 0, BUFFER_SIZE); // Limpiar el búfer
            int valread = read(new_socket, buffer, BUFFER_SIZE);
            if (valread > 0) {
                std::cout << "Mensaje recibido del cliente: " << buffer << std::endl;
            } else if (valread == 0) {
                std::cout << "Cliente desconectado." << std::endl;
                break; // Salir del bucle del cliente si se desconecta
            } else {
                std::cerr << "Error al leer el mensaje." << std::endl;
                break;
            }
        }

        close(new_socket); // Cerrar la conexión con el cliente
    }
    close(server_fd);
    return 0;
}
