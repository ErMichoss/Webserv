#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char message[BUFFER_SIZE];
    char buffer[BUFFER_SIZE] = {0};

    // Crear socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Error al crear el socket" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convertir dirección IP del servidor
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Dirección inválida o no soportada" << std::endl;
        return -1;
    }

    // Conectarse al servidor
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Error al conectar con el servidor" << std::endl;
        return -1;
    }
	
	std::cout << "Conectado al servidor. Escriba 'salir' para terminar." << std::endl;

    while (true) { // Bucle para enviar múltiples mensajes
        std::cout << "Ingrese un mensaje para enviar al servidor: ";
        std::cin.getline(message, BUFFER_SIZE);

        if (std::string(message) == "salir") {
            std::cout << "Desconectándose..." << std::endl;
            break;
        }

        send(sock, message, strlen(message), 0);
        std::cout << "Mensaje enviado al servidor." << std::endl;
    }

    return 0;
}