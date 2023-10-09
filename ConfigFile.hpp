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
        ConfigFile(const ConfigFile &src);
        ConfigFile &operator=(const ConfigFile &src);
        ~ConfigFile();

        int loadDataConfigFile(const std::string &filename);
		void setValuesConfigFile();
		std::string valuesFromMap(std::string index);
		int convertStrToInt(std::string str);

    private:
        std::map<std::string, std::string> configMap;

		std::string host;
		int port;
		std::string server_name;
		std::string root;
        std::map<int, std::string> error_pages;
		int maxClientBodySize;

};

#endif
