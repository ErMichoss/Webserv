#include "ServerManager.hpp"

/**
 * @brief The constructor for the ServerManager class
 */
ServerManager::ServerManager(ConfigParser::Server server_conf, int socket){
	this->server_confs.push_back(server_conf);
	this->server_fd = socket;
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
std::string ServerManager::handlePost(std::string request, std::string server_root) {
    std::size_t header_end = request.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        return "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<h1>400 Bad Request1</h1>";
    }

    std::string headers = request.substr(0, header_end);
    std::size_t content_length_pos = headers.find("Content-Length: ");
    if (content_length_pos == std::string::npos) {
        return "HTTP/1.1 411 Length Required\r\nContent-Type: text/html\r\n\r\n<h1>411 Length Required</h1>";
    }

    std::size_t content_type_pos = headers.find("Content-Type: multipart/form-data;");
    if (content_type_pos == std::string::npos) {
        return "HTTP/1.1 415 Unsupported Media Type\r\nContent-Type: text/html\r\n\r\n<h1>415 Unsupported Media Type</h1>";
    }

    std::string boundary_prefix = "boundary=";
    std::size_t boundary_pos = headers.find(boundary_prefix, content_type_pos);
    if (boundary_pos == std::string::npos) {
        return "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<h1>400 Bad Request2</h1>";
    }
    std::string boundary = "--" + headers.substr(boundary_pos + boundary_prefix.size());
    boundary = boundary.substr(0, boundary.find("\r\n"));

    std::string body = request.substr(header_end + 4);
	std::cout << body << std::endl;
    std::size_t file_start = body.find(boundary);
    if (file_start == std::string::npos) {
        return "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<h1>400 Bad Request3</h1>";
    }
    file_start += boundary.size() + 2;

    std::size_t file_end = body.find(boundary, file_start);
    if (file_end == std::string::npos) {
        return "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<h1>400 Bad Request4</h1>";
    }
    std::string file_content = body.substr(file_start, file_end - file_start);

    std::size_t filename_pos = file_content.find("filename=\"");
    if (filename_pos == std::string::npos) {
        return "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<h1>400 Bad Request5</h1>";
    }
    std::string filename = file_content.substr(filename_pos + 10);
    filename = filename.substr(0, filename.find("\""));

    std::size_t data_start = file_content.find("\r\n\r\n");
    if (data_start == std::string::npos) {
        return "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<h1>400 Bad Request6</h1>";
    }
    data_start += 4;
    std::string file_data = file_content.substr(data_start);

    std::string file_path = server_root + "/" + filename;
    int fd = open(file_path.c_str(), O_CREAT | O_WRONLY, 0666);
    if (fd < 0) {
        return "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\n\r\n<h1>500 Internal Server Error</h1>";
    }
    write(fd, file_data.c_str(), file_data.size());
    close(fd);

    return "HTTP/1.1 201 Created\r\nContent-Type: text/html\r\n\r\n<h1>File Uploaded Successfully</h1>";
}


/**
 * @brief Serves the static file that the client request throught the GET request.
 * 
 * @param request_path the path of the file the client requests
 * @param server_root the path were the servers static files are located
 * 
 * @return a std::string of the static file contents.
 */
std::string ServerManager::getFile(std::string request_path, std::string server_root){
	std::string path = server_root + request_path;
	std::cout << path << std::endl;
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
	size_t index = 0;

	req_stream >> method >> path >> protocol;

	for (size_t i = 0; i < server_conf.locations.size(); i++){
		if (server_conf.locations[i].path == path){
			index = i;
			break;
		}
	}
	if (server_conf.locations[index].limits.empty())
		server_conf.locations[index].limits.push_back("NONE");
	if (method == "GET"){
		if (server_conf.locations[index].limits[0] == "NONE" || !this->checkLimits(server_conf.locations[index].limits, "GET")){
			if (path == "/")
				path = "/" + server_conf.locations[index].index;
			return getFile(path, server_conf.root);
		}
		return "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/html\r\n\r\n<h1>405 Method Not Allowed</h1>";
	} else if (method == "POST"){
		if (server_conf.locations[index].limits[0] == "NONE" || !this->checkLimits(server_conf.locations[index].limits, "GET")){
			if (path == "/upload") {
				return handlePost(request, server_conf.root);
			}
		}
		return "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/html\r\n\r\n<h1>405 Method Not Allowed</h1>";
	} else if (method == "DELETE"){
		//ejecutar DELETE
		return "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>200 DELETE/h1>";
	}
	return "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/html\r\n\r\n<h1>405 Method Not Allowed</h1>";
}

/**
 * @brief Starts the server so it can handle request and connections.
 */
void ServerManager::startServer(){
	std::vector<struct pollfd> fds;
	struct pollfd server_pollfd = {this->server_fd, POLLIN, 0};
    fds.push_back(server_pollfd);

	signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

	while (1) {
		int count = poll(&fds[0], fds.size(), -1);

		if (!running)
			break;
			
		if (count < 0){
			std::cerr << "Error en poll: ";
			perror("poll");
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
						ConfigParser::Server server_conf = getServerName(std::string(buffer, bytes));
						std::string const response = handle_request(std::string(buffer, bytes), server_conf);
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
	close(fds[0].fd);
	close(this->server_fd);
}

ConfigParser::Server ServerManager::getServerName(std::string request){
	std::istringstream req_stream(request);
	std::string protocol, host, temp;
        
    std::getline(req_stream, temp, '/');
    std::getline(req_stream, host, '/');

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

