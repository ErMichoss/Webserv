#include "ConfigParser.hpp"

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
 * @brief auxiliar method for the extractServerConf method. It gets the location information from the configuartion file.
 * 
 * @param location A pointer to a Location struct that is going to contain the Location configuration.
 * @param line The line from which it is going to exctract the information.
 * 
 * @return on success 0, on failure 1, and if it reads the Configuration close param it returns 2.
 */
int ConfigParser::extractLocationConf(Location& location, std::string line){
	if (line.substr(0, 13) == "CLOSELOCATION") return 2;

	if (line.substr(0, 4) == "root"){
		std::string root;

		if (this->checkColon(4, line)) return 1;

		root = line.substr(5);
		this->trim(root);

		location.root = root;
	} else if (line.substr(0, 5) == "index"){
		std::string index;

		if (this->checkColon(5, line)) return 1;

		index = line.substr(6);
		this->trim(index);

		location.index = index;
	} else if (line.substr(0, 9) == "autoindex") {
		std::string aindex;

		if (this->checkColon(9, line)) return 1;

		aindex = line.substr(10);
		this->trim(aindex);

		if (aindex != "on" && aindex != "off"){
			std::cerr << "Error: Unexpected word " << aindex << " on line: " << line << std::endl;
			return 1;
		} else if (aindex == "on"){
			location.autoindex = true;
		} else {
			location.autoindex = false;
		}
	} else if (line.substr(0, 15) == "redirect_target"){
		std::string rtarget;

		if (this->checkColon(15, line)) return 1;

		rtarget = line.substr(16);
		this->trim(rtarget);

		location.redirect_target = rtarget;
	} else if (line.substr(0, 5) == "limit"){
		std::vector<std::string> limits;

		if (this->checkColon(5, line)) return 1;

		std::string limit = line.substr(6);
		this->trim(limit);

		std::istringstream stream(limit);
		std::string palabra;
		while (stream >> palabra) {
			limits.push_back(palabra);
		}

		location.limits = limits;
	} else if ( !line.empty() &&line.c_str()[0] != '#') {
		std::cerr << "Unexpected line: " << line << std::endl; 
		return 1;
	}
	return 0;
}

/**
 * @brief auxiliar method for the addServerConf method. It gets the server information from the configuartion file.
 * 
 * @param server A pointer to a Server struct that is going to contain the Server configuration.
 * @param line The line from which it is going to exctract the information.
 * 
 * @return on success 0, on failure 1, and if it reads the Configuration close param it returns 2.
 */
int ConfigParser::extractServerConf(Server& server, std::string line){

	if (!line.empty()) {
		size_t index = line.find("#", 0);
		if (index != std::string::npos) line = line.substr(0, index);

		if (line.substr(0, 11) == "CLOSESERVER") return 2;

		if (line.substr(0, 6) == "listen") {
			std::string portNum;

			if (this->checkColon(6, line)) return 1;
			
			portNum = line.substr(7);
			this->trim(portNum);
			if (!this->checkAllDigit(portNum)){
				std::cerr << "Error: Non-numeric port in 'listen' configuration" << portNum << std::endl;
				return 1;
			}

			server.port = atoi(portNum.c_str());
		} else if (line.substr(0, 4) == "host") {
			std::string host;

			if (this->checkColon(4, line)) return 1;

			host = line.substr(5);
			this->trim(host);

			server.host = host;
		} else if (line.substr(0, 11) == "server_name"){
			std::string snValue;

			if (this->checkColon(11, line)) return 1;

			snValue = line.substr(12);
			this->trim(snValue);

			server.server_name = snValue;
		} else if (line.substr(0, 4) == "root"){
			std::string root;

			if (this->checkColon(4, line)) return 1;

			root = line.substr(5);
			this->trim(root);

			server.root = root;
		} else if (line.substr(0, 3) == "cgi"){
			std::string cgi;

			if (this->checkColon(3, line)) return 1;

			cgi = line .substr(4);
			this->trim(cgi);

			server.cgi = cgi;
		} else if (line.substr(0, 8) == "LOCATION"){
			Location location;
			std::string path;

			if (this->checkColon(8, line)) return 1;

			path = line.substr(9);
			this->trim(path);

			location.path = path;

			while (std::getline(this->file, line)){
				this->trim(line);
				int result = this->extractLocationConf(location, line);
				if (result == 2) break;
				else if (result == 1) return 1;
			}

			server.locations.push_back(location);
		} else if ( !line.empty() &&line.c_str()[0] != '#') {
			std::cerr << "Unexpected line: " << line << std::endl; 
			return 1;
		}

		for (size_t i = 0; i < server.locations.size(); i++){
			if (server.locations[i].root == "")
				server.locations[i].root = server.root;
		}
	}
	return 0;
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
				int result = this->extractServerConf(server, line);
				if (result == 2) break;
				else if (result == 1) return 1;
			}

			this->servers.push_back(server);
			server.locations.clear();
		}
	}
	return 0;
}