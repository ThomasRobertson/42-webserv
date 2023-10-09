#include "ConfigFile.hpp"

ConfigFile::ConfigFile()
{
    return ;
}

ConfigFile::ConfigFile(const ConfigFile &src)
{
	*this = src;
    return ;
}

ConfigFile &ConfigFile::operator=(const ConfigFile &src)
{
	this->configMap = src.configMap;
    return *this;
}

ConfigFile::~ConfigFile()
{
    return ;
}



int ConfigFile::loadDataConfigFile(const std::string &filename)
{
    std::string line;
    std::string leftDateStr;
    std::string rightRateStr;
    size_t positionComma;
    size_t positionTab;
    std::string value;


    std::ifstream file(filename.c_str());
    if (!file.is_open())
        return 0;
    while (std::getline(file, line))
    {
        positionTab = line.find('	');
        positionComma = line.find(' ');
        
        if (positionTab != std::string::npos)
        {
			if (positionComma != std::string::npos)
        	{
            	leftDateStr = line.substr(1, positionComma -1);
            	rightRateStr = line.substr(positionComma + 1);

				value = rightRateStr;
            	this->configMap[leftDateStr] = rightRateStr;
                // std::cout << "Index:" << leftDateStr << "\nValue:" << value << std::endl;

			}
        }
    }
    file.close();

	setValuesConfigFile();

    return 1;
}


std::string ConfigFile::valuesFromMap(std::string index)
{
    std::map<std::string, std::string>::const_iterator it;
    it = this->configMap.find(index);

	if (it != this->configMap.end())
    	return it->second;
	else
    	return "Key not found";

    return "Key not found";
}

// template<typename KeyType, typename ValueType>
// ValueType ConfigFile::valuesFromMap(KeyType index)
// {
//     typename std::map<KeyType, ValueType>::const_iterator it;
//     it = this->configMap.lower_bound(index);

//     return it->second;
// }


void ConfigFile::setValuesConfigFile()
{
	this->host = valuesFromMap("host");
	this->port = convertStrToInt(valuesFromMap("port"));
	this->server_name = valuesFromMap("server_name");
	this->root = valuesFromMap("root");
	this->maxClientBodySize = convertStrToInt(valuesFromMap("maxClientBodySize"));


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