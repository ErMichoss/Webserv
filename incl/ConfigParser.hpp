#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include <string>
# include <map>
# include <vector>
# include <sstream>
# include <iostream>
# include <fstream>
# include <cstdlib>
# include <cstring>

int onlySpace(std::string str);

class ConfigParser{
	public:
		//Structs
		struct Location {
        	std::string path;
        	std::string root;
        	std::string index;
        	bool autoindex;
        	std::string redirect_target;
			std::vector<std::string> limits;

			Location(const std::string& p = "/", const std::string& r = "", const std::string& i = "index.html",
             bool a = false, const std::string& rt = "")
        	: path(p), root(r), index(i), autoindex(a), redirect_target(rt) {}
    	};

    	struct Server {
        	int port;
    		std::string host;
    		std::string server_name;
    		std::string root;
			std::string cgi;
    		std::vector<Location> locations; 

			Server(int p = 80, const std::string& h = "127.0.0.1", const std::string& sn = "", const std::string& r = "/var/www/html",
           const std::vector<Location>& locs = std::vector<Location>())
        	: port(p), host(h), server_name(sn), root(r), locations(locs) {}
    	};
	
	private:
		//Atributtes
		std::vector<Server> servers;
		std::ifstream file;

	public:
		//public methods
		ConfigParser(char const* fileUrl);
		~ConfigParser(){}

		int addServerConf();

		std::vector<ConfigParser::Server> getServers(){
			return this->servers;
		}

	private:
		//auxiliar methods
		int extractServerConf(Server& server, std::string line);
		int extractLocationConf(Location& location, std::string line);
		std::string exctractString(int index, std::string line);

		/**
		 * A function to get rid of blanc spaces at the start and end of a string.
		 * @param &str a reference to the string that is going to be modified. 
		 */
		void trim(std::string& str) const {
			if (str.length() > 0 && !onlySpace(str)){
        		size_t first = str.find_first_not_of(" \t");
        		size_t last = str.find_last_not_of(" \t");
        		str = str.substr(first, (last - first + 1));
			} else {
				str.clear();
			}
    	}

		/**
		 * A function that checks if there is a colon where it should be one in the conf file
		 * @param pos The position in the line where the colo should be.
		 * @param line The line we are checking.
		 */
		int checkColon(int pos, std::string line){
			int exit = 0;

			if (line.c_str()[pos] != ':'){
				std::cerr << "Error: Unexpected " << line.c_str()[pos] << "in the conf line: " << line << std::endl;
				exit = 1;
			}
			return exit;
		}

		/**
		 * @brief A function that checks if a string only contains digits.
		 * 
		 * @param numStr the string that is going to be checked.
		 * 
		 * @return on success 1, on failure 2.
		 */
		int checkAllDigit(std::string numStr){
			int exit = 1;
			for (size_t i = 0; i < std::strlen(numStr.c_str()); i++){
				if (!std::isdigit(numStr.c_str()[i])){
					exit = 0;
				}
			}
			return exit;
		}
};

#endif