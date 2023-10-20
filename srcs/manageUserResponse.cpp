#include "Server.hpp"
#include "ConfigFile.hpp"

std::string manageUserResponse(UserRequest userRequest, ConfigFile configFile)
{
	(void)configFile;
	std::string fileName = userRequest.root;
	// if (fileName == "/")
	// std::cout << configFile.getHtmlPage(fileName) << std::endl;
	// std::cout << "file to get: " << fileName << std::endl;

	(void)userRequest;
	std::ifstream htmlFile("www/srcs/index.html"); 
	if (!htmlFile.is_open())
	{
		std::cerr << "Failed to open HTML file." << std::endl;
		return "";
	}

	std::string htmlContent((std::istreambuf_iterator<char>(htmlFile)), std::istreambuf_iterator<char>());

	std::string response = "HTTP/1.1 200 OK\r\n";
	response += "Content-Type: text/html\r\n";
	response += "Content-Length: " + htmlContent + "\r\n";
	response += "\r\n";
	response += htmlContent;

	return response;
}

UserRequest getUserRequest(std::string requestStr)
{
    UserRequest data;

	std::cout << requestStr << std::endl;
    size_t spaceSepPos = requestStr.find(' '); // first space char after "GET /scripts/script.js HTTP/1.1"
    data.method = requestStr.substr(0, spaceSepPos);
    requestStr.erase(0, spaceSepPos + 1);
    spaceSepPos = requestStr.find(' '); // first space char after "GET /scripts/script.js HTTP/1.1"
    data.root = requestStr.substr(0, spaceSepPos);
    return data;
}