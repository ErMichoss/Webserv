#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include "webserv.h"

class ConfigParser{
	private:
		std::map<std::string, std::string> configData;
		std::string file;

	public:
		ConfigParser(std::string file);
		~ConfigParser();

		int parseFile();

		std::map<std::string, std::string> getFileData() const;
};

#endif