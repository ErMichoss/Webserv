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

void ServerManager::removePipeIn(int fd) {
	std::vector<int>::iterator it = std::remove(this->fdcgi_in.begin(), this->fdcgi_in.end(), fd);

	this->fdcgi_in.erase(it, this->fdcgi_in.end());
}

void ServerManager::removePipeOut(int fd) {
	std::vector<int>::iterator it = std::remove(this->fdcgi_out.begin(), this->fdcgi_out.end(), fd);

	this->fdcgi_out.erase(it, this->fdcgi_out.end());
}

void ServerManager::setActiveClient(int fd) {
	this->active_client = fd;
}

void ServerManager::readErrorPages(std::string header, std::string body, int client){
	client_response[client] = header + "Content-Length: " + ft_itoa(std::strlen(body.c_str())) + "\r\n\r\n" + body;
}

void ServerManager::writePOST(int fd){
	ssize_t totalEnviado = 0;
    ssize_t longitud = w_size[fd];

    while (totalEnviado < longitud) {
        ssize_t restante = longitud - totalEnviado;
        ssize_t aEnviar = (restante < BUFFER_SIZE) ? restante : BUFFER_SIZE;

        ssize_t bytesEnviados = send(fd, to_write[fd].c_str() + totalEnviado, aEnviar, 0);
        if (bytesEnviados == -1) {
            readErrorPages(HTTP415, server_conf.error_pages[500], active_client);
			return ;
        }
        totalEnviado += bytesEnviados;
		std::cerr << __FILE__ << ":" << __LINE__ << ": " << totalEnviado << std::endl;
		std::cerr << __FILE__ << ":" << __LINE__ << ": " << longitud << std::endl;
		//std::cerr << to_write[fd] << std::endl;
		//std::cerr << "Lo escrito: " << to_write[fd].substr(totalEnviado, bytesEnviados) << " Ocupa: " << bytesEnviados << std::endl;
		// int fdf = open("file", O_CREAT | O_RDWR, 0644);
		// write(fdf, to_write[fd].c_str() + totalEnviado - bytesEnviados , bytesEnviados);
    }
	fdcgi_in.push_back(fd);
	removePipeOut(fd);
	end_write[fd] = true;
}

void ServerManager::readCgi(int pipe) {
	char buffer[BUFFER_SIZE];
	int client_id = pipe_client[pipe];
	client_response[client_id] = "HTTP/1.1 200 OK\r\n";

	ssize_t bytes_read = 0;
	bytes_read = read(pipe, buffer, sizeof(buffer));
	if (bytes_read < (ssize_t)sizeof(buffer) && bytes_read > -1){
		client_response[pipe].append(buffer, bytes_read);
		close(pipe);
		std::size_t header_end = client_response[pipe].find("\r\n\r\n");
		if (header_end == std::string::npos) {
			client_response[pipe].erase();
			stopped_value[client_id] = false;
			this->removePipeIn(pipe);
			readErrorPages(HTTP400, server_conf.error_pages[400], client_id);
			return ;
		}
		std::string content = client_response[pipe].substr(header_end + 4);
		client_response[client_id] += "Content-Length: " + ft_itoa(std::strlen(content.c_str())) + "\r\n";
		client_response[client_id] += client_response[pipe];
		client_response[pipe].erase();
		this->removePipeIn(pipe);
		stopped_value[client_id] = false;
		if (!valid_body[client_id]){
			readErrorPages(HTTP413, server_conf.error_pages[413], client_id);
			return ;
		}
	} else if (bytes_read > -1) {
		client_response[pipe].append(buffer, bytes_read);
	} else {
		close(pipe);
		std::size_t header_end = client_response[pipe].find("\r\n\r\n");
		if (header_end == std::string::npos) {
			this->removePipeIn(pipe);
			client_response[pipe].erase();
			stopped_value[client_id] = false;
			readErrorPages(HTTP400, server_conf.error_pages[400], client_id);
			return ;
		}
		this->removePipeIn(pipe);
		std::string content = client_response[pipe].substr(header_end + 4);
		client_response[client_id] += "Content-Length: " + ft_itoa(std::strlen(content.c_str())) + "\r\n";
		client_response[client_id] += client_response[pipe];
		client_response[pipe].erase();
		stopped_value[client_id] = false;
	}
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
		client_response[active_client] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 19\r\n\r\n<h1>200 DELETE</h1>";
		return ; 
	} else {
		client_response[active_client] = HTTP404 + server_conf.error_pages[404];
		return ;
	}
}

std::string ServerManager::getFileName(std::string body, std::string boundary){
	std::size_t file_part_start = body.find(boundary);
	if (file_part_start == std::string::npos) {
		readErrorPages(HTTP400, server_conf.error_pages[400], active_client);
		return "";
	}

	std::size_t disposition_pos = body.find("Content-Disposition: form-data;", file_part_start);
	if (disposition_pos == std::string::npos) {
		readErrorPages(HTTP400, server_conf.error_pages[400], active_client);
		return "";
	}

	std::size_t filename_pos = body.find("filename=\"", disposition_pos);
	if (filename_pos == std::string::npos) {
		readErrorPages(HTTP400, server_conf.error_pages[400], active_client);
		return "";
	}
	filename_pos += 10;

	std::size_t filename_end = body.find("\"", filename_pos);
	if (filename_end == std::string::npos) {
		readErrorPages(HTTP400, server_conf.error_pages[400], active_client);
		return "";
	}

	std::string filename = body.substr(filename_pos, filename_end - filename_pos);
	return (filename);
}

void ServerManager::handlePostUpload(std::string request, std::string server_root){
	std::size_t header_end = request.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        readErrorPages(HTTP400, server_conf.error_pages[400], active_client);
        return;
    }

    std::string headers = request.substr(0, header_end);

    std::size_t content_length_pos = headers.find("Content-Length: ");
    if (content_length_pos == std::string::npos) {
        readErrorPages(HTTP411, server_conf.error_pages[411], active_client);
        return;
    }
    std::string content_length = headers.substr(content_length_pos + 16);
    std::size_t content_length_end = content_length.find("\r\n");
    if (content_length_end != std::string::npos) {
        content_length = content_length.substr(0, content_length_end);
    }
    this->valid_body[active_client] = checkBodySize(content_length);

    std::size_t content_type_pos = headers.find("Content-Type: ");
    if (content_type_pos == std::string::npos) {
        readErrorPages(HTTP415, server_conf.error_pages[415], active_client);
        return;
    }
    std::string content_type = headers.substr(content_type_pos + 14);
    std::size_t content_type_end = content_type.find("\r\n");
    if (content_type_end != std::string::npos) {
        content_type = content_type.substr(0, content_type_end);
    }

    if (content_type.find("multipart/form-data") == std::string::npos) {
        readErrorPages(HTTP400, server_conf.error_pages[400], active_client);
        return;
    }

    std::string boundaryPrefix = "boundary=";
    size_t boundary_pos = content_type.find(boundaryPrefix);
    if (boundary_pos == std::string::npos) {
        readErrorPages(HTTP400, server_conf.error_pages[400], active_client);
        return;
    }
    std::string boundary = "--" + content_type.substr(boundary_pos + boundaryPrefix.length()); // Agregar "--" al inicio

    std::string body = request.substr(header_end + 4);

    std::size_t file_start = body.find(boundary);
    if (file_start == std::string::npos) {
        readErrorPages(HTTP400, server_conf.error_pages[400], active_client);
        return;
    }
    file_start = body.find("\r\n\r\n", file_start);
    if (file_start == std::string::npos) {
        readErrorPages(HTTP400, server_conf.error_pages[400], active_client);
        return;
    }
    file_start += 4;

    std::size_t file_end = body.find(boundary, file_start);
    if (file_end == std::string::npos) {
        readErrorPages(HTTP400, server_conf.error_pages[400], active_client);
        return;
    }

    std::string file_content = body.substr(file_start, file_end - file_start);
    
    std::stringstream file_stream;
    file_stream << file_content;
	std::string file_name = getFileName(body, boundary);
	if (file_name == "")
		return;
	std::string file_path = server_root + "/uploads" + file_name;
    std::ofstream output_file(file_path.c_str(), std::ios::binary);
    if (!output_file) {
        readErrorPages(HTTP500, server_conf.error_pages[500], active_client);
        return;
    }
    output_file << file_stream.str();
    output_file.close();

	client_response[active_client] = "HTTP/1.1 200 OK\r\n";
	client_response[active_client] += "Content-Type: text/html\r\n";
	client_response[active_client] += "Content-Length: 33\r\n\r\n";
	client_response[active_client] += "<h1>File Uploaded Succesfuly</h1>";
}

void ServerManager::handlePost(std::string request, std::string request_path, std::string server_root){
	std::size_t header_end = request.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        readErrorPages(HTTP400, server_conf.error_pages[400], active_client);
		return ;
    }

    std::string headers = request.substr(0, header_end);
    std::size_t content_length_pos = headers.find("Content-Length: ");
    if (content_length_pos == std::string::npos) {
        readErrorPages(HTTP411, server_conf.error_pages[411], active_client);
		return ;
    }
    std::string content_length = headers.substr(content_length_pos + 16);
    std::size_t content_length_end = content_length.find("\r\n");
    if (content_length_end != std::string::npos) {
        content_length = content_length.substr(0, content_length_end);
    }
	this->valid_body[active_client] = checkBodySize(content_length);
    std::size_t content_type_pos = headers.find("Content-Type: ");
    if (content_type_pos == std::string::npos) {
        readErrorPages(HTTP415, server_conf.error_pages[415], active_client);
		return ;
    }
    std::string content_type = headers.substr(content_type_pos + 14);
    std::size_t content_type_end = content_type.find("\r\n");
    if (content_type_end != std::string::npos) {
        content_type = content_type.substr(0, content_type_end);
    }
	//std::cout << request << std::endl;
    setenv("REQUEST_METHOD", "POST", 1);
    setenv("CONTENT_TYPE", content_type.c_str(), 1);
    setenv("SCRIPT_FILENAME", (server_root + request_path).c_str(), 1);
    setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
    setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
    setenv("REDIRECT_STATUS", "true", 1);
	std::string body = request.substr(header_end + 4);
	setenv("CONTENT_LENGTH", ft_itoa(std::strlen(body.c_str())).c_str() , 1);
	//std::cout << body << std::endl;
	int pipes[2];
if (socketpair(AF_LOCAL, SOCK_STREAM, 0, pipes) == -1) {
    readErrorPages(HTTP500, server_conf.error_pages[500], active_client);
    return;
}

pid_t pid = fork();
if (pid == -1) {
    readErrorPages(HTTP500, server_conf.error_pages[500], active_client);
    return;
}
if (pid == 0) {
    dup2(pipes[0], STDOUT_FILENO);
    dup2(pipes[0], STDIN_FILENO); 
    close(pipes[0]);
    close(pipes[1]);

	chdir(server_conf.locations[_location].root.c_str());
	
    std::string command = "/usr/bin/php-cgi";

    char *const args[] = {const_cast<char *>(command.c_str()), const_cast<char *>(request_path.c_str()), NULL};
    execve(args[0], args, environ);
    exit(1);
} else {
    struct pollfd socket = { pipes[1], POLLOUT, 0 };
    fds.push_back(socket);
    fdcgi_out.push_back(pipes[1]);
	pipe_client[pipes[1]] = active_client;
	to_write[pipes[1]] = body;
	w_size[pipes[1]] = body.size();
	end_write[pipes[1]] = false;
	stopped_value[active_client] = true;
}
return;

}

void ServerManager::getDir(std::string path, std::string uri){
	struct dirent *read_dir;
	DIR* dir = opendir(path.c_str());

	if (dir == NULL){
		readErrorPages(HTTP500, server_conf.error_pages[500], active_client);
		return;
	}
	std::string html = "<ul>";
	while ((read_dir = readdir(dir))){
		html += "<li><a href='" + uri + "/" + read_dir->d_name + "'>" + read_dir->d_name + "</a></li>";
	}
	html += "</ul>";
	client_response[active_client] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n";
	client_response[active_client] += "Content-Length: " + ft_itoa(std::strlen(html.c_str())) + "\r\n\r\n";
	client_response[active_client] += html;
}

void ServerManager::getFile(std::string request_path, std::string server_root, std::string cgi, std::string request) {
	std::string path = server_root + request_path;
	struct stat statbuf;
	std::memset(&statbuf, 0, sizeof(statbuf));
	if (stat(path.c_str(), &statbuf) == -1) {
		readErrorPages(HTTP404, server_conf.error_pages[404], active_client);
	}
	if (S_ISDIR(statbuf.st_mode) && !server_conf.locations[_location].index.empty()){
		path += server_conf.locations[_location].index;
	} else if (S_ISDIR(statbuf.st_mode) && server_conf.locations[_location].autoindex){
		getDir(path, request_path);
		return;
	} else if (!S_ISREG(statbuf.st_mode)){
		readErrorPages(HTTP404, server_conf.error_pages[404], active_client);
	}
	if (path.find("." + server_conf.cgi) != std::string::npos && !cgi.empty()) {
		unsetenv("CONTENT_TYPE");
		unsetenv("CONTENT_LENGTH");
		setenv("REQUEST_METHOD", "GET", 1);
		setenv("SCRIPT_FILENAME", path.c_str(), 1);
		setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
		setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
		setenv("REDIRECT_STATUS", "1", 1);
		size_t start = request.find_first_of('?');
		if (start != std::string::npos){
			size_t end = request.find_first_of(' ', start);
			if (end != std::string::npos){
				std::string quri = request.substr(start, end - start);
				setenv("QUERY_STRING", quri.c_str(), 1);
			} else {
				setenv("QUERY_STRING", "", 1);
			}
		} else {
			setenv("QUERY_STRING", "", 1);
		}

		int pipes[2];
		if (socketpair(AF_LOCAL, SOCK_STREAM, 0, pipes) == -1){
			client_response[active_client] = HTTP500 + this->server_conf.error_pages[500];
			return ;
		}
		//std::cout << pipes[0] << std::endl;
		pid_t pid = fork();
		if (pid == -1){
			client_response[active_client] = HTTP500 + this->server_conf.error_pages[500];
			return ;
		}
		if (pid == 0){
			dup2(pipes[1], STDOUT_FILENO);
			close(pipes[1]);
			close(pipes[0]);

			std::string command = "/usr/bin/php-cgi";
			char *const args[] = {const_cast<char *>(command.c_str()), const_cast<char *>(path.c_str()), NULL};
			execve(args[0], args, environ);
			exit(1);
		} else {
			close(pipes[1]);
			struct pollfd socket = { pipes[0], POLLIN, 0 };
			fds.push_back(socket);
			fdcgi_in.push_back(pipes[0]);
			pipe_client[pipes[0]] = active_client;
			stopped_value[active_client] = true;
		}
		return ;
	}
	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file.is_open()){
		readErrorPages(HTTP404, server_conf.error_pages[404], active_client);
		return ;
	}
	std::stringstream body;
	body << file.rdbuf();
	file.close();

	std::stringstream content_length; 
	content_length << body.str().size();

	client_response[active_client] = "HTTP/1.1 200 OK\r\n";
	client_response[active_client] += "Content-Type: " + this->getContentType(this->findExtension(path)) + "\r\n";
	client_response[active_client] += "Content-Length: " + content_length.str() + "\r\n\r\n";
	client_response[active_client] += body.str();
}

std::string ServerManager::findDir(std::string path){
	std::size_t last_bar = path.find_last_of("/");
	if (last_bar != std::string::npos){
		std::string dir = path.substr(last_bar);
		if (findExtension(dir) == "")
			return dir;
		else {
			dir = path.substr(0, last_bar);
			return dir;
		}
	}
	return "";
}

bool ServerManager::checkBodySize(std::string content_length){
	if (server_conf.body_size == -1)
		return true;
	if (server_conf.body_size < atoi(content_length.c_str())){
		return false;
	}
	return true;
}

void ServerManager::handle_request(std::string const request, ConfigParser::Server server_conf) {
	std::string request_copy = request;
	std::istringstream req_stream(request_copy);
	std::string method, path, protocol;
	std::size_t index = 0;
	client_response[active_client] = "";

	req_stream >> method >> path >> protocol;

	std::string directory_path = "";
	std::string file = "";
	std::size_t last_bar = path.find_last_of("/");
	if (last_bar != std::string::npos){
		directory_path = findDir(path);
		file = path.substr(last_bar);
		if (file == directory_path)
			file = "";
	}

	for (std::size_t i = 0; i < server_conf.locations.size(); i++) {
		if (server_conf.locations[i].path == directory_path) {
			index = i;
			break;
		}
	}

	if (server_conf.locations[index].limits.empty()){
		server_conf.locations[index].limits.push_back("NONE");
	}
	_location = index;
	if (server_conf.locations[index].root.empty()){
		server_conf.locations[index].root = server_conf.root;
	}
	this->valid_body[active_client] = true;
	if (!server_conf.locations[index].redirect_target.empty()) {
		std::map<int, std::string>::iterator it = server_conf.locations[index].redirect_target.begin();
		int code = it->first;
		std::string loc = it->second;
		if (code == 301) {
			client_response[active_client] = "HTTP/1.1 301 Moved Permanently\r\nLocation: " + loc + file + "\r\n\r\n";
		}
	} else if (method == "GET") {
		if (server_conf.locations[index].limits[0] == "NONE" || !this->checkLimits(server_conf.locations[index].limits, "GET")) {
			getFile(path, server_conf.locations[index].root, server_conf.cgi, request);
			return ;
		}
		readErrorPages(HTTP405, server_conf.error_pages[405], active_client);
		return ;
	} else if (method == "POST") {
		if (server_conf.locations[index].limits[0] == "NONE" || !this->checkLimits(server_conf.locations[index].limits, "POST")) {
			if (path == "/upload") { handlePostUpload(request, server_conf.locations[index].root); return;}
			handlePost(request, path, server_conf.locations[index].root);
			return ;
		}
		readErrorPages(HTTP405, server_conf.error_pages[405], active_client);
		return ;
	} else if (method == "DELETE") {
		if (server_conf.locations[index].limits[0] == "NONE" || !this->checkLimits(server_conf.locations[index].limits, "DELETE")) { 
			handle_delete(server_conf.locations[index].root, request);
			return ;
		}
		readErrorPages(HTTP405, server_conf.error_pages[405], active_client);
		return ;
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
