#ifndef CONFIGFILE_HPP
# define CONFIGFILE_HPP

#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>

class ConfigFile
{
    public:
        ConfigFile();
        ~ConfigFile();

        int loadDataConfigFile(const std::string &filename);
		void setValuesConfigFile();
		std::string valuesFromMap(std::string index);
		int convertStrToInt(std::string str);

        std::string getHost();
        std::string getPort();

    private:
        std::map<std::string, std::string> configMap;

		std::string host;
		std::string port;
		std::string server_name;
		std::string root;
        std::map<int, std::string> error_pages;
		int maxClientBodySize;

};

#endif
