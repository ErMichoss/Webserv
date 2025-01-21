#ifndef SERVERMANAGER_HPP
# define SERVERMANAGER_HPP

# include "webserv.h"
# include "ConfigParser.hpp"

class ConfigParser;

class ServerManager{
	public:
		//Public Atributes
		ConfigParser::Server server_conf;
		
	private:
		//Atributes
		std::vector<ConfigParser::Server> server_confs;
		std::vector<int> clients;
		
		int server_fd;
		std::string response;

	public:
		//Public Methods
		ServerManager(ConfigParser::Server server_conf, int socket);
		~ServerManager();
		
		void startServer();
		ConfigParser::Server getServersConf();
		void addConf(ConfigParser::Server server_conf);
		ConfigParser::Server getServerName(std::string request);
		int getServerFd();
		void addClient(int fd);
		std::vector<int> getClients();
		void removeClient(int fd);

		void handle_request(std::string const request, ConfigParser::Server server_conf);

	private:
		//Private Methods
		int checkLimits(std::vector<std::string> limits, std::string search) const;
		void getFile(std::string request_path, std::string server_root, std::string cgi, std::string request);

};

#endif