#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include "webserv.h"

class ConfigParser{
	public:
		//Structs
		struct Location {
        	std::string path = "/";
        	std::string root = "";
        	std::string index = "index.html";
        	bool autoindex = false;
        	std::string redirect_target = "";
        	std::map<std::string, std::string> fastcgi_params = {};
    	};

    	struct Server {
        	int port = 80;
    		std::string host = "0.0.0.0";
    		std::string server_name = "";
    		std::string root = "/var/www/html";
    		std::vector<Location> locations = {
        		{"/", "/var/www/html", "index.html", false, "", {}}
    		}; 
    	};
	
	private:
		//Atributtes
		std::vector<Server> servers;
		std::ifstream file;

	public:
		//public methods
		ConfigParser(std::string fileUrl);
		~ConfigParser(){}

		int addServerConf();

		std::vector<ConfigParser::Server> getServers(){
			return this->servers;
		}
	private:
		//auxiliar methods

		/**
		 * A function to get rid of blanc spaces at the start and end of a string.
		 * @param[in] &str a reference to the string that is going to be modified. 
		 */
		void trim(std::string& str) const {
			if (str.length() > 0){
        		size_t first = str.find_first_not_of(" \t");
        		size_t last = str.find_last_not_of(" \t");
        		str = str.substr(first, (last - first + 1));
			}
    	}

		/**
		 * A function that checks if there is a colon where it should be one in the conf file
		 * @param[in] pos The position in the line where the colo should be.
		 * @param[in] line The line we are checking.
		 */
		int checkColon(int pos, std::string line){
			int exit = 0;

			if (line.c_str()[pos] != ':'){
				std::cerr << "Error: Unexpected " << line.c_str()[pos] << "in the conf line: " << line << std::endl;
				exit = 1;
			}
			return exit;
		}
};

#endif