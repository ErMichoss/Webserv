#include "../incl/ServerManager.hpp"

ServerManager::ServerManager(ConfigParser::Server server_conf, int socket){
	this->server_conf = server_conf;
	this->server_fd = socket;
}

std::string ServerManager::getFile(std::string request_path, std::string server_root){
	std::string path = server_root + request_path;
	int fd = open(path.c_str(), O_RDONLY);
	if (fd < 0)
		return "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1>404 Not Found</h1>";

	char buffer[BUFFER_SIZE];
	std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
	ssize_t bytes;
	while ((bytes = read(fd, buffer, sizeof(buffer))) > 0){
		response.append(buffer, bytes);
	}
	
	close(fd);
	return response;
}

/**
 * @brief handles GET, POST and DELETE HTTP requests.
 * 
 * @param request The incoming request from the client.
 * @param server_conf The configuration of the server that was requested.
 * 
 * @return on success returns the request on failure returns 405
 */
std::string ServerManager::handle_request(std::string const request, ConfigParser::Server server_conf){
	std::istringstream req_stream(request);
	std::string method, path, protocol;

	req_stream >> method >> path >> protocol;

	if (method == "GET"){
		if (path == "/") path = "/index.html";
		return getFile(path, server_conf.root);
	} else if (method == "POST"){
		//ejecutar POST
		return "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>200 POST</h1>";
	} else if (method == "DELETE"){
		//ejecutar DELETE
		return "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>200 DELETE/h1>";
	}
	return "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/html\r\n\r\n<h1>405 Method Not Allowed</h1>";
}

void ServerManager::startServer(){
	std::vector<struct pollfd> fds;
	struct pollfd server_pollfd = {this->server_fd, POLLIN, 0};
    fds.push_back(server_pollfd);

	while (true) {
		int count = poll(&fds[0], fds.size(), -1);
		if (count < 0){
			//Mensaje de Error
			std::cerr << "Error en poll" << std::endl;
			break;
		}

		//bucle de conexiones
		for (size_t i = 0; i < fds.size(); i++){
			if (fds[i].revents & POLLIN){
				if (fds[i].fd == this->server_fd){
					//Nueva conexion
					struct sockaddr_in client;
					socklen_t client_len = sizeof(client);
					int client_fd = accept(this->server_fd, (struct sockaddr*)&client, &client_len);
					if (client_fd >= 0){
						struct pollfd poll_client = {client_fd, POLLIN, 0};
						fds.push_back(poll_client);
						//mensaje de exito
						std::cout << "Cliente conectado: " << client_fd << std::endl;
					} else {
						//mensaje de error
						std::cerr << "No se pudo conectar el cliente: " << client_fd << std::endl;
					}	
				} else {
					//Manejar cliente
					//leer datos char buffer[BUFFER_SIZE];
					char buffer[BUFFER_SIZE];
					std::memset(buffer, 0, sizeof(buffer));
					ssize_t bytes = read(fds[i].fd, buffer, sizeof(buffer));
					if (bytes > 0){
						std::string const response = handle_request(std::string(buffer, bytes), this->server_conf);
						send(fds[i].fd, response.c_str(), strlen(response.c_str()), 0);
						std::cout << "Client disconnected: " << fds[i].fd << std::endl;
						close(fds[i].fd);
						fds.erase(fds.begin() + i);
						--i;
					} else {
						// Desconexion o error.
						std::cout << "Client disconnected: " << fds[i].fd << std::endl;
						close(fds[i].fd);
						fds.erase(fds.begin() + i);
						--i;
					}
				}
			}
		}
	}

	close(this->server_fd);
}

