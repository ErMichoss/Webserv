#include "../incl/webserv.h"
#include "../incl/ConfigParser.hpp"

int main(int argc, char *argv[]){
	int exit = 0;

	if (argc == 2){
		std::string file = argv[1];
		ConfigParser cp(file);
		cp.addServerConf();
		std::vector<ConfigParser::Server> servers = cp.getServers();
		std::vector<ConfigParser::Server>::iterator it = servers.begin();
		std::cout << servers.at(0).port << std::endl;
	} else {
		std::cout << "Debes meter 1 archivo de configuración" << std::endl;
		exit = 1;
	}
	return exit;
}