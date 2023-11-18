#include "GenerateMethod.hpp"
#include "cgi.hpp"
#include "utils.hpp"
#include <ostream>

std::string GenerateMethod::GETMethod(Client client, Server server)
{
	std::string response, fileLocation, contentType, status, htmlContent;
	bool is_dir = false;
	fileLocation = server.getFileRoute(client.request.root, status, client.request.method, is_dir);
	std::cout << "File location : " << fileLocation << std::endl;
	std::cout << "Status : " << status << std::endl;

	if (status != "200")
	{
		std::cout << "Error while looking for the file, error : " << status << std::endl;
		contentType = getContentType(".html");
		return getErrorPageResponse(client, server, status);
	}

	if (is_dir)
	{
		std::cout << "Listing Directory." << std::endl;
		return listingDirectory(fileLocation, client.request.root);
	}

	contentType = getContentType(fileLocation);

	std::ifstream file(fileLocation.c_str());
	if (!file.is_open())
	{
		std::cout << "Could not open file." << std::endl;
		contentType = getContentType(".html");
		return getErrorPageResponse(client, server, "500");
	}

	std::map<std::string, std::string> CGIMaps = server.getCgiPages();
	std::cout << "cgi : " << std::string(parseFileExtension(fileLocation)) << std::endl;
	if (CGIMaps.find(parseFileExtension(fileLocation)) != CGIMaps.end())
	{
		CgiHandler CGI(client, server, fileLocation, CGIMaps.find(parseFileExtension(fileLocation))->second);
		htmlContent = CGI.execute();
	}
	else
	{
		htmlContent = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	}
	std::cout << "Content generated for " << fileLocation << "." << std::endl;
	std::cout << "Body : \n" << htmlContent << std::endl;
	ClientResponse clientReponse(status, contentType, htmlContent);
	response = clientReponse.getReponse();

	return response;
}

std::string GenerateMethod::getErrorPageResponse(Client client, Server server, std::string errorCode)
{
    std::string response, fileLocation, contentType, content;
	
	contentType = "text/html";
    
	try
	{
		fileLocation = server.getErrorPageRoute(errorCode);
		std::ifstream file(fileLocation.c_str());
		if (!file.is_open())
		{
			errorCode = "500";
			content = getErrorPageResponse(client, server, errorCode);
		}
		content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	}
	catch (const std::exception&)
	{
		content = generateErrorPage(errorCode);
	}

	ClientResponse clientReponse(errorCode, contentType, content);

	response = clientReponse.getReponse();
	return response;
}

std::string GenerateMethod::listingDirectory(const std::string &fileLocation, std::string &fileName)
{
	std::string contentType, status, content;

	std::stringstream directoryListing;
	directoryListing << "<html><head><title>Webserv Listing</title><link rel=\"stylesheet\" type=\"text/css\" href=\"/styles/styleListing.css\"></head><body><h1 class=\"listing-title\">" << fileLocation << "</h1>";

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
	else
	{
		status = "500";
		content = generateErrorPage(status);
		contentType = "text/html";
		ClientResponse clientReponse(status, contentType, content);
		return clientReponse.getReponse();
	}
	
	directoryListing << "</div></body></html>";

	status = "200";
	contentType = "text/html";
	content = directoryListing.str();
	ClientResponse clientReponse(status, contentType, content);

	return clientReponse.getReponse();
}
