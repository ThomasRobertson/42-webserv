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

		// std::vector<std::string> hostVec;
		std::vector<std::vector<std::string> > portsVec;

        std::vector<std::map<std::string, std::string> > configMap;
        std::vector<std::map<std::string, page> > htmlPage;
        std::vector<std::map<std::string, std::string> > errorsMap;
        std::vector<std::map<std::string, std::string> > cgiMap;

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
        std::string getPort(int serverIndex);
        int getMaxClientBodySize(int serverIndex);
        std::string getErrorPages(std::string errorCode, int serverIndex);
        std::string getCgiPages(std::string cgiName, int serverIndex);
        std::string getFileRoute(std::string location, std::string &status, int serverIndex);

};

#endif
