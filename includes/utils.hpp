#pragma once

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include "Settings.hpp"
#include <ctime>

#define nullptr NULL
#define DELIMITER "="

time_t getDate();
std::string getContentType(std::string fileName);
std::pair<std::string, std::string> getStatus(std::string statusCode);
std::string sizeToString(size_t value);
std::string generateErrorPage(std::string errorCode = "500");
std::string parseFileName(std::string fileLocation);
std::string parseFileExtension(std::string fileLocation);
