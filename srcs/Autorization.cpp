#include "Autorization.hpp"

std::string Autorization::generateErrorPage(std::string realm)
{
	const std::string spaceDelimiter = " ";
	const std::string newLineDelimiter = "\r\n";
	std::string response;

	response = "HTTP/1.1 401 Unauthorized\n";
	response += "WWW-Authenticate: Basic realm=\"Access to staging site\"";

	return response;	
}

int Autorization::ShallYouPass()
{
	if (isAuthLocationPresent() == false)
		return RUN_YOU_FOOLS;
	if (isRequestWithAutorization() == false)
		return NO_AUTH_HEADER_FOUND;
	if (isAutorizationValid() == true)
		return RUN_YOU_FOOLS;
	else
		return YOU_SALL_NOT_PASS;
}

bool Autorization::isAutorizationValid()
{
	std::ifstream authFile(_location.second.authBasicUserFile.c_str());
	if (!authFile.is_open())
		return false;

	std::string line;
	while (std::getline(authFile, line))
	{
		if (line == _client.request.credential)
			return true;
	}
	return false;
}

bool Autorization::isRequestWithAutorization()
{
	if (_client.request.credential.empty())
		return false;
	return true;
}

bool Autorization::isAuthLocationPresent()
{
	if (!_location.second.authBasic.empty())
		return true;
	return false;
}
