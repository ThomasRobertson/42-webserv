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
    std::string locationStr;
    std::string locationIndexStr;
    std::string locationMethodStr;

    size_t positionSpace;
    size_t positionSemicolon;
    size_t positionLeftBracket;
    size_t positionTab;
    size_t positionDoubleTab;
    size_t positionLocation;
    page myPage;

    std::ifstream file(filename.c_str());
    if (!file.is_open())
        return 0;
    while (std::getline(file, line))
    {
        positionTab = line.find('	');
        positionDoubleTab = line.find("		");
        positionSpace = line.find(' ');
        positionSemicolon = line.find(';');
        positionLeftBracket = line.find('{');
        positionLocation = line.find("	location");          

        if (positionLocation != std::string::npos)
        {
            locationStr = line.substr(positionSpace + 2, (positionLeftBracket) - (positionSpace + 2));
            
            if (!std::getline(file, line))
                break;
            positionSpace = line.find(' ');
            positionSemicolon = line.find(';');
            locationIndexStr = line.substr(positionSpace + 1, (positionSemicolon) - (positionSpace + 1));
            myPage.index = locationIndexStr;
            if (!std::getline(file, line))
                break;
            positionSpace = line.find(' ');
            positionSemicolon = line.find(';');
            locationMethodStr = line.substr(positionSpace + 1, (positionSemicolon) - (positionSpace + 1));
            myPage.method = locationMethodStr;
            this->htmlPage[locationStr] = myPage;
        }

		else if (positionTab != std::string::npos)
        {
        	leftIndexStr = line.substr(1, positionSpace - 1);
        	rightValueStr = line.substr(positionSpace + 1, (positionSemicolon) - (positionSpace + 1));

        	this->configMap[leftIndexStr] = rightValueStr;
		}
        
    }
    file.close();

    std::map<std::string, page>::iterator it;
    for (it = htmlPage.begin(); it != htmlPage.end(); ++it)
        std::cout << "Key: " << it->first << " Index: " << it->second.index << " Method: " << it->second.method << std::endl;

	setValuesConfigFile();

    return 1;
}

void ConfigFile::setValuesConfigFile()
{
	this->host = this->configMap["host"];
	this->port = this->configMap["port"];
	this->root = this->configMap["root"];
	this->maxClientBodySize = convertStrToInt(this->configMap["maxClientBodySize"]);

    std::map<std::string, page>::iterator it;
    for (it = htmlPage.begin(); it != htmlPage.end(); ++it)
        std::cout << "Key: " << it->first << " Index: " << it->second.index << " Method: " << it->second.method << std::endl;

    std::cout << "host: " << this->host << std::endl;
    std::cout << "port: " << this->port << std::endl;
    std::cout << "root: " << this->root << std::endl;
    std::cout << "maxClientBodySize: " << this->maxClientBodySize << std::endl;
}

int ConfigFile::convertStrToInt(std::string str)
{
	int valueInt = std::atoi(str.c_str());
    if (valueInt != 0 || (valueInt == 0 && str[0] == '0'))
        	return valueInt;
    return -1;
}

// std::string ConfigFile::getHtmlPage(std::string location)
// {
//     if 
// }