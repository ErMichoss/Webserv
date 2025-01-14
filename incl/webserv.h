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
# include <pthread.h>
# include <semaphore.h>
# include <functional>
# include <cstdio>
# include <algorithm>
# include <string>
# include <sys/wait.h>
# include <cstring>
# include "ServerManager.hpp"
# include "ConfigParser.hpp"

# define HTTP400 "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n"
# define HTTP401 "HTTP/1.1 401 Unauthorized\r\nContent-Type: text/html\r\n\e\n"
# define HTTP402 "HTTP/1.1 402 Payment Required\r\nContent-Type: text/html\r\n\r\n"
# define HTTP403 "HTTP/1.1 403 Forbidden\r\nContent-Type: text/html\r\n\r\n"
# define HTTP404 "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n"
# define HTTP405 "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/html\r\n\r\n"
# define HTTP406 "HTTP/1.1 406 Not Acceptable\r\nContent-Type: text/html\r\n\r\n"
# define HTTP407 "HTTP/1.1 407 Proxy Authentication Required\r\nContent-Type: text/html\r\n\r\n"
# define HTTP408 "HTTP/1.1 408 Request Timeout\r\nContent-Type: text/html\r\n\r\n"
# define HTTP409 "HTTP/1.1 409 Conflict\r\nContent-Type: text/html\r\n\r\n"
# define HTTP410 "HTTP/1.1 410 Gone\r\nContent-Type: text/html\r\n\r\n"
# define HTTP411 "HTTP/1.1 411 Length Required\r\nContent-Type: text/html\r\n\r\n"
# define HTTP412 "HTTP/1.1 412 Precondition Failed\r\nContent-Type: text/html\r\n\r\n"
# define HTTP413 "HTTP/1.1 413 Payload Too Large\r\nContent-Type: text/html\r\n\r\n"
# define HTTP414 "HTTP/1.1 414 URI Too Long\r\nContent-Type: text/html\r\n\r\n"
# define HTTP415 "HTTP/1.1 415 Unsupported Media Type\r\nContent-Type: text/html\r\n\r\n"
# define HTTP416 "HTTP/1.1 416 Range Not Satisfiable\r\nContent-Type: text/html\r\n\r\n"
# define HTTP417 "HTTP/1.1 417 Expectation Failed\r\nContent-Type: text/html\r\n\r\n"
# define HTTP418 "HTTP/1.1 418 I'm a teapot\r\nContent-Type: text/html\r\n\r\n"
# define HTTP421 "HTTP/1.1 421 Misdirected Request\r\nContent-Type: text/html\r\n\r\n"
# define HTTP422 "HTTP/1.1 422 Unprocessable Entity\r\nContent-Type: text/html\r\n\r\n"
# define HTTP423 "HTTP/1.1 423 Locked\r\nContent-Type: text/html\r\n\r\n"
# define HTTP424 "HTTP/1.1 424 Failed Dependency\r\nContent-Type: text/html\r\n\r\n"
# define HTTP426 "HTTP/1.1 426 Upgrade Required\r\nContent-Type: text/html\r\n\r\n"
# define HTTP428 "HTTP/1.1 428 Precondition Required\r\nContent-Type: text/html\r\n\r\n
# define HTTP429 "HTTP/1.1 429 Too Many Requests\r\nContent-Type: text/html\r\n\r\n"
# define HTTP431 "HTTP/1.1 431 Request Header Fields Too Large\r\nContent-Type: text/html\r\n\r\n"
# define HTTP451 "HTTP/1.1 451 Unavailable For Legal Reasons\r\nContent-Type: text/html\r\n\r\n"

# define HTTP500 "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\n\r\n"
# define HTTP501 "HTTP/1.1 501 Not Implemented\r\nContent-Type: text/html\r\n\r\n"
# define HTTP502 "HTTP/1.1 502 Bad Gateway\r\nContent-Type: text/html\r\n\r\n"
# define HTTP503 "HTTP/1.1 503 Service Unavailable\r\nContent-Type: text/html\r\n\r\n"
# define HTTP504 "HTTP/1.1 504 Gateway Timeout\r\nContent-Type: text/html\r\n\r\n"
# define HTTP505 "HTTP/1.1 505 HTTP Version Not Supported\r\nContent-Type: text/html\r\n\r\n"
# define HTTP506 "HTTP/1.1 506 Variant Also Negotiates\r\nContent-Type: text/html\r\n\r\n"
# define HTTP507 "HTTP/1.1 507 Insufficient Storage\r\nContent-Type: text/html\r\n\r\n"
# define HTTP508 "HTTP/1.1 508 Loop Detected\r\nContent-Type: text/html\r\n\r\n"
# define HTTP510 "HTTP/1.1 510 Not Extended\r\nContent-Type: text/html\r\n\r\n"
# define HTTP511 "HTTP/1.1 511 Network Authentication Required\r\nContent-Type: text/html\r\n\r\n"

# ifndef BUFFER_SIZE
#  define BUFFER_SIZE 8192
# endif

class ServerManager;

extern char **environ;
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

/**
 * @brief Basic itoa for decimals
 * 
 * @param n de number its going to turn to a std::string
 * 
 * @return The number on a std::string format;
 */
std::string ft_itoa(std::size_t n);

//*** SIGNALS ***

/**
 * @brief manages the signals ctrl-C while the server is executing allowing a save exit.
 * 
 * @param signal the signal we are going to handle.
 */
void handle_signal(int signal);

#endif