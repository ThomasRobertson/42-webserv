#include "Server.hpp"
#include "ConfigFile.hpp"
#include "StartServers.hpp"
#include <sstream>
#include "ClientResponse.hpp"

bool DEBUG_VERBOSE = false;

std::string StartServers::getUserResponse(Client client)
{
	std::string response, fileLocation, contentType, status;

	Server currentServer = this->_serversVec[client.serverIndex];

	if (client.request.method == "POST")
	{
		std::string httpResponse = "HTTP/1.1 200 OK\r\n";
    	httpResponse += "Content-Type: application/json\r\n\r\n";
    	httpResponse += "{\"status\": \"success\", \"message\": \"The POST request was processed successfully.\", \"data\": {\"key1\": \"value10\", \"key2\": \"value2\"}}";
		return httpResponse;
	}

	if (client.request.method == "DELETE")
	{
		std::string httpResponse = "HTTP/1.1 200 OK\r\n";
    	httpResponse += "Content-Type: application/json\r\n\r\n";
    	httpResponse += "{\"status\": \"success\", \"message\": \"The DELETE request was processed successfully.\"}";
		return httpResponse;
	}

	fileLocation = currentServer.getFileRoute(client.request.root, status, client.request.method);

	contentType = getContentType(fileLocation);

	std::ifstream file(fileLocation.c_str());
	if (!file.is_open())
	{
		fileLocation = currentServer.getErrorPageRoute("500");
	}

	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	ClientResponse clientReponse(status, contentType, content);
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

UserRequest StartServers::getUserRequest(std::string requestStr)
{
    UserRequest data;

    size_t spaceSepPos = requestStr.find(' '); // first space char after "GET /scripts/script.js HTTP/1.1"
    data.method = requestStr.substr(0, spaceSepPos);
    requestStr.erase(0, spaceSepPos + 1);
    spaceSepPos = requestStr.find(' '); // first space char after "GET /scripts/script.js HTTP/1.1"
    data.root = requestStr.substr(0, spaceSepPos);
    return data;
}