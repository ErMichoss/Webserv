#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include "webserv.h"

class DefaultConfig{
	private:
		std::string id;
		std::string value;
	public:
		DefaultConfig(std::string id, std::string value){
			this->id = id;
			this->value = value;
		}
		~DefaultConfig(){}
		
		std::string getId(){
			return this->id;
		}

		std::string getValue(){
			return this->value;
		}
};

class LocationConfig{
	private:
		std::string id;
		std::list<DefaultConfig> values;
	public:
		LocationConfig(std::string id){
			this->id = id;
		}

		~LocationConfig(){}

		std::string getId(){
			return this->id;
		}

		void addValue(std::string id, std::string value){
			DefaultConfig dc(id, value):

			this->values.pushBack(dc);
		}

		std::list<DefaultConfig> getValues(){
			return this->values;
		}
};


class ConfigParser{
	private:
		std::list<DefaultConfig> defaultConfig;
		std::list<LocationConfig> locationConfig;

	public:
		
};

#endif