#include "ServerManager.hpp"

//static bool running = true;
sem_t semaphore; 

/**
 * @brief The constructor for the ServerManager class
 */
ServerManager::ServerManager(ConfigParser::Server server_conf, int socket) {
    this->server_confs.push_back(server_conf);
    this->server_fd = socket;
}

ServerManager::~ServerManager() {}

void ServerManager::setSocketLinger(int socket_df)
{
	struct linger sl;
	sl.l_onoff = 1;
	sl.l_linger = 0;

	if (setsockopt(socket_df, SOL_SOCKET, SO_LINGER, &sl, sizeof(sl)) < 0)
    	perror("setsockopt(SO_LINGER) failed");
}

/**
 * @brief deletes the requested resource
 * 
 * @param resource the resource that is going to be deleted.
 * 
 * @return true on success false on failure.
*/
bool ServerManager::deleteResource(std::string resource){
	if (std::remove(resource.c_str()) == 0){
		return true;
	} else {
		return false;
	}
}
/**
 * @brief handle delete request
 * 
 * @param request the request recieved
 * 
 * @return 200 on success 404 on failure.
*/
std::string ServerManager::handle_delete(std::string root, std::string request){
	std::size_t pos = request.find(" ") + 1;
	std::size_t pos_end = request.find(" ", pos);
	std::string resource = root;
	resource += request.substr(pos, pos_end - pos);
	std::cout << resource << std::endl;

	if(deleteResource(resource)){
		return "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>200 DELETE/h1>";
	} else {
		return HTTP404;
	}
}


/**
 * @brief The handlePost function handles HTTP POST requests containing file data sent by the client.
 * 
 * @param request The request that has been sent to the server.
 * @param server_root the path were the servers static files are located.
 * 
 * @details line 1 of the function to line 4 -> find the end of the header, if the delimiter is not found an error 400 is send.
 * line 6 of the function to line 10 -> extract the headers of the request and verify Content-Lenght, if is not present an error 411(Lenght Required) is send.
 * line 12 of the function to line 15 -> verify the content is multipart/form-data, if is not specified an error 415(Unsupported Media Type) is send.
 * line 17 of the function to line 24 -> obtain the multipart delimiter, on failure an error 400 is send.
 * line 26 of the function to line 38 -> extract the request body, on failure an error 400 is send.
 * line 40 of the function to line 46 -> extract the name of the file, on failure an error 400 is send.
 * line 48 of the function to line 53 -> extract the file data, on faiure an error 400 is send.
 * line 55 of the function to line 62 -> save the file in the server.
 * 
 * 
 * @returns on failure a Error Message with its coresponding id, on success a Success Message with its coresponding id.
 */
std::string ServerManager::handlePostUpload(std::string request, std::string server_root) {
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
    int fd = open(file_path.c_str(), O_CREAT | O_WRONLY, 0666);
    if (fd < 0) {
        return HTTP500 + this->active_server.error_pages[500];
    }
    write(fd, file_data.c_str(), file_data.size());
    close(fd);

    return "HTTP/1.1 201 Created\r\nContent-Type: text/html\r\n\r\n<h1>File Uploaded Successfully</h1>";
}


/** 
* @brief executes php with a POST request.
* 
* @param reuquest The HTTP request the server recieves from the client.
* @param request_path The path were the php is going to be executed.
* @param server_root Were the file is in the server.
*
* @return on success a 200 and the response to the request, on failure the corresponding HTTP error.
*/
std::string ServerManager::handlePost(std::string request, std::string request_path, std::string server_root) {
	std::cout << "Entra al POST" << std::endl;

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

	int fd_read[2];
	int fd_write[2];
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

	/*struct pollfd write_pipe = {fd_write[1], POLLIN, 0};
	this->fds.push_back(write_pipe);*/

	write(fd_write[1], body.c_str(), body.size());
	close(fd_write[1]);
	close(fd_read[1]);
	std::string output;
	char buffer[BUFFER_SIZE];
	std::memset(buffer, 0, BUFFER_SIZE);

	/*struct pollfd read_pipe = {fd_read[0], POLLIN, 0};
	this->fds.push_back(read_pipe);*/

	while (read(fd_read[0], buffer, 1023) > 0){
		output.append(buffer);
		std::memset(buffer, 0, 1024);
	}
	close(fd_read[0]);

    header_end = output.find("\r\n\r\n"); 
    if (header_end == std::string::npos) {
        std::cerr << "Respuesta PHP malformada." << std::endl;
        return HTTP500 + this->active_server.error_pages[500];
    }

	headers.clear();
    headers = output.substr(0, header_end);
    std::string content = output.substr(header_end + 4);

    std::string response = "HTTP/1.1 200 OK\r\n";
    response += headers + "\r\n";
	response += "Connection: close\r\n";
    response += "Content-Length: " + ft_itoa(content.size()) + "\r\n\r\n";
    response += content;
	std::cout << "Sale del POST" << std::endl;
    return response;
}


/**
 * @brief Serves the static file that the client request throught the GET request.
 * 
 * @param request_path the path of the file the client requests
 * @param server_root the path were the servers static files are located
 * 
 * @return a std::string of the static file contents.
 */
std::string ServerManager::getFile(std::string request_path, std::string server_root, std::string cgi, std::string request){
	std::string path = server_root + request_path;
	std::cout << "Entra al GET" << std::endl;
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
			return HTTP500;
		}

		pid_t pid = fork();
		if (pid == -1) {
			return HTTP500;
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

			char buffer[BUFFER_SIZE];
			std::string response = "HTTP/1.1 200 OK\r\n";
			std::string aux;

			ssize_t bytes_read;
			while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer))) > 0) {
				aux.append(buffer, bytes_read);
			}
			close(pipe_fd[0]);
			int status;
			waitpid(pid, &status, 0);
			std::size_t header_end = aux.find("\r\n\r\n");
			if (header_end == std::string::npos) {
				return HTTP400 + this->active_server.error_pages[400];
			}
			std::string content = aux.substr(header_end + 4);
			response += "Connection: close\r\n";
			response += "Content-Length:" + ft_itoa(std::strlen(content.c_str())) + "\r\n";
			response += "Content-Type: text/html\r\n";
			response += aux;

			std::cout << "Sale del GET" << std::endl;
			return response;
		}
	}

	int fd = open(path.c_str(), O_RDONLY);
	if (fd < 0){
		return HTTP400 + this->active_server.error_pages[400];
	}

	char buffer[BUFFER_SIZE];
	std::string response = "HTTP/1.1 200 OK\r\nConnection: close\r\n";
	std::string extension = this->findExtension(path);
	std::string content_type = this->getContentType(extension);
	response += "Content-Type: " + content_type + "\r\n";
	std::string content;
	ssize_t bytes;
	while ((bytes = read(fd, buffer, sizeof(buffer))) > 0){
		content.append(buffer, bytes);
	}
	response += "Content-Length:" + ft_itoa(std::strlen(content.c_str())) + "\r\n\r\n";
	response += content;
	
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
	size_t index = 0;

	req_stream >> method >> path >> protocol;

	std::size_t last_bar = path.find_last_of("/");
	std::string directory_path = path.substr(0, last_bar);
	std::string file = path.substr(last_bar);
	for (size_t i = 0; i < server_conf.locations.size(); i++){
		if (server_conf.locations[i].path == directory_path){
			index = i;
			break;
		}
	}
	if (!server_conf.locations[index].redirect_target.empty()) {
		std::map<int, std::string>::iterator it = server_conf.locations[index].redirect_target.begin();
		int code = it->first;
		std::string loc = it->second;
		if (code == 301) {
			return "HTTP/1.1 301 Moved Permanently\r\nLocation: " + loc + file + "\r\n\r\n";
		}
	}
	if (server_conf.locations[index].limits.empty())
		server_conf.locations[index].limits.push_back("NONE");
	if (method == "GET"){
		if (server_conf.locations[index].limits[0] == "NONE" || !this->checkLimits(server_conf.locations[index].limits, "GET")){
			if (path == "/")
				path = "/" + server_conf.locations[index].index;
			return getFile(path, server_conf.locations[index].root, server_conf.cgi, request);
		}
		return HTTP405 + this->active_server.error_pages[405];
	} else if (method == "POST"){
		if (server_conf.locations[index].limits[0] == "NONE" || !this->checkLimits(server_conf.locations[index].limits, "GET")){
			if (path == "/upload") {
				return handlePostUpload(request, server_conf.locations[index].root);
			}
			return handlePost(request, path, server_conf.locations[index].root);
		}
		return HTTP405 + this->active_server.error_pages[405];
	} else if (method == "DELETE"){
		return handle_delete(server_conf.locations[index].root, request);
	}
	return HTTP405 + this->active_server.error_pages[405];
}

/**
 * @brief Starts the server so it can handle request and connections.
 */
/*void ServerManager::startServer(){
	std::string prontf;

	struct pollfd server_pollfd = {this->server_fd, POLLIN, 0};
    this->fds.push_back(server_pollfd);

	struct pollfd stdin_pollfd = {STDIN_FILENO, POLLIN, 0};
	this->fds.push_back(stdin_pollfd);

	int count = poll(&(this->fds[0]), this->fds.size(), -1);

	if (count < 0){
		std::cerr << "Error en poll: ";
		perror("poll");
		running = false;
		return ;
	}
	for (size_t i = 0; i < fds.size(); i++){
		if (fds[i].revents & POLLIN){
			if (fds[i].fd == STDIN_FILENO) {
				std::getline(std::cin, prontf);
				if (prontf == "exit"){
					std::cout << "Exiting server...\n";
					running = false;
					break;
				}
			}
			else if (fds[i].fd == this->server_fd){
				struct sockaddr_in client;
				socklen_t client_len = sizeof(client);
				int client_fd = accept(this->server_fd, (struct sockaddr*)&client, &client_len);
				if (client_fd >= 0){
					struct pollfd poll_client = {client_fd, POLLIN, 0};
					fds.push_back(poll_client);
					std::cout << "Cliente conectado: " << client_fd << std::endl;
				} else {
					std::cerr << "No se pudo conectar el cliente: " << client_fd << std::endl;
				}	
			} else {
				char buffer[BUFFER_SIZE];
				std::memset(buffer, 0, sizeof(buffer));
				ssize_t bytes = read(fds[i].fd, buffer, sizeof(buffer));
				if (bytes > 0){
					ConfigParser::Server server_conf = getServerName(std::string(buffer, bytes));
					std::string const response = handle_request(std::string(buffer, bytes), server_conf);
					send(fds[i].fd, response.c_str(), strlen(response.c_str()), 0);
					std::cout << "Client disconnected: " << fds[i].fd << std::endl;
					close(fds[i].fd);
					fds.erase(fds.begin() + i);
					--i;
				} else {
					std::cout << "Client disconnected: " << fds[i].fd << std::endl;
					close(fds[i].fd);
					fds.erase(fds.begin() + i);
					--i;
				}
			}
		}
	}
	for (size_t i = 0; i < fds.size(); i++){
		if (fds[i].fd == this->server_fd || fds[i].fd == STDIN_FILENO){
			fds.erase(fds.begin() + i);
			--i;
		}

	}
}*/

ConfigParser::Server ServerManager::getServerName(std::string request){
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
ConfigParser::Server ServerManager::getServersConf(){
	return this->server_confs[0];
}

/**
 * @brief Adds a server conf to the ServerManager object.
 * 
 * @param server_conf The server configuration that is going to be added.
 */
void ServerManager::addConf(ConfigParser::Server server_conf){
	this->server_confs.push_back(server_conf);
}

/**
 * @brief Checks if the method is in the limits vector.
 * 
 * @param limits The limits vector for the location in question.
 * @param search The method that is going to be checked.
 * 
 * @return If the method is found true, otherwise false.
 */
int ServerManager::checkLimits(std::vector<std::string> limits, std::string search) const{
	int exit = 0;
	for (size_t i = 0; i < limits.size(); i++){
		if (limits[i] == search){
			exit = 1;
		}
	}
	return exit;
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
