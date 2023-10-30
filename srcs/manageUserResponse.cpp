#include "Server.hpp"
#include "ConfigFile.hpp"
#include <sstream>
#include "Settings.hpp"
#include "utils.hpp"
#include "ClientResponse.hpp"

bool DEBUG_VERBOSE = true;



std::string getUserResponse(UserRequest userRequest, ConfigFile configFile)
{
	if (DEBUG_VERBOSE) std::cout << "---------------------- REQUEST ----------------------" << std::endl;
	if (DEBUG_VERBOSE) std::cout << userRequest.root << std::endl;
	std::string response, fileLocation, contentType, status;

	fileLocation = configFile.getFileRoute(userRequest.root, status, userRequest.method);

	contentType = getContentType(fileLocation);

	std::cout << "file: " << fileLocation << " status: " << status << "content_type: " << contentType << std::endl;

	std::ifstream file(fileLocation.c_str());
	if (!file.is_open())
	{
		fileLocation = configFile.getErrorPageRoute("500");
	}

	std::string htmlContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	ClientResponse clientReponse(status, contentType, htmlContent);
	response = clientReponse.getReponse();

	// if (status == "200" && file.is_open())
	// {
	// 	if (DEBUG_VERBOSE) std::cout << "case 1 file: " << fileLocation << std::endl;
	// 	std::string htmlContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	// 	ClientResponse clientReponse(status, contentType, htmlContent);
	// 	response = clientReponse.getReponse();

	// 	response = "HTTP/1.1 " + status + " OK\r\n";
	// 	response += "Connection: keep-alive\r\n";
	// 	response += "Content-Type: " + contentType + "\r\n";
	// 	response += "Content-Length: " + sizeToString(htmlContent.size()) + "\r\n\r\n";
	// 	response += htmlContent;
	// }
	// else if ((contentType == "text/css" || contentType == "text/javascript") && file.is_open())
	// {
	// 	if (DEBUG_VERBOSE) std::cout << "case 2 file: " << fileLocation << std::endl;
	// 	std::string htmlContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	// 	// ClientResponse clientReponse(status, contentType, htmlContent);
	// 	// response = clientReponse.getReponse();

	// 	response = "HTTP/1.1 " + status + " Not Found\r\n";
	// 	response += "Connection: keep-alive\r\n";
	// 	response += "Content-Type: " + contentType + "\r\n";
	// 	response += "Content-Length: " + sizeToString(htmlContent.size()) + "\r\n\r\n";
	// 	response += htmlContent;
	// }
	// else if (status == "404" && contentType == "text/html" && file.is_open())
	// {
	// 	if (DEBUG_VERBOSE) std::cout << "case 3 file: " << fileLocation << std::endl;
	// 	std::string htmlContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	// 	// ClientResponse clientReponse(status, contentType, htmlContent);
	// 	// response = clientReponse.getReponse();

	// 	response = "HTTP/1.1 " + status + " Not Found\r\n";
	// 	response += "Connection: keep-alive\r\n";
	// 	response += "Content-Type: " + contentType + "\r\n";
	// 	response += "Content-Length: " + sizeToString(htmlContent.size()) + "\r\n\r\n";
	// 	response += htmlContent;
	// }
	// else
	// {
	// 	response = "HTTP/1.1 404 Not Found\r\n\r\n";
	// 	response += "Connection: keep-alive\r\n";
	// 	if (DEBUG_VERBOSE) std::cout << "404 NOT FOUND: " << fileLocation << std::endl;
	// }

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