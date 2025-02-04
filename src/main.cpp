#include "webserv.h"
#include "ConfigParser.hpp"
#include "ServerManager.hpp"
#include <functional>

bool running = true;
std::vector<struct pollfd> fds;
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

	std::memset(&addr_info, 0, sizeof(addr_info));
	addr_info.ai_family = AF_INET;
	addr_info.ai_socktype = SOCK_STREAM;
	addr_info.ai_flags = 0;

	char port_str[6];
	for (int i = 4, temp = port; i >= 0; --i, temp /= 10) {
        port_str[i] = '0' + (temp % 10);
    }
    port_str[5] = '\0';

	int status = getaddrinfo(host.c_str(), port_str, &addr_info, &res);
	if (status != 0){
		std::cerr << "Error: en getaddrinfo: " << gai_strerror(status) << std::endl;
		return -1;
	}

	head = res;
	while (res != NULL){
        socketfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (socketfd < 0){
            std::cerr << "Error: no se pudo crear el socket: " << strerror(errno) << std::endl;
        } else {
            int optval = 1;
            if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
                std::cerr << "Error: no se pudo activar SO_REUSEADDR: " << strerror(errno) << std::endl;
                close(socketfd);
                return -1;
            }

            if (bind(socketfd, res->ai_addr, res->ai_addrlen) == 0) {
                break;
            }
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

void setSocketLinger(int socket_df) {
	struct linger sl;
	sl.l_onoff = 1;
	sl.l_linger = 0;

	if (setsockopt(socket_df, SOL_SOCKET, SO_LINGER, &sl, sizeof(sl)) < 0)
    	perror("setsockopt(SO_LINGER) failed");
}

void pollinHandler(struct pollfd fd, std::vector<ServerManager>& servers, std::size_t* index) {
    for (std::size_t i = 0; i < servers.size(); i++) {
        if (fd.fd == servers[i].getServerFd()) {
            struct sockaddr_in client;
            socklen_t client_len = sizeof(client);
            int client_fd = accept(servers[i].getServerFd(), (struct sockaddr*)&client, &client_len);
            if (client_fd >= 0) {
                struct pollfd poll_client = { client_fd, POLLIN, 0 };
                fds.push_back(poll_client);
                servers[i].addClient(client_fd);
				std::cout << "Event: Client connected: " << client_fd << std::endl;
            } else {
                std::cerr << "Error: Client could not connect" << std::endl;
            }
			return ;
        } else if (!servers[i].fdcgi_in.empty() && std::find(servers[i].fdcgi_in.begin(), servers[i].fdcgi_in.end(), fd.fd) != servers[i].fdcgi_in.end()) {
			std::cout << "Event: Pipe Reads" << std::endl;
            servers[i].readCgi(fd.fd);
			if (servers[i].stopped_value[servers[i].pipe_client[fd.fd]] == false){
				std::vector<struct pollfd>::iterator ss = fds.begin() + *index;
                fds.erase(ss);
			}
			std::cout << "Event: Exits Pipe Reads" << std::endl;
			return;
        } else if (!servers[i].clients.empty() && std::find(servers[i].getClients().begin(), servers[i].getClients().end(), fd.fd) != servers[i].getClients().end()) {
            char buffer[BUFFER_SIZE];
            std::memset(buffer, 0, sizeof(buffer));
            ssize_t bytes = read(fd.fd, buffer, sizeof(buffer));
            if (bytes > 0) {
                ConfigParser::Server server_conf = servers[i].getServerName(std::string(buffer, bytes));
                servers[i].server_conf = server_conf;
                servers[i].setActiveClient(fd.fd);
                servers[i].handle_request(std::string(buffer, bytes), server_conf);
				std::cout << "Client handeled: " << fd.fd << std::endl;
                fds[*index].events = POLLOUT;
            } else {
                servers[i].removeClient(fd.fd);
                std::cout << "Event: Client Disconnected: " << fd.fd << std::endl;
                close(fd.fd);
                std::vector<struct pollfd>::iterator it = fds.begin() + *index;
                fds.erase(it);
            }
			return;
		}
    }
}

void polloutHandler(struct pollfd fd, std::vector<ServerManager>& servers, std::size_t* index) {
    for (std::size_t i = 0; i < servers.size(); i++) {
        if (std::find(servers[i].clients.begin(), servers[i].clients.end(), fd.fd) != servers[i].clients.end() && servers[i].stopped_value[fd.fd] == false) {
            send(fd.fd, servers[i].client_response[fd.fd].c_str(), servers[i].client_response[fd.fd].size(), 0);
			std::cout << "Event: Response sended: " << fd.fd << std::endl;
			fds[*index].events = POLLIN;
			return;
        } else if (std::find(servers[i].fdcgi_out.begin(),servers[i].fdcgi_out.end(), fd.fd) != servers[i].fdcgi_out.end()) {
			std::cout << "Event: Entra a escribir en Pipe" << std::endl;
			servers[i].writePOST(fd.fd);
			fds[*index].events = POLLIN;
			std::cout << "Event: Sale de escribir en Pipe" << std::endl;
			return;
        }
    }
}

int main(int argc, char *argv[]) {
	if (argc > 2){
		std::cerr << "Error: Invalid number of arguments (Only 1 config file needed)";
		return 1;
	}
	char const* file;
	if (argc == 2)
		file = argv[1];
	else 
		file = "conf/server.conf";
	ConfigParser ConfigFile(file);
	ConfigFile.addServerConf();
	std::vector<ConfigParser::Server> servers_conf = ConfigFile.getServers();
	std::vector<ServerManager> servers = std::vector<ServerManager>();

	signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

	for (std::size_t i = 0; i < servers_conf.size(); i++){
		int index = hostport_match(servers, servers_conf[i]);
		if (index == -1){
			int socketfd = create_socket(servers_conf[i].port, servers_conf[i].host);
			if (socketfd >= 0){
				ServerManager server(servers_conf[i], socketfd);
				servers.push_back(server);
			} 
			else { std::cerr << "No se pudo crear el socket para " << servers_conf[i].host << ":" << servers_conf[i].port << std::endl; }
		} else {
			servers[index].addConf(servers_conf[i]);
		}
	}

	for (std::size_t i = 0; i < servers.size(); i++){
		struct pollfd new_poll = { servers[i].getServerFd(), POLLIN, 0 };
		fds.push_back(new_poll);
	}

	while (running) {
		std::vector<struct pollfd> copia = fds;
		int count = poll(&copia[0], copia.size(), -1);
		if (count < 0) {
			std::cerr << "Error: There was an error in Poll";
			running = false;
			break;
		}
		for (std::size_t i = 0; i < copia.size(); i++) {
			if (copia[i].revents & POLLIN) {
				pollinHandler(copia[i], servers, &i);
			} else if (copia[i].revents & POLLOUT) {
				polloutHandler(copia[i], servers, &i);
			}
		}
	}

	return 0;
}