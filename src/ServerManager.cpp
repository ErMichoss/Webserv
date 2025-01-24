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

std::vector<int> ServerManager::getFdCgiIn() {
	return this->fdcgi_in;
}

std::vector<int> ServerManager::getFdCgiOut() {
	return this->fdcgi_out;
}

void ServerManager::addClient(int fd) {
	this->clients.push_back(fd);
}

std::vector<int> ServerManager::getClients() {
	return this->clients;
}

void ServerManager::removeClient(int fd) {
	std::vector<int>::iterator it = std::remove(this->clients.begin(), this->clients.end(), fd);

	this->clients.erase(it, this->clients.end());
}

void ServerManager::setActiveClient(int fd) {
	this->active_client = fd;
}

void ServerManager::readCgi(int pipe) {
	char buffer[BUFFER_SIZE];
	int client_id = pipe_client[pipe];
	client_response[client_id] = "HTTP/1.1 200 OK\r\n";
	std::string aux;

	ssize_t bytes_read = 0;
	while ((bytes_read == read(pipe, buffer, sizeof(buffer))) > 0) {
		aux.append(buffer, bytes_read);
	}
	close(pipe);
	std::size_t header_end = aux.find("\r\n\r\n");
	if (header_end == std::string::npos) {
		client_response[client_id] = HTTP400 + server_conf.error_pages[400];
	}
	std::string content = aux.substr(header_end + 4);
	client_response[client_id] += "Connection: close\r\n";
	client_response[client_id] += "Content-Lenght: " + ft_itoa(std::strlen(content.c_str())) + "\r\n";
	client_response[client_id] += "Content-Type: text/html\r\n";
	client_response[client_id] += aux;
	std::vector<int>::iterator it = std::remove(this->clients.begin(), this->clients.end(), client_id);
	this->clients.erase(it, this->clients.end());
	struct pollfd client_out = { client_id, POLLOUT, 0 };
	fds.push_back(client_out);
}


bool ServerManager::deleteResource(std::string resource){
	if (std::remove(resource.c_str()) == 0){
		return true;
	} else {
		return false;
	}
}

void ServerManager::handle_delete(std::string root, std::string request) {
	std::size_t pos = request.find(" ") + 1;
	std::size_t pos_end = request.find(" ", pos);
	std::string resource = root;
	resource += request.substr(pos, pos_end - pos);
	std::cout << resource << std::endl;

	if(deleteResource(resource)){
		client_response[active_client] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>200 DELETE/h1>";
	} else {
		client_response[active_client] = HTTP404 + server_conf.error_pages[404];
	}
}

void ServerManager::handlePostUpload(std::string request, std::string server_root) {
	std::size_t header_end = request.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        client_response[active_client] = HTTP400 + this->server_conf.error_pages[400];
    }

    std::string headers = request.substr(0, header_end);
    std::size_t content_length_pos = headers.find("Content-Length: ");
    if (content_length_pos == std::string::npos) {
        client_response[active_client] =  HTTP411 + this->server_conf.error_pages[411];
    }

    std::size_t content_type_pos = headers.find("Content-Type: multipart/form-data;");
    if (content_type_pos == std::string::npos) {
        client_response[active_client] = HTTP415 + this->server_conf.error_pages[415];
    }
    std::string boundary_prefix = "boundary=";
    std::size_t boundary_pos = headers.find(boundary_prefix, content_type_pos);
    if (boundary_pos == std::string::npos) {
        client_response[active_client] = HTTP400 + this->server_conf.error_pages[400];
    }
    std::string boundary = "--" + headers.substr(boundary_pos + boundary_prefix.size());
    boundary = boundary.substr(0, boundary.find("\r\n"));

    std::string body = request.substr(header_end + 4);
    std::size_t file_start = body.find(boundary);
    if (file_start == std::string::npos) {
        client_response[active_client] = HTTP400 + this->server_conf.error_pages[400];
    }
    file_start += boundary.size() + 2;

    std::size_t file_end = body.find(boundary, file_start);
    if (file_end == std::string::npos) {
        client_response[active_client] = HTTP400 + this->server_conf.error_pages[400];
    }
    std::string file_content = body.substr(file_start, file_end - file_start);

    std::size_t filename_pos = file_content.find("filename=\"");
    if (filename_pos == std::string::npos) {
        client_response[active_client] = HTTP400 + this->server_conf.error_pages[400];
    }
    std::string filename = file_content.substr(filename_pos + 10);
    filename = filename.substr(0, filename.find("\""));

    std::size_t data_start = file_content.find("\r\n\r\n");
    if (data_start == std::string::npos) {
        client_response[active_client] = HTTP400 + server_conf.error_pages[400];
    }
    data_start += 4;
    std::string file_data = file_content.substr(data_start);

    std::string file_path = server_root + "/" + filename;
    fd_upload = open(file_path.c_str(), O_CREAT | O_WRONLY, 0666);
}

void ServerManager::handlePost(std::string request, std::string request_path, std::string server_root){
	std::size_t header_end = request.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        client_response[active_client] = HTTP400 + this->server_conf.error_pages[400];
    }

    std::string headers = request.substr(0, header_end);
    std::size_t content_length_pos = headers.find("Content-Length: ");
    if (content_length_pos == std::string::npos) {
        client_response[active_client] = HTTP411 + this->server_conf.error_pages[411];
    }

    std::string content_length = headers.substr(content_length_pos + 16);
    std::size_t content_length_end = content_length.find("\r\n");
    if (content_length_end != std::string::npos) {
        content_length = content_length.substr(0, content_length_end);
    }

    std::size_t content_type_pos = headers.find("Content-Type: ");
    if (content_type_pos == std::string::npos) {
        client_response[active_client] = HTTP415 + this->server_conf.error_pages[415];
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

	int pipes[2];
if (socketpair(AF_LOCAL, SOCK_STREAM, 0, pipes) == -1) {
    client_response[active_client] = HTTP500 + this->server_conf.error_pages[500];
    return;
}

pid_t pid = fork();
if (pid == -1) {
    client_response[active_client] = HTTP500 + this->server_conf.error_pages[500];
    return;
}

if (pid == 0) {
    dup2(pipes[0], STDOUT_FILENO);
    dup2(pipes[0], STDIN_FILENO); 
    close(pipes[0]);
    close(pipes[1]);

    std::string command = "/usr/bin/php-cgi";

    char *const args[] = {const_cast<char *>(command.c_str()), const_cast<char *>(request_path.c_str()), NULL};
    execve(args[0], args, environ);
    exit(1);
} else {
    struct pollfd socket = { pipes[1], POLLOUT, 0 };
    fds.push_back(socket);
    fdcgi_out.push_back(pipes[1]);
}
return;

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

		int pipes[2];
		if (socketpair(AF_LOCAL, SOCK_STREAM, 0, pipes) == -1){
			client_response[active_client] = HTTP500 + this->server_conf.error_pages[500];
		}
		pid_t pid = fork();
		if (pid == -1){
			client_response[active_client] = HTTP500 + this->server_conf.error_pages[500];
		}
		if (pid == 0){
			dup2(pipes[0], STDOUT_FILENO);
			close(pipes[0]);
			close(pipes[1]);

			std::string command = "/usr/bin/php-cgi";
			char *const args[] = {const_cast<char *>(command.c_str()), const_cast<char *>(path.c_str()), NULL};
			execve(args[0], args, environ);
			exit(1);
		} else {
			struct pollfd socket = { pipes[1], POLLIN, 0 };
			fds.push_back(socket);
			fdcgi_in.push_back(pipes[1]);
			pipe_client[pipes[1]] = active_client;
			struct pollfd client_stop = { active_client, POLLERR, 0 };
            fds.push_back(client_stop);
		}
		return ;
	}
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open()){
		client_response[active_client] = HTTP404 + server_conf.error_pages[404];
	}
	std::stringstream body;
	body << file.rdbuf();
	file.close();

	std::stringstream content_length; 
	content_length << body.str().size();

	client_response[active_client] = "HTTPHTTP/1.1 200 OK\r\n";
	client_response[active_client] += "Content-Type: " + this->getContentType(this->findExtension(path)) + "\r\n";
	client_response[active_client] += "Content-Length: " + content_length.str() + "\r\n";
	std::cout << "Response writen" << std::endl;
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

	/*if (!server_conf.locations[index].redirect_target.empty()) {
		std::map<int, std::string>::iterator it = server_conf.locations[index].redirect_target.begin();
		int code = it->first;
		std::string loc = it->second;
		if (code == 301) {
			client_response[active_client] = "HTTP/1.1 301 Moved Permanently\r\nLocation: " + loc + file + "\r\n\r\n";
		}
	} else */if (method == "GET") {
		if (server_conf.locations[index].limits[0] == "NONE" || !this->checkLimits(server_conf.locations[index].limits, "GET")) {
			if (path == "/") { path = "/" + server_conf.locations[index].index; }
			getFile(path, server_conf.locations[index].root, server_conf.cgi, request);
			return ;
		}
		client_response[active_client] = HTTP405 + server_conf.error_pages[405];
	} /* else if (method == "POST") {
		if (server_conf.locations[index].limits[0] == "NONE" || !this->checkLimits(server_conf.locations[index].limits, "POST")) {
			if (path == "/upload") { handlePostUpload(request, server_conf.locations[index].root); }
			else { handlePost(request, path, server_conf.locations[index].root); }
		}
		client_response[active_client] = HTTP405 + server_conf.error_pages[405];
	} else if (method == "DELETE") {
		if (server_conf.locations[index].limits[0] == "NONE" || !this->checkLimits(server_conf.locations[index].limits, "DELETE")) { 
			handle_delete(server_conf.locations[index].root, request);
		}
		client_response[active_client] = HTTP405 + server_conf.error_pages[405];
	}*/
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

std::string ServerManager::findExtension(const std::string& url) {
    size_t pos = url.find_last_of('.');
    
    if (pos == std::string::npos || pos == url.length() - 1) {
        return "";
    }

    return url.substr(pos + 1);
}

std::string ServerManager::getContentType(const std::string& extension){
	if (extension == "txt")
		return ("text/plain");
	else if (extension == "csv")
		return ("text/csv");
	else if (extension == "html" || extension == "htm")
		return ("text/html");
	else if (extension == "json")
		return ("application/json");
	else if (extension == "xml")
		return ("application/xml");
	else if  (extension == "jpg" || extension == "jpeg")
		return ("image/jpeg");
	else if (extension  == "png")
		return ("image/png");
	else if (extension == "gif")
		return ("image/gif");
	else if (extension == "svg")
		return ("image/svg+xml");
	else if (extension == "mp3")
		return ("audio/mpeg");
	else if (extension == "mp4")
		return ("video/mp4");
	else if (extension == "wav")
		return ("audio/wav");
	else if (extension == "webm")
		return ("video/webm");
	else if (extension == "pdf")
		return ("application/pdf");
	else if (extension == "doc" || extension == "docx")
		return ("applicaton/msword");
	else if (extension == "xls" || extension == "xlsx")
		return ("application/vnd.ms-excel");
	else if (extension == "zip")
		return ("application/zip");
	else
		return ("application/octet-stream");
}
