#include "getMethod.hpp"

std::string GenerateMethod::GETMethod(Client client, Server server)
{
	std::string response, fileLocation, contentType, status;
	fileLocation = server.getFileRoute(client.request.root, status, client.request.method);
	std::cout << "File location : " << fileLocation << std::endl;

	if (status != "200")
	{
		contentType = getContentType(".html");
		return getErrorPageResponse(client, server, status);
	}

	if (*(fileLocation.rbegin()) == '/')
	{
		// do dir listing
		(void)status;
	}

	contentType = getContentType(fileLocation);

	std::ifstream file(fileLocation.c_str());
	if (!file.is_open())
	{
		contentType = getContentType(".html");
		return getErrorPageResponse(client, server, "500");
	}

	std::string htmlContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

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
