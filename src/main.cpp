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

void setSocketLinger(int socket_df)
{
	struct linger sl;
	sl.l_onoff = 1;
	sl.l_linger = 0;

	if (setsockopt(socket_df, SOL_SOCKET, SO_LINGER, &sl, sizeof(sl)) < 0)
    	perror("setsockopt(SO_LINGER) failed");
}

pollinHandler(pollfd fd, std::vector<ServerManager> servers, std::size_t index) {
	for (std::size_t i = 0; i < servers.size(); i++) {
		if (fd.fd == servers[i].getServerFd()) {
			struct sockaddr_in client;
			socklen_t client_len = sizeof(client);
			int client_fd = accept(servers[i].getServerFd(), (struct sockaddr*)&client, &client_len);
			if (client_fd >= 0) {
				struct pollfd poll_client = { client_fd, POLLIN, 0 };
				fds.push_back(poll_client);
				servers[i].addClient(client_fd);
			} else {
				std::cerr << "Error: Client could no connect" << std::cerr;
			}
		} else if (std::find(
			servers[i].getClients().begin(),
			servers[i].getClients().end(),
			fd.fd
		) != servers[i].getClients().end()) {
			char buffer[BUFFER_SIZE];
			std::memset (buffer, 0, sizeof(buffer));
			std::ssize_t bytes read(fd.fd, buffer, sizeof(buffer));
			if (bytes > 0){
				ConfigParser::Server server_conf = servers[i].getServerName(std::string(buffer, bytes));
				server[i].server_conf = server_conf;
				server[i].setActiveClient(fd.fd);
				severs[i].handle_request(std::string(buffer, bytes), server_conf);
				fds.erase(fds.begin() + index)
				--index;
			} else {
				server[i].removeClient(fd.fd);
				std::cout << "Envent: Client Disconnected: " << fd.fd << std::endl;
				close(fd[index].fd);
				fds.erase(fds.begin() + index)
				--index;
			}
		}
	}
}

int main(int argc, char *argv[]) {
	if (argc != 2){
		std::cerr << "Error: Invalid number of arguments (Only 1 config file needed)";
		return 1;
	}
	char const* file = argv[1];
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
		int count = poll(&fds[0], fds.size(), -1);
		if (count < 0) {
			std::cerr << "Error: "; perror(poll);
			running = false;
			break;
		}

		for (std::size_t i = 0; i < fds.size(); i++) {
			if (fds[i].revents & POLLIN) {
				pollinHandler(fd[i].fd, servers, i);
			} else if (fds[i].revents & POLLOUT) {

			}
		}
	}

	return 0;
}