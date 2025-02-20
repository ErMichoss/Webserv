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
		std::vector<int> clients;
		std::vector<int> fdcgi_in;
		std::vector<int> fdcgi_out;
		std::map<int, bool> stopped_value;
		std::map<int, int> pipe_client;
		std::map<int, std::size_t> w_size;
		std::map<int, bool> end_write;
		std::map<int, std::string> client_request;
		
	private:
		//Atributes
		std::map<int, std::string> to_write;
		std::vector<ConfigParser::Server> server_confs;
		int active_client;
		

		int server_fd;
		int _location;
		int fd_read[2];
		int fd_write[2];
		int file_to_get;
		int fd_upload;
		std::string response;
		std::map<int, bool> valid_body;

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
		std::vector<int> getFdCgiIn();
		std::vector<int> getFdCgiOut();
		void readCgi(int pipe);
		void writePOST(int fd);
		void handle_request(std::string const request, ConfigParser::Server server_conf);

	private:
		//Private Methods
		void getDir(std::string path, std::string uri);
		bool checkBodySize(std::string content_length);
		std::string findDir(std::string path);
		int	checkLimits(std::vector<std::string> limits, std::string search) const;
		void getFile(std::string request_path, std::string server_root, std::string cgi, std::string request);
		void handlePost(std::string request, std::string request_path, std::string server_root);
		void handlePostUpload(std::string request, std::string server_root);
		void handle_delete(std::string root, std::string request);
		void readErrorPages(std::string header, std::string body, int client);
		bool deleteResource(std::string resource);
		std::string findExtension(const std::string& url);
		std::string getContentType(const std::string& extension);

};

#endif