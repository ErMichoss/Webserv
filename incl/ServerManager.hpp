#include "webserv.h"
#include "ConfigParser.hpp"

class ServerManager{
	private:
		//Atributes
		ConfigParser::Server server_conf;
		int server_fd;

	public:
		//Public Methods
		ServerManager(ConfigParser::Server server_conf, int socket);
		~ServerManager(){}
		
		void startServer();

	private:
		//Private Methods
		std::string handle_request(std::string const request, ConfigParser::Server server_conf);
		std::string getFile(std::string request_path, std::string server_root);
};