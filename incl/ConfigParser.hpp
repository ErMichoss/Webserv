#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include "webserv.h"

class ConfigParser{
	public:
		//Structs
		struct Location {
        	std::string path;
        	std::string root;
        	std::string index;
        	bool autoindex;
        	std::string redirect_target;
        	std::map<std::string, std::string> fastcgi_params;
    	};

    	struct Server {
        	int port;
        	std::string server_name;
        	std::string root;
        	std::vector<Location> locations;
    	};
	
	private:
		//Atributtes
		std::vector<Server> servers;

	public:
		//public methods
		
	private:
		//auxiliar methods
		void trim(std::string& str) const {
        	size_t first = str.find_first_not_of(" \t");
        	size_t last = str.find_last_not_of(" \t");
        	str = str.substr(first, (last - first + 1));
    	}
};

#endif