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
        std::map<std::string, std::string> cgiMap;
		int maxClientBodySize;

    public:
        ConfigFile();
        ~ConfigFile();

        int loadDataConfigFile(const std::string &filename);
		void setValuesConfigFile();
        void displayValuesConfigFile();
		std::string valuesFromMap(std::string index);
		int convertStrToInt(std::string str);
        void splitStrInVector(std::string input, char delimiter, std::vector<std::string> &result);
        int splitStrInMap(std::string input, char delimiter, std::map<std::string, std::string> &result);


        std::string getHost() const {return host;}
        std::string getPort() const {return port;}
		std::string getServerName() const {return server_name;}
		std::string getRoot() const {return root;}
        std::string getErrorPages(std::string errorCode) const {return errorsMap.at(errorCode);}
        std::string getCgiPages(std::string cgiName) const {return cgiMap.at(cgiName);}

        std::string getFileRoute(std::string location, std::string &status);

};

#endif
