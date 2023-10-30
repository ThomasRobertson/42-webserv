#include "ConfigFile.hpp"

ConfigFile::ConfigFile()
{
	return ;
}

ConfigFile::~ConfigFile()
{
	return ;
}

std::string ConfigFile::getErrorPageRoute(std::string errorCode)
{
	std::string fileLocation;

	if (errorsMap.find(errorCode) == errorsMap.end())
	{
		std::cerr << "NOT IMPLEMENTED : No error file " << errorCode << "(" << fileLocation << ")" << std::endl;
		throw std::runtime_error("No valid page found, cannot continue execution.");
	}
	fileLocation = HTML_DIR + errorsMap[errorCode];
	if (access(fileLocation.c_str(), R_OK) != 0)
	{
		std::cerr << "NOT IMPLEMENTED : Cannot access error file " << errorCode << "(" << fileLocation << ")" <<  std::endl;
		throw std::runtime_error("No valid error page found, cannot continue execution.");
	}

	return fileLocation;
}

std::string ConfigFile::getFileRoute(std::string fileName, std::string &status, std::string method)
{
	std::string fileLocation;

	if (htmlPage.find(fileName) != htmlPage.end()) // fileLocation is not a file but a parent directory
		fileName = htmlPage[fileName].index;
	fileLocation = HTML_DIR;
	fileLocation += root; //! must by without trailing '/'
	fileLocation += fileName;
	// std::cout << "fileadress: " <<fileAddress << std::endl;
	int accessMode;
	if (method == "GET")
		accessMode = R_OK;
	else if (method == "POST" || method == "DELETE")
		accessMode = W_OK;
	else
	{
		accessMode = F_OK;
		#ifdef DEBUG
		throw std::runtime_error("Unknow method !");
		#endif // DEBUG
	}
	if (access(fileLocation.c_str(), accessMode) == 0)
		status = "200";
	else
	{
		status = "404";
		fileLocation = getErrorPageRoute(status);
	}
	return fileLocation;
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
	std::string cgiNameStr;
	std::string cgiPathStr;

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
	size_t positionCgi;
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
		positionCgi = line.find("	cgi");

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
					splitStrInVector(locationMethodStr, ' ', methodsVector);

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
			else if (positionCgi != std::string::npos && line[1] != '	')
			{
				if (positionSemicolon == std::string::npos)
					return 0;
				cgiNameStr = line.substr(positionSpace + 1, (positionSemicolon) - (positionSpace + 1));
				if (!splitStrInMap(cgiNameStr, ' ', this->cgiMap))
					return 0;
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
		std::cout << "Code: " << it2->first << " Path: " << it2->second << std::endl;

	std::map<std::string, std::string>::iterator it3;
	for (it3 = cgiMap.begin(); it3 != cgiMap.end(); ++it3)
		std::cout << "Name: " << it3->first << " Path: " << it3->second << std::endl;

	std::cout << getErrorPages("404") << std::endl;
	std::cout << getCgiPages(".php") << std::endl;
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
		result[result.begin()->first] = token;
	if (tokenCount != 1)
		return 0;

	return 1;
}
