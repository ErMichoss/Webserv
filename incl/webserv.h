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

# ifndef BUFFER_SIZE
#  define BUFFER_SIZE 8192
# endif

#endif