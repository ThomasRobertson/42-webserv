#include "cgi.hpp"
#include "Settings.hpp"

#define SSTR(x) static_cast< std::ostringstream & >(( std::ostringstream() << std::dec << x) ).str()

void CgiHandler::build_args_env()
{
	std::string value;

	value = "SERVER_SOFTWARE=";
	value += SERVER_SOFTWARE;
	_environ.push_back(value);

	value = "SERVER_NAME=";
	value += _server.getHost();
	_environ.push_back(value);

	value = "GATEWAY_INTERFACE=";
	value += GATEWAY_INTERFACE;
	_environ.push_back(value);

	value = "DOCUMENT_ROOT=";
	value += _server.getRoot();
	_environ.push_back(value);

	value = "SERVER_PROTOCOL=";
	value += SERVER_PROTOCOL;
	_environ.push_back(value);

	value = "REQUEST_METHOD=";
	value += _client.request.method;
	_environ.push_back(value);

	value = "PATH_INFO=";
	value += _fileLocation;
	_environ.push_back(value);

	value = "PATH_TRANSLATED=";
	value += _fileLocation;
	_environ.push_back(value);

	value = "SCRIPT_NAME=";
	value += _cgi_path;
	_environ.push_back(value);

	value = "QUERY_STRING=";
	value += "";
	_environ.push_back(value);

	value = "REMOTE_HOST=";
	value += "";
	_environ.push_back(value);

	value = "REMOTE_ADDR=";
	value += "127.0.0.1";
	_environ.push_back(value);

	if (_client.request.method == "POST") {
		value = "CONTENT_TYPE=";
		value += getContentType(_fileLocation);
		_environ.push_back(value);

		value = "CONTENT_LENGTH=";
		value += SSTR(_client.request.contentLength);
		_environ.push_back(value);
	}

	value = "REQUEST_URI=";
	value += _fileLocation;
	_environ.push_back(value);

	value = "HTTP_COOKIE=";
	std::vector<std::string>::iterator it = _client.request.cookies.begin();
	while (it != _client.request.cookies.end())
	{
		value += *it;
		it++;
		if (it != _client.request.cookies.end())
			value += ", ";
	}
	_environ.push_back(value);
}

void CgiHandler::child_is_in_orbit() {
	char **args = new char *[3];
	args[0] = const_cast<char *>(_cgi_path.c_str());
	args[1] = const_cast<char *>(_fileLocation.c_str());
	args[2] = NULL;

	char **environ = new char *[_environ.size() + 1];
	std::vector<std::string>::iterator it = _environ.begin();
	size_t i = 0;
	while (it != _environ.end()) {
		environ[i] = const_cast<char *>(strdup(it->c_str()));
		it++;
		i++;
	}
	environ[_environ.size()] = NULL;

	_child_pid = execve(_cgi_path.c_str(), args, environ);

	delete[] args;
	delete[] environ;
	std::perror("Cannot launch CGI");
	std::exit(EXIT_FAILURE);
}

void CgiHandler::launch_child()
{
	_child_pid = fork();
	if (_child_pid == -1) {
		std::runtime_error("Houston we have a problem, abord launch of child");
	}
	else if (_child_pid == 0)
	{
		dup2(_child_out_pipe, STDOUT_FILENO);
		dup2(_child_in_pipe, STDIN_FILENO);
		child_is_in_orbit();
	}
}

std::string CgiHandler::capture_child_return()
{
	char read_buffer[CGI_BUFFER_SIZE + 1];
	ssize_t size_read;
	std::string return_str;
	int wstatus;

	pid_t status = waitpid(-1, &wstatus, 0);
	if (status == -1)
		std::cerr << "No child launched or error while waiting.\n";

	if (WIFEXITED(wstatus))
	{
		if (WEXITSTATUS(wstatus) == 1)
			throw std::runtime_error("Child exit code is 1.");
	}

	lseek(_child_out_pipe, 0, SEEK_SET);
	do {
		size_read =
			read(_child_out_pipe, read_buffer, CGI_BUFFER_SIZE);
		if (size_read == -1)
			throw std::runtime_error("Could not read from file.");
		read_buffer[size_read] = '\0';
		if (size_read != 0)
			return_str += read_buffer;
	} while (size_read == CGI_BUFFER_SIZE);
	return (return_str);
}

#define WRITE_SIZE 32768

void CgiHandler::sendBody() {
	// std::cout << "SIZE: " << _body.size() << std::endl;
	if (_body.size() != 0)
		write(_child_in_pipe, _body.c_str(), _body.size());
	// std::cout << "SEND" << std::endl;
	lseek(_child_in_pipe, 0, SEEK_SET);
}

std::string CgiHandler::generateReturnResponse(std::string return_str)
{
	std::string status, contentBody, cookieSet, contentType, line, bodyBuffer;
	std::vector<std::string> extraHeaders;
	std::istringstream returnStream(return_str);

	// std::cout << "SIZE : " << return_str.size() << std::endl;

	while (std::getline(returnStream, line))
	{
		if (line == "" || line.find_first_not_of("\r\n") == std::string::npos)
			break;
		else if (line.find(':') == std::string::npos)
		{
			contentBody = line;
			contentBody += "\n";
			break ;
		}

		std::string field_name = line.substr(0, line.find_first_of(':'));
		std::string field_value = line.substr(line.find_first_of(':') + 1);
		
		std::vector<std::string> field_values;
		do
		{
			field_value.erase(0, field_value.find_first_not_of(' '));
			std::string value = field_value.substr(0, field_value.find_first_of(';'));
			field_value.erase(0, field_value.find_first_of(';') + 1);
			if (value.find_first_of(';') != std::string::npos)
				value.erase(value.find_first_of(';'));
			field_values.push_back(value);
		} while (field_value.find_first_of(';') != std::string::npos);

		if (field_name == "Content-Type") {
			for (std::vector<std::string>::iterator it = field_values.begin(); it != field_values.end(); it++)
			{
				if (!contentType.empty() && it == field_values.begin())
					contentType = *it;
				else
				{
					std::string str;
					str = field_name;
					str += ": ";
					str += *it;
					extraHeaders.push_back(str);
				}
			}
		}
		else if (field_name == "Status") {
			if (!status.empty() || field_values.size() != 1)
				throw std::invalid_argument("");
			status = field_values.front();
		}
		else if (field_name == "Location") {
			for (std::vector<std::string>::iterator it = field_values.begin(); it != field_values.end(); it++)
			{
				std::string str;
				str = field_name;
				str += ": ";
				str += *it;
				extraHeaders.push_back(str);
			}
		}
		else if (field_name == "Set-Cookie") {
			for (std::vector<std::string>::iterator it = field_values.begin(); it != field_values.end(); it++)
				cookieSet += *it;
		}
		else {
			for (std::vector<std::string>::iterator it = field_values.begin(); it != field_values.end(); it++)
			{
				std::string str;
				str = field_name;
				str += ": ";
				str += *it;
				extraHeaders.push_back(str);
			}
		}
	}

	while (std::getline(returnStream, bodyBuffer))
	{
		if (contentBody.empty() && (bodyBuffer == "" || bodyBuffer.find_first_not_of("\r\n") == std::string::npos))
			continue;
		contentBody += bodyBuffer;
		contentBody += "\n";
		if (returnStream.eof())
			break;
	}
	if (contentType.empty())
		contentType = "text/plain";
	if (status.empty())
		status = "200 OK";
	ClientResponse clientResponse(false, status, contentType, contentBody, "", cookieSet, extraHeaders);

	return clientResponse.getReponse();
}

std::string CgiHandler::execute() {
	build_args_env();

	sendBody();
	launch_child();
	std::string return_str = capture_child_return();
	return_str = generateReturnResponse(return_str);

	return (return_str);
}
