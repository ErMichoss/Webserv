#ifndef SERVERMANAGER_HPP
# define SERVERMANAGER_HPP

# include "webserv.h"
# include "ConfigParser.hpp"

class ConfigParser;

class ServerManager{
	public:
		//Public Atributes
		ConfigParser::Server server_conf;
		std::map<int, std::string> client_response;
		
	private:
		//Atributes
		std::vector<ConfigParser::Server> server_confs;
		std::vector<int> clients;
		int active_client;
		
		std::vector<int> fdcgi;

		int server_fd;
		int fd_read[2];
		int fd_write[2];
		int file_to_get;
		int fd_upload;
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
		void setActiveClient(int fd);

		void handle_request(std::string const request, ConfigParser::Server server_conf);

	private:
		//Private Methods
		int	checkLimits(std::vector<std::string> limits, std::string search) const;
		void getFile(std::string request_path, std::string server_root, std::string cgi, std::string request);
		void handlePost(std::string request, std::string request_path, std::string server_root);
		void handlePostUpload(std::string request, std::string server_root);
		void handle_delete(std::string root, std::string request);
		bool deleteResource(std::string resource);
		std::string findExtension(const std::string& url);
		std::string getContentType(const std::string& extension);

};

#endif