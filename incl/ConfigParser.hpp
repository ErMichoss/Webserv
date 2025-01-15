#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include <string>
# include <map>
# include <vector>
# include <sstream>
# include <iostream>
# include <fstream>
# include <cstdlib>
# include <cstring>

int onlySpace(std::string str);

class ConfigParser{
	public:
		//Structs
		struct Location {
        	std::string path;
        	std::string root;
        	std::string index;
        	bool autoindex;
        	std::string redirect_target;
			std::vector<std::string> limits;

			Location(const std::string& p = "/", const std::string& r = "", const std::string& i = "index.html",
             bool a = false, const std::string& rt = "")
        	: path(p), root(r), index(i), autoindex(a), redirect_target(rt) {}
    	};

    	struct Server {
        	int port;
    		std::string host;
    		std::string server_name;
    		std::string root;
			std::string cgi;
			std::map<int, std::string> error_pages;
    		std::vector<Location> locations; 

			
			Server(int p = 80, const std::string& h = "127.0.0.1", const std::string& sn = "", const std::string& r = "/var/www/html",
           const std::vector<Location>& locs = std::vector<Location>())
        	: port(p), host(h), server_name(sn), root(r), locations(locs) {
				error_pages[400] = "<h1>400 Bad Request</h1><p>Your request is malformed or cannot be processed.</p>";
				error_pages[401] = "<h1>401 Unauthorized</h1><p>You must authenticate to access this resource.</p>";
				error_pages[402] = "<h1>402 Payment Required</h1><p>Payment is required to proceed.</p>";
				error_pages[403] = "<h1>403 Forbidden</h1><p>You do not have permission to access this resource.</p>";
				error_pages[404] = "<h1>404 Not Found</h1><p>The requested resource could not be found.</p>";
				error_pages[405] = "<h1>405 Method Not Allowed</h1><p>The HTTP method is not allowed for this resource.</p>";
				error_pages[406] = "<h1>406 Not Acceptable</h1><p>The requested resource is not acceptable.</p>";
				error_pages[407] = "<h1>407 Proxy Authentication Required</h1><p>Authentication is required through a proxy.</p>";
				error_pages[408] = "<h1>408 Request Timeout</h1><p>The server timed out waiting for your request.</p>";
				error_pages[409] = "<h1>409 Conflict</h1><p>There was a conflict in your request.</p>";
				error_pages[410] = "<h1>410 Gone</h1><p>The resource is no longer available.</p>";
				error_pages[411] = "<h1>411 Length Required</h1><p>The request must include a Content-Length header.</p>";
				error_pages[412] = "<h1>412 Precondition Failed</h1><p>A precondition in the request failed.</p>";
				error_pages[413] = "<h1>413 Payload Too Large</h1><p>The request payload is too large.</p>";
				error_pages[414] = "<h1>414 URI Too Long</h1><p>The requested URI is too long.</p>";
				error_pages[415] = "<h1>415 Unsupported Media Type</h1><p>The media type is not supported.</p>";
				error_pages[416] = "<h1>416 Range Not Satisfiable</h1><p>The range specified is not satisfiable.</p>";
				error_pages[417] = "<h1>417 Expectation Failed</h1><p>The server cannot meet the expectations specified in the request.</p>";
				error_pages[418] = "<h1>418 I'm a teapot</h1><p>This server is a teapot, not a coffee maker.</p>";
				error_pages[421] = "<h1>421 Misdirected Request</h1><p>The request was misdirected to an inappropriate server.</p>";
				error_pages[422] = "<h1>422 Unprocessable Entity</h1><p>The request was well-formed but could not be processed.</p>";
				error_pages[423] = "<h1>423 Locked</h1><p>The resource is locked.</p>";
				error_pages[424] = "<h1>424 Failed Dependency</h1><p>A dependency for the request failed.</p>";
				error_pages[426] = "<h1>426 Upgrade Required</h1><p>The client should upgrade to a different protocol.</p>";
				error_pages[428] = "<h1>428 Precondition Required</h1><p>The request requires preconditions.</p>";
				error_pages[429] = "<h1>429 Too Many Requests</h1><p>You have sent too many requests in a short period.</p>";
				error_pages[431] = "<h1>431 Request Header Fields Too Large</h1><p>The request header fields are too large.</p>";
				error_pages[451] = "<h1>451 Unavailable For Legal Reasons</h1><p>The resource is unavailable due to legal reasons.</p>";
				error_pages[500] = "<h1>500 Internal Server Error</h1><p>An internal server error occurred.</p>";
				error_pages[501] = "<h1>501 Not Implemented</h1><p>The server does not support the functionality required.</p>";
				error_pages[502] = "<h1>502 Bad Gateway</h1><p>The server received an invalid response from an upstream server.</p>";
				error_pages[503] = "<h1>503 Service Unavailable</h1><p>The server is currently unavailable.</p>";
				error_pages[504] = "<h1>504 Gateway Timeout</h1><p>The server did not receive a timely response from an upstream server.</p>";
				error_pages[505] = "<h1>505 HTTP Version Not Supported</h1><p>The server does not support the HTTP protocol version.</p>";
				error_pages[506] = "<h1>506 Variant Also Negotiates</h1><p>The server has an internal configuration error.</p>";
				error_pages[507] = "<h1>507 Insufficient Storage</h1><p>The server is unable to store the representation needed.</p>";
				error_pages[508] = "<h1>508 Loop Detected</h1><p>The server detected an infinite loop.</p>";
				error_pages[510] = "<h1>510 Not Extended</h1><p>Further extensions are required to complete the request.</p>";
				error_pages[511] = "<h1>511 Network Authentication Required</h1><p>Network authentication is required.</p>";
			};
    	};
	
	private:
		//Atributtes
		std::vector<Server> servers;
		std::ifstream file;
		int i;

	public:
		//public methods
		ConfigParser(char const* fileUrl);
		~ConfigParser(){}

		int addServerConf();

		std::vector<ConfigParser::Server> getServers(){
			return this->servers;
		}

	private:
		//auxiliar methods
		int extractServerConf(Server& server, std::string line);
		int extractLocationConf(Location& location, std::string line);
		std::string exctractString(int index, std::string line);

		/**
		 * A function to get rid of blanc spaces at the start and end of a string.
		 * @param &str a reference to the string that is going to be modified. 
		 */
		void trim(std::string& str) const {
			if (str.length() > 0 && !onlySpace(str)){
        		size_t first = str.find_first_not_of(" \t");
        		size_t last = str.find_last_not_of(" \t");
        		str = str.substr(first, (last - first + 1));
			} else {
				str.clear();
			}
    	}

		/**
		 * A function that checks if there is a colon where it should be one in the conf file
		 * @param pos The position in the line where the colo should be.
		 * @param line The line we are checking.
		 */
		int checkColon(int pos, std::string line){
			int exit = 0;

			if (line.c_str()[pos] != ':'){
				std::cerr << "Error: Unexpected " << line.c_str()[pos] << "in the conf line: " << line << std::endl;
				exit = 1;
			}
			return exit;
		}

		/**
		 * @brief A function that checks if a string only contains digits.
		 * 
		 * @param numStr the string that is going to be checked.
		 * 
		 * @return on success 1, on failure 2.
		 */
		int checkAllDigit(std::string numStr){
			int exit = 1;
			for (size_t i = 0; i < std::strlen(numStr.c_str()); i++){
				if (!std::isdigit(numStr.c_str()[i])){
					exit = 0;
				}
			}
			return exit;
		}
};

#endif