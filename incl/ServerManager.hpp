#ifndef SERVERMANAGER_HPP
# define SERVERMANAGER_HPP

# include "webserv.h"
# include "ConfigParser.hpp"

class ConfigParser;

class ServerManager{
	private:
		//Atributes
		std::vector<ConfigParser::Server> server_confs;
		int server_fd;
		pthread_t monitor_thread;

	public:
		//Public Methods
		ServerManager(ConfigParser::Server server_conf, int socket);
		~ServerManager();
		
		void startServer();
		ConfigParser::Server getServersConf();
		void addConf(ConfigParser::Server server_conf);

	private:
		//Private Methods
		static void* monitor_exit_command_static(void* arg);
    	void monitor_exit_command();
		std::string handle_request(std::string const request, ConfigParser::Server server_conf);
		std::string getFile(std::string request_path, std::string server_root, std::string cgi);
		std::string handlePostUpload(std::string request, std::string server_root);
		std::string handlePost(std::string request, std::string request_path, std::string server_root);
		ConfigParser::Server getServerName(std::string request);
		int checkLimits(std::vector<std::string> limits, std::string search) const;
};

#endif