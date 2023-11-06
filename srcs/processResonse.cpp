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
							directoryListing << "<div class=\"listing-buttons-container\"><button class=\"listing-buttons\" onclick='location.href=\"" << entryName.substr(0, entryName.length() - 5) << "\";'>" << entryName.substr(0, entryName.length() - 5) << "</button><br>";
	        	    }
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

std::string getRequestBody(std::string request)
{
	size_t boundaryStartPos, boundaryEndPos, bodyStartPos, bodyEndPos;
	std::string boundary, body;

    boundaryStartPos = request.find("boundary=") + std::strlen("boundary=");
	boundaryEndPos = request.find("\r", boundaryStartPos);
	boundary = "--" + request.substr(boundaryStartPos, boundaryEndPos - boundaryStartPos);

	bodyStartPos = request.find(boundary) + boundary.size();
	bodyStartPos = request.find("\r\n\r\n", bodyStartPos) + std::strlen("\r\n\r\n");
	bodyEndPos = request.find(boundary + "--") - std::strlen("\r\n");

	body = request.substr(bodyStartPos, bodyEndPos - bodyStartPos);

	return body;
}

std::string getFileName(UserRequest request)
{
	size_t nameStartPos = request.body.find("filename=") + std::strlen("filename=\"");
	size_t nameEndPos = request.body.find("\"", nameStartPos);
	
	return request.body.substr(nameStartPos, nameEndPos - nameStartPos);
}

void createFile(UserRequest request) // check if file already exists with same name
{
	std::string fileName = getFileName(request);
	std::string body = getRequestBody(request.body);

	std::ofstream outputFile(("POST/" + fileName).c_str(), std::ios::binary);
    if (outputFile.is_open())
    {
        outputFile.write(body.c_str(), body.size());
        outputFile.close();
        std::cout << "Binary data has been written to " << std::endl;
    }
    else
        std::cerr << "Failed to open the file for writing." << std::endl;
}

void StartServers::processResponse(epoll_event currentEvent)
{
    std::string response;
	Client currentClient = _clientList[currentEvent.data.fd];
    // struct epoll_event event;

    std::cout << "----------------------- NEW REPONSE: " << currentEvent.data.fd << " -----------------------" << std::endl;
    if (currentClient.request.method == "POST")
	{
        // std::cout << currentClient.request.body << std::endl;
		createFile(currentClient.request);
	}
    if (currentClient.request.method == "DELETE")
	{
        // std::cout << currentClient.request.body << std::endl;
		createFile(currentClient.request);
	}
    response = getUserResponse(currentClient);

    write(currentEvent.data.fd, response.c_str(), response.length());
    // std::cout << "response sent: " << response.substr(0, 200) << std::endl;

    // event.data.fd = currentEvent.data.fd;
    // event.events = EPOLLIN;
    // epoll_ctl(_epollFd, EPOLL_CTL_MOD, currentEvent.data.fd, &event);

    _clientList.erase(currentEvent.data.fd);
    epoll_ctl(_epollFd, EPOLL_CTL_DEL, currentEvent.data.fd, NULL);
    close(currentEvent.data.fd);

    std::cout << RED << "[i] Client disconnected: " << currentEvent.data.fd << DEFAULT << std::endl;
}