#include "ConfigFile.hpp"

ConfigFile::ConfigFile()
{
	return ;
}

ConfigFile::~ConfigFile()
{
	return ;
}

std::string ConfigFile::getHost(int serverIndex)
{
	return this->_configVecOfMap[serverIndex]["host"];
}

std::vector<std::string> ConfigFile::getPort(int serverIndex)
{
	return this->_portsVec[serverIndex];
}

int ConfigFile::getMaxClientBodySize(int serverIndex)
{
	return convertStrToInt(this->_configVecOfMap[serverIndex]["maxClientBodySize"]);
}

std::map<std::string, page> ConfigFile::getFileRoutes(int serverIndex)
{
    return this->_htmlPageVecOfMap[serverIndex];
}

std::map<std::string, std::string> ConfigFile::getErrorPages(int serverIndex)
{
	return this->_errorsVecOfMap[serverIndex];
}

std::map<std::string, std::string> ConfigFile::getCgiPages(int serverIndex)
{
	return this->_cgiVecOfMap[serverIndex];
}

int ConfigFile::getServerNumber()
{
	return serverNumber;
}

std::string ConfigFile::getServerName(int serverIndex)
{
	return this->_configVecOfMap[serverIndex]["server_name"];
}

std::string ConfigFile::getRoot(int serverIndex)
{
	return this->_configVecOfMap[serverIndex]["root"];
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

	int serverIndex = 0;

	std::map<std::string, std::string> newErrorMap;
	std::map<std::string, page> newHtmlPageMap;
	std::map<std::string, std::string> newConfigMap;
	std::map<std::string, std::string> newCgiMap;


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
		size_t positionTab = line.find('\t');
		size_t positionSpace = line.find(' ');
		size_t positionSemicolon = line.find(';');
		size_t positionLeftBracket = line.find('{');
		size_t positionLocation = line.find("\tlocation");
		size_t positionErrors = line.find("\terror_page");
		size_t positionCgi = line.find("\tcgi");
		size_t positionPorts = line.find("\tlisten");

		if (line == "server {\r")
		{
			serverIndex++;
			continue;
		}
		if (positionLocation != std::string::npos && line[1] != '\t' && line[9] == ' ' && line[10] != ' ')
		{
			page newPage;
			newPage.listing = false;

			std::string locationStr = line.substr(positionSpace + 1, (positionLeftBracket) - (positionSpace + 2));
			while (std::getline(file, line))
			{
				positionSpace = line.find(' ');
				positionSemicolon = line.find(';');

				size_t positionRightBracket = line.find("\t}");
				size_t positionIndex = line.find("\t\tindex");
				size_t positionMethods = line.find("\t\tmethods");
				size_t positionListing = line.find("\t\tautoindex");
				size_t positionRoot = line.find("\t\troot");

				if (positionRightBracket != std::string::npos && line[1] != '\t')
				{
					break;
				}
				else if (positionIndex != std::string::npos && line[2] != '\t')
				{
					if (positionSemicolon == std::string::npos)
						return 0;
					std::string locationIndexStr = line.substr(positionSpace + 1, (positionSemicolon) - (positionSpace + 1));
					newPage.index = locationIndexStr;
				}
				else if (positionMethods != std::string::npos && line[2] != '\t')
				{
					if (positionSemicolon == std::string::npos)
						return 0;
					std::string locationMethodStr = line.substr(positionSpace + 1, (positionSemicolon) - (positionSpace + 1));
					std::vector<std::string> methodsVector;
					splitStrInVector(locationMethodStr, ' ', methodsVector);

					newPage.methods = methodsVector;
				}
				else if (positionListing != std::string::npos && line[2] != '\t')
				{
					bool listingBool = false;
					if (positionSemicolon == std::string::npos)
						return 0;
					std::string locationListingStr = line.substr(positionSpace + 1, (positionSemicolon) - (positionSpace + 1));
					if (locationListingStr == "on")
						listingBool = true;
					else if (locationListingStr == "off")
						listingBool = false;
					else
						return 0;
					newPage.listing = listingBool;
				}
				else if (positionRoot != std::string::npos && line[2] != '\t')
				{
					if (positionSemicolon == std::string::npos)
						return 0;
					std::string locationRootStr = line.substr(positionSpace + 1, (positionSemicolon) - (positionSpace + 1));
					if (*locationRootStr.rbegin() == '/')
						return 0;
					newPage.rootDir = locationRootStr;
				}
				else
				{
					// std::cout << line << std::endl;
					return 0;
				}
			}
			newHtmlPageMap[locationStr] = newPage;
		}

		else if (positionTab != std::string::npos)
		{
			if (positionSemicolon == std::string::npos)
				return 0;
			else if (positionPorts != std::string::npos && line[1] != '\t')
			{
				if (positionSemicolon == std::string::npos)
					return 0;
				std::string portsStr = line.substr(positionSpace + 1, (positionSemicolon) - (positionSpace + 1));
				std::vector<std::string> portsVector;
				splitStrInVector(portsStr, ' ', portsVector);
				this->_portsVec.push_back(portsVector);
				portsVector.clear();
			}
			else if (positionErrors != std::string::npos && line[1] != '\t')
			{
				if (positionSemicolon == std::string::npos)
					return 0;
				std::string errorCodeStr = line.substr(positionSpace + 1, (positionSpace + 4) - (positionSpace + 1));
				std::string errorPathStr = line.substr(positionSpace + 5, (positionSemicolon) - (positionSpace + 5));
				newErrorMap[errorCodeStr] = errorPathStr;
			}
			else if (positionCgi != std::string::npos && line[1] != '\t')
			{
				if (positionSemicolon == std::string::npos)
					return 0;
				std::string cgiNameStr = line.substr(positionSpace + 1, (positionSemicolon) - (positionSpace + 1));
				if (!splitStrInMap(cgiNameStr, ' ', newCgiMap))
					return 0;
			}
			else
			{
				std::string leftIndexStr = line.substr(1, positionSpace - 1);
				std::string rightValueStr = line.substr(positionSpace + 1, (positionSemicolon) - (positionSpace + 1));
				newConfigMap[leftIndexStr] = rightValueStr;
			}
		}
		else if (line != "}" && line != "\n" && line != "\r" && line != "}\n" && line != "}\r")
		{
			// std::cout << line << std::endl;
			return 0;
		}
		else if (line == "}" || line == "}\n" || line == "}\r")
		{
			this->_configVecOfMap.push_back(newConfigMap);
			this->_errorsVecOfMap.push_back(newErrorMap);
			this->_htmlPageVecOfMap.push_back(newHtmlPageMap);
			this->_cgiVecOfMap.push_back(newCgiMap);

			setValuesConfigFile(serverIndex);
		}
	}
	file.close();

	if (line != "}")
	{
		return 0;
	}

	return 1;
}

void ConfigFile::setValuesConfigFile(int serverIndex)
{
	this->serverNumber = serverIndex;
	// displayValuesConfigFile(serverIndex);
}

void ConfigFile::displayValuesConfigFile(int serverIndex)
{
	std::cout << "Server number " << serverIndex + 1 << std::endl;
	std::cout << "host: " << this->_configVecOfMap[serverIndex]["host"] << std::endl;
	std::cout << "port: " << this->_portsVec[serverIndex][0] << std::endl;
	std::cout << "maxClientBodySize: " << convertStrToInt(this->_configVecOfMap[serverIndex]["maxClientBodySize"]) << std::endl;

	std::map<std::string, page>::iterator it;
	for (it = _htmlPageVecOfMap[serverIndex].begin(); it != _htmlPageVecOfMap[serverIndex].end(); it++)
	{
		// std::cout << "Key: " << it->first << " Index: " << it->second.index << " Method 1: " << it->second.methods[0] << " Method 2: " << it->second.methods[1] << std::endl;
		std::cout << "Key: " << it->first << " Index: " << it->second.index << " Method 1: " << it->second.methods[0] << std::endl;
	}
	std::map<std::string, std::string>::iterator it2;
	for (it2 = _errorsVecOfMap[serverIndex].begin(); it2 != _errorsVecOfMap[serverIndex].end(); it2++)
		std::cout << "Code: " << it2->first << " Path: " << it2->second << std::endl;

	std::map<std::string, std::string>::iterator it3;
	for (it3 = _cgiVecOfMap[serverIndex].begin(); it3 != _cgiVecOfMap[serverIndex].end(); it3++)
		std::cout << "Name: " << it3->first << " Path: " << it3->second <<  std::endl;
}

int ConfigFile::convertStrToInt(std::string str)
{
	int valueInt = std::atoi(str.c_str());
	if (valueInt != 0 || (valueInt == 0 && str[0] == '0'))
			return valueInt;
	return -1;
}


void ConfigFile::splitStrInVector(std::string input, char delimiter, std::vector<std::string> &result)
{
	std::string token;
	for (size_t i = 0; i < input.length(); ++i)
	{
		if (input[i] != delimiter)
			token += input[i];
		else
		{
			result.push_back(token);
			token.clear();
		}
	}
	result.push_back(token);
}

int ConfigFile::splitStrInMap(std::string input, char delimiter, std::map<std::string, std::string> &result)
{
	std::string token;
	int tokenCount = 0;

	for (size_t i = 0; i < input.length(); ++i)
	{
		if (input[i] != delimiter)
			token += input[i];
		else
		{
			if (tokenCount == 0)
				result[token] = ""; 
			else
				return 0;

			token.clear();
			tokenCount++;
		}
	}

	if (tokenCount == 1)
	{
    	result[result.begin()->first] = token;
	}
    if (tokenCount != 1)
		return 0;

	return 1;
}
