#include "../incl/webserv.h"
#include "../incl/ConfigParser.hpp"


/**
 * @brief Creates a socket for a port and host of your choice.
 * 
 * @param port The value of the port the socket is going to listen to.
 * @param host The value of the host the from where the socket listens.
 * 
 * @details addr_info, res They are addrinfo structs used to create the socket.
 * 
 * @return if it fails it returns -1 on success it returns the sockets fd.
 */
int create_socket(int port, std::string host){
	struct addrinfo addr_info, *res;
	int socketfd;

	addr_info.ai_family = AF_INET;
	addr_info.ai_socktype = SOCK_STREAM;
	//addr_info.ai_flags = AI_PASSIVE; // Usa la ip local si la introducida es NULL

	// convertir el puerto a cadena para getaddrinfo.
	char port_str[6];
	for (int i = 4, temp = port; i >= 0; --i, temp /= 10) {
        port_str[i] = '0' + (temp % 10);
    }
    port_str[5] = '\0';

	// obtener informacion de la direccion.
	int status = getaddrinfo(host.c_str(), port_str, &addr_info, &res);
	if (status != 0){
		std::cerr << "Error: en getaddrinfo: " << gai_strerror(status) << std::endl;
		return -1;
	}

	socketfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (socketfd < 0){
		std::cerr << "Error: no se pudo crear el socket" << std::endl;
		freeaddrinfo(res);
		return -1;
	}

	if (bind(socketfd, res->ai_addr, res->ai_addrlen) < 0){
		std::cerr << "Error: no se pudo vincular el socket" << std::endl;
		freeaddrinfo(res);
		return -1;
	}

	freeaddrinfo(res);

	//configurar el socket para escuchar.
	if (listen(socketfd, 10) < 0){
		std::cerr << "Error: no se pudo escuchar en el socket" << std::endl;
	}
	
	std::cout << "Socket creado y escuchando en " << host << ":" << port << std::endl;

	return socketfd;
}

int main(int argc, char *argv[]){
	int exit = 0;

	if (argc == 2){
		char const* file = argv[1];
		ConfigParser cp(file);
		cp.addServerConf();
		std::vector<ConfigParser::Server> servers = cp.getServers();
		std::vector<int> sockets;
		std::vector<struct pollfd> fds;

		for (std::vector<ConfigParser::Server>::iterator it = servers.begin(); it != servers.end(); it++){
			int socketfd = create_socket(it->port, it->host);
			if (socketfd >= 0){
				sockets.push_back(socketfd);
				struct pollfd fd = {socketfd, POLLIN, 0};
				fds.push_back(fd);
			} 
			else { std::cerr << "No se pudo crear el socket para " << it->host << ":" << it->port << std::endl; }
		}


    	while (true) {
			int event_count = poll(fds.data(), fds.size(), 300);
			if (event_count < 0){
				std::cerr << "Error: poll" << std::endl;
				break;
			}

			 // Iterar sobre los descriptores de archivo monitoreados
            for (size_t i = 0; i < fds.size(); ++i) {
                if (fds[i].revents & POLLIN) { // Si hay datos disponibles
                    if (fds[i].fd == sockets[i]) {
                        // Si el socket es el del servidor, aceptar una nueva conexión
                        struct sockaddr_in client_addr;
                        socklen_t addr_len = sizeof(client_addr);
                        int client_fd = accept(fds[i].fd, (struct sockaddr*)&client_addr, &addr_len);
                        if (client_fd < 0) {
                            perror("accept");
                            continue;
                        }

                        // Agregar el nuevo socket del cliente a la lista de fds
						struct pollfd fdc = {client_fd, POLLIN, 0};
                        fds.push_back(fdc);
                        std::cout << "Nueva conexión aceptada\n";
                    } else {
                        // Si el socket es un cliente, leer datos
                        char buffer[1024] = {0};
                        int bytes_read = read(fds[i].fd, buffer, sizeof(buffer));
                        if (bytes_read > 0) {
                            std::cout << "Solicitud recibida: " << buffer << "\n";
                            // Responder con una respuesta HTTP simple
                            write(fds[i].fd, "HTTP/1.1 200 OK\r\n\r\nHola Mundo", 31);
                        } else if (bytes_read == 0) {
                            // Si no hay datos, el cliente cerró la conexión
                            std::cout << "Cliente desconectado\n";
                            close(fds[i].fd);
                            fds.erase(fds.begin() + i);  // Eliminar el socket del cliente del vector
                            --i; // Ajustar el índice después de eliminar el socket
                        } else {
                            perror("read");
                        }
                    }
                }
            }
    	}

    	// Cerrar los sockets antes de salir
    	for (std::vector<int>::iterator it = sockets.begin(); it != sockets.end(); ++it) {
        	close(*it);
    	}

    	return 0;
	} else {
		std::cout << "Debes meter 1 archivo de configuración" << std::endl;
		exit = 1;
	}
	return exit;
}