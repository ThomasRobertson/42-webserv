#include "getMethod.hpp"

std::string GenerateMethod::GETMethod(Client client, Server server)
{
	std::string response, fileLocation, contentType, status;
	fileLocation = server.getFileRoute(client.request.root, status, client.request.method);

	if (status != "200")
	{
		contentType = getContentType(".html");
		return server.getErrorPageRoute(status);
	}

	contentType = getContentType(fileLocation);

	std::ifstream file(fileLocation.c_str());
	if (!file.is_open())
	{
		contentType = getContentType(".html");
		return server.getErrorPageRoute("500");
	}

	std::string htmlContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	ClientResponse clientReponse(status, contentType, htmlContent);
	response = clientReponse.getReponse();

	return response;
}
