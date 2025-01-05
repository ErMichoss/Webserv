#include "webserv.h"

int onlySpace(std::string str){
	int exit = 1;
	for (size_t i = 0; i < str.length(); i++){
		if (str.c_str()[i] != ' ' && str.c_str()[i] != '\t')
			exit = 0;
	}
	return exit;
}

void handle_signal(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\nSignal received. Shutting down server...\n";
        running = false;
    }
}

int hostport_match(std::vector<ServerManager>& servers, ConfigParser::Server server_conf){
	int exit = -1;
	if (servers.size() < 1){
		return exit;
	}
	
	for (size_t i = 0; i < servers.size(); i++){
		ConfigParser::Server saved_server_conf = servers[i].getServersConf();
		if (saved_server_conf.host == server_conf.host && saved_server_conf.port == server_conf.port){
			exit = i;
			break;
		}
	}
	return exit;
}
