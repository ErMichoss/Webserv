#include "../incl/webserv.h"
#include "../incl/ConfigParser.hpp"


/**
 * @brief handles GET, POST and DELETE HTTP requests
 * 
 * @param request The incoming request from the client
 * 
 * @return on success returns the request on failure returns 405
 */
std::string handle_request(std::string const request){
	std::istringstream req_stream(request);
	std::string method, path, protocol;

	req_stream >> method >> path >> protocol;

	if (method == "GET"){
		if (path == "/") path = "/index.html";
		//return getFile(path);
	} else if (method == "POST"){
		//ejecutar POST
	} else if (method == "DELETE"){
		//ejecutar DELETE
	} else {
		//devuelve 405
	}
}

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

	std::memset(&addr_info, 0, sizeof(addr_info)); // IMPORTANTE
	addr_info.ai_family = AF_INET;
	addr_info.ai_socktype = SOCK_STREAM;
	addr_info.ai_flags = 0;

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
	if (argc != 2){
		std::cerr << "Error: Invalid number of arguments (Only 1 config file needed)";
		return 1;
	}
	char const* file = argv[1];
	ConfigParser ConfigFile(file);
	ConfigFile.addServerConf();
	std::vector<ConfigParser::Server> servers = ConfigFile.getServers();
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

	//bucle principal
	while (true) {
		int count = poll(&fds[0], fds.size(), -1);
		if (count < 0){
			//Mensaje de Error
			break;
		}

		//bucle de conexiones
		for (size_t i = 0; i < fds.size(); i++){
			if (fds[i].revents & POLLIN){
				for (size_t j = 0; j < sockets.size(); i++){
					if (fds[i].fd == sockets[j]){
						//Nueva conexion
						struct sockaddr_in client;
						socklen_t client_len = sizeof(client);
						int client_fd = accept(sockets[j], (struct sockaddr*)&client, &client_len);
						if (client_fd >= 0){
							struct pollfd poll_client = {client_fd, POLLIN, 0};
							fds.push_back(poll_client);
							//mensaje de exito
						} else {
							//mensaje de error
						}	
					} else {
						//Manejar cliente
						//leer datos char buffer[BUFFER_SIZE];

					}
				}
			}
		}
	}

	//cerrar todas las conexiones
	return 0;
}