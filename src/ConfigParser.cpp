#include "../incl/ConfigParser.hpp"

/**
 * This is the constructor of the class that is gonna manage the configuration file.
 * @param[in] fileUrl The url of the config file
 */
ConfigParser::ConfigParser(std::string fileUrl): file(fileUrl){
	if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + fileUrl);
	}
}

/**
 * This method is going to add a Server struct to the std::list<Server> of this class.
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
				if (line.substr(0, 6) == "SERVER")
					break;
				if (line.substr(0, 6) == "listen"){
					if (this->checkColon(6, line))
						return 1;
					std::string portNum = line.substr(7);
					this->trim(portNum);
					for (char c : portNum){
						if (!std::isdigit(c)){
							std::cerr << "Error: Non-numeric port in 'listen' configuration" << portNum << std::endl;
						}
					}
					server.port = std::atoi(portNum.c_str());
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
				}
				/* else if (line.substr(0, 8) == "LOCATION"){
					Location location;

					if (this->checkColon(9, line))
						return 1;
					path = line.substr(10, (line.length() - 10));
					this->trim(path);
					location.path = path;
					while (std::getline(this->file, line)){
						

					}
				}*/
			}
		}
		this->servers.push_back(server);
	}
	return 0;
}