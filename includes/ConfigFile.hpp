#ifndef CONFIGFILE_HPP
# define CONFIGFILE_HPP

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>

struct page {
    std::string index;
    std::vector<std::string> methods;
};

class ConfigFile
{
    private:
        std::map<std::string, std::string> configMap;

		std::string host;
		std::string port;
		std::string server_name;
		std::string root;
        std::map<std::string, page> htmlPage;
        std::map<std::string, std::string> errorsMap;
		int maxClientBodySize;

    public:
        ConfigFile();
        ~ConfigFile();

        int loadDataConfigFile(const std::string &filename);
		void setValuesConfigFile();
        void displayValuesConfigFile();
		std::string valuesFromMap(std::string index);
		int convertStrToInt(std::string str);
        void splitStr(std::string input, char delimiter, std::vector<std::string> &tokens);

        std::string getHost();
        std::string getPort();
        std::string getErrorPages(std::string errorCode);
        std::string getFileRoute(std::string location, std::string &status);

};

#endif
