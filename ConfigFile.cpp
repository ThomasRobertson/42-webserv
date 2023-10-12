#include "ConfigFile.hpp"

ConfigFile::ConfigFile()
{
    return ;
}

ConfigFile::~ConfigFile()
{
    return ;
}


std::string ConfigFile::getHost()
{
    return this->host;
}

std::string ConfigFile::getPort()
{
    return this->port;
}

int ConfigFile::loadDataConfigFile(const std::string &filename)
{
    std::string line;
    std::string leftIndexStr;
    std::string rightValueStr;
    size_t positionSpace;
    size_t positionSemicolon;
    size_t positionTab;


    std::ifstream file(filename.c_str());
    if (!file.is_open())
        return 0;
    while (std::getline(file, line))
    {
        positionTab = line.find('	');
        positionSpace = line.find(' ');
        positionSemicolon = line.find(';');
        

		if (positionTab != std::string::npos)
        {
        	leftIndexStr = line.substr(1, positionSpace - 1);
        	rightValueStr = line.substr(positionSpace + 1, (positionSemicolon) - (positionSpace + 1));

        	this->configMap[leftIndexStr] = rightValueStr;
		}

    }
    file.close();

	setValuesConfigFile();

    return 1;
}


void ConfigFile::setValuesConfigFile()
{
	this->host = this->configMap["host"];
	this->port = this->configMap["port"];
	this->server_name = this->configMap["server_name"];
	this->root = this->configMap["root"];
	this->maxClientBodySize = convertStrToInt(this->configMap["maxClientBodySize"]);


    std::cout << "host: " << this->host << std::endl;
    std::cout << "port: " << this->port << std::endl;
    std::cout << "server_name: " << this->server_name << std::endl;
    std::cout << "root: " << this->root << std::endl;
    std::cout << "maxClientBodySize: " << this->maxClientBodySize << std::endl;

}

int ConfigFile::convertStrToInt(std::string str)
{
	int valueInt = std::atoi(str.c_str());
    if (valueInt != 0 || (valueInt == 0 && str[0] == '0'))
        	return valueInt;
    else
        return -1;
    return -1;
}