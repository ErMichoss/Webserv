#include "../incl/webserv.h"

int main(int argc, char *argv[]){
	int exit = 0;

	if (argc == 2){
		std::string file = argv[1];
	} else {
		std::cout << "Debes meter 1 archivo de configuración" << std::endl;
		exit = 1;
	}
	return exit;
}