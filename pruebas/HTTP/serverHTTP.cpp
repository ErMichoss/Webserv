#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 4096

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Crear el socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }
	/*
	Socket: sirve para comunicar dos dispostivos utilizando protocolos de red como TCP y UDP, nosotros vamos a utilizar TCP.
	*/

    // Configurar opciones del socket
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { //se activa la reutilización de la dirección (SO_REUSEADDR) y el puerto (SO_REUSEPORT) para evitar problemas si el servidor se reinicia rápidamente
        perror("Error en setsockopt");
        exit(EXIT_FAILURE);
    }

    // Configurar dirección y puerto
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
	/*
	Basicamente esto sirve para más adelante en la funcion bind() asociaremos el sockert guarador en 'server_fr' con estas configuraciones.
	*/

    // Asociar el socket al puerto
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Error en bind");
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones
    if (listen(server_fd, 3) < 0) {
        perror("Error en listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Servidor HTTP escuchando en el puerto " << PORT << std::endl;

    while (true) {
        std::cout << "Esperando una conexión..." << std::endl;

        // Aceptar una conexión
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Error en accept");
            exit(EXIT_FAILURE);
        }

        std::cout << "Cliente conectado." << std::endl;

        // Leer la solicitud del cliente
        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        if (valread > 0) {
            std::cout << "Solicitud recibida:\n" << buffer << std::endl;

            // Respuesta HTTP
            const char* http_response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: 46\r\n"
                "\r\n"
                "<html><body><h1>Hola, Mundo!</h1></body></html>\n";

            send(new_socket, http_response, strlen(http_response), 0);
            std::cout << "Respuesta enviada al cliente." << std::endl;
        }

        // Cerrar la conexión con el cliente
        close(new_socket);
        std::cout << "Conexión cerrada." << std::endl;
    }

    // Cerrar el socket del servidor
    close(server_fd);
    return 0;
}
