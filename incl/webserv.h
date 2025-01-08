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

# define HTTP400 "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<h1>400 Bad Request</h1>"
# define HTTP401 "HTTP/1.1 401 Unauthorized\r\nContent-Type: text/html\r\n\e\n<h1>401 Unauthorized</h1>"
# define HTTP402 "HTTP/1.1 402 Payment Required\r\nContent-Type: text/html\r\n\r\n<h1>402 Payment Required</h1>"
# define HTTP403 "HTTP/1.1 403 Forbidden\r\nContent-Type: text/html\r\n\r\n<h1>403 Forbidden</h1>"
# define HTTP404 "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1>404 Not Found</h1>"
# define HTTP405 "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/html\r\n\r\n<h1>405 Method Not Allowed</h1>"
# define HTTP406 "HTTP/1.1 406 Not Acceptable\r\nContent-Type: text/html\r\n\r\n<h1>406 Not Acceptable</h1>"
# define HTTP407 "HTTP/1.1 407 Proxy Authentication Required\r\nContent-Type: text/html\r\n\r\n<h1>407 Proxy Authentication Required</h1>"
# define HTTP408 "HTTP/1.1 408 Request Timeout\r\nContent-Type: text/html\r\n\r\n<h1>408 Request Timeout</h1>"
# define HTTP409 "HTTP/1.1 409 Conflict\r\nContent-Type: text/html\r\n\r\n<h1>409 Conflict</h1>"
# define HTTP410 "HTTP/1.1 410 Gone\r\nContent-Type: text/html\r\n\r\n<h1>410 Gone</h1>"
# define HTTP411 "HTTP/1.1 411 Length Required\r\nContent-Type: text/html\r\n\r\n<h1>411 Length Required</h1>"
# define HTTP412 "HTTP/1.1 412 Precondition Failed\r\nContent-Type: text/html\r\n\r\n<h1>412 Precondition Failed</h1>"
# define HTTP413 "HTTP/1.1 413 Payload Too Large\r\nContent-Type: text/html\r\n\r\n<h1>413 Payload Too Large</h1>"
# define HTTP414 "HTTP/1.1 414 URI Too Long\r\nContent-Type: text/html\r\n\r\n<h1>414 URI Too Long</h1>"
# define HTTP415 "HTTP/1.1 415 Unsupported Media Type\r\nContent-Type: text/html\r\n\r\n<h1>415 Unsupported Media Type</h1>"
# define HTTP416 "HTTP/1.1 416 Range Not Satisfiable\r\nContent-Type: text/html\r\n\r\n<h1>416 Range Not Satisfiable</h1>"
# define HTTP417 "HTTP/1.1 417 Expectation Failed\r\nContent-Type: text/html\r\n\r\n<h1>417 Expectation Failed</h1>"
# define HTTP418 "HTTP/1.1 418 I'm a teapot\r\nContent-Type: text/html\r\n\r\n<h1>418 I'm a teapot</h1>"
# define HTTP421 "HTTP/1.1 421 Misdirected Request\r\nContent-Type: text/html\r\n\r\n<h1>421 Misdirected Request</h1>"
# define HTTP422 "HTTP/1.1 422 Unprocessable Entity\r\nContent-Type: text/html\r\n\r\n<h1>422 Unprocessable Entity</h1>"
# define HTTP423 "HTTP/1.1 423 Locked\r\nContent-Type: text/html\r\n\r\n<h1>423 Locked</h1>"
# define HTTP424 "HTTP/1.1 424 Failed Dependency\r\nContent-Type: text/html\r\n\r\n<h1>424 Failed Dependency</h1>"
# define HTTP426 "HTTP/1.1 426 Upgrade Required\r\nContent-Type: text/html\r\n\r\n<h1>426 Upgrade Required</h1>"
# define HTTP428 "HTTP/1.1 428 Precondition Required\r\nContent-Type: text/html\r\n\r\n<h1>428 Precondition Required</h1>"
# define HTTP429 "HTTP/1.1 429 Too Many Requests\r\nContent-Type: text/html\r\n\r\n<h1>429 Too Many Requests</h1>"
# define HTTP431 "HTTP/1.1 431 Request Header Fields Too Large\r\nContent-Type: text/html\r\n\r\n<h1>431 Request Header Fields Too Large</h1>"
# define HTTP451 "HTTP/1.1 451 Unavailable For Legal Reasons\r\nContent-Type: text/html\r\n\r\n<h1>451 Unavailable For Legal Reasons</h1>"

# define HTTP500 "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\n\r\n<h1>500 Internal Server Error</h1>"
# define HTTP501 "HTTP/1.1 501 Not Implemented\r\nContent-Type: text/html\r\n\r\n<h1>501 Not Implemented</h1>"
# define HTTP502 "HTTP/1.1 502 Bad Gateway\r\nContent-Type: text/html\r\n\r\n<h1>502 Bad Gateway</h1>"
# define HTTP503 "HTTP/1.1 503 Service Unavailable\r\nContent-Type: text/html\r\n\r\n<h1>503 Service Unavailable</h1>"
# define HTTP504 "HTTP/1.1 504 Gateway Timeout\r\nContent-Type: text/html\r\n\r\n<h1>504 Gateway Timeout</h1>"
# define HTTP505 "HTTP/1.1 505 HTTP Version Not Supported\r\nContent-Type: text/html\r\n\r\n<h1>505 HTTP Version Not Supported</h1>"
# define HTTP506 "HTTP/1.1 506 Variant Also Negotiates\r\nContent-Type: text/html\r\n\r\n<h1>506 Variant Also Negotiates</h1>"
# define HTTP507 "HTTP/1.1 507 Insufficient Storage\r\nContent-Type: text/html\r\n\r\n<h1>507 Insufficient Storage</h1>"
# define HTTP508 "HTTP/1.1 508 Loop Detected\r\nContent-Type: text/html\r\n\r\n<h1>508 Loop Detected</h1>"
# define HTTP510 "HTTP/1.1 510 Not Extended\r\nContent-Type: text/html\r\n\r\n<h1>510 Not Extended</h1>"
# define HTTP511 "HTTP/1.1 511 Network Authentication Required\r\nContent-Type: text/html\r\n\r\n<h1>511 Network Authentication Required</h1>"

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