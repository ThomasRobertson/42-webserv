#include "ClientResponse.hpp"

ClientResponse::ClientResponse(std::string status, std::string contentType, std::string contentBody) : _status(status), _contentType(contentType), _contentBody(contentBody), _serverProtocol(SERVER_PROTOCOL), _serverSoftware(SERVER_SOFTWARE)
{
	#ifdef DEBUG

	assert(!_status.empty());
	if (_status == "200")
	{
		assert(!_contentType.empty());
		assert(!_contentBody.empty());
	}

	#endif // DEBUG
	std::stringstream _dateStream;
	std::time_t result = std::time(NULL);
	_dateStream << std::gmtime(&result);
	_date = _dateStream.str();

	_contentLength = sizeToString(_contentBody.length());
}

void ClientResponse::generateResponse()
{
	static const std::string spaceDelimiter = " ";
	static const std::string newLineDelimiter = "\r\n";

	_reponse = _serverProtocol;
	_reponse += spaceDelimiter;

	std::pair<std::string, std::string> statusCode = getStatus(_status);
	_reponse += statusCode.first;
	_reponse += spaceDelimiter;
	_reponse += statusCode.second;
	_reponse += newLineDelimiter;

	// _reponse += "Connection: keep-alive";
	// _reponse += newLineDelimiter;

	_reponse += "Server: ";
	_reponse += _serverSoftware;
	_reponse += newLineDelimiter;

	// _reponse += "Date: ";
	// _reponse += _date;
	// _reponse += newLineDelimiter;

	_reponse += "Content-Type: ";
	_reponse += _contentType;
	_reponse += newLineDelimiter;

	_reponse += "Content-Length: ";
	_reponse += _contentLength;
	_reponse += newLineDelimiter;

	// _reponse += "Accept-Ranges: bytes";
	// _reponse += newLineDelimiter;

	_reponse += newLineDelimiter;
	_reponse += _contentBody;
}

std::string ClientResponse::getReponse()
{
	generateResponse();
	return (_reponse);
}
