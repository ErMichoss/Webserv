#ifndef WEBSERV_H
# define WEBSERV_H

# include <map>
# include <fstream>
# include <arpa/inet.h>
# include <netdb.h>
# include <iostream>
# include <vector>
# include <string>
# include <cstring>
# include <cstdlib>
# include <sstream>
# include <fcntl.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <poll.h>
# include <sys/stat.h>
# include <dirent.h>
# include <csignal>
# include "ServerManager.hpp"
# include "ConfigParser.hpp"

# ifndef BUFFER_SIZE
#  define BUFFER_SIZE 8192
# endif

class ServerManager;

/**
 * Global variable to manage the server status.
 */
static bool running = true;

//*** UTILS ***

/**
 * @brief Checks if a strings has only spaces ot tabs.
 * 
 * @param str The string that is going to get checked by the functions.
 * 
 * @return 0 if the string has something else, 1 if it has only spaces or tabs.
 */
int onlySpace(std::string str);

/**
 * @brief Checks if there is already a socket with that hostport.
 * 
 * @param servers A vector of servers with opened sockets.
 * @param server_conf A server configuraion.
 * 
 * @return On success the index of the server that the hostport matches, on failure -1.
 */
int hostport_match(std::vector<ServerManager>& servers, ConfigParser::Server server_conf);

//*** SIGNALS ***

/**
 * @brief manages the signals ctrl-C while the server is executing allowing a save exit.
 * 
 * @param signal the signal we are going to handle.
 */
void handle_signal(int signal);

#endif