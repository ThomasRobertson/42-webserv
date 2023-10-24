#include "Server.hpp"
#include "ConfigFile.hpp"
#include <sstream>

bool DEBUG_VERBOSE = false;

std::string sizeToString(size_t value) {
    std::ostringstream oss;

    oss << value;
    return oss.str();
}

std::string getContentType(std::string fileName)
{
	std::string type;

	size_t dotPos = fileName.find_last_of(".");
    if (dotPos != std::string::npos)
	{
		if (fileName.substr(dotPos + 1) == "html")
			type = "text/html";
		if (fileName.substr(dotPos + 1) == "css")
			type = "text/css";
		if (fileName.substr(dotPos + 1) == "js")
			type = "text/javascript";
		if (fileName.substr(dotPos + 1) == "png")
			type = "image/png";
    }
	else
		type = "text/html";
    return type;
}

std::string getUserResponse(UserRequest userRequest, ConfigFile configFile, int serverIndex)
{
	if (DEBUG_VERBOSE) std::cout << "---------------------- REQUEST ----------------------" << std::endl;
	if (DEBUG_VERBOSE) std::cout << userRequest.root << std::endl;
	std::string response, fileName, contentType, status;

	fileName = userRequest.root;
	contentType = getContentType(fileName);

	if (contentType == "text/html" || contentType == "image/png")
		fileName = configFile.getFileRoute(fileName, status, serverIndex);

	if (status == "404" && contentType == "text/html")
		fileName = configFile.getErrorPages(status, serverIndex);

	fileName = "www" + fileName;

	std::ifstream file(fileName.c_str());

	if (status == "200" && file.is_open())
	{
		if (DEBUG_VERBOSE) std::cout << "case 1 file: " << fileName << std::endl;
		std::string htmlContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		response = "HTTP/1.1 " + status + " OK\r\n";
		response += "Content-Type: " + contentType + "\r\n";
		response += "Content-Length: " + sizeToString(htmlContent.size()) + "\r\n\r\n";
		response += htmlContent;
	}
	else if ((contentType == "text/css" || contentType == "text/javascript") && file.is_open())
	{
		if (DEBUG_VERBOSE) std::cout << "case 2 file: " << fileName << std::endl;
		std::string htmlContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		response = "HTTP/1.1 " + status + " Not Found\r\n";
		response += "Content-Type: " + contentType + "\r\n";
		response += "Content-Length: " + sizeToString(htmlContent.size()) + "\r\n\r\n";
		response += htmlContent;
	}
	else if (status == "404" && contentType == "text/html" && file.is_open())
	{
		if (DEBUG_VERBOSE) std::cout << "case 3 file: " << fileName << std::endl;
		std::string htmlContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		response = "HTTP/1.1 " + status + " Not Found\r\n";
		response += "Content-Type: " + contentType + "\r\n";
		response += "Content-Length: " + sizeToString(htmlContent.size()) + "\r\n\r\n";
		response += htmlContent;
	}
	else
	{
		response = "HTTP/1.1 404 Not Found\r\n\r\n";
		if (DEBUG_VERBOSE) std::cout << "404 NOT FOUND: " << fileName << std::endl;
	}

	return response;
}

UserRequest getUserRequest(std::string requestStr)
{
    UserRequest data;

    size_t spaceSepPos = requestStr.find(' '); // first space char after "GET /scripts/script.js HTTP/1.1"
    data.method = requestStr.substr(0, spaceSepPos);
    requestStr.erase(0, spaceSepPos + 1);
    spaceSepPos = requestStr.find(' '); // first space char after "GET /scripts/script.js HTTP/1.1"
    data.root = requestStr.substr(0, spaceSepPos);
    return data;
}