#ifndef SERVERMANAGER_HPP
# define SERVERMANAGER_HPP

# include "webserv.h"
# include "ConfigParser.hpp"

class ConfigParser;

class ServerManager{

	private:
		//Atributes
		std::vector<ConfigParser::Server> server_confs;
		std::vector<int> clients_fd;
		ConfigParser::Server active_server;
		std::map<int, std::string> errors;
		int server_fd;

	public:
		//Public Methods
		ServerManager(ConfigParser::Server server_conf, int socket);
		~ServerManager();
		
		void startServer();
		ConfigParser::Server getServersConf();
		void addToClients(int fd) { this->clients_fd.push_back(fd); };
		std::vector<int> getClients() { return this->clients_fd; };
		int checkClients(int fd) {
			for (size_t i = 0; i < this->clients_fd.size(); i++) {
				if (this->clients_fd[i] == fd)
					return fd;
			}
			return -1;
		};
		void removeClient(int fd) {
			std::vector<int>::iterator it = std::remove(this->clients_fd.begin(), this->clients_fd.end(), fd);
    		this->clients_fd.erase(it, this->clients_fd.end());
		}
		int getServerfd(){ return this->server_fd; };
		void addConf(ConfigParser::Server server_conf);
		void setSocketLinger(int socket_df);
		std::string handle_request(std::string const request, ConfigParser::Server server_conf);
		ConfigParser::Server getServerName(std::string request);

	private:
		//Private Methods
		static void* monitor_exit_command_static(void* arg);
    	void monitor_exit_command();
		std::string findExtension(const std::string& url);
		std::string getContentType(const std::string& extension);
		std::string getFile(std::string request_path, std::string server_root, std::string cgi, std::string request);
		std::string handlePostUpload(std::string request, std::string server_root);
		std::string handlePost(std::string request, std::string request_path, std::string server_root);
		std::string handle_delete(std::string root, std::string request);
		bool deleteResource(std::string resource);
		int checkLimits(std::vector<std::string> limits, std::string search) const;
};

#endif