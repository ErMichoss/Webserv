#include "ServerManager.hpp"


/**
 * @brief The constructor for the ServerManager class
 */
ServerManager::ServerManager(ConfigParser::Server server_conf, int socket) {
    this->server_confs.push_back(server_conf);
    this->server_fd = socket;
}

ServerManager::~ServerManager() {}

int ServerManager::getServerFd() {
	return this->server_fd;
}

void ServerManager::addClient(int fd) {
	this->clients.push_back(fd);
}

std::vector<int> ServerManager::getClients() {
	return this->clients;
}

void ServerManager::removeClient(int fd) {
	std::vector<int>::iterator it = std::remove(
		this->clients.begin(),
		this->clients.end(),
		fd
	)

	this->clients.erase(it, this->clients.end());
}

void ServerManager::handle_request(std::string const request, ConfigParser::Server server_conf) {
	std::istringstream req_stream(request);
	std::string method, path, protocol;
	std::size_t index = 0;

	req_stream >> method >> path >> protocol;

	std::size_t last_bar = path.find_last_of("/");
	std::string directory_path = path.substr(0, last_bar);
	std::string file = path.substr(last_bar);

	for (std::size_t i = 0; i < server_conf.locations.size(); i++) {
		if (server_conf.locations[i].path == directory_path) {
			index = i;
			break;
		}
	}

	if (server_conf.locations[index].limits.empty()){
		server_conf.locations[index].limits.push_back("NONE");
	}

	if (!server_conf.locations[index].redirect_target.empty()) {
		std::map<int, std::string>::iterator it = server_conf.locations[index].redirect_target.begin();
		int code = it->first;
		std::string loc = it->second;
		if (code == 301) {
			this->response = "HTTP/1.1 301 Moved Permanently\r\nLocation: " + loc + file + "\r\n\r\n";
		}
	} else if (method == "GET") {
		if (
			server_conf.locations[index].limits == "NONE" || 
			!this->checkLimits(server_conf.locations[index].limits, "GET")
		) {
			if (path == "/") { path = "/" + server_conf.locations[index].index; }
			getFile(path, server_conf.locations[index].root, server_conf.cgi, request);
		}
		response = HTTP405 + server_conf.error_pages[405];
	} else if (method == "POST") {
		if (
			server_conf.locations[index].limits == "NONE" || 
			!this->checkLimits(server_conf.locations[index].limits, "POST")
		) {
			if (path == "/upload") { handlePostUpload(request, server_conf.locations[index].root); }
			else { handlePost(request, path, server_conf.locations[index].root); }
		}
		response = HTTP405 + server_conf.error_pages[405];
	} else if (method == "DELETE") {
		if (
			server_conf.locations[index].limits == "NONE" || 
			!this->checkLimits(server_conf.locations[index].limits, "DELETE")
		) { 
			handleDelete(server_conf.locations[index].root, request);
		}
		response = HTTP405 + server_conf.error_pages[405];
	}
}

void ServerManager::getFile(std::string request_path, std::string server_root, std::string cgi, std::string request) {
	std::string path = server_root + request_path;
	if (path.find(".php") != std::string::npos && !cgi.empty()) {
		unsetenv("CONTENT_TYPE");
		unsetenv("CONTENT_LENGTH");
		setenv("REQUEST_METHOD", "GET", 1);
		setenv("SCRIPT_FILENAME", path.c_str(), 1);
		setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
		setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
		setenv("REDIRECT_STATUS", "1", 1);
		size_t start = request.find_first_of('?');
		size_t end = request.find_first_of(' ', start);
		std::string quri = request.substr(start, end - start);
		setenv("QUERY_STRING", quri.c_str(), 1);

		int pipe_fd[2];
		if (pipe(pipe_fd) == -1) {
			return HTTP500 + server_conf.error_pages[500];
		}

		pid_t pid = fork();
		if (pid == -1) {
			return HTTP500 + server_conf.error_pages[500];
		}

		if (pid == 0) {
			close(pipe_fd[0]);
			dup2(pipe_fd[1], STDOUT_FILENO);
			close(pipe_fd[1]);

			std::string command = "/usr/bin/php-cgi";
			std::cout << command << std::endl;
			char *const args[] = {const_cast<char *>(command.c_str()), const_cast<char *>(path.c_str()), NULL};
			execve(args[0], args, environ);
			exit(1);
		} else {
			close(pipe_fd[1]);
			
			struct pollfd new_fd = { pipe_fd[0], POLLIN, 0};
			fds.push_back(new_fd);
		}
	}
}

int ServerManager::checkLimits(std::vector<std::string> limits, std::string search) const{
	int exit = 0;
	for (size_t i = 0; i < limits.size(); i++){
		if (limits[i] == search){
			exit = 1;
		}
	}
	return exit;
}

ConfigParser::Server ServerManager::getServerName(std::string request) {
    std::size_t host_pos = request.find("Host: ");
    if (host_pos == std::string::npos) {
        return this->server_confs[0];
    }

    std::string host = request.substr(host_pos + 6);
    std::size_t host_end = host.find("\r\n");
    if (host_end != std::string::npos) {
        host = host.substr(0, host_end);
    }
	std::size_t colon_pos = host.find(":");
	if (colon_pos != std::string::npos){
		host = host.substr(0, colon_pos);
	}
	for (size_t i = 0; i < this->server_confs.size(); i++){
		if (this->server_confs[i].server_name == host)
			return this->server_confs[i];
	}
	return this->server_confs[0];
}

/**
 * @brief Gets the private attribute server_confs.
 * 
 * @return returns a vector<ConfigParser::Server> with the server_confs of the object.
 */
ConfigParser::Server ServerManager::getServersConf() {
	return this->server_confs[0];
}

/**
 * @brief Adds a server conf to the ServerManager object.
 * 
 * @param server_conf The server configuration that is going to be added.
 */
void ServerManager::addConf(ConfigParser::Server server_conf) {
	this->server_confs.push_back(server_conf);
}
