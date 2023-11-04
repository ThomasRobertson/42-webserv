#include "Server.hpp"
#include "ConfigFile.hpp"
#include "StartServers.hpp"
#include "ClientResponse.hpp"

std::string StartServers::listingDirectory(Client client)
{
	std::string response, fileLocation, fileName, contentType, status;
	Server currentServer = this->_serversVec[client.serverIndex];
	
	fileLocation = currentServer.getFileRoute(client.request.root, status, client.request.method);
	fileName = currentServer.getFileName(client.request.root);
	// std::cout << "fileName: " << fileName << std::endl;

	struct stat path_stat;
	if (stat(fileLocation.c_str(), &path_stat) == 0)
	{
	    if (S_ISDIR(path_stat.st_mode))
		{
			std::cout << client.request.root << std::endl;
			if (currentServer.getListing(client.request.root))
				listingOn(response, fileLocation, fileName);
			else
				listingOff(response, fileLocation, currentServer);
		}
		else
			fileIsNotDirectory(response, fileLocation, contentType, status, currentServer);
	}
	return response;
}

void StartServers::listingOn(std::string &response, std::string &fileLocation, std::string &fileName)
{
	std::string contentType, status;

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
				directoryListing << "<div class=\"listing-buttons-container\"><button class=\"listing-buttons\" onclick='location.href=\"" << fileName << "/" << entryName << "\";'>" << entryName.substr(0, entryName.length() - 5) << "</button><br>";        	    }
		closedir(dir);
	}
	
	directoryListing << "</div></body></html>";


	status = "200 OK";
	contentType = "text/html";
	std::string content = directoryListing.str();
	ClientResponse clientReponse(status, contentType, content);

	response = clientReponse.getReponse();
}

void StartServers::listingOff(std::string &response, std::string &fileLocation, Server currentServer)
{
	std::string contentType, status;

	status = "404 Not found";
	contentType = "text/html";

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

void StartServers::fileIsNotDirectory(std::string &response, std::string &fileLocation, std::string &contentType, std::string &status, Server currentServer)
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