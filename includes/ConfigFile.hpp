#ifndef CONFIGFILE_HPP
# define CONFIGFILE_HPP

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <unistd.h>

#include "Settings.hpp"

struct page {
    std::string index;
    std::vector<std::string> methods;
	std::string rootDir;
    bool listing;
	std::string authBasic;
	std::string authBasicUserFile;
	std::string postRoot;
};

class ConfigFile
{
    private:

		// std::vector<std::string> hostVec;
		std::vector<std::vector<std::string> > _portsVec;

        std::vector<std::map<std::string, std::string> > _configVecOfMap;
        std::vector<std::map<std::string, page> > _htmlPageVecOfMap;
        std::vector<std::map<std::string, std::string> > _errorsVecOfMap;
        std::vector<std::map<std::string, std::string> > _cgiVecOfMap;

        int serverNumber;
		// int maxClientBodySize;

    public:
        ConfigFile();
        ~ConfigFile();

        int loadDataConfigFile(const std::string &filename);
		void setValuesConfigFile(int serverIndex);
        void displayValuesConfigFile(int serverIndex);
		std::string valuesFromMap(std::string index);
		int convertStrToInt(std::string str);
        void splitStrInVector(std::string input, char delimiter, std::vector<std::string> &result);
        int splitStrInMap(std::string input, char delimiter, std::map<std::string, std::string> &result);

        std::string getHost(int serverIndex);
        std::vector<std::string> getPort(int serverIndex);
        int getMaxClientBodySize(int serverIndex);
        std::map<std::string, std::string> getErrorPages(int serverIndex);
        std::map<std::string, std::string> getCgiPages(int serverIndex);
        std::map<std::string, page> getFileRoutes(int serverIndex);
        int getServerNumber();
        std::string getServerName(int serverIndex);
        std::string getRoot(int serverIndex);
        std::string getPostRoot(int serverIndex);



        std::string getFileRoute(std::string fileName, std::string &status, std::string method);
		std::string getErrorPageRoute(std::string errorCode);
};

#endif
