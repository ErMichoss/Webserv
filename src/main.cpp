#include "webserv.h"
#include "ConfigParser.hpp"
#include "ServerManager.hpp"
#include <functional>

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
	struct addrinfo addr_info, *res, *head;
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

	head = res;
	while (res != NULL){
		socketfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (socketfd < 0){
			std::cerr << "Error: no se pudo crear el socket" << std::endl;
		} else if (bind(socketfd, res->ai_addr, res->ai_addrlen) == 0){
			break;
		}
		close(socketfd);
		res = res->ai_next;
	}
	if (res == NULL){
		freeaddrinfo(head);
		return -1;
	}
	freeaddrinfo(head);

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
	//mando el archivo de configuracion a la clase para que me lo parsee.
	ConfigParser ConfigFile(file);
	//Inicia el parseo del arhivo
	ConfigFile.addServerConf();
	//pilla las configuraciones de los servers guardados en la structura server
	std::vector<ConfigParser::Server> servers_conf = ConfigFile.getServers();
	std::vector<ServerManager> servers = std::vector<ServerManager>();

	for (size_t i = 0; i < servers_conf.size(); i++){
		//Recorro todas las configuraciones
		int index = hostport_match(servers, servers_conf[i]); // compruebo si ya se ha abierto un servidor con ese mismo host:port
		//esto lo hago porque si tienen lo mismo el servidor es esencialmente el mismo pero con diferentes configuraciones
		//para eso sirve el server_name, para si hay mas de un servidor en un mismo host:port saber a cual se esta refiriendo el cliente.
		if (index == -1){ // Si no hay ningun otro servidor con el mismo host:port creamos un nuevo socket para este servidor
			//Crea el socker
			int socketfd = create_socket(servers_conf[i].port, servers_conf[i].host);
			if (socketfd >= 0){// si se crea bien le creo un objeto ServerManager para manejar el servidor,
				ServerManager server(servers_conf[i], socketfd);
				servers.push_back(server);
			} 
			else { std::cerr << "No se pudo crear el socket para " << servers_conf[i].host << ":" << servers_conf[i].port << std::endl; }
		} else { // Si ya existe simplemente se lo añado a el objeto con el que tenga el mismo host:port
			servers[index].addConf(servers_conf[i]);
		}
	}
	//inicio los servers
	for (size_t i = 0; i < servers.size(); i++){
		servers[i].startServer();
	}

	return 0;
}