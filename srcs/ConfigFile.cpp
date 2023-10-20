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

std::string ConfigFile::getFileRoute(std::string location, std::string &status)
{
    std::map<std::string, page>::iterator it = this->htmlPage.find(location);
    status = "200";

    if (it == this->htmlPage.end())
    {
        status = "404";
        return this->htmlPage["/404"].index;
    }

    return this->htmlPage[location].index;
}

std::string ConfigFile::getErrorPages(std::string errorCode)
{
	return this->errorsMap[errorCode];
}

bool hasSingleTabLocation(const std::string &line)
{
	size_t tabPos = line.find("\t");
	size_t locPos = line.find("location");
	
	if (tabPos != std::string::npos && locPos != std::string::npos)
		return tabPos < locPos;
	
	return false;
}

int ConfigFile::loadDataConfigFile(const std::string &filename)
{
	std::string line;
	std::string leftIndexStr;
	std::string rightValueStr;
	std::string locationStr;
	std::string locationIndexStr;
	std::string locationMethodStr;
	std::string errorCodeStr;
	std::string errorPathStr;

	size_t positionSpace;
	size_t positionSemicolon;
	size_t positionLeftBracket;
	size_t positionRightBracket;
	size_t positionTab;
	// size_t positionDoubleTab;
	size_t positionLocation;
	size_t positionIndex;
	size_t positionMethods;
	size_t positionErrors;
	page myPage;

	std::ifstream file(filename.c_str());
	if (!file.is_open())
		return 0;
	

	if (std::getline(file, line) && line != "server {\r")
	{
		file.close();
		return 0;
	}

	while (std::getline(file, line))
	{
		positionTab = line.find('	');
		// positionDoubleTab = line.find("		");
		positionSpace = line.find(' ');
		positionSemicolon = line.find(';');
		positionLeftBracket = line.find('{');
		positionLocation = line.find("	location");
		positionErrors = line.find("	error_page");

		if (positionLocation != std::string::npos && line[1] != '	' && line[9] == ' ' && line[10] != ' ')
		{
			locationStr = line.substr(positionSpace + 1, (positionLeftBracket) - (positionSpace + 2));
			while (std::getline(file, line))
			{
				positionSpace = line.find(' ');
				positionSemicolon = line.find(';');
				positionRightBracket = line.find("	}");
				positionIndex = line.find("		index");
				positionMethods = line.find("		methods");
				if (positionRightBracket != std::string::npos && line[1] != '	')
				{
					break;
				}
				else if (positionIndex != std::string::npos && line[2] != '	')
				{
					if (positionSemicolon == std::string::npos)
						return 0;
					locationIndexStr = line.substr(positionSpace + 1, (positionSemicolon) - (positionSpace + 1));
					myPage.index = locationIndexStr;
				}
				else if (positionMethods != std::string::npos && line[2] != '	')
				{
					if (positionSemicolon == std::string::npos)
						return 0;
					locationMethodStr = line.substr(positionSpace + 1, (positionSemicolon) - (positionSpace + 1));
					std::vector<std::string> methodsVector;
					splitStr(locationMethodStr, ' ', methodsVector);

					myPage.methods = methodsVector;
				}
				else
				{
					std::cout << line << std::endl;
					return 0;
				}
			}
			this->htmlPage[locationStr] = myPage;
		}

		else if (positionTab != std::string::npos)
		{
			if (positionSemicolon == std::string::npos)
			{
				return 0;
			}

			else if (positionErrors != std::string::npos && line[1] != '	')
			{
				if (positionSemicolon == std::string::npos)
					return 0;
				errorCodeStr = line.substr(positionSpace + 1, (positionSpace + 4) - (positionSpace + 1));
				errorPathStr = line.substr(positionSpace + 5, (positionSemicolon) - (positionSpace + 5));
				this->errorsMap[errorCodeStr] = errorPathStr;
			}
			else
			{
				leftIndexStr = line.substr(1, positionSpace - 1);
				rightValueStr = line.substr(positionSpace + 1, (positionSemicolon) - (positionSpace + 1));

				this->configMap[leftIndexStr] = rightValueStr;
			}
		}
		else if (line != "}" && line != "\n" && line != "\r")
		{
			std::cout << line << std::endl;
			return 0;
		}
	}
	file.close();

	if (line != "}")
	{
		return 0;
	}
	setValuesConfigFile();

	return 1;
}

void ConfigFile::setValuesConfigFile()
{
	this->host = this->configMap["host"];
	this->port = this->configMap["port"];
	this->maxClientBodySize = convertStrToInt(this->configMap["maxClientBodySize"]);

	displayValuesConfigFile();
}

void ConfigFile::displayValuesConfigFile()
{
	std::cout << "host: " << this->host << std::endl;
	std::cout << "port: " << this->port << std::endl;
	std::cout << "maxClientBodySize: " << this->maxClientBodySize << std::endl;

	std::map<std::string, page>::iterator it;
	for (it = htmlPage.begin(); it != htmlPage.end(); ++it)
	{
		// std::cout << "Key: " << it->first << " Index: " << it->second.index << " Method 1: " << it->second.methods[0] << " Method 2: " << it->second.methods[1] << std::endl;
		std::cout << "Key: " << it->first << " Index: " << it->second.index << " Method 1: " << it->second.methods[0] << std::endl;
	}
	std::map<std::string, std::string>::iterator it2;
	for (it2 = errorsMap.begin(); it2 != errorsMap.end(); ++it2)
	{
		// std::cout << "Key: " << it->first << " Index: " << it->second.index << " Method 1: " << it->second.methods[0] << " Method 2: " << it->second.methods[1] << std::endl;
		std::cout << "Code: " << it2->first << " Path: " << it2->second << std::endl;
	}

	std::cout << getErrorPages("404") << std::endl;
}

int ConfigFile::convertStrToInt(std::string str)
{
	int valueInt = std::atoi(str.c_str());
	if (valueInt != 0 || (valueInt == 0 && str[0] == '0'))
			return valueInt;
	return -1;
}


void ConfigFile::splitStr(std::string input, char delimiter, std::vector<std::string> &tokens)
{
	std::string token;
	for (size_t i = 0; i < input.length(); ++i)
	{
		if (input[i] != delimiter)
			token += input[i];
		else
		{
			tokens.push_back(token);
			token.clear();
		}
	}
	tokens.push_back(token);
}