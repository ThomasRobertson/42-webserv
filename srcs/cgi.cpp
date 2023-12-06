#include "cgi.hpp"
#include "Settings.hpp"

#define SSTR(x) static_cast< std::ostringstream & >(( std::ostringstream() << std::dec << x) ).str()

void CgiHandler::build_args_env() {
	std::string value;
	// --- SERVER ENVIRON ---

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


	// _environ.push_back("SERVER_NAME" + _server.getHost());
	// _environ.push_back("GATEWAY_INTERFACE" + GATEWAY_INTERFACE);
	// _environ.push_back("DOCUMENT_ROOT" + _server.getRoot());

	// // --- REQUEST ENVIRON ---

	// _environ.push_back("SERVER_PROTOCOL" + SERVER_PROTOCOL); //? Is it HTTP
	// or does it change ?
	// // _environ.push_back("SERVER_PORT" + _server.getPort()); //?From request
	// or server ? _environ.push_back("REQUEST_METHOD" +
	// _client.request.method); _environ.push_back("PATH_INFO" + _fileLocation);
	// _environ.push_back("PATH_TRANSLATED" + _fileLocation);
	// _environ.push_back("SCRIPT_NAME" + _cgi_path);
	// _environ.push_back("QUERY_STRING" + ""); //Todo, parsing string query
	// _environ.push_back("REMOTE_HOST" + ""); //? Can we leave it empty ?
	// _environ.push_back("REMOTE_ADDR" + "127.0.0.1"); //TODO : IP adress of
	// client
	// // _environ.push_back("AUTH_TYPE" + AUTH_TYPE);
	// // _environ.push_back("REMOTE_USER" + "");
	// // _environ.push_back("REMOTE_IDENT" + ""); //? AUTH TYPE, REMOTE USER
	// AND IDENT just if renable by server, implementation needed ? if
	// (_client.request.method == "POST")
	// {
	// 	// _environ.push_back("CONTENT_TYPE" + ""); //TODO: From request
	// 	// _environ.push_back("CONTENT_LENGTH" + ""); //TODO: From request
	// }
	// _environ.push_back("REQUEST_URI" + _fileLocation);

	// // --- CLIENT ENVIRON ---

	// // _environ["HTTP_ACCEPT" + ""); //Todo: what MIME can the client accept,
	// from client. Needed ?
	// // _environ["HTTP_ACCEPT_LANGUAGE" + ""); //Todo: Language accepted, from
	// client. Needed ?
	// // _environ["HTTP_USER_AGENT" + "" //Todo: User agent, from client.
	// Needed ? _environ["HTTP_COOKIE" + ""); //?Do I have to define it?
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

	// std::cout << "About to launch : " << _cgi_path << std::endl;
	_child_pid = execve(_cgi_path.c_str(), args, environ);

	delete[] args;
	delete[] environ;
	std::perror("Cannot launch CGI");
	std::exit(EXIT_FAILURE);
}

void CgiHandler::launch_child() {
	// check_args_env(); //TODO: to implement

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

std::string CgiHandler::capture_child_return() {
	char read_buffer[CGI_BUFFER_SIZE + 1];
	ssize_t size_read;
	std::string return_str;
	int wstatus;

	pid_t status = waitpid(-1, &wstatus, 0);
	if (status == -1)
		std::cerr << "No child launched or error while waiting.\n";

	if (WIFEXITED(wstatus))
		std::cout << "Child return code : " << WEXITSTATUS(wstatus)
				  << std::endl;	 //! For debug only, to be removed.

	lseek(_child_out_pipe, 0, SEEK_SET);
	do {
		size_read =
			read(_child_out_pipe, read_buffer, CGI_BUFFER_SIZE);
		if (size_read == -1)
			throw std::runtime_error("Could not read from file.");  // TODO: add errno output
		read_buffer[size_read] = '\0';
		if (size_read != 0)
			return_str += read_buffer;
	} while (size_read == CGI_BUFFER_SIZE);
	return (return_str);
}

#define WRITE_SIZE 32768

void CgiHandler::sendBody() {
	std::cout << "SIZE: " << _body.size() << std::endl;
	if (_body.size() != 0)
	{
		write(_child_in_pipe, _body.c_str(), _body.size());
		// lseek(_child_in_pipe, 0, SEEK_SET);
	}
	std::cout << "SEND" << std::endl;
	// char buff[1025];
	// buff[1024] = '\0';
	// ssize_t readLenght;
	// do {
	// 	memset(buff, '\0', 1024);
	// 	readLenght = read(_child_in_pipe, buff, 1024);
	// 	std::cout << buff;
	// }while (readLenght != 0);
	lseek(_child_in_pipe, 0, SEEK_SET);
}

std::string CgiHandler::generateReturnResponse(std::string return_str) {
	std::string status;
	std::string contentBody;
	std::string cookieSet;
	std::vector<std::string> extraHeaders;
	std::string contentType;

	std::istringstream returnStream(return_str);
	std::cout << "SIZE : " << return_str.size() << std::endl;
	std::string line, bodyBuffer;

	while (std::getline(returnStream, line)) {
		if (line == "" || line == "\r\n" || line == "\n")
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
	
	while (std::getline(returnStream, bodyBuffer)) {
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
	// if (pipe(_child_out_pipe))
	// 	throw std::runtime_error("Could not pipe process.");

	// _child_in_pipe = open("temp_cgi", O_RDWR | O_TRUNC | O_CREAT);
	// if (_child_in_pipe == -1)
	// {
	// 	close(_child_out_pipe[PIPE_READ]);
	// 	close(_child_out_pipe[PIPE_WRITE]);
	// 	throw std::runtime_error("Could not open temp file for CGI.");
	// }
	build_args_env();

	sendBody();
	launch_child();
	std::string return_str = capture_child_return();
	std::cout << RED << "The return str is : " << return_str << DEFAULT << std::endl;
	return_str = generateReturnResponse(return_str);
	std::cout << GREEN << "reponse is:" << return_str << DEFAULT << std::endl;

	return (return_str);
}
