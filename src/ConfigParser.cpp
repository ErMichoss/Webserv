#include "../incl/ConfigParser.hpp"

/**
 * @brief This is the constructor of the class that is gonna manage the configuration file.
 * @param fileUrl The url of the config file
 */
ConfigParser::ConfigParser(char const* fileUrl): file(fileUrl){
	if (!file.is_open()) {
        std::cerr << "Could not open file" << std::endl;
	}
}

/**
 * @brief This method is going to add a Server struct to the std::list<Server> of this class.
 * @return returns 0 if everything went OK returns 1 if something went wrong.
 */
int ConfigParser::addServerConf(){
	std::string line;
	Server server;

	while (std::getline(this->file, line)){
		this->trim(line);
		if (line.substr(0, 6) == "SERVER"){
			while(std::getline(this->file, line)){
				this->trim(line);
				size_t index = line.find("#", 0);
				if (index != 0)
					line = line.substr(0, index);
				if (line.substr(0, 6) == "SERVER")
					break;
				if (line.substr(0, 6) == "listen"){
					if (this->checkColon(6, line))
						return 1;
					std::string portNum = line.substr(7);
					this->trim(portNum);
					for (int i = 0; i < std::strlen(portNum.c_str()); i++){
						if (!std::isdigit(portNum.c_str()[i])){
							std::cerr << "Error: Non-numeric port in 'listen' configuration" << portNum << std::endl;
						}
					}
					server.port = atoi(portNum.c_str());
				} else if (line.substr(0, 4) == "host"){
					if (this->checkColon(4, line))
						return 1;
					std::string host = line.substr(5);
					this->trim(host);
					server.host = host;
				} else if (line.substr(0, 11) == "server_name"){
					if (this->checkColon(11, line))
						return 1;
					std::string snValue = line.substr(12);
					this->trim(snValue);
					server.server_name = snValue;
				} else if (line.substr(0, 4) == "root"){
					if (this->checkColon(4, line))
						return 1;
					std::string rootValue = line.substr(5);
					this->trim(rootValue);
					server.root = rootValue;
				} else if (line.substr(0, 8) == "LOCATION"){
					Location location;

					if (this->checkColon(8, line))
						return 1;
					std::string path = line.substr(10, (line.length() - 10));
					this->trim(path);
					location.path = path;
					while (std::getline(this->file, line)){
						this->trim(line);
						if (line.substr(0, 6) == "SERVER" || line.substr(0, 8) == "LOCATION")
							break;
						if (line.substr(0, 4) == "root"){
							if (this->checkColon(4, line))
								return 1;
							std::string root = line.substr(5);
							this->trim(root);
							location.root = root;
						} else if (line.substr(0, 5) == "index") {
							if (this->checkColon(5, line))
								return 1;
							std::string index = line.substr(6);
							this->trim(index);
							location.index = index;
						} else if (line.substr(0, 9) == "autoindex"){
							if (this->checkColon(9, line))
								return 1;
							std::string autoindex = line.substr(10);
							this->trim(autoindex);
							if (autoindex != "on" && autoindex != "off"){
								std::cerr << "Error: Unexpected word " << autoindex << " on line: " << line << std::endl;
								return 1;
							} else if (autoindex == "on"){
								location.autoindex = true;
							} else {
								location.autoindex = false;
							}
						} else if (line.substr(0, 15) == "redirect_target"){
							if (this->checkColon(15, line))
								return 1;
							std::string redirect = line.substr(16);
							this->trim(redirect);
							location.redirect_target = redirect;
						}
					}
					server.locations.push_back(location);
				} else if ( !line.empty() &&line.c_str()[0] != '#'){
					std::cerr << "Unexpected line: " << line << std::endl;
					return 1;
				}
			}
		}
		this->servers.push_back(server);
	}
	return 0;
}