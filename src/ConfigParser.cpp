#include "../incl/ConfigParser.hpp"

ConfigParser::ConfigParser(std::string file){
	this->file = file;
}

ConfigParser::~ConfigParser(){}

std::map<std::string, std::string> ConfigParser::getFileData() const{
	return this->configData;
}

int ConfigParser::parseFile(){
	std::ifstream inputFile(this->file);
	if (!inputFile.is_open()){
		std::cerr << "Error: Could not open the configuration file" << std::endl;
		std::cerr << "File Url: " << this->file << std::endl;
		return 1;
	}

	inputFile.close();
	return 0;
}