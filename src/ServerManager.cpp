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

void ServerManager::setActiveClient(int fd) {
	this->active_client = fd;
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

bool ServerManager::deleteResource(std::string resource){
	if (std::remove(resource.c_str()) == 0){
		return true;
	} else {
		return false;
	}
}

void ServerManager::handle_delete(std::string root, std::string request){
	std::size_t pos = request.find(" ") + 1;
	std::size_t pos_end = request.find(" ", pos);
	std::string resource = root;
	resource += request.substr(pos, pos_end - pos);
	std::cout << resource << std::endl;

	if(deleteResource(resource)){
		response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>200 DELETE/h1>";
	} else {
		response HTTP404 + server_conf.error_pages[404];
	}
}

void SeverManager::handlePostUpload(std::string request, std::string server_root) {
	std::size_t header_end = request.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        return HTTP400 + this->active_server.error_pages[400];
    }

    std::string headers = request.substr(0, header_end);
    std::size_t content_length_pos = headers.find("Content-Length: ");
    if (content_length_pos == std::string::npos) {
        return HTTP411 + this->active_server.error_pages[411];
    }

    std::size_t content_type_pos = headers.find("Content-Type: multipart/form-data;");
    if (content_type_pos == std::string::npos) {
        return HTTP415 + this->active_server.error_pages[415];
    }
    std::string boundary_prefix = "boundary=";
    std::size_t boundary_pos = headers.find(boundary_prefix, content_type_pos);
    if (boundary_pos == std::string::npos) {
        return HTTP400 + this->active_server.error_pages[400];
    }
    std::string boundary = "--" + headers.substr(boundary_pos + boundary_prefix.size());
    boundary = boundary.substr(0, boundary.find("\r\n"));

    std::string body = request.substr(header_end + 4);
    std::size_t file_start = body.find(boundary);
    if (file_start == std::string::npos) {
        return HTTP400 + this->active_server.error_pages[400];
    }
    file_start += boundary.size() + 2;

    std::size_t file_end = body.find(boundary, file_start);
    if (file_end == std::string::npos) {
        return HTTP400 + this->active_server.error_pages[400];
    }
    std::string file_content = body.substr(file_start, file_end - file_start);

    std::size_t filename_pos = file_content.find("filename=\"");
    if (filename_pos == std::string::npos) {
        return HTTP400 + this->active_server.error_pages[400];
    }
    std::string filename = file_content.substr(filename_pos + 10);
    filename = filename.substr(0, filename.find("\""));

    std::size_t data_start = file_content.find("\r\n\r\n");
    if (data_start == std::string::npos) {
        return HTTP400 + this->active_server.error_pages[400];
    }
    data_start += 4;
    std::string file_data = file_content.substr(data_start);

    std::string file_path = server_root + "/" + filename;
    fd_upload = open(file_path.c_str(), O_CREAT | O_WRONLY, 0666);

	struct pollfd write_pipe = {fd_write[1], POLLOUT, 0};
	this->fds.push_back(write_pipe);

	struct pollfd changed_client = { this->active_client, POLLERR, 0 };
	fds.push_back(changed_client);
}

void ServerManager::handlePost(std::string request, std::string request_path, std::string server_root){
	std::size_t header_end = request.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        return HTTP400 + this->active_server.error_pages[400];
    }

    std::string headers = request.substr(0, header_end);
    std::size_t content_length_pos = headers.find("Content-Length: ");
    if (content_length_pos == std::string::npos) {
        return HTTP411 + this->active_server.error_pages[411];
    }

    std::string content_length = headers.substr(content_length_pos + 16);
    std::size_t content_length_end = content_length.find("\r\n");
    if (content_length_end != std::string::npos) {
        content_length = content_length.substr(0, content_length_end);
    }

    std::size_t content_type_pos = headers.find("Content-Type: ");
    if (content_type_pos == std::string::npos) {
        return HTTP415 + this->active_server.error_pages[415];
    }

    std::string content_type = headers.substr(content_type_pos + 14);
    std::size_t content_type_end = content_type.find("\r\n");
    if (content_type_end != std::string::npos) {
        content_type = content_type.substr(0, content_type_end);
    }

    setenv("REQUEST_METHOD", "POST", 1);
    setenv("CONTENT_TYPE", content_type.c_str(), 1);
    setenv("CONTENT_LENGTH", content_length.c_str() , 1);
    setenv("SCRIPT_FILENAME", (server_root + request_path).c_str(), 1);
    setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
    setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
    setenv("REDIRECT_STATUS", "1", 1);

	std::string body = request.substr(header_end + 4);

	pipe(fd_read);
	pipe(fd_write);
	pid_t pid = fork();
	if (pid == 0){
		char* matriz[2];
		matriz[0] = (char *)"php-cgi";
		matriz[1] = NULL;
		dup2(fd_read[1], 1);
		close(fd_read[0]);
		close(fd_read[1]);
		dup2(fd_write[0], 0);
		close(fd_write[1]);
		close(fd_write[0]);
		execve("/usr/bin/php-cgi", matriz, environ);
		exit(EXIT_FAILURE);
	}
	close(fd_write[0]);

	struct pollfd write_pipe = {fd_write[1], POLLOUT, 0};
	this->fds.push_back(write_pipe);

	struct pollfd read_pipe = {fd_read[0], POLLIN, 0};
	this->fds.push_back(read_pipe);

    struct pollfd changed_client = { this->active_client, POLLERR, 0 };
	fds.push_back(changed_client);
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

			struct pollfd changed_client = { this->active_client, POLLERR, 0 };
			fds.push_back(changed_client);
		}
	}
	file_to_get = open(path.c_str(), O_RDONLY);

	struct pollfd new_fd = { file_to_get, POLLIN, 0};
	fds.push_back(new_fd);

	struct pollfd changed_client = { this->active_client, POLLERR, 0 };
	fds.push_back(changed_client);
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
