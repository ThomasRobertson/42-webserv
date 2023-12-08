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

struct Location {
    std::string index;
    std::vector<std::string> methods;
	std::string rootDir;
    bool listing;
	std::string authBasic;
	std::string authBasicUserFile;
	std::string postRoot;
	std::string redirect;
};

class ConfigFile
{
    private:

		// std::vector<std::string> hostVec;
		std::vector<std::vector<std::string> > _portsVec;

        std::vector<std::map<std::string, std::string> > _configVecOfMap;
        std::vector<std::map<std::string, Location> > _htmlLocationVecOfMap;
        std::vector<std::map<std::string, std::string> > _errorsVecOfMap;
        std::vector<std::map<std::string, std::string> > _cgiVecOfMap;

        int serverNumber;

    public:
        ConfigFile();
        ~ConfigFile();

        int loadDataConfigFile(const std::string &filename);
		void setValuesConfigFile(int serverIndex);
        void displayValuesConfigFile(int serverIndex);
		int convertStrToInt(std::string str);
        void splitStrInVector(std::string input, char delimiter, std::vector<std::string> &result);
        int splitStrInMap(std::string input, char delimiter, std::map<std::string, std::string> &result);

        std::string getHost(int serverIndex);
        std::vector<std::string> getPort(int serverIndex);
        int getMaxClientBodySize(int serverIndex);
        std::map<std::string, std::string> getErrorPages(int serverIndex);
        std::map<std::string, std::string> getCgiPages(int serverIndex);
        std::map<std::string, Location> getFileRoutes(int serverIndex);
        int getServerNumber();
        std::string getServerName(int serverIndex);
        std::string getRoot(int serverIndex);
        std::string getPostRoot(int serverIndex);

        bool hasSingleTabLocation(const std::string &line);
        int setLocationIndex(Location &newPage, std::string line, size_t positionSemicolon, size_t positionSpace);
        int setLocationMethods(Location &newPage, std::string line, size_t positionSemicolon, size_t positionSpace);
        int setLocationListing(Location &newPage, std::string line, size_t positionSemicolon, size_t positionSpace);
        int setLocationRoot(Location &newPage, std::string line, size_t positionSemicolon, size_t positionSpace);
        int setLocationAuthBasic(Location &newPage, std::string line, size_t positionSemicolon, size_t positionSpace);
        int setLocationAuthBasicUserFile(Location &newPage, std::string line, size_t positionSemicolon, size_t positionSpace);
        int setLocationPostRoot(Location &newPage, std::string line, size_t positionSemicolon, size_t positionSpace);
        int setLocationRedirect(Location &newPage, std::string line, size_t positionSemicolon, size_t positionSpace);

        int checkFile(std::ifstream &file, std::string line);

        int setPorts(std::string line, size_t positionSemicolon, size_t positionSpace);
        int setErrors(std::string line, size_t positionSemicolon, size_t positionSpace, std::map<std::string, std::string> &newErrorMap);
        int setCgi(std::string line, size_t positionSemicolon, size_t positionSpace, std::map<std::string, std::string> &newCgiMap);


};

#endif
