#include "Server.hpp"
#include "ConfigFile.hpp"
#include "StartServers.hpp"
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

	struct stat path_stat;
	if (stat(fileLocation.c_str(), &path_stat) == 0)
	{
	    if (S_ISDIR(path_stat.st_mode))
		{
			std::cout << client.request.root << std::endl;
			if (currentServer.getListing(client.request.root))
			{
				std::stringstream directoryListing;
				directoryListing << "<html><head><title>Webserv Listing</title><link rel=\"stylesheet\" type=\"text/css\" href=\"styles/styleListing.css\"></head><body><h1 class=\"listing-title\">" << fileLocation << "</h1>";

				DIR *dir = opendir(fileLocation.c_str());
				if (dir != NULL)
				{
					struct dirent *entry;
					while ((entry = readdir(dir)))
					{
						std::string entryName = entry->d_name;
						if (entryName != "." && entryName != ".." && entryName.substr(entryName.find_last_of(".") + 1) == "html")
							directoryListing << "<div class=\"listing-buttons-container\"><button class=\"listing-buttons\" onclick='location.href=\"" << entryName.substr(0, entryName.length() - 5) << "\";'>" << entryName.substr(0, entryName.length() - 5) << "</button><br>";        	    }
					closedir(dir);
				}
		
				directoryListing << "</div></body></html>";

				status = "200 OK";
				contentType = "text/html";
				std::string content = directoryListing.str();
				ClientResponse clientReponse(status, contentType, content);

				response = clientReponse.getReponse();
			}
			else
			{
				fileLocation = currentServer.getErrorPageRoute("404");
				std::ifstream file(fileLocation.c_str());
				if (!file.is_open())
				{
					fileLocation = currentServer.getErrorPageRoute("500");
				}
				std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

				ClientResponse clientReponse(status, contentType, content);

				response = clientReponse.getReponse();
			}

		}
		else
		{
			contentType = getContentType(fileLocation);

			std::ifstream file(fileLocation.c_str());
			if (!file.is_open())
			{
				fileLocation = currentServer.getErrorPageRoute("500");
			}

			std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

			ClientResponse clientReponse(status, contentType, content);

			response = clientReponse.getReponse();
		}
	}

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
    UserRequest request;

    size_t spaceSepPos = requestStr.find(' '); // first space char after "GET /scripts/script.js HTTP/1.1"
    request.method = requestStr.substr(0, spaceSepPos);
    requestStr.erase(0, spaceSepPos + 1);
    spaceSepPos = requestStr.find(' '); // first space char after "GET /scripts/script.js HTTP/1.1"
    request.root = requestStr.substr(0, spaceSepPos);
    return request;
}